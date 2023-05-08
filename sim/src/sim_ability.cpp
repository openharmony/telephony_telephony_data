/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include <cstdint>

#include "ability_context.h"
#include "ability_loader.h"
#include "abs_rdb_predicates.h"
#include "abs_shared_result_set.h"
#include "data_ability_predicates.h"
#include "data_storage_errors.h"
#include "data_storage_log_wrapper.h"
#include "new"
#include "predicates_utils.h"
#include "rdb_errno.h"
#include "sim_data.h"
#include "uri.h"
#include "utility"
#include "values_bucket.h"

namespace OHOS {
using AppExecFwk::AbilityLoader;
using AppExecFwk::Ability;
namespace Telephony {
void SimAbility::OnStart(const AppExecFwk::Want &want)
{
    DATA_STORAGE_LOGI("SimAbility::OnStart\n");
    Ability::OnStart(want);
    auto abilityContext = GetAbilityContext();
    if (abilityContext == nullptr) {
        DATA_STORAGE_LOGE("SimAbility::OnStart GetAbilityContext is null");
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
        InitUriMap();
        if (helper_.Init() == NativeRdb::E_OK) {
            initRdbStore = true;
        } else {
            DATA_STORAGE_LOGE("SimAbility::OnStart rdb init fail!");
            initRdbStore = false;
        }
    } else {
        initDatabaseDir = false;
        DATA_STORAGE_LOGE("SimAbility::OnStart##the databaseDir is empty\n");
    }
}

int SimAbility::Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
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
    SimUriType simUriType = ParseUriType(tempUri);
    int64_t id = DATA_STORAGE_ERROR;
    if (simUriType == SimUriType::SIM_INFO) {
        helper_.Insert(id, value, TABLE_SIM_INFO);
    } else {
        DATA_STORAGE_LOGI("SimAbility::Insert##uri = %{public}s\n", uri.ToString().c_str());
    }
    return id;
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> SimAbility::Query(
    const Uri &uri, const std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    if (!TelephonyPermission::CheckPermission(Permission::GET_TELEPHONY_STATE)) {
        DATA_STORAGE_LOGE("Permission denied!");
        return nullptr;
    }
    std::shared_ptr<NativeRdb::AbsSharedResultSet> resultSet = nullptr;
    if (!IsInitOk()) {
        return resultSet;
    }
    Uri tempUri = uri;
    SimUriType simUriType = ParseUriType(tempUri);
    if (simUriType == SimUriType::SIM_INFO) {
        NativeRdb::AbsRdbPredicates *absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_SIM_INFO);
        if (absRdbPredicates != nullptr) {
            ConvertPredicates(predicates, absRdbPredicates);
            resultSet = helper_.Query(*absRdbPredicates, columns);
            delete absRdbPredicates;
            absRdbPredicates = nullptr;
        } else {
            DATA_STORAGE_LOGE("SimAbility::Update  NativeRdb::AbsRdbPredicates is null!");
        }
    } else {
        DATA_STORAGE_LOGI("SimAbility::Query##uri = %{public}s\n", uri.ToString().c_str());
    }
    return resultSet;
}

int SimAbility::Update(
    const Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
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
    SimUriType simUriType = ParseUriType(tempUri);
    switch (simUriType) {
        case SimUriType::SIM_INFO: {
            NativeRdb::AbsRdbPredicates *absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_SIM_INFO);
            if (absRdbPredicates != nullptr) {
                int changedRows = 0;
                ConvertPredicates(predicates, absRdbPredicates);
                result = helper_.Update(changedRows, value, *absRdbPredicates);
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
                DATA_STORAGE_LOGE("SimAbility::Update  SetCard fail!");
                result = static_cast<int>(LoadProFileErrorType::SET_CARD_FAIL);
            }
            break;
        }
        default:
            DATA_STORAGE_LOGI("SimAbility::Update##uri = %{public}s\n", uri.ToString().c_str());
            break;
    }
    return result;
}

int SimAbility::SetCard(const NativeRdb::ValuesBucket &value)
{
    if (!TelephonyPermission::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        DATA_STORAGE_LOGE("Permission denied!");
        return DATA_STORAGE_ERR_PERMISSION_ERR;
    }
    int result = DATA_STORAGE_ERROR;
    if (!value.HasColumn(SimData::SLOT_INDEX)) {
        DATA_STORAGE_LOGE("SimAbility::Update##the slot_index in valuesBucket does not exist");
        return result;
    }
    if (!value.HasColumn(SimData::CARD_TYPE)) {
        DATA_STORAGE_LOGE("SimAbility::Update##the card_type in valuesBucket does not exist");
        return result;
    }
    NativeRdb::ValueObject valueObject;
    bool isExistSlotId = value.GetObject(SimData::SLOT_INDEX, valueObject);
    if (!isExistSlotId) {
        DATA_STORAGE_LOGE("SimAbility::Update##failed to get slot_index value in valuesBucket");
        return result;
    }
    int slotId = 0;
    valueObject.GetInt(slotId);

    bool isExistCardType = value.GetObject(SimData::CARD_TYPE, valueObject);
    if (!isExistCardType) {
        DATA_STORAGE_LOGE("SimAbility::Update##failed to get card_type value in valuesBucket");
        return result;
    }
    int cardType = 0;
    valueObject.GetInt(cardType);
    result = helper_.SetDefaultCardByType(slotId, cardType);
    return result;
}

int SimAbility::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
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
    SimUriType simUriType = ParseUriType(tempUri);
    if (simUriType == SimUriType::SIM_INFO) {
        NativeRdb::AbsRdbPredicates *absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_SIM_INFO);
        if (absRdbPredicates != nullptr) {
            ConvertPredicates(predicates, absRdbPredicates);
            int deletedRows = 0;
            result = helper_.Delete(deletedRows, *absRdbPredicates);
            delete absRdbPredicates;
            absRdbPredicates = nullptr;
        } else {
            DATA_STORAGE_LOGE("SimAbility::Update  NativeRdb::AbsRdbPredicates is null!");
        }
    } else {
        DATA_STORAGE_LOGI("SimAbility::Delete##uri = %{public}s\n", uri.ToString().c_str());
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

void SimAbility::InitUriMap()
{
    simUriMap = {
        {"/sim/sim_info", SimUriType::SIM_INFO},
        {"/sim/sim_info/set_card", SimUriType::SET_CARD}
    };
}

std::string SimAbility::GetType(const Uri &uri)
{
    DATA_STORAGE_LOGI("SimAbility::GetType##uri = %{public}s\n", uri.ToString().c_str());
    std::string retval(uri.ToString());
    return retval;
}

int SimAbility::OpenFile(const Uri &uri, const std::string &mode)
{
    DATA_STORAGE_LOGI("SimAbility::OpenFile##uri = %{public}s\n", uri.ToString().c_str());
    Uri tempUri = uri;
    SimUriType simUriType = ParseUriType(tempUri);
    return static_cast<int>(simUriType);
}

SimUriType SimAbility::ParseUriType(Uri &uri)
{
    DATA_STORAGE_LOGI("SimAbility::ParseUriType start\n");
    SimUriType simUriType = SimUriType::UNKNOW;
    std::string uriPath = uri.ToString();
    if (!uriPath.empty()) {
        helper_.ReplaceAllStr(uriPath, ":///", "://");
        Uri tempUri(uriPath);
        std::string path = tempUri.GetPath();
        if (!path.empty()) {
            DATA_STORAGE_LOGI("SimAbility::ParseUriType##path = %{public}s\n", path.c_str());
            std::map<std::string, SimUriType>::iterator it = simUriMap.find(path);
            if (it != simUriMap.end()) {
                simUriType = it->second;
                DATA_STORAGE_LOGI("SimAbility::ParseUriType##simUriType = %{public}d\n", simUriType);
            }
        }
    }
    return simUriType;
}

void SimAbility::ConvertPredicates(
    const NativeRdb::DataAbilityPredicates &dataPredicates, NativeRdb::AbsRdbPredicates *absRdbPredicates)
{
    NativeRdb::PredicatesUtils::SetWhereClauseAndArgs(
        absRdbPredicates, dataPredicates.GetWhereClause(), dataPredicates.GetWhereArgs());
    NativeRdb::PredicatesUtils::SetAttributes(absRdbPredicates, dataPredicates.IsDistinct(),
        dataPredicates.GetIndex(), dataPredicates.GetGroup(), dataPredicates.GetOrder(), dataPredicates.GetLimit(),
        dataPredicates.GetOffset());
}

REGISTER_AA(SimAbility);
}  // namespace Telephony
}  // namespace OHOS
