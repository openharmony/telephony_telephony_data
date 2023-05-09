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

#include "pdp_profile_ability.h"

#include <cstdint>

#include "ability_context.h"
#include "ability_loader.h"
#include "abs_rdb_predicates.h"
#include "abs_shared_result_set.h"
#include "data_ability_predicates.h"
#include "data_storage_errors.h"
#include "data_storage_log_wrapper.h"
#include "new"
#include "pdp_profile_data.h"
#include "predicates_utils.h"
#include "rdb_errno.h"
#include "uri.h"
#include "utility"

namespace OHOS {
using AppExecFwk::AbilityLoader;
using AppExecFwk::Ability;
namespace Telephony {
void PdpProfileAbility::OnStart(const AppExecFwk::Want &want)
{
    DATA_STORAGE_LOGI("PdpProfileAbility::OnStart\n");
    Ability::OnStart(want);
    auto abilityContext = GetAbilityContext();
    if (abilityContext == nullptr) {
        DATA_STORAGE_LOGE("PdpProfileAbility::OnStart GetAbilityContext is null");
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
        int rdbInitCode = helper_.Init();
        if (rdbInitCode == NativeRdb::E_OK) {
            initRdbStore = true;
        } else {
            DATA_STORAGE_LOGE("PdpProfileAbility::OnStart rdb init fail!");
            initRdbStore = false;
        }
    } else {
        initDatabaseDir = false;
        DATA_STORAGE_LOGE("PdpProfileAbility::OnStart##the databaseDir is empty!");
    }
}

int PdpProfileAbility::Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
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
        helper_.Insert(id, value, TABLE_PDP_PROFILE);
    } else {
        DATA_STORAGE_LOGE("PdpProfileAbility::Insert##uri = %{public}s", uri.ToString().c_str());
    }
    return id;
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> PdpProfileAbility::Query(
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
    PdpProfileUriType pdpProfileUriType = ParseUriType(tempUri);
    if (pdpProfileUriType == PdpProfileUriType::PDP_PROFILE) {
        NativeRdb::AbsRdbPredicates *absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_PDP_PROFILE);
        if (absRdbPredicates != nullptr) {
            ConvertPredicates(predicates, absRdbPredicates);
            resultSet = helper_.Query(*absRdbPredicates, columns);
            delete absRdbPredicates;
            absRdbPredicates = nullptr;
        } else {
            DATA_STORAGE_LOGE("PdpProfileAbility::Delete  NativeRdb::AbsRdbPredicates is null!");
        }
    } else {
        DATA_STORAGE_LOGE("PdpProfileAbility::Query##uri = %{public}s", uri.ToString().c_str());
    }
    return resultSet;
}

int PdpProfileAbility::Update(
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
        int changedRows = 0;
        ConvertPredicates(predicates, absRdbPredicates);
        result = helper_.Update(changedRows, value, *absRdbPredicates);
        delete absRdbPredicates;
        absRdbPredicates = nullptr;
    } else if (result == DATA_STORAGE_ERROR) {
        DATA_STORAGE_LOGE("PdpProfileAbility::Update  NativeRdb::AbsRdbPredicates is null!");
    }
    return result;
}

int PdpProfileAbility::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
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
            ConvertPredicates(predicates, absRdbPredicates);
            int deletedRows = 0;
            result = helper_.Delete(deletedRows, *absRdbPredicates);
            delete absRdbPredicates;
            absRdbPredicates = nullptr;
        } else {
            DATA_STORAGE_LOGE("PdpProfileAbility::Delete  NativeRdb::AbsRdbPredicates is null!");
        }
    } else {
        DATA_STORAGE_LOGI("PdpProfileAbility::Delete##uri = %{public}s\n", uri.ToString().c_str());
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

void PdpProfileAbility::InitUriMap()
{
    pdpProfileUriMap = {
        {"/net/pdp_profile", PdpProfileUriType::PDP_PROFILE},
        {"/net/pdp_profile/reset", PdpProfileUriType::RESET}
    };
}

std::string PdpProfileAbility::GetType(const Uri &uri)
{
    DATA_STORAGE_LOGI("PdpProfileAbility::GetType##uri = %{public}s\n", uri.ToString().c_str());
    std::string retval(uri.ToString());
    return retval;
}

int PdpProfileAbility::OpenFile(const Uri &uri, const std::string &mode)
{
    DATA_STORAGE_LOGI("PdpProfileAbility::OpenFile##uri = %{public}s\n", uri.ToString().c_str());
    Uri tempUri = uri;
    PdpProfileUriType pdpProfileUriType = ParseUriType(tempUri);
    return static_cast<int>(pdpProfileUriType);
}

PdpProfileUriType PdpProfileAbility::ParseUriType(Uri &uri)
{
    DATA_STORAGE_LOGI("PdpProfileAbility::ParseUriType start\n");
    PdpProfileUriType pdpProfileUriType = PdpProfileUriType::UNKNOW;
    std::string uriPath = uri.ToString();
    if (!uriPath.empty()) {
        helper_.ReplaceAllStr(uriPath, ":///", "://");
        Uri tempUri(uriPath);
        std::string path = tempUri.GetPath();
        if (!path.empty()) {
            DATA_STORAGE_LOGI("PdpProfileAbility::ParseUriType##path = %{public}s\n", path.c_str());
            std::map<std::string, PdpProfileUriType>::iterator it = pdpProfileUriMap.find(path);
            if (it != pdpProfileUriMap.end()) {
                pdpProfileUriType = it->second;
                DATA_STORAGE_LOGI("PdpProfileAbility::ParseUriType##pdpProfileUriType = %{public}d\n",
                    pdpProfileUriType);
            }
        }
    }
    return pdpProfileUriType;
}

void PdpProfileAbility::ConvertPredicates(
    const NativeRdb::DataAbilityPredicates &dataPredicates, NativeRdb::AbsRdbPredicates *absRdbPredicates)
{
    NativeRdb::PredicatesUtils::SetWhereClauseAndArgs(
        absRdbPredicates, dataPredicates.GetWhereClause(), dataPredicates.GetWhereArgs());
    NativeRdb::PredicatesUtils::SetAttributes(absRdbPredicates, dataPredicates.IsDistinct(),
        dataPredicates.GetIndex(), dataPredicates.GetGroup(), dataPredicates.GetOrder(), dataPredicates.GetLimit(),
        dataPredicates.GetOffset());
}

REGISTER_AA(PdpProfileAbility);
}  // namespace Telephony
}  // namespace OHOS
