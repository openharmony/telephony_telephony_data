/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "pdp_profile_ability.h"

#include <cstdint>

#include "ability_context.h"
#include "ability_loader.h"
#include "abs_rdb_predicates.h"
#include "abs_shared_result_set.h"
#include "datashare_ext_ability.h"
#include "datashare_predicates.h"
#include "data_storage_errors.h"
#include "data_storage_log_wrapper.h"
#include "new"
#include "pdp_profile_data.h"
#include "rdb_errno.h"
#include "rdb_utils.h"
#include "telephony_datashare_stub_impl.h"
#include "uri.h"
#include "utility"

namespace OHOS {
using AppExecFwk::Ability;
using AppExecFwk::AbilityLoader;
namespace Telephony {
const int32_t CHANGED_ROWS = 0;
static const std::map<std::string, PdpProfileUriType> pdpProfileUriMap_ = {
    { "/net/pdp_profile", PdpProfileUriType::PDP_PROFILE },
    { "/net/pdp_profile/reset", PdpProfileUriType::RESET },
};

PdpProfileAbility::PdpProfileAbility() : DataShareExtAbility() {}

PdpProfileAbility::~PdpProfileAbility() {}

PdpProfileAbility* PdpProfileAbility::Create()
{
    DATA_STORAGE_LOGI("PdpProfileAbility::Create begin.");
    auto self =  new PdpProfileAbility();
    self->DoInit();
    return self;
}

void PdpProfileAbility::DoInit()
{
    if (initDatabaseDir && initRdbStore) {
        DATA_STORAGE_LOGE("DoInit has done");
        return;
    }
    auto abilityContext = AbilityRuntime::Context::GetApplicationContext();
    if (abilityContext == nullptr) {
        DATA_STORAGE_LOGE("DoInit GetAbilityContext is null");
        return;
    }
    // switch database dir to el1 for init before unlock
    abilityContext->SwitchArea(0);
    std::string path = abilityContext->GetDatabaseDir();
    DATA_STORAGE_LOGI("GetDatabaseDir: %{public}s", path.c_str());
    if (!path.empty()) {
        initDatabaseDir = true;
        path.append("/");
        helper_.UpdateDbPath(path);
        int rdbInitCode = helper_.Init();
        if (rdbInitCode == NativeRdb::E_OK) {
            initRdbStore = true;
        } else {
            DATA_STORAGE_LOGE("DoInit rdb init fail!");
            initRdbStore = false;
        }
    } else {
        DATA_STORAGE_LOGE("DoInit##databaseDir is empty!");
        initDatabaseDir = false;
    }
}

sptr<IRemoteObject> PdpProfileAbility::OnConnect(const AAFwk::Want &want)
{
    DATA_STORAGE_LOGI("PdpProfileAbility %{public}s begin.", __func__);
    Extension::OnConnect(want);
    sptr<DataShare::TelephonyDataShareStubImpl> remoteObject =
        new (std::nothrow) DataShare::TelephonyDataShareStubImpl();
    if (remoteObject == nullptr) {
        DATA_STORAGE_LOGE("%{public}s No memory allocated for DataShareStubImpl", __func__);
        return nullptr;
    }
    remoteObject->SetPdpProfileAbility(std::static_pointer_cast<PdpProfileAbility>(shared_from_this()));
    DATA_STORAGE_LOGI("PdpProfileAbility %{public}s end.", __func__);
    return remoteObject->AsObject();
}

void PdpProfileAbility::OnStart(const AppExecFwk::Want &want)
{
    DATA_STORAGE_LOGI("PdpProfileAbility::OnStart");
    Extension::OnStart(want);
    DoInit();
}

int PdpProfileAbility::Insert(const Uri &uri, const DataShare::DataShareValuesBucket &value)
{
    if (!TelephonyPermission::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        DATA_STORAGE_LOGE("Permission denied!");
        return DATA_STORAGE_ERR_PERMISSION_ERR;
    }
    if (!IsInitOk()) {
        return DATA_STORAGE_ERROR;
    }
    std::lock_guard<std::mutex> guard(lock_);
    Uri tempUri = uri;
    PdpProfileUriType pdpProfileUriType = ParseUriType(tempUri);
    int64_t id = DATA_STORAGE_ERROR;
    if (pdpProfileUriType == PdpProfileUriType::PDP_PROFILE) {
        OHOS::NativeRdb::ValuesBucket values = RdbDataShareAdapter::RdbUtils::ToValuesBucket(value);
        helper_.Insert(id, values, TABLE_PDP_PROFILE);
    } else {
        DATA_STORAGE_LOGE("PdpProfileAbility::Insert##uri = %{public}s", uri.ToString().c_str());
    }
    return id;
}

std::shared_ptr<DataShare::DataShareResultSet> PdpProfileAbility::Query(const Uri &uri,
    const DataShare::DataSharePredicates &predicates, std::vector<std::string> &columns,
    DataShare::DatashareBusinessError &businessError)
{
    if (!TelephonyPermission::CheckPermission(Permission::GET_TELEPHONY_STATE)) {
        DATA_STORAGE_LOGE("Permission denied!");
        return nullptr;
    }
    std::shared_ptr<DataShare::DataShareResultSet> sharedPtrResult = nullptr;
    if (!IsInitOk()) {
        return nullptr;
    }
    Uri tempUri = uri;
    PdpProfileUriType pdpProfileUriType = ParseUriType(tempUri);
    if (pdpProfileUriType == PdpProfileUriType::PDP_PROFILE) {
        NativeRdb::AbsRdbPredicates *absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_PDP_PROFILE);
        if (absRdbPredicates != nullptr) {
            NativeRdb::RdbPredicates rdbPredicates = ConvertPredicates(absRdbPredicates->GetTableName(), predicates);
            auto result = helper_.Query(rdbPredicates, columns);
            if (result == nullptr) {
                DATA_STORAGE_LOGE("PdpProfileAbility::Query  NativeRdb::ResultSet is null!");
                delete absRdbPredicates;
                absRdbPredicates = nullptr;
                return nullptr;
            }
            auto queryResultSet = RdbDataShareAdapter::RdbUtils::ToResultSetBridge(result);
            sharedPtrResult = std::make_shared<DataShare::DataShareResultSet>(queryResultSet);
            delete absRdbPredicates;
            absRdbPredicates = nullptr;
        } else {
            DATA_STORAGE_LOGE("PdpProfileAbility::Query  NativeRdb::AbsRdbPredicates is null!");
        }
    } else {
        DATA_STORAGE_LOGE("PdpProfileAbility::Query##uri = %{public}s", uri.ToString().c_str());
    }
    return sharedPtrResult;
}

int PdpProfileAbility::Update(
    const Uri &uri, const DataShare::DataSharePredicates &predicates,
    const DataShare::DataShareValuesBucket &value)
{
    if (!TelephonyPermission::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        DATA_STORAGE_LOGE("Permission denied!");
        return DATA_STORAGE_ERR_PERMISSION_ERR;
    }
    int result = DATA_STORAGE_ERROR;
    if (!IsInitOk()) {
        return result;
    }
    std::lock_guard<std::mutex> guard(lock_);
    Uri tempUri = uri;
    PdpProfileUriType pdpProfileUriType = ParseUriType(tempUri);
    NativeRdb::AbsRdbPredicates *absRdbPredicates = nullptr;
    switch (pdpProfileUriType) {
        case PdpProfileUriType::PDP_PROFILE: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_PDP_PROFILE);
            break;
        }
        case PdpProfileUriType::RESET: {
            result = helper_.ResetApn();
            if (result != NativeRdb::E_OK) {
                DATA_STORAGE_LOGE("PdpProfileAbility::Update  ResetApn fail!");
                result = static_cast<int>(LoadProFileErrorType::RESET_APN_FAIL);
            }
            break;
        }
        default:
            DATA_STORAGE_LOGE("PdpProfileAbility::Update##uri = %{public}s", uri.ToString().c_str());
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
        DATA_STORAGE_LOGE("PdpProfileAbility::Update  NativeRdb::AbsRdbPredicates is null!");
    }
    return result;
}

int PdpProfileAbility::Delete(const Uri &uri, const DataShare::DataSharePredicates &predicates)
{
    if (!TelephonyPermission::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        DATA_STORAGE_LOGE("Permission denied!");
        return DATA_STORAGE_ERR_PERMISSION_ERR;
    }
    int result = DATA_STORAGE_ERROR;
    if (!IsInitOk()) {
        return result;
    }
    std::lock_guard<std::mutex> guard(lock_);
    Uri tempUri = uri;
    PdpProfileUriType pdpProfileUriType = ParseUriType(tempUri);
    if (pdpProfileUriType == PdpProfileUriType::PDP_PROFILE) {
        NativeRdb::AbsRdbPredicates *absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_PDP_PROFILE);
        if (absRdbPredicates != nullptr) {
            NativeRdb::RdbPredicates rdbPredicates = ConvertPredicates(absRdbPredicates->GetTableName(), predicates);
            int deletedRows = CHANGED_ROWS;
            result = helper_.Delete(deletedRows, rdbPredicates);
            delete absRdbPredicates;
            absRdbPredicates = nullptr;
        } else {
            DATA_STORAGE_LOGE("PdpProfileAbility::Delete  NativeRdb::AbsRdbPredicates is null!");
        }
    } else {
        DATA_STORAGE_LOGI("PdpProfileAbility::Delete##uri = %{public}s", uri.ToString().c_str());
    }
    return result;
}

bool PdpProfileAbility::IsInitOk()
{
    if (!initDatabaseDir) {
        DATA_STORAGE_LOGE("PdpProfileAbility::IsInitOk initDatabaseDir failed!");
        return false;
    }
    if (!initRdbStore) {
        DATA_STORAGE_LOGE("PdpProfileAbility::IsInitOk initRdbStore failed!");
        return false;
    }
    return true;
}

std::string PdpProfileAbility::GetType(const Uri &uri)
{
    DATA_STORAGE_LOGI("PdpProfileAbility::GetType##uri = %{public}s", uri.ToString().c_str());
    std::string retval(uri.ToString());
    return retval;
}

int PdpProfileAbility::OpenFile(const Uri &uri, const std::string &mode)
{
    DATA_STORAGE_LOGI("PdpProfileAbility::OpenFile##uri = %{public}s", uri.ToString().c_str());
    Uri tempUri = uri;
    PdpProfileUriType pdpProfileUriType = ParseUriType(tempUri);
    return static_cast<int>(pdpProfileUriType);
}

PdpProfileUriType PdpProfileAbility::ParseUriType(Uri &uri)
{
    DATA_STORAGE_LOGI("PdpProfileAbility::ParseUriType start");
    PdpProfileUriType pdpProfileUriType = PdpProfileUriType::UNKNOW;
    std::string uriPath = uri.ToString();
    if (!uriPath.empty()) {
        helper_.ReplaceAllStr(uriPath, ":///", "://");
        Uri tempUri(uriPath);
        std::string path = tempUri.GetPath();
        if (!path.empty() && !pdpProfileUriMap_.empty()) {
            DATA_STORAGE_LOGI("PdpProfileAbility::ParseUriType##path = %{public}s", path.c_str());
            auto it = pdpProfileUriMap_.find(path);
            if (it != pdpProfileUriMap_.end()) {
                pdpProfileUriType = it->second;
                DATA_STORAGE_LOGI("PdpProfileAbility::ParseUriType##pdpProfileUriType = %{public}d",
                    pdpProfileUriType);
            }
        }
    }
    return pdpProfileUriType;
}

OHOS::NativeRdb::RdbPredicates PdpProfileAbility::ConvertPredicates(
    const std::string &tableName, const DataShare::DataSharePredicates &predicates)
{
    OHOS::NativeRdb::RdbPredicates res = RdbDataShareAdapter::RdbUtils::ToPredicates(predicates, tableName);
    return res;
}
} // namespace Telephony
} // namespace OHOS
