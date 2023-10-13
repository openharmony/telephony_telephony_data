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

#include "global_ecc_ability.h"

#include "ability_context.h"
#include "ability_loader.h"
#include "abs_rdb_predicates.h"
#include "abs_shared_result_set.h"
#include "data_storage_errors.h"
#include "data_storage_log_wrapper.h"
#include "datashare_ext_ability.h"
#include "datashare_predicates.h"
#include "new"
#include "global_ecc_data.h"
#include "permission_util.h"
#include "rdb_errno.h"
#include "rdb_utils.h"
#include "telephony_datashare_stub_impl.h"
#include "uri.h"
#include "utility"

namespace OHOS {
namespace Telephony {
const int32_t CHANGED_ROWS = 0;
static const std::map<std::string, GlobalEccType> g_globalEccTypeMap = {
    {"/globalparams/ecc_list", GlobalEccType::ECC_LIST}
};

GlobalEccAbility::GlobalEccAbility() : DataShareExtAbility() {}

GlobalEccAbility::~GlobalEccAbility() {}

GlobalEccAbility* GlobalEccAbility::Create()
{
    DATA_STORAGE_LOGI("GlobalEccAbility::Create begin.");
    auto self = new GlobalEccAbility();
    self->DoInit();
    return self;
}

void GlobalEccAbility::DoInit()
{
    if (initDatabaseDir && initRdbStore) {
        DATA_STORAGE_LOGE("DoInit has done.");
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
        initDatabaseDir = true;
        path.append("/");
        helper_.UpdateDbPath(path);
        if (helper_.Init() == NativeRdb::E_OK) {
            initRdbStore = true;
        } else {
            DATA_STORAGE_LOGE("DoInit rdb init fail!");
            initRdbStore = false;
        }
    } else {
        DATA_STORAGE_LOGE("path is empty");
        initDatabaseDir = false;
    }
}

sptr<IRemoteObject> GlobalEccAbility::OnConnect(const AAFwk::Want &want)
{
    DATA_STORAGE_LOGI("GlobalEccAbility %{public}s begin.", __func__);
    Extension::OnConnect(want);
    sptr<DataShare::TelephonyDataShareStubImpl> remoteObject =
        new (std::nothrow) DataShare::TelephonyDataShareStubImpl();
    if (remoteObject == nullptr) {
        DATA_STORAGE_LOGE("%{public}s No memory allocated for DataShareStubImpl", __func__);
        return nullptr;
    }
    remoteObject->SetGlobalEccAbility(std::static_pointer_cast<GlobalEccAbility>(shared_from_this()));
    DATA_STORAGE_LOGI("GlobalEccAbility %{public}s end.", __func__);
    return remoteObject->AsObject();
}

void GlobalEccAbility::OnStart(const AppExecFwk::Want &want)
{
    DATA_STORAGE_LOGI("GlobalEccAbility::OnStart");
    Extension::OnStart(want);
    DoInit();
}

int32_t GlobalEccAbility::Insert(const Uri &uri, const DataShare::DataShareValuesBucket &value)
{
    if (!PermissionUtil::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        DATA_STORAGE_LOGE("Permission denied!");
        return DATA_STORAGE_ERR_PERMISSION_ERR;
    }
    if (!IsInitOk()) {
        return DATA_STORAGE_ERROR;
    }
    std::lock_guard<std::mutex> lock(lock_);
    Uri uriTemp = uri;
    GlobalEccType globalEccUriType = ParseUriType(uriTemp);
    int64_t id = DATA_STORAGE_ERROR;
    if (globalEccUriType == GlobalEccType::ECC_LIST) {
        OHOS::NativeRdb::ValuesBucket values = RdbDataShareAdapter::RdbUtils::ToValuesBucket(value);
        helper_.Insert(id, values, TABLE_GLOBAL_ECC);
    } else {
        DATA_STORAGE_LOGE("GlobalEccAbility::Insert##uri = %{public}s", uri.ToString().c_str());
    }
    return id;
}

std::shared_ptr<DataShare::DataShareResultSet> GlobalEccAbility::Query(const Uri &uri,
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
    Uri uriTemp = uri;
    GlobalEccType globalEccUriType = ParseUriType(uriTemp);
    if (globalEccUriType == GlobalEccType::ECC_LIST) {
        NativeRdb::AbsRdbPredicates *absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_GLOBAL_ECC);
        if (absRdbPredicates != nullptr) {
            NativeRdb::RdbPredicates rdbPredicates = ConvertPredicates(absRdbPredicates->GetTableName(), predicates);
            auto result = helper_.Query(rdbPredicates, columns);
            if (result == nullptr) {
                DATA_STORAGE_LOGE("GlobalEccAbility::Query  NativeRdb::ResultSet is null!");
                delete absRdbPredicates;
                absRdbPredicates = nullptr;
                return nullptr;
            }
            auto queryResultSet = RdbDataShareAdapter::RdbUtils::ToResultSetBridge(result);
            sharedPtrResult = std::make_shared<DataShare::DataShareResultSet>(queryResultSet);
            delete absRdbPredicates;
            absRdbPredicates = nullptr;
        } else {
            DATA_STORAGE_LOGE("GlobalEccAbility::Query  NativeRdb::AbsRdbPredicates is null!");
        }
    } else {
        DATA_STORAGE_LOGE("GlobalEccAbility::Query##uri = %{public}s", uri.ToString().c_str());
    }
    return sharedPtrResult;
}

int GlobalEccAbility::Update(
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
    std::lock_guard<std::mutex> guard(lock_);
    Uri uriTemp = uri;
    GlobalEccType globalEccUriType = ParseUriType(uriTemp);
    NativeRdb::AbsRdbPredicates *absRdbPredicates = nullptr;
    switch (globalEccUriType) {
        case GlobalEccType::ECC_LIST: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_GLOBAL_ECC);
            break;
        }
        default:
            DATA_STORAGE_LOGE("GlobalEccAbility::Update##uri = %{public}s", uri.ToString().c_str());
            break;
    }
    if (absRdbPredicates != nullptr) {
        int changedRows = CHANGED_ROWS;
        NativeRdb::RdbPredicates rdbPredicates = ConvertPredicates(absRdbPredicates->GetTableName(), predicates);
        OHOS::NativeRdb::ValuesBucket values = RdbDataShareAdapter::RdbUtils::ToValuesBucket(value);
        result = helper_.Update(changedRows, values, rdbPredicates);
        delete absRdbPredicates;
        absRdbPredicates = nullptr;
    } else if (result == DATA_STORAGE_ERROR) {
        DATA_STORAGE_LOGE("GlobalEccAbility::Update  NativeRdb::AbsRdbPredicates is null!");
    }
    return result;
}

int GlobalEccAbility::Delete(const Uri &uri, const DataShare::DataSharePredicates &predicates)
{
    if (!PermissionUtil::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        DATA_STORAGE_LOGE("Permission denied!");
        return DATA_STORAGE_ERR_PERMISSION_ERR;
    }
    int result = DATA_STORAGE_ERROR;
    if (!IsInitOk()) {
        return result;
    }
    std::lock_guard<std::mutex> guard(lock_);
    Uri uriTemp = uri;
    GlobalEccType globalEccUriType = ParseUriType(uriTemp);
    if (globalEccUriType == GlobalEccType::ECC_LIST) {
        NativeRdb::AbsRdbPredicates *absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_GLOBAL_ECC);
        if (absRdbPredicates != nullptr) {
            NativeRdb::RdbPredicates rdbPredicates = ConvertPredicates(absRdbPredicates->GetTableName(), predicates);
            int deletedRows = CHANGED_ROWS;
            result = helper_.Delete(deletedRows, rdbPredicates);
            delete absRdbPredicates;
            absRdbPredicates = nullptr;
        } else {
            DATA_STORAGE_LOGE("GlobalEccAbility::Delete  NativeRdb::AbsRdbPredicates is null!");
        }
    } else {
        DATA_STORAGE_LOGI("GlobalEccAbility::Delete##uri = %{public}s", uri.ToString().c_str());
    }
    return result;
}

bool GlobalEccAbility::IsInitOk()
{
    if (!initDatabaseDir) {
        DATA_STORAGE_LOGE("GlobalEccAbility::IsInitOk initDatabaseDir failed!");
        return false;
    }
    if (!initRdbStore) {
        DATA_STORAGE_LOGE("GlobalEccAbility::IsInitOk initRdbStore failed!");
        return false;
    }
    return true;
}

std::string GlobalEccAbility::GetType(const Uri &uri)
{
    DATA_STORAGE_LOGI("GlobalEccAbility::GetType##uri = %{public}s", uri.ToString().c_str());
    std::string retval(uri.ToString());
    return retval;
}

int GlobalEccAbility::OpenFile(const Uri &uri, const std::string &mode)
{
    DATA_STORAGE_LOGI("GlobalEccAbility::OpenFile##uri = %{public}s", uri.ToString().c_str());
    Uri uriTemp = uri;
    GlobalEccType globalEccUriType = ParseUriType(uriTemp);
    return static_cast<int>(globalEccUriType);
}

GlobalEccType GlobalEccAbility::ParseUriType(Uri &uri)
{
    DATA_STORAGE_LOGI("GlobalEccAbility::ParseUriType start");
    GlobalEccType globalEccUriType = GlobalEccType::UNKNOW;
    std::string uriPath = uri.ToString();
    if (!uriPath.empty()) {
        helper_.ReplaceAllStr(uriPath, ":///", "://");
        Uri uriTemp(uriPath);
        std::string path = uriTemp.GetPath();
        if (!path.empty() && !g_globalEccTypeMap.empty()) {
            auto it = g_globalEccTypeMap.find(path);
            if (it != g_globalEccTypeMap.end()) {
                globalEccUriType = it->second;
                DATA_STORAGE_LOGI("GlobalEccAbility::ParseUriType##globalEccUriType = %{public}d",
                    globalEccUriType);
            }
        }
    }
    return globalEccUriType;
}

OHOS::NativeRdb::RdbPredicates GlobalEccAbility::ConvertPredicates(
    const std::string &tableName, const DataShare::DataSharePredicates &predicates)
{
    OHOS::NativeRdb::RdbPredicates res = RdbDataShareAdapter::RdbUtils::ToPredicates(predicates, tableName);
    return res;
}
} // namespace Telephony
} // namespace OHOS