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

#include "sim_ability.h"

#include "ability_context.h"
#include "ability_loader.h"
#include "abs_rdb_predicates.h"
#include "abs_shared_result_set.h"
#include "data_storage_errors.h"
#include "data_storage_log_wrapper.h"
#include "datashare_ext_ability.h"
#include "datashare_predicates.h"
#include "new"
#include "permission_util.h"
#include "rdb_errno.h"
#include "rdb_utils.h"
#include "sim_data.h"
#include "telephony_datashare_stub_impl.h"
#include "uri.h"
#include "utility"
#include "values_bucket.h"

namespace OHOS {
using AbilityRuntime::Extension;
using AbilityRuntime::Runtime;
using AppExecFwk::Ability;
using AppExecFwk::AbilityLoader;
namespace Telephony {
const int32_t CHANGED_ROWS = 0;
static const std::map<std::string, SimUriType> simUriMap_ = {
    { "/sim/sim_info", SimUriType::SIM_INFO },
    { "/sim/sim_info/set_card", SimUriType::SET_CARD },
};

SimAbility::SimAbility() : DataShareExtAbility() {}

SimAbility::~SimAbility() {}

SimAbility* SimAbility::Create()
{
    DATA_STORAGE_LOGD("SimAbility::Create begin.");
    auto self =  new SimAbility();
    self->DoInit();
    return self;
}

static DataShare::DataShareExtAbility *TelephonyDataShareCreator(const std::unique_ptr<Runtime> &runtime)
{
    DATA_STORAGE_LOGD("sim TelephonyDataCreator::%{public}s begin.", __func__);
    return SimAbility::Create();
}

__attribute__((constructor)) void RegisterDataShareCreator()
{
    DATA_STORAGE_LOGD("TelephonyDataCreator::%{public}s", __func__);
    DataShare::DataShareExtAbility::SetCreator(TelephonyDataShareCreator);
}

void SimAbility::DoInit()
{
    if (initDatabaseDir && initRdbStore) {
        DATA_STORAGE_LOGD("DoInit has done");
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
    if (!path.empty()) {
        initDatabaseDir = true;
        path.append("/");
        helper_.UpdateDbPath(path);
        if (helper_.Init() == NativeRdb::E_OK) {
            initRdbStore = true;
        } else {
            DATA_STORAGE_LOGE("DoInit rdb init failed!");
            initRdbStore = false;
        }
    } else {
        DATA_STORAGE_LOGE("DoInit##databaseDir is empty!");
        initDatabaseDir = false;
    }
}

sptr<IRemoteObject> SimAbility::OnConnect(const AAFwk::Want &want)
{
    DATA_STORAGE_LOGD("SimAbility %{public}s begin.", __func__);
    Extension::OnConnect(want);
    sptr<DataShare::TelephonyDataShareStubImpl> remoteObject =
        new (std::nothrow) DataShare::TelephonyDataShareStubImpl();
    if (remoteObject == nullptr) {
        DATA_STORAGE_LOGE("%{public}s No memory allocated for DataShareStubImpl", __func__);
        return nullptr;
    }
    remoteObject->SetSimAbility(std::static_pointer_cast<SimAbility>(shared_from_this()));
    DATA_STORAGE_LOGD("SimAbility %{public}s end.", __func__);
    return remoteObject->AsObject();
}

void SimAbility::OnStart(const AppExecFwk::Want &want)
{
    DATA_STORAGE_LOGD("SimAbility::OnStart");
    Extension::OnStart(want);
    DoInit();
}

int SimAbility::Insert(const Uri &uri, const DataShare::DataShareValuesBucket &value)
{
    if (!PermissionUtil::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        DATA_STORAGE_LOGE("Permission denied!");
        return DATA_STORAGE_ERR_PERMISSION_ERR;
    }
    if (!IsInitOk()) {
        return DATA_STORAGE_ERROR;
    }
    std::lock_guard<std::mutex> guard(lock_);
    Uri tempUri = uri;
    SimUriType simUriType = ParseUriType(tempUri);
    int64_t id = DATA_STORAGE_ERROR;
    if (simUriType == SimUriType::SIM_INFO) {
        OHOS::NativeRdb::ValuesBucket values = RdbDataShareAdapter::RdbUtils::ToValuesBucket(value);
        helper_.Insert(id, values, TABLE_SIM_INFO);
    } else {
        DATA_STORAGE_LOGI("SimAbility::Insert##uri = %{public}s", uri.ToString().c_str());
    }
    return id;
}

std::shared_ptr<DataShare::DataShareResultSet> SimAbility::Query(
    const Uri &uri, const DataShare::DataSharePredicates &predicates, std::vector<std::string> &columns,
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
    SimUriType simUriType = ParseUriType(tempUri);
    if (simUriType == SimUriType::SIM_INFO) {
        NativeRdb::AbsRdbPredicates *absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_SIM_INFO);
        if (absRdbPredicates != nullptr) {
            NativeRdb::RdbPredicates rdbPredicates = ConvertPredicates(absRdbPredicates->GetTableName(), predicates);
            auto resultSet = helper_.Query(rdbPredicates, columns);
            if (resultSet == nullptr) {
                DATA_STORAGE_LOGE("SimAbility::Query  NativeRdb::ResultSet is null!");
                delete absRdbPredicates;
                absRdbPredicates = nullptr;
                return nullptr;
            }
            auto queryResultSet = RdbDataShareAdapter::RdbUtils::ToResultSetBridge(resultSet);
            sharedPtrResult = std::make_shared<DataShare::DataShareResultSet>(queryResultSet);
            delete absRdbPredicates;
            absRdbPredicates = nullptr;
        } else {
            DATA_STORAGE_LOGE("SimAbility::Query  NativeRdb::AbsRdbPredicates is null!");
        }
    } else {
        DATA_STORAGE_LOGI("SimAbility::Query failed##uri = %{public}s", uri.ToString().c_str());
    }
    return sharedPtrResult;
}

int SimAbility::Update(
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
    Uri tempUri = uri;
    SimUriType simUriType = ParseUriType(tempUri);
    switch (simUriType) {
        case SimUriType::SIM_INFO: {
            NativeRdb::AbsRdbPredicates *absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_SIM_INFO);
            if (absRdbPredicates != nullptr) {
                int changedRows = CHANGED_ROWS;
                NativeRdb::RdbPredicates rdbPredicates =
                    ConvertPredicates(absRdbPredicates->GetTableName(), predicates);
                OHOS::NativeRdb::ValuesBucket values = RdbDataShareAdapter::RdbUtils::ToValuesBucket(value);
                result = helper_.Update(changedRows, values, rdbPredicates);
                delete absRdbPredicates;
                absRdbPredicates = nullptr;
            } else {
                DATA_STORAGE_LOGE("SimAbility::Update  NativeRdb::AbsRdbPredicates is null!");
            }
            break;
        }
        case SimUriType::SET_CARD: {
            result = SetCard(value);
            if (result != NativeRdb::E_OK) {
                DATA_STORAGE_LOGE("SimAbility::Update  SetCard failed!");
                result = static_cast<int>(LoadProFileErrorType::SET_CARD_FAIL);
            }
            break;
        }
        default:
            DATA_STORAGE_LOGI("SimAbility::Update##uri = %{public}s", uri.ToString().c_str());
            break;
    }
    return result;
}

int SimAbility::SetCard(const DataShare::DataShareValuesBucket &sharedValue)
{
    if (!PermissionUtil::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        DATA_STORAGE_LOGE("Permission denied!");
        return DATA_STORAGE_ERR_PERMISSION_ERR;
    }
    OHOS::NativeRdb::ValuesBucket value = RdbDataShareAdapter::RdbUtils::ToValuesBucket(sharedValue);
    int result = DATA_STORAGE_ERROR;
    if (!value.HasColumn(SimData::SIM_ID)) {
        DATA_STORAGE_LOGE("SimAbility::Update##the sim_id in valuesBucket does not exist!");
        return result;
    }
    if (!value.HasColumn(SimData::CARD_TYPE)) {
        DATA_STORAGE_LOGE("SimAbility::Update##the card_type in valuesBucket does not exist!");
        return result;
    }
    NativeRdb::ValueObject valueObject;
    bool isExistSimId = value.GetObject(SimData::SIM_ID, valueObject);
    if (!isExistSimId) {
        DATA_STORAGE_LOGE("SimAbility::Update##failed to get sim_id value in valuesBucket!");
        return result;
    }
    int simId = 0;
    valueObject.GetInt(simId);

    bool isExistCardType = value.GetObject(SimData::CARD_TYPE, valueObject);
    if (!isExistCardType) {
        DATA_STORAGE_LOGE("SimAbility::Update##failed to get card_type value in valuesBucket!");
        return result;
    }
    int cardType = 0;
    valueObject.GetInt(cardType);
    result = helper_.SetDefaultCardByType(simId, cardType);
    return result;
}

int SimAbility::Delete(const Uri &uri, const DataShare::DataSharePredicates &predicates)
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
    Uri tempUri = uri;
    SimUriType simUriType = ParseUriType(tempUri);
    if (simUriType == SimUriType::SIM_INFO) {
        NativeRdb::AbsRdbPredicates *absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_SIM_INFO);
        if (absRdbPredicates != nullptr) {
            NativeRdb::RdbPredicates rdbPredicates = ConvertPredicates(absRdbPredicates->GetTableName(), predicates);
            int deletedRows = 0;
            result = helper_.Delete(deletedRows, rdbPredicates);
            delete absRdbPredicates;
            absRdbPredicates = nullptr;
        } else {
            DATA_STORAGE_LOGE("SimAbility::Update  NativeRdb::AbsRdbPredicates is null!");
        }
    } else {
        DATA_STORAGE_LOGI("SimAbility::Delete failed##uri = %{public}s", uri.ToString().c_str());
    }
    return result;
}

bool SimAbility::IsInitOk()
{
    if (!initDatabaseDir) {
        DATA_STORAGE_LOGE("SimAbility::IsInitOk initDatabaseDir failed!");
    } else if (!initRdbStore) {
        DATA_STORAGE_LOGE("SimAbility::IsInitOk initRdbStore failed!");
    }
    return initDatabaseDir && initRdbStore;
}

std::string SimAbility::GetType(const Uri &uri)
{
    DATA_STORAGE_LOGI("SimAbility::GetType##uri = %{public}s", uri.ToString().c_str());
    std::string retval(uri.ToString());
    return retval;
}

int SimAbility::OpenFile(const Uri &uri, const std::string &mode)
{
    DATA_STORAGE_LOGI("SimAbility::OpenFile##uri = %{public}s", uri.ToString().c_str());
    Uri tempUri = uri;
    SimUriType simUriType = ParseUriType(tempUri);
    return static_cast<int>(simUriType);
}

SimUriType SimAbility::ParseUriType(Uri &uri)
{
    DATA_STORAGE_LOGI("SimAbility::ParseUriType start");
    SimUriType simUriType = SimUriType::UNKNOW;
    std::string uriPath = uri.ToString();
    if (!uriPath.empty()) {
        helper_.ReplaceAllStr(uriPath, ":///", "://");
        Uri tempUri(uriPath);
        std::string path = tempUri.GetPath();
        if (!path.empty() && !simUriMap_.empty()) {
            DATA_STORAGE_LOGI("SimAbility::ParseUriType##path = %{public}s", path.c_str());
            auto it = simUriMap_.find(path);
            if (it != simUriMap_.end()) {
                simUriType = it->second;
                DATA_STORAGE_LOGI("SimAbility::ParseUriType##simUriType = %{public}d", simUriType);
            }
        }
    }
    return simUriType;
}

OHOS::NativeRdb::RdbPredicates SimAbility::ConvertPredicates(
    const std::string &tableName, const DataShare::DataSharePredicates &predicates)
{
    OHOS::NativeRdb::RdbPredicates res = RdbDataShareAdapter::RdbUtils::ToPredicates(predicates, tableName);
    return res;
}
} // namespace Telephony
} // namespace OHOS
