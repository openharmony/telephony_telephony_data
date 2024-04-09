/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "global_params_ability.h"

#include "ability_context.h"
#include "ability_loader.h"
#include "abs_rdb_predicates.h"
#include "abs_shared_result_set.h"
#include "data_storage_errors.h"
#include "data_storage_log_wrapper.h"
#include "datashare_ext_ability.h"
#include "datashare_predicates.h"
#include "new"
#include "global_params_data.h"
#include "permission_util.h"
#include "rdb_errno.h"
#include "rdb_utils.h"
#include "telephony_datashare_stub_impl.h"
#include "uri.h"
#include "utility"

namespace OHOS {
using AppExecFwk::AbilityLoader;
using AppExecFwk::Ability;
namespace Telephony {
static const std::map<std::string, GlobalParamsUriType> g_globalParamsTypeMap = {
    {"/globalparams/num_matchs", GlobalParamsUriType::NUMBER_MATCHS},
    {"/globalparams/ecc_data", GlobalParamsUriType::ECC_LIST},
};

GlobalParamsAbility::GlobalParamsAbility() : DataShareExtAbility() {}

GlobalParamsAbility::~GlobalParamsAbility() {}

GlobalParamsAbility* GlobalParamsAbility::Create()
{
    DATA_STORAGE_LOGD("GlobalParamsAbility::Create begin.");
    auto self = new GlobalParamsAbility();
    self->DoInit();
    return self;
}

void GlobalParamsAbility::DoInit()
{
    if (initDatabaseDir_ && initRdbStore_) {
        DATA_STORAGE_LOGI("DoInit has done.");
        return;
    }
    auto abilityContext = AbilityRuntime::Context::GetApplicationContext();
    if (abilityContext == nullptr) {
        DATA_STORAGE_LOGE("abilityContext is nullptr.");
        return;
    }
    // switch database dir to el1 for init before unlock
    abilityContext->SwitchArea(0);
    std::string path = abilityContext->GetDatabaseDir();
    if (!path.empty()) {
        initDatabaseDir_ = true;
        path.append("/");
        helper_.UpdateDbPath(path);
        if (helper_.Init() == NativeRdb::E_OK) {
            initRdbStore_ = true;
        } else {
            DATA_STORAGE_LOGE("DoInit rdb init fail!");
            initRdbStore_ = false;
        }
    } else {
        DATA_STORAGE_LOGE("path is empty");
        initDatabaseDir_ = false;
    }
}

sptr<IRemoteObject> GlobalParamsAbility::OnConnect(const AAFwk::Want &want)
{
    DATA_STORAGE_LOGI("GlobalParamsAbility %{public}s begin.", __func__);
    Extension::OnConnect(want);
    sptr<DataShare::TelephonyDataShareStubImpl> remoteObject =
        new (std::nothrow) DataShare::TelephonyDataShareStubImpl();
    if (remoteObject == nullptr) {
        DATA_STORAGE_LOGE("%{public}s No memory allocated for DataShareStubImpl", __func__);
        return nullptr;
    }
    remoteObject->SetGlobalParamsAbility(std::static_pointer_cast<GlobalParamsAbility>(shared_from_this()));
    DATA_STORAGE_LOGI("GlobalParamsAbility %{public}s end.", __func__);
    return remoteObject->AsObject();
}

void GlobalParamsAbility::OnStart(const AppExecFwk::Want &want)
{
    DATA_STORAGE_LOGI("GlobalParamsAbility::OnStart");
    Extension::OnStart(want);
    DoInit();
}

int32_t GlobalParamsAbility::Insert(const Uri &uri, const DataShare::DataShareValuesBucket &value)
{
    if (!PermissionUtil::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        DATA_STORAGE_LOGE("Permission denied!");
        return DATA_STORAGE_ERR_PERMISSION_ERR;
    }
    if (!IsInitOk()) {
        return DATA_STORAGE_ERROR;
    }
    DATA_STORAGE_LOGE("GlobalParamsAbility::The insert capability is not supported.");
    return DATA_STORAGE_ERROR;
}

std::shared_ptr<DataShare::DataShareResultSet> GlobalParamsAbility::Query(const Uri &uri,
    const DataShare::DataSharePredicates &predicates, std::vector<std::string> &columns,
    DataShare::DatashareBusinessError &businessError)
{
    if (!PermissionUtil::CheckPermission(Permission::GET_TELEPHONY_STATE)) {
        DATA_STORAGE_LOGE("Permission denied!");
        return nullptr;
    }
    std::shared_ptr<DataShare::DataShareResultSet> sharedPtrResult = nullptr;
    if (!IsInitOk()) {
        return nullptr;
    }
    Uri tempUri = uri;
    GlobalParamsUriType globalParamsUriType = ParseUriType(tempUri);
    NativeRdb::AbsRdbPredicates *absRdbPredicates = nullptr;
    switch (globalParamsUriType) {
        case GlobalParamsUriType::NUMBER_MATCHS: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_NUMBER_MATCH);
            break;
        }
        case GlobalParamsUriType::ECC_LIST: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_ECC_DATA);
            break;
        }
        default:
            DATA_STORAGE_LOGE("GlobalParamsAbility::Query failed##uri = %{public}s", uri.ToString().c_str());
            break;
    }
    if (absRdbPredicates != nullptr) {
        NativeRdb::RdbPredicates rdbPredicates = ConvertPredicates(absRdbPredicates->GetTableName(), predicates);
        auto result = helper_.Query(rdbPredicates, columns);
        if (result == nullptr) {
            DATA_STORAGE_LOGE("GlobalParamsAbility::Query  NativeRdb::ResultSet is null!");
            delete absRdbPredicates;
            absRdbPredicates = nullptr;
            return nullptr;
        }
        auto queryResultSet = RdbDataShareAdapter::RdbUtils::ToResultSetBridge(result);
        sharedPtrResult = std::make_shared<DataShare::DataShareResultSet>(queryResultSet);
        delete absRdbPredicates;
        absRdbPredicates = nullptr;
    } else {
        DATA_STORAGE_LOGE("GlobalParamsAbility::Query##uri = %{public}s", uri.ToString().c_str());
    }
    return sharedPtrResult;
}

int GlobalParamsAbility::Update(
    const Uri &uri, const DataShare::DataSharePredicates &predicates,
    const DataShare::DataShareValuesBucket &value)
{
    if (!PermissionUtil::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        DATA_STORAGE_LOGE("Permission denied!");
        return DATA_STORAGE_ERR_PERMISSION_ERR;
    }
    int result = DATA_STORAGE_ERROR;
    if (!IsInitOk()) {
        return result;
    }
    DATA_STORAGE_LOGE("GlobalParamsAbility::The update capability is not supported.");
    return result;
}

int GlobalParamsAbility::Delete(const Uri &uri, const DataShare::DataSharePredicates &predicates)
{
    if (!PermissionUtil::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        DATA_STORAGE_LOGE("Permission denied!");
        return DATA_STORAGE_ERR_PERMISSION_ERR;
    }
    int result = DATA_STORAGE_ERROR;
    if (!IsInitOk()) {
        return result;
    }
    DATA_STORAGE_LOGE("GlobalParamsAbility::The delete capability is not supported.");
    return result;
}

bool GlobalParamsAbility::IsInitOk()
{
    if (!initDatabaseDir_) {
        DATA_STORAGE_LOGE("GlobalParamsAbility::IsInitOk initDatabaseDir_ failed!");
        return false;
    }
    if (!initRdbStore_) {
        DATA_STORAGE_LOGE("GlobalParamsAbility::IsInitOk initRdbStore_ failed!");
        return false;
    }
    return true;
}

std::string GlobalParamsAbility::GetType(const Uri &uri)
{
    DATA_STORAGE_LOGI("GlobalParamsAbility::GetType##uri = %{public}s", uri.ToString().c_str());
    std::string retval(uri.ToString());
    return retval;
}

int GlobalParamsAbility::OpenFile(const Uri &uri, const std::string &mode)
{
    DATA_STORAGE_LOGI("GlobalParamsAbility::OpenFile##uri = %{public}s", uri.ToString().c_str());
    Uri tempUri = uri;
    GlobalParamsUriType globalParamsUriType = ParseUriType(tempUri);
    return static_cast<int>(globalParamsUriType);
}

GlobalParamsUriType GlobalParamsAbility::ParseUriType(Uri &uri)
{
    DATA_STORAGE_LOGD("GlobalParamsAbility::ParseUriType start");
    GlobalParamsUriType globalParamsUriType = GlobalParamsUriType::UNKNOW;
    std::string uriPath = uri.ToString();
    if (!uriPath.empty()) {
        helper_.ReplaceAllStr(uriPath, ":///", "://");
        Uri tempUri(uriPath);
        std::string path = tempUri.GetPath();
        if (!path.empty() && !g_globalParamsTypeMap.empty()) {
            auto it = g_globalParamsTypeMap.find(path);
            if (it != g_globalParamsTypeMap.end()) {
                globalParamsUriType = it->second;
                DATA_STORAGE_LOGD(
                    "GlobalParamsAbility::ParseUriType##globalParamsUriType = %{public}d", globalParamsUriType);
            }
        }
    }
    return globalParamsUriType;
}

OHOS::NativeRdb::RdbPredicates GlobalParamsAbility::ConvertPredicates(
    const std::string &tableName, const DataShare::DataSharePredicates &predicates)
{
    OHOS::NativeRdb::RdbPredicates res = RdbDataShareAdapter::RdbUtils::ToPredicates(predicates, tableName);
    return res;
}
} // namespace Telephony
} // namespace OHOS
