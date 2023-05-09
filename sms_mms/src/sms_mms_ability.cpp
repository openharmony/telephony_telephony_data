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

#include "sms_mms_ability.h"

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
#include "sms_mms_data.h"
#include "uri.h"
#include "utility"

namespace OHOS {
using AppExecFwk::Ability;
using AppExecFwk::AbilityLoader;
namespace Telephony {
void SmsMmsAbility::OnStart(const AppExecFwk::Want &want)
{
    DATA_STORAGE_LOGI("SmsMmsAbility::OnStart\n");
    Ability::OnStart(want);
    auto abilityContext = GetAbilityContext();
    if (abilityContext == nullptr) {
        DATA_STORAGE_LOGE("SmsMmsAbility::OnStart GetAbilityContext is null");
        return;
    }
    // switch database dir to el1 for init before unlock
    abilityContext->SwitchArea(0);
    std::string path = abilityContext->GetDatabaseDir();
    DATA_STORAGE_LOGI("GetDatabaseDir: %{public}s", path.c_str());
    if (!path.empty()) {
        initDatabaseDir = true;
        path.append("/");
        InitUriMap();
        helper_.UpdateDbPath(path);
        if (helper_.Init() == NativeRdb::E_OK) {
            initRdbStore = true;
        } else {
            DATA_STORAGE_LOGE("SmsMmsAbility::OnStart rdb init fail!");
            initRdbStore = false;
        }
    } else {
        initDatabaseDir = false;
        DATA_STORAGE_LOGE("SmsMmsAbility::OnStart##the databaseDir is empty\n");
    }
}

int SmsMmsAbility::Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
{
    if (!TelephonyPermission::CheckPermission(Permission::READ_MESSAGES)) {
        DATA_STORAGE_LOGE("Permission denied!");
        return DATA_STORAGE_ERR_PERMISSION_ERR;
    }
    if (!IsInitOk()) {
        return DATA_STORAGE_ERROR;
    }
    std::lock_guard<std::mutex> guard(lock_);
    Uri tempUri = uri;
    MessageUriType messageUriType = ParseUriType(tempUri);
    int64_t id = DATA_STORAGE_ERROR;
    switch (messageUriType) {
        case MessageUriType::SMS_MMS: {
            helper_.Insert(id, value, TABLE_SMS_MMS_INFO);
            break;
        }
        case MessageUriType::MMS_PROTOCOL: {
            helper_.Insert(id, value, TABLE_MMS_PROTOCOL);
            break;
        }
        case MessageUriType::SMS_SUBSECTION: {
            helper_.Insert(id, value, TABLE_SMS_SUBSECTION);
            break;
        }
        case MessageUriType::MMS_PART: {
            helper_.Insert(id, value, TABLE_MMS_PART);
            break;
        }
        default:
            DATA_STORAGE_LOGI("SmsMmsAbility::Insert##uri = %{public}s\n", uri.ToString().c_str());
            break;
    }
    return id;
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> SmsMmsAbility::Query(
    const Uri &uri, const std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    if (!TelephonyPermission::CheckPermission(Permission::READ_MESSAGES)) {
        DATA_STORAGE_LOGE("Permission denied!");
        return nullptr;
    }
    std::shared_ptr<NativeRdb::AbsSharedResultSet> resultSet = nullptr;
    if (!IsInitOk()) {
        return resultSet;
    }
    Uri tempUri = uri;
    MessageUriType messageUriType = ParseUriType(tempUri);
    NativeRdb::AbsRdbPredicates *absRdbPredicates = nullptr;
    switch (messageUriType) {
        case MessageUriType::SMS_MMS: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_SMS_MMS_INFO);
            break;
        }
        case MessageUriType::MMS_PROTOCOL: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_MMS_PROTOCOL);
            break;
        }
        case MessageUriType::SMS_SUBSECTION: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_SMS_SUBSECTION);
            break;
        }
        case MessageUriType::MMS_PART: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_MMS_PART);
            break;
        }
        case MessageUriType::MAX_GROUP: {
            resultSet = helper_.QueryMaxGroupId();
            break;
        }
        case MessageUriType::UNREAD_TOTAL: {
            resultSet = helper_.StatisticsUnRead();
            break;
        }
        default:
            DATA_STORAGE_LOGI("SmsMmsAbility::Query##uri = %{public}s\n", uri.ToString().c_str());
            break;
    }
    if (absRdbPredicates != nullptr) {
        ConvertPredicates(predicates, absRdbPredicates);
        resultSet = helper_.Query(*absRdbPredicates, columns);
        delete absRdbPredicates;
        absRdbPredicates = nullptr;
    } else {
        DATA_STORAGE_LOGE("SmsMmsAbility::Query  NativeRdb::AbsRdbPredicates is null!");
    }
    return resultSet;
}

int SmsMmsAbility::Update(
    const Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
{
    if (!TelephonyPermission::CheckPermission(Permission::READ_MESSAGES)) {
        DATA_STORAGE_LOGE("Permission denied!");
        return DATA_STORAGE_ERR_PERMISSION_ERR;
    }
    int result = DATA_STORAGE_ERROR;
    if (!IsInitOk()) {
        return result;
    }
    std::lock_guard<std::mutex> guard(lock_);
    Uri tempUri = uri;
    MessageUriType messageUriType = ParseUriType(tempUri);
    NativeRdb::AbsRdbPredicates *absRdbPredicates = nullptr;
    switch (messageUriType) {
        case MessageUriType::SMS_MMS: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_SMS_MMS_INFO);
            break;
        }
        case MessageUriType::MMS_PROTOCOL: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_MMS_PROTOCOL);
            break;
        }
        case MessageUriType::SMS_SUBSECTION: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_SMS_SUBSECTION);
            break;
        }
        case MessageUriType::MMS_PART: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_MMS_PART);
            break;
        }
        default:
            DATA_STORAGE_LOGI("SmsMmsAbility::Update##uri = %{public}s\n", uri.ToString().c_str());
            break;
    }
    if (absRdbPredicates != nullptr) {
        int changedRows = 0;
        ConvertPredicates(predicates, absRdbPredicates);
        result = helper_.Update(changedRows, value, *absRdbPredicates);
        delete absRdbPredicates;
        absRdbPredicates = nullptr;
    } else {
        DATA_STORAGE_LOGE("SmsMmsAbility::Update  NativeRdb::AbsRdbPredicates is null!");
    }
    return result;
}

int SmsMmsAbility::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    if (!TelephonyPermission::CheckPermission(Permission::READ_MESSAGES)) {
        DATA_STORAGE_LOGE("Permission denied!");
        return DATA_STORAGE_ERR_PERMISSION_ERR;
    }
    int result = DATA_STORAGE_ERROR;
    if (!IsInitOk()) {
        return result;
    }
    std::lock_guard<std::mutex> guard(lock_);
    Uri tempUri = uri;
    MessageUriType messageUriType = ParseUriType(tempUri);
    NativeRdb::AbsRdbPredicates *absRdbPredicates = nullptr;
    switch (messageUriType) {
        case MessageUriType::SMS_MMS: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_SMS_MMS_INFO);
            break;
        }
        case MessageUriType::THIRTY: {
            result = helper_.DeleteDataByThirty();
            if (result != NativeRdb::E_OK) {
                DATA_STORAGE_LOGE("SmsMmsAbility::Delete  DeleteDataByThirty fail!");
                result = static_cast<int>(LoadProFileErrorType::DELETE_THIRTY_DATA_FAIL);
            }
            break;
        }
        case MessageUriType::MMS_PROTOCOL: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_MMS_PROTOCOL);
            break;
        }
        case MessageUriType::SMS_SUBSECTION: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_SMS_SUBSECTION);
            break;
        }
        case MessageUriType::MMS_PART: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_MMS_PART);
            break;
        }
        default:
            DATA_STORAGE_LOGI("SmsMmsAbility::Delete##uri = %{public}s\n", uri.ToString().c_str());
            break;
    }
    if (absRdbPredicates != nullptr) {
        ConvertPredicates(predicates, absRdbPredicates);
        int deletedRows = 0;
        result = helper_.Delete(deletedRows, *absRdbPredicates);
        delete absRdbPredicates;
        absRdbPredicates = nullptr;
    } else if (result == DATA_STORAGE_ERROR) {
        DATA_STORAGE_LOGE("SmsMmsAbility::Delete  NativeRdb::AbsRdbPredicates is null!");
    }
    return result;
}

bool SmsMmsAbility::IsInitOk()
{
    if (!initDatabaseDir) {
        DATA_STORAGE_LOGE("SmsMmsAbility::IsInitOk initDatabaseDir failed!");
    } else if (!initRdbStore) {
        DATA_STORAGE_LOGE("SmsMmsAbility::IsInitOk initRdbStore failed!");
    }
    return initDatabaseDir && initRdbStore;
}

void SmsMmsAbility::InitUriMap()
{
    smsMmsUriMap = {
        {"/sms_mms/sms_mms_info", MessageUriType::SMS_MMS},
        {"/sms_mms/sms_mms_info/thirty", MessageUriType::THIRTY},
        {"/sms_mms/sms_mms_info/max_group", MessageUriType::MAX_GROUP},
        {"/sms_mms/sms_mms_info/unread_total", MessageUriType::UNREAD_TOTAL},
        {"/sms_mms/mms_protocol", MessageUriType::MMS_PROTOCOL},
        {"/sms_mms/sms_subsection", MessageUriType::SMS_SUBSECTION},
        {"/sms_mms/mms_part", MessageUriType::MMS_PART}
    };
}

std::string SmsMmsAbility::GetType(const Uri &uri)
{
    DATA_STORAGE_LOGI("SmsMmsAbility::GetType##uri = %{public}s\n", uri.ToString().c_str());
    std::string retval(uri.ToString());
    return retval;
}

int SmsMmsAbility::OpenFile(const Uri &uri, const std::string &mode)
{
    DATA_STORAGE_LOGI("SmsMmsAbility::OpenFile##uri = %{public}s\n", uri.ToString().c_str());
    Uri tempUri = uri;
    MessageUriType messageUriType = ParseUriType(tempUri);
    return static_cast<int>(messageUriType);
}

int SmsMmsAbility::BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
{
    if (!TelephonyPermission::CheckPermission(Permission::READ_MESSAGES)) {
        DATA_STORAGE_LOGE("Permission denied!");
        return DATA_STORAGE_ERR_PERMISSION_ERR;
    }
    int result = DATA_STORAGE_ERROR;
    if (!IsInitOk()) {
        return result;
    }
    std::lock_guard<std::mutex> guard(lock_);
    Uri tempUri = uri;
    MessageUriType messageUriType = ParseUriType(tempUri);
    int64_t id = DATA_STORAGE_ERROR;
    if (messageUriType == MessageUriType::SMS_MMS) {
        result = helper_.BatchInsertSmsMmsInfo(id, values);
    } else {
        DATA_STORAGE_LOGI("SmsMmsAbility::BatchInsert##uri = %{public}s\n", uri.ToString().c_str());
    }
    return result;
}

MessageUriType SmsMmsAbility::ParseUriType(Uri &uri)
{
    DATA_STORAGE_LOGI("SmsMmsAbility::ParseUriType start\n");
    MessageUriType messageUriType = MessageUriType::UNKNOW;
    std::string uriPath = uri.ToString();
    if (!uriPath.empty()) {
        helper_.ReplaceAllStr(uriPath, ":///", "://");
        Uri tempUri(uriPath);
        std::string path = tempUri.GetPath();
        if (!path.empty()) {
            DATA_STORAGE_LOGI("SmsMmsAbility::ParseUriType##path = %{public}s\n", path.c_str());
            std::map<std::string, MessageUriType>::iterator it = smsMmsUriMap.find(path);
            if (it != smsMmsUriMap.end()) {
                messageUriType = it->second;
                DATA_STORAGE_LOGI("SmsMmsAbility::ParseUriType##messageUriType = %{public}d\n", messageUriType);
            }
        }
    }
    return messageUriType;
}

void SmsMmsAbility::ConvertPredicates(
    const NativeRdb::DataAbilityPredicates &dataPredicates, NativeRdb::AbsRdbPredicates *absRdbPredicates)
{
    NativeRdb::PredicatesUtils::SetWhereClauseAndArgs(
        absRdbPredicates, dataPredicates.GetWhereClause(), dataPredicates.GetWhereArgs());
    NativeRdb::PredicatesUtils::SetAttributes(absRdbPredicates, dataPredicates.IsDistinct(),
        dataPredicates.GetIndex(), dataPredicates.GetGroup(), dataPredicates.GetOrder(), dataPredicates.GetLimit(),
        dataPredicates.GetOffset());
}

REGISTER_AA(SmsMmsAbility);
} // namespace Telephony
} // namespace OHOS
