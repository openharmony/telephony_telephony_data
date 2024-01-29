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

#include "sms_mms_ability.h"

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
#include "sms_mms_data.h"
#include "telephony_datashare_stub_impl.h"
#include "uri.h"
#include "utility"

namespace OHOS {
using AppExecFwk::Ability;
using AppExecFwk::AbilityLoader;
namespace Telephony {
const int32_t CHANGED_ROWS = 0;
static const std::map<std::string, MessageUriType> smsMmsUriMap_ = {
    { "/sms_mms/sms_mms_info", MessageUriType::SMS_MMS },
    { "/sms_mms/sms_mms_info/thirty", MessageUriType::THIRTY },
    { "/sms_mms/sms_mms_info/max_group", MessageUriType::MAX_GROUP },
    { "/sms_mms/sms_mms_info/unread_total", MessageUriType::UNREAD_TOTAL },
    { "/sms_mms/mms_protocol", MessageUriType::MMS_PROTOCOL },
    { "/sms_mms/sms_subsection", MessageUriType::SMS_SUBSECTION },
    { "/sms_mms/mms_part", MessageUriType::MMS_PART },
    { "/sms_mms/session", MessageUriType::SESSION },
    { "/sms_mms/mms_pdu", MessageUriType::MMS_PDU },
};

SmsMmsAbility::SmsMmsAbility() : DataShareExtAbility() {}

SmsMmsAbility::~SmsMmsAbility() {}

SmsMmsAbility* SmsMmsAbility::Create()
{
    DATA_STORAGE_LOGI("SmsMmsAbility::Create begin.");
    auto self =  new SmsMmsAbility();
    self->DoInit();
    return self;
}

void SmsMmsAbility::DoInit()
{
    if (initDatabaseDir && initRdbStore) {
        DATA_STORAGE_LOGI("DoInit has done");
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

sptr<IRemoteObject> SmsMmsAbility::OnConnect(const AAFwk::Want &want)
{
    DATA_STORAGE_LOGI("SmsMmsAbility::OnConnect");
    Extension::OnConnect(want);
    sptr<DataShare::TelephonyDataShareStubImpl> remoteObject =
        new (std::nothrow) DataShare::TelephonyDataShareStubImpl();
    if (remoteObject == nullptr) {
        DATA_STORAGE_LOGE("%{public}s No memory allocated for DataShareStubImpl", __func__);
        return nullptr;
    }
    remoteObject->SetSmsMmsAbility(std::static_pointer_cast<SmsMmsAbility>(shared_from_this()));
    DATA_STORAGE_LOGI("SmsMmsAbility %{public}s end.", __func__);
    return remoteObject->AsObject();
}

void SmsMmsAbility::OnStart(const AppExecFwk::Want &want)
{
    DATA_STORAGE_LOGI("SmsMmsAbility::OnStart");
    Extension::OnStart(want);
    DoInit();
}

int SmsMmsAbility::Insert(const Uri &uri, const DataShare::DataShareValuesBucket &value)
{
    if (!PermissionUtil::CheckPermission(Permission::READ_MESSAGES)) {
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
    OHOS::NativeRdb::ValuesBucket values = RdbDataShareAdapter::RdbUtils::ToValuesBucket(value);
    switch (messageUriType) {
        case MessageUriType::SMS_MMS: {
            helper_.Insert(id, values, TABLE_SMS_MMS_INFO);
            break;
        }
        case MessageUriType::MMS_PROTOCOL: {
            helper_.Insert(id, values, TABLE_MMS_PROTOCOL);
            break;
        }
        case MessageUriType::SMS_SUBSECTION: {
            helper_.Insert(id, values, TABLE_SMS_SUBSECTION);
            break;
        }
        case MessageUriType::MMS_PART: {
            helper_.Insert(id, values, TABLE_MMS_PART);
            break;
        }
        case MessageUriType::SESSION: {
            helper_.Insert(id, values, TABLE_SESSION);
            break;
        }
        case MessageUriType::MMS_PDU: {
            helper_.Insert(id, values, TABLE_MMS_PDU);
            break;
        }
        default:
            DATA_STORAGE_LOGI("SmsMmsAbility::Insert##uri = %{public}s", uri.ToString().c_str());
            break;
    }
    return id;
}

std::shared_ptr<DataShare::DataShareResultSet> SmsMmsAbility::Query(
    const Uri &uri, const DataShare::DataSharePredicates &predicates, std::vector<std::string> &columns,
    DataShare::DatashareBusinessError &businessError)
{
    if (!PermissionUtil::CheckPermission(Permission::READ_MESSAGES)) {
        DATA_STORAGE_LOGE("Permission denied!");
        return nullptr;
    }
    if (!IsInitOk()) {
        return nullptr;
    }
    Uri tempUri = uri;
    MessageUriType messageUriType = ParseUriType(tempUri);
    if (messageUriType == MessageUriType::MAX_GROUP || messageUriType == MessageUriType::UNREAD_TOTAL) {
        return GetResultSet(messageUriType, uri);
    }
    NativeRdb::AbsRdbPredicates *absRdbPredicates = GetPredicates(messageUriType, uri);
    std::shared_ptr<DataShare::DataShareResultSet> sharedPtrResult = nullptr;
    if (absRdbPredicates != nullptr) {
        NativeRdb::RdbPredicates rdbPredicates = ConvertPredicates(absRdbPredicates->GetTableName(), predicates);
        auto resultSet = helper_.Query(rdbPredicates, columns);
        if (resultSet == nullptr) {
            DATA_STORAGE_LOGE("SmsMmsAbility::Query  NativeRdb::ResultSet is null!");
            delete absRdbPredicates;
            absRdbPredicates = nullptr;
            return nullptr;
        }
        auto queryResultSet = RdbDataShareAdapter::RdbUtils::ToResultSetBridge(resultSet);
        sharedPtrResult = std::make_shared<DataShare::DataShareResultSet>(queryResultSet);
        delete absRdbPredicates;
        absRdbPredicates = nullptr;
    } else {
        DATA_STORAGE_LOGE("SmsMmsAbility::Query  NativeRdb::AbsRdbPredicates is null!");
    }
    return sharedPtrResult;
}

int SmsMmsAbility::Update(
    const Uri &uri, const DataShare::DataSharePredicates &predicates,
    const DataShare::DataShareValuesBucket &value)
{
    if (!PermissionUtil::CheckPermission(Permission::READ_MESSAGES)) {
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
    NativeRdb::AbsRdbPredicates *absRdbPredicates = GetPredicates(messageUriType, uri);
    if (absRdbPredicates != nullptr) {
        int changedRows = CHANGED_ROWS;
        NativeRdb::RdbPredicates rdbPredicates = ConvertPredicates(absRdbPredicates->GetTableName(), predicates);
        OHOS::NativeRdb::ValuesBucket values = RdbDataShareAdapter::RdbUtils::ToValuesBucket(value);
        result = helper_.Update(changedRows, values, rdbPredicates);
        delete absRdbPredicates;
        absRdbPredicates = nullptr;
    } else {
        DATA_STORAGE_LOGE("SmsMmsAbility::Update  NativeRdb::AbsRdbPredicates is null!");
    }
    return result;
}

NativeRdb::AbsRdbPredicates *SmsMmsAbility::GetPredicates(MessageUriType messageUriType, const Uri &uri)
{
    NativeRdb::AbsRdbPredicates *absRdbPredicates = nullptr;
    switch (messageUriType) {
        case MessageUriType::SMS_MMS: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_SMS_MMS_INFO);
            return absRdbPredicates;
        }
        case MessageUriType::MMS_PROTOCOL: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_MMS_PROTOCOL);
            return absRdbPredicates;
        }
        case MessageUriType::SMS_SUBSECTION: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_SMS_SUBSECTION);
            return absRdbPredicates;
        }
        case MessageUriType::MMS_PART: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_MMS_PART);
            return absRdbPredicates;
        }
        case MessageUriType::SESSION: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_SESSION);
            return absRdbPredicates;
        }
        case MessageUriType::MMS_PDU: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_MMS_PDU);
            return absRdbPredicates;
        }
        default:
            DATA_STORAGE_LOGD("GetPredicates##uri = %{private}s", uri.ToString().c_str());
            return absRdbPredicates;
    }
}

std::shared_ptr<DataShare::DataShareResultSet> SmsMmsAbility::GetResultSet(
    MessageUriType messageUriType, const Uri &uri)
{
    std::shared_ptr<DataShare::DataShareResultSet> sharedPtrResult = nullptr;
    std::shared_ptr<NativeRdb::ResultSet> resultSet = nullptr;
    switch (messageUriType) {
        case MessageUriType::MAX_GROUP: {
            resultSet = helper_.QueryMaxGroupId();
            if (resultSet == nullptr) {
                DATA_STORAGE_LOGE("ResultSet is null!");
                return nullptr;
            }
            auto queryResultSet = RdbDataShareAdapter::RdbUtils::ToResultSetBridge(resultSet);
            sharedPtrResult = std::make_shared<DataShare::DataShareResultSet>(queryResultSet);
            break;
        }
        case MessageUriType::UNREAD_TOTAL: {
            resultSet = helper_.StatisticsUnRead();
            if (resultSet == nullptr) {
                DATA_STORAGE_LOGE("ResultSet is null!");
                return nullptr;
            }
            auto queryResultSet = RdbDataShareAdapter::RdbUtils::ToResultSetBridge(resultSet);
            sharedPtrResult = std::make_shared<DataShare::DataShareResultSet>(queryResultSet);
            break;
        }
        default:
            DATA_STORAGE_LOGI("uri = %{public}s", uri.ToString().c_str());
            break;
    }
    return sharedPtrResult;
}

int SmsMmsAbility::Delete(const Uri &uri, const DataShare::DataSharePredicates &predicates)
{
    if (!PermissionUtil::CheckPermission(Permission::READ_MESSAGES)) {
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
    NativeRdb::AbsRdbPredicates *absRdbPredicates = CreateAbsRdbPredicates(messageUriType, result, tempUri);
    if (absRdbPredicates != nullptr) {
        NativeRdb::RdbPredicates rdbPredicates = ConvertPredicates(absRdbPredicates->GetTableName(), predicates);
        int deletedRows = CHANGED_ROWS;
        result = helper_.Delete(deletedRows, rdbPredicates);
        delete absRdbPredicates;
        absRdbPredicates = nullptr;
    } else if (result == DATA_STORAGE_ERROR) {
        DATA_STORAGE_LOGE("SmsMmsAbility::Delete NativeRdb::AbsRdbPredicates is null!");
    }
    return result;
}

NativeRdb::AbsRdbPredicates *SmsMmsAbility::CreateAbsRdbPredicates(MessageUriType messageUriType, int &result, Uri uri)
{
    NativeRdb::AbsRdbPredicates *absRdbPredicates = nullptr;
    switch (messageUriType) {
        case MessageUriType::SMS_MMS: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_SMS_MMS_INFO);
            return absRdbPredicates;
        }
        case MessageUriType::THIRTY: {
            result = helper_.DeleteDataByThirty();
            if (result != NativeRdb::E_OK) {
                DATA_STORAGE_LOGE("SmsMmsAbility::Delete  DeleteDataByThirty fail!");
                result = static_cast<int>(LoadProFileErrorType::DELETE_THIRTY_DATA_FAIL);
            }
            return absRdbPredicates;
        }
        case MessageUriType::MMS_PROTOCOL: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_MMS_PROTOCOL);
            return absRdbPredicates;
        }
        case MessageUriType::SMS_SUBSECTION: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_SMS_SUBSECTION);
            return absRdbPredicates;
        }
        case MessageUriType::MMS_PART: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_MMS_PART);
            return absRdbPredicates;
        }
        case MessageUriType::SESSION: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_SESSION);
            return absRdbPredicates;
        }
        case MessageUriType::MMS_PDU: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_MMS_PDU);
            return absRdbPredicates;
        }
        default:
            DATA_STORAGE_LOGI("SmsMmsAbility::Delete##uri = %{public}s", uri.ToString().c_str());
            return absRdbPredicates;
    }
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

std::string SmsMmsAbility::GetType(const Uri &uri)
{
    DATA_STORAGE_LOGI("SmsMmsAbility::GetType##uri = %{public}s", uri.ToString().c_str());
    std::string retval(uri.ToString());
    return retval;
}

int SmsMmsAbility::OpenFile(const Uri &uri, const std::string &mode)
{
    DATA_STORAGE_LOGI("SmsMmsAbility::OpenFile##uri = %{public}s", uri.ToString().c_str());
    Uri tempUri = uri;
    MessageUriType messageUriType = ParseUriType(tempUri);
    return static_cast<int>(messageUriType);
}

int SmsMmsAbility::BatchInsert(const Uri &uri, const std::vector<DataShare::DataShareValuesBucket> &values)
{
    if (!PermissionUtil::CheckPermission(Permission::READ_MESSAGES)) {
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
        DATA_STORAGE_LOGI("SmsMmsAbility::BatchInsert##uri = %{public}s", uri.ToString().c_str());
    }
    return result;
}

MessageUriType SmsMmsAbility::ParseUriType(Uri &uri)
{
    DATA_STORAGE_LOGD("SmsMmsAbility::ParseUriType start");
    MessageUriType messageUriType = MessageUriType::UNKNOW;
    std::string uriPath = uri.ToString();
    if (!uriPath.empty()) {
        helper_.ReplaceAllStr(uriPath, ":///", "://");
        Uri tempUri(uriPath);
        std::string path = tempUri.GetPath();
        if (!path.empty() && !smsMmsUriMap_.empty()) {
            DATA_STORAGE_LOGD("SmsMmsAbility::ParseUriType##path = %{private}s", path.c_str());
            auto it = smsMmsUriMap_.find(path);
            if (it != smsMmsUriMap_.end()) {
                messageUriType = it->second;
                DATA_STORAGE_LOGD("SmsMmsAbility::ParseUriType##messageUriType = %{private}d", messageUriType);
            }
        }
    }
    return messageUriType;
}

OHOS::NativeRdb::RdbPredicates SmsMmsAbility::ConvertPredicates(
    const std::string &tableName, const DataShare::DataSharePredicates &predicates)
{
    OHOS::NativeRdb::RdbPredicates res = RdbDataShareAdapter::RdbUtils::ToPredicates(predicates, tableName);
    return res;
}
} // namespace Telephony
} // namespace OHOS
