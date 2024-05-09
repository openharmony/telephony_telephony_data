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

#include "ability_context.h"
#include "ability_loader.h"
#include "abs_rdb_predicates.h"
#include "abs_shared_result_set.h"
#include "core_service_client.h"
#include "data_storage_errors.h"
#include "data_storage_log_wrapper.h"
#include "datashare_ext_ability.h"
#include "datashare_predicates.h"
#include "new"
#include "pdp_profile_data.h"
#include "permission_util.h"
#include "preferences_util.h"
#include "rdb_errno.h"
#include "rdb_utils.h"
#include "string_ex.h"
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
    { "/net/pdp_profile/init", PdpProfileUriType::INIT},
    { "/net/pdp_profile/reset", PdpProfileUriType::RESET },
    { "/net/pdp_profile/preferapn", PdpProfileUriType::PREFER_APN },
};

PdpProfileAbility::PdpProfileAbility() : DataShareExtAbility() {}

PdpProfileAbility::~PdpProfileAbility() {}

PdpProfileAbility* PdpProfileAbility::Create()
{
    DATA_STORAGE_LOGD("PdpProfileAbility::Create begin.");
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

int PdpProfileAbility::BatchInsert(const Uri &uri, const std::vector<DataShare::DataShareValuesBucket> &values)
{
    if (!PermissionUtil::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        DATA_STORAGE_LOGE("Permission denied!");
        return DATA_STORAGE_ERR_PERMISSION_ERR;
    }
    if (!IsInitOk()) {
        return DATA_STORAGE_ERROR;
    }
    Uri tempUri = uri;
    PdpProfileUriType pdpProfileUriType = ParseUriType(tempUri);
    int result = DATA_STORAGE_ERROR;
    if (pdpProfileUriType == PdpProfileUriType::INIT && !values.empty()) {
        for (const auto &item : values) {
            OHOS::NativeRdb::ValuesBucket valuesBucket = RdbDataShareAdapter::RdbUtils::ToValuesBucket(item);
            int slotId = 0;
            if (GetIntFromValuesBucket(valuesBucket, "slotId", slotId) != NativeRdb::E_OK) {
                continue;
            }
            std::string opkey;
            GetTargetOpkey(slotId, opkey);
            result = helper_.InitAPNDatabase(slotId, opkey, true);
            DATA_STORAGE_LOGI(
                "PdpProfileAbility::BatchInsert INIT, slotId = %{public}d, result = %{public}d", slotId, result);
        }
    }
    return result;
}

int PdpProfileAbility::Insert(const Uri &uri, const DataShare::DataShareValuesBucket &value)
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

std::string PdpProfileAbility::GetQueryKey(const std::string &queryString, const std::string &key)
{
    size_t pos = queryString.find(key);
    if (pos != std::string::npos) {
        return queryString.substr(pos + key.length());
    }
    return "";
}

int PdpProfileAbility::GetPreferApn(const std::string &queryString)
{
    auto preferencesUtil = DelayedSingleton<PreferencesUtil>::GetInstance();
    if (preferencesUtil == nullptr) {
        DATA_STORAGE_LOGE("preferencesUtil is nullptr!");
        return NativePreferences::E_ERROR;
    }
    int preferApnId = preferencesUtil->ObtainInt(PREFER_APN_ID + GetQueryKey(queryString, "simId="),
                                                 INVALID_PROFILE_ID);
    DATA_STORAGE_LOGI("%{public}d.", preferApnId);
    return preferApnId;
}

int PdpProfileAbility::SetPreferApn(int simId, int profileId)
{
    auto preferencesUtil = DelayedSingleton<PreferencesUtil>::GetInstance();
    if (preferencesUtil == nullptr) {
        DATA_STORAGE_LOGE("preferencesUtil is nullptr!");
        return NativePreferences::E_ERROR;
    }
    DATA_STORAGE_LOGI("simId:%{public}d profileId:%{public}d", simId, profileId);
    return preferencesUtil->SaveInt(PREFER_APN_ID + std::to_string(simId), profileId);
}

std::shared_ptr<DataShare::DataShareResultSet> PdpProfileAbility::Query(const Uri &uri,
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
    PdpProfileUriType pdpProfileUriType = ParseUriType(tempUri);
    if (pdpProfileUriType == PdpProfileUriType::PDP_PROFILE || pdpProfileUriType == PdpProfileUriType::PREFER_APN) {
        NativeRdb::AbsRdbPredicates *absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_PDP_PROFILE);
        if (absRdbPredicates == nullptr) {
            DATA_STORAGE_LOGE("PdpProfileAbility::Query  NativeRdb::AbsRdbPredicates is null!");
            return sharedPtrResult;
        }
        std::shared_ptr<NativeRdb::ResultSet> result = nullptr;
        if (pdpProfileUriType == PdpProfileUriType::PREFER_APN) {
            DataShare::DataSharePredicates preferapnPredicates;
            preferapnPredicates.EqualTo(PdpProfileData::PROFILE_ID, GetPreferApn(tempUri.GetQuery()));
            result = helper_.Query(ConvertPredicates(absRdbPredicates->GetTableName(), preferapnPredicates), columns);
        } else if (pdpProfileUriType == PdpProfileUriType::PDP_PROFILE) {
            result = QueryPdpProfile(tempUri, absRdbPredicates->GetTableName(), predicates, columns);
        }
        if (result == nullptr) {
            DATA_STORAGE_LOGE("PdpProfileAbility::Query  NativeRdb::ResultSet is null!");
            delete absRdbPredicates;
            return nullptr;
        }
        auto queryResultSet = RdbDataShareAdapter::RdbUtils::ToResultSetBridge(result);
        sharedPtrResult = std::make_shared<DataShare::DataShareResultSet>(queryResultSet);
        delete absRdbPredicates;
        return sharedPtrResult;
    }
    DATA_STORAGE_LOGE("PdpProfileAbility::Query##uri = %{public}s", uri.ToString().c_str());
    return sharedPtrResult;
}

int PdpProfileAbility::Update(
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
    PdpProfileUriType pdpProfileUriType = ParseUriType(tempUri);
    NativeRdb::AbsRdbPredicates *absRdbPredicates = nullptr;
    switch (pdpProfileUriType) {
        case PdpProfileUriType::PDP_PROFILE: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_PDP_PROFILE);
            break;
        }
        case PdpProfileUriType::RESET: {
            result = ResetApn(value);
            if (result != NativeRdb::E_OK) {
                DATA_STORAGE_LOGE("PdpProfileAbility::Update  ResetApn fail!");
                result = static_cast<int>(LoadProFileErrorType::RESET_APN_FAIL);
            }
            break;
        }
        case PdpProfileUriType::PREFER_APN: {
            result = (UpdatePreferApn(value) == NativeRdb::E_OK)
                         ? NativeRdb::E_OK
                         : static_cast<int>(LoadProFileErrorType::PREFER_APN_FAIL);
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

int PdpProfileAbility::UpdatePreferApn(const DataShare::DataShareValuesBucket &sharedValue)
{
    OHOS::NativeRdb::ValuesBucket value = RdbDataShareAdapter::RdbUtils::ToValuesBucket(sharedValue);
    int result = DATA_STORAGE_ERROR;
    NativeRdb::ValueObject valueObject;
    if (!HasColumnValue(value, PdpProfileData::PROFILE_ID, valueObject)) {
        return result;
    }
    double temp = 0;
    int profileId = INVALID_PROFILE_ID;
    if (valueObject.GetDouble(temp) == NativeRdb::E_OK) {
        profileId = ceil(temp);
    }
    if (!HasColumnValue(value, PdpProfileData::SIM_ID, valueObject)) {
        return result;
    }
    int simId = DEFAULT_SIM_ID;
    if (valueObject.GetDouble(temp) == NativeRdb::E_OK) {
        simId = ceil(temp);
    }
    return SetPreferApn(simId, profileId);
}

bool PdpProfileAbility::HasColumnValue(
    const OHOS::NativeRdb::ValuesBucket &value, const char *columnName, NativeRdb::ValueObject &valueObject)
{
    if (!value.HasColumn(columnName)) {
        DATA_STORAGE_LOGE("the column in valuesBucket does not exist!");
        return false;
    }
    bool isExistValue = value.GetObject(columnName, valueObject);
    if (!isExistValue) {
        DATA_STORAGE_LOGE("failed to get value in valuesBucket!");
        return false;
    }
    return true;
}

int PdpProfileAbility::Delete(const Uri &uri, const DataShare::DataSharePredicates &predicates)
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
    DATA_STORAGE_LOGD("PdpProfileAbility::ParseUriType start");
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

std::shared_ptr<NativeRdb::ResultSet> PdpProfileAbility::QueryPdpProfile(Uri &uri, const std::string &tableName,
    const DataShare::DataSharePredicates &predicates, std::vector<std::string> &columns)
{
    const std::string &simIdStr = GetQueryKey(uri.GetQuery(), "simId=");
    std::string opkey;
    int simId = DEFAULT_SIM_ID;
    if (StrToInt(simIdStr, simId)) {
        int32_t slotId = DelayedRefSingleton<CoreServiceClient>::GetInstance().GetSlotId(simId);
        GetTargetOpkey(slotId, opkey);
    }
    if (opkey.empty() || strcmp(opkey.c_str(), INVALID_OPKEY) == 0) {
        return helper_.Query(ConvertPredicates(tableName, predicates), columns);
    }
    constexpr int32_t FIELD_IDX = 0;
    auto &operations = predicates.GetOperationList();
    std::vector<DataShare::OperationItem> operationsRes;
    bool isMccMnc = false;
    for (const auto &oper : operations) {
        if (oper.singleParams.empty()) {
            operationsRes.push_back(oper);
            continue;
        }
        std::string filed = static_cast<std::string>(oper.GetSingle(FIELD_IDX));
        if (strcmp(filed.c_str(), PdpProfileData::MCCMNC) == 0 && oper.operation == DataShare::EQUAL_TO) {
            isMccMnc = true;
            operationsRes.push_back({DataShare::EQUAL_TO, {PdpProfileData::OPKEY, opkey}});
            continue;
        }
        operationsRes.push_back(oper);
    }
    DATA_STORAGE_LOGI(
        "PdpProfileAbility::QueryPdpProfile, simId= %{public}d, isMccMnc= %{public}d", simId, isMccMnc);
    if (isMccMnc) {
        const std::shared_ptr<NativeRdb::ResultSet> &result =
            helper_.Query(ConvertPredicates(tableName, DataShare::DataSharePredicates(move(operationsRes))), columns);
        if (result != nullptr) {
            int count = 0;
            result->GetRowCount(count);
            DATA_STORAGE_LOGI("PdpProfileAbility::QueryPdpProfile, count= %{public}d", count);
            if (count > 0) {
                return result;
            }
        }
    }
    return helper_.Query(ConvertPredicates(tableName, predicates), columns);
}
 
int PdpProfileAbility::ResetApn(const DataShare::DataShareValuesBucket &valuesBucket)
{
    OHOS::NativeRdb::ValuesBucket values = RdbDataShareAdapter::RdbUtils::ToValuesBucket(valuesBucket);
    int simId = DEFAULT_SIM_ID;
    if (GetIntFromValuesBucket(values, "simId", simId) != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("PdpProfileAbility::ResetApn no simId!");
        return helper_.ResetApn();
    }
    int32_t slotId = DelayedRefSingleton<CoreServiceClient>::GetInstance().GetSlotId(simId);
    std::string opkey;
    GetTargetOpkey(slotId, opkey);
    if (opkey.empty() || strcmp(opkey.c_str(), INVALID_OPKEY) == 0) {
        DATA_STORAGE_LOGW("PdpProfileAbility::ResetApn opkey empty!");
        return helper_.ResetApn();
    }
    DATA_STORAGE_LOGI("PdpProfileAbility::ResetApn##simId = %{public}d", simId);
    SetPreferApn(simId, -1);
    NativeRdb::RdbPredicates rdbPredicates(TABLE_PDP_PROFILE);
    rdbPredicates.EqualTo(PdpProfileData::OPKEY, opkey);
    int deletedRows = CHANGED_ROWS;
    helper_.Delete(deletedRows, rdbPredicates);
    int result = helper_.InitAPNDatabase(slotId, opkey, false);
    if (result != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("PdpProfileAbility::ResetApn fail!");
        result = static_cast<int>(LoadProFileErrorType::RESET_APN_FAIL);
    }
    return result;
}

int PdpProfileAbility::GetIntFromValuesBucket(OHOS::NativeRdb::ValuesBucket &bucket, const char *key, int &value)
{
    NativeRdb::ValueObject valueObject;
    if (!HasColumnValue(bucket, key, valueObject)) {
        return OPERATION_ERROR;
    }
    if (valueObject.GetType() == NativeRdb::ValueObject::TYPE_INT) {
        return valueObject.GetInt(value);
    }
    if (valueObject.GetType() == NativeRdb::ValueObject::TYPE_DOUBLE) {
        double temp = 0;
        if (valueObject.GetDouble(temp) == NativeRdb::E_OK) {
            value = ceil(temp);
            return NativeRdb::E_OK;
        }
    }
    return OPERATION_ERROR;
}
 
void PdpProfileAbility::GetTargetOpkey(int slotId, std::string &opkey)
{
    std::u16string opkeyU16;
    DelayedRefSingleton<CoreServiceClient>::GetInstance().GetTargetOpkey(slotId, opkeyU16);
    opkey = Str16ToStr8(opkeyU16);
    DATA_STORAGE_LOGI(
        "PdpProfileAbility::GetTargetOpkey##slotId = %{public}d, opkey = %{public}s", slotId, opkey.c_str());
}
} // namespace Telephony
} // namespace OHOS
