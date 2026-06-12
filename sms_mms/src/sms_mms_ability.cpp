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

#include <filesystem>
#include <fstream>
#include "ability_context.h"
#include "ability_loader.h"
#include "abs_rdb_predicates.h"
#include "abs_shared_result_set.h"
#include "data_storage_errors.h"
#include "data_storage_log_wrapper.h"
#include "datashare_ext_ability.h"
#include "datashare_predicates.h"
#include "datashare_predicates_def.h"
#include "hitrace_meter.h"
#include "permission_util.h"
#include "rdb_errno.h"
#include "rdb_utils.h"
#include "sms_mms_data.h"
#include "telephony_datashare_stub_impl.h"
#include "uri.h"
#include "rdb_sql_utils.h"
#include "hi_audit.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "screenlock_manager.h"
#include <unordered_map>
#include <memory>
#include <sys/time.h>

namespace OHOS {
using AppExecFwk::Ability;
using AppExecFwk::AbilityLoader;
namespace Telephony {
const int32_t CHANGED_ROWS = 0;
std::string SmsMmsAbility::bundleName_;
constexpr int32_t RETRY_TIMES = 2;
constexpr int32_t RETRY_INTERVAL = 500;
constexpr int RETRY_ERROR_CODE_COUNT = 6;
const int RETRY_ERROR_CODE_LIST[RETRY_ERROR_CODE_COUNT] = {14800024, 14800025, 14800026, 14800028, 14800029, 14800047};
static const std::map<std::string, MessageUriType> smsMmsUriMap_ = {
    { "/sms_mms/sms_mms_info", MessageUriType::SMS_MMS },
    { "/sms_mms/sms_mms_info/thirty", MessageUriType::THIRTY },
    { "/sms_mms/sms_mms_info/max_group", MessageUriType::MAX_GROUP },
    { "/sms_mms/sms_mms_info/unread_total", MessageUriType::UNREAD_TOTAL },
    { "/sms_mms/sms_mms_info/select_info_count", MessageUriType::SELECT_INFO_COUNT },
    { "/sms_mms/sms_mms_info/select_contact_id", MessageUriType::SELECT_CONTACT_ID },
    { "/sms_mms/mms_info", MessageUriType::MMS },
    { "/sms_mms/mms_protocol", MessageUriType::MMS_PROTOCOL },
    { "/sms_mms/sms_subsection", MessageUriType::SMS_SUBSECTION },
    { "/sms_mms/rcs_info", MessageUriType::RCS },
    { "/sms_mms/mms_part", MessageUriType::MMS_PART },
    { "/sms_mms/session", MessageUriType::SESSION },
    { "/sms_mms/mms_pdu", MessageUriType::MMS_PDU },
    { "/sms_mms/risk_url_record", MessageUriType::RISK_URL_RECORD },
    { "/sms_mms/clean_invalid_session", MessageUriType::CLEAN_INVALID_SESSION },
    { "/sms_mms/delete_session", MessageUriType::DELETE_SESSION },
    { "/sms_mms/query_mms_paths_with_sessionIds", MessageUriType::QUERY_MMS_PATHS_WITH_SESSIONIDS },
    { "/sms_mms/mms_info_customized", MessageUriType::QUERY_MMS_INFO_VIEW_CUSTOMIZED},
    { "/sms_mms/mms_pdu_lt", MessageUriType::MMS_PDU_LT },
    { "/sms_mms/chatbots", MessageUriType::CHATBOTS },
    { "/sms_mms/specific_chatbots", MessageUriType::SPECIFICCHATBOTS },
    { "/sms_mms/favorate_info", MessageUriType::SMS_MMS_FAVORITE },
    { "/sms_mms/smc", MessageUriType::SMC },
    { "/sms_mms/conversion_session", MessageUriType::CONVERSION_SESSION },
    { "/sms_mms/conversion_no_notify_session", MessageUriType::CONVERSION_NO_NOTIFY_SESSION },
    { "/sms_mms/matched_recipient", MessageUriType::MATCHED_RECIPIENT },
    { "/sms_mms/batch_mark_unread", MessageUriType::BATCH_MARK_READ },
    { "/sms_mms/single_mark_unread", MessageUriType::SINGLE_MARK_READ },
    { "/sms_mms/msg_search_index", MessageUriType::MAG_SEARCH_INDEX },
    { "/sms_mms/invalid_session", MessageUriType::INVALID_SESSION },
    { "/sms_mms/invalid_info", MessageUriType::INVALID_SMS_INFO },
    { "/sms_mms/invalid_rcs_info", MessageUriType::INVALID_RCS_INFO },
    { "/sms_mms/invalid_rcs_info_with_msg", MessageUriType::INVALID_RCS_INFO_WITH_MSG },
    { "/sms_mms/invalid_part_info", MessageUriType::INVALID_PART_INFO },
};

int64_t SmsMmsAbility::GetSystemTime()
{
    struct timeval curTime;
    curTime.tv_sec = 0;
    curTime.tv_usec = 0;
    gettimeofday(&curTime, NULL);
    int64_t timestamp = static_cast<int64_t>(
        curTime.tv_sec * 1000 + curTime.tv_usec / 1000);
    return timestamp;
}

void SmsMmsAbility::CheckAndReportTimeoutError(RDBOperation operation, const int64_t &startMs, const Uri &uri)
{
    int64_t duration = GetSystemTime() - startMs;
    if (duration > dfxAdapter_.GetExeTimeout()) {
        dfxAdapter_.ReportDataBaseError(static_cast<int>(operation), uri.ToString(), duration);
    }
}

void SmsMmsAbility::InitializeQueryHandlers() {
    queryHandlers_ = {
        {MessageUriType::QUERY_MMS_INFO_VIEW_CUSTOMIZED, [this](const DataShare::DataSharePredicates& p) {
            return GetQueryMmsInfoResultSet(p);
        }},
        {MessageUriType::MAG_SEARCH_INDEX, [this](const DataShare::DataSharePredicates& p) {
            return GetMsgCoordinates(p);
        }},
        {MessageUriType::CONVERSION_SESSION, [this](const DataShare::DataSharePredicates& p) {
            return GetQueryConversionListResult(p, true);
        }},
        {MessageUriType::CONVERSION_NO_NOTIFY_SESSION, [this](const DataShare::DataSharePredicates& p) {
            return GetQueryConversionListResult(p, false);
        }},
        {MessageUriType::MATCHED_RECIPIENT, [this](const DataShare::DataSharePredicates& p) {
            return GetQueryMatchedRecipientResult(p);
        }},
        {MessageUriType::INVALID_SESSION, [this](const DataShare::DataSharePredicates& p) {
            return GetQueryInvalidData(MessageUriType::INVALID_SESSION);
        }},
        {MessageUriType::INVALID_SMS_INFO, [this](const DataShare::DataSharePredicates& p) {
            return GetQueryInvalidData(MessageUriType::INVALID_SMS_INFO);
        }},
        {MessageUriType::INVALID_RCS_INFO, [this](const DataShare::DataSharePredicates& p) {
            return GetQueryInvalidData(MessageUriType::INVALID_RCS_INFO);
        }},
        {MessageUriType::INVALID_RCS_INFO_WITH_MSG, [this](const DataShare::DataSharePredicates& p) {
            return GetQueryInvalidData(MessageUriType::INVALID_RCS_INFO_WITH_MSG);
        }},
        {MessageUriType::INVALID_PART_INFO, [this](const DataShare::DataSharePredicates& p) {
            return GetQueryInvalidData(MessageUriType::INVALID_PART_INFO);
        }},
        {MessageUriType::QUERY_MMS_PATHS_WITH_SESSIONIDS, [this](const DataShare::DataSharePredicates& p) {
            DATA_STORAGE_LOGI("query mms paths with sessionIds");
            return QueryMmsPathsWithSessionIdsResult(p);
        }},
        {MessageUriType::MAX_GROUP, [this](const DataShare::DataSharePredicates& p) {
            return GetResultSet(MessageUriType::MAX_GROUP);
        }},
        {MessageUriType::UNREAD_TOTAL, [this](const DataShare::DataSharePredicates& p) {
            return GetResultSet(MessageUriType::UNREAD_TOTAL);
        }},
        {MessageUriType::SELECT_INFO_COUNT, [this](const DataShare::DataSharePredicates& p) {
            return GetResultSet(MessageUriType::SELECT_INFO_COUNT);
        }},
        {MessageUriType::SELECT_CONTACT_ID, [this](const DataShare::DataSharePredicates& p) {
            return GetResultSet(MessageUriType::SELECT_CONTACT_ID);
        }}
    };
}

SmsMmsAbility::SmsMmsAbility() : DataShareExtAbility()
{
    InitializeQueryHandlers();
    DATA_STORAGE_LOGI("new SmsMmsAbility");
}

SmsMmsAbility::~SmsMmsAbility()
{
    DATA_STORAGE_LOGI("~SmsMmsAbility");
}

SmsMmsAbility* SmsMmsAbility::Create()
{
    auto self =  new SmsMmsAbility();
    self->DoInit();
    DATA_STORAGE_LOGI("SmsMmsAbility::Create begin.");
    return self;
}

void SmsMmsAbility::DoInit()
{
    DATA_STORAGE_LOGE("SmsMmsAbility::DoInit");
    std::lock_guard<std::mutex> lock(initMtx_);
    if (initDatabaseDir && initRdbStore) {
        DATA_STORAGE_LOGI("DoInit has done");
        return;
    }
    std::string el1DbPath = dbPath + "rdb/";
    auto mkdirOk = NativeRdb::RdbSqlUtils::CreateDirectory(el1DbPath);
    if (mkdirOk != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("CreateDirectory %{public}s failed, error %{public}d", el1DbPath.c_str(), mkdirOk);
        return;
    }
    initDatabaseDir = true;

    // 创建el2目录代码逻辑
    std::string el2DbPath = dbPathE + "rdb/";
    mkdirOk = NativeRdb::RdbSqlUtils::CreateDirectory(el2DbPath);
    if (mkdirOk != NativeRdb::E_OK) {
        // 解锁前进程启动，解锁后需要监听二次挂载。否则，EL2有可能没挂载导致访问失败。
        DATA_STORAGE_LOGE("CreateDirectory %{public}s failed, error %{public}d", el2DbPath.c_str(), mkdirOk);
    }

    if (helper_.Init() == NativeRdb::E_OK) {
        initRdbStore = true;
    } else {
        DATA_STORAGE_LOGE("DoInit rdb init failed!");
        initRdbStore = false;
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
    int64_t startMs = GetSystemTime();
    if (!PermissionUtil::CheckPermission(Permission::READ_MESSAGES)) {
        DATA_STORAGE_LOGE("Permission denied!");
        return DATA_STORAGE_ERR_PERMISSION_ERR;
    }
    Uri tempUri = uri;
    MessageUriType messageUriType = ParseUriType(tempUri);
    if (!IsInitOk()) {
        if (!(messageUriType == MessageUriType::SMS_SUBSECTION || messageUriType == MessageUriType::MMS_PDU_LT)) {
            DATA_STORAGE_LOGE("SmsMmsDB IsInitOk failed!");
            return DATA_STORAGE_ERROR;
        }
    }
    std::lock_guard<std::mutex> guard(lock_);
    std::string opt = "INSERT ";
    opt.append(tempUri.ToString().c_str());
    OHOS::HiAudit::GetInstance().Write(MakeAuditLog(opt));
    int64_t id = DATA_STORAGE_ERROR;
    OHOS::NativeRdb::ValuesBucket values = RdbDataShareAdapter::RdbUtils::ToValuesBucket(value);
    switch (messageUriType) {
        case MessageUriType::SMS_MMS: {
            InsertRetry(&id, values, TABLE_SMS_MMS_INFO);
            break;
        }
        case MessageUriType::RCS: {
            InsertRetry(&id, values, TABLE_RCS_INFO);
            break;
        }
        case MessageUriType::MMS_PROTOCOL: {
            InsertRetry(&id, values, TABLE_MMS_PROTOCOL);
            break;
        }
        case MessageUriType::SMS_SUBSECTION: {
            helper_.InsertLT(id, values, TABLE_SMS_SUBSECTION);
            break;
        }
        case MessageUriType::MMS_PART: {
            InsertRetry(&id, values, TABLE_MMS_PART);
            break;
        }
        case MessageUriType::SESSION: {
            InsertRetry(&id, values, TABLE_SESSION);
            break;
        }
        case MessageUriType::MMS_PDU: {
            InsertRetry(&id, values, TABLE_MMS_PDU);
            break;
        }
        case MessageUriType::RISK_URL_RECORD: {
            InsertRetry(&id, values, TABLE_RISK_URL_RECORD);
            break;
        }
        case MessageUriType::MMS_PDU_LT: {
            DATA_STORAGE_LOGI("Insert  MMS_PDU_LT");
            helper_.InsertLT(id, values, TABLE_MMS_PDU);
            break;
        }
        case MessageUriType::SMC: {
            helper_.Insert(id, values, TABLE_SMC_INFO);
            break;
        }
        default:
            id = InsertEx(messageUriType, values);
            CheckAndReportTimeoutError(RDBOperation::INSERT, startMs, uri);
            return id;
    }
    CheckAndReportTimeoutError(RDBOperation::INSERT, startMs, uri);
    return id;
}

int SmsMmsAbility::InsertEx(MessageUriType type, const NativeRdb::ValuesBucket &values)
{
    int64_t id = DATA_STORAGE_ERROR;
    switch (type) {
        case MessageUriType::SMS_MMS_FAVORITE: {
            InsertRetry(&id, values, TABLE_FAVORITE_INFO);
            break;
        }
        case MessageUriType::CHATBOTS: {
            InsertRetry(&id, values, TABLE_CHAT_BOTS_INFO);
            break;
        }
        case MessageUriType::SPECIFICCHATBOTS: {
            InsertRetry(&id, values, TABLE_SPECIFIC_CHAT_BOTS_INFO);
            break;
        }
        default:
            DATA_STORAGE_LOGI("SmsMmsAbility::Insert##uri");
            break;
    }
    return id;
}

std::shared_ptr<DataShare::DataShareResultSet> SmsMmsAbility::Query(
    const Uri &uri, const DataShare::DataSharePredicates &predicates, std::vector<std::string> &columns,
    DataShare::DatashareBusinessError &businessError)
{
    std::shared_ptr<DataShare::DataShareResultSet> sharedPtrResult = nullptr;
    int64_t startMs = GetSystemTime();
    DATA_STORAGE_LOGI("SmsMmsDB Query Begin | uri: %{public}s  : ", uri.ToString().c_str());
    if (!PermissionUtil::CheckPermission(Permission::READ_MESSAGES)) {
        DATA_STORAGE_LOGE("SmsMmsDB Permission denied!");
        return nullptr;
    }
    Uri tempUri = uri;
    MessageUriType messageUriType = ParseUriType(tempUri);
    if (!IsInitOk()) {
        return nullptr;
    }
    // 使用映射表查找并调用对应的处理函数
    auto it = queryHandlers_.find(messageUriType);
    if (it != queryHandlers_.end()) {
        sharedPtrResult = it->second(predicates);
        CheckAndReportTimeoutError(RDBOperation::QUERY, startMs, uri);
        return sharedPtrResult;
    }

    // 通用逻辑处理
    sharedPtrResult = HandleCommonQuery(messageUriType, uri, predicates, columns);
    CheckAndReportTimeoutError(RDBOperation::QUERY, startMs, uri);
    return sharedPtrResult;
}

std::shared_ptr<DataShare::DataShareResultSet> SmsMmsAbility::HandleCommonQuery(
    MessageUriType messageUriType, const Uri &uri, const DataShare::DataSharePredicates &predicates,
    std::vector<std::string> &columns)
{
    NativeRdb::AbsRdbPredicates *absRdbPredicates = GetPredicates(messageUriType, uri);
    std::shared_ptr<DataShare::DataShareResultSet> sharedPtrResult = nullptr;

    if (absRdbPredicates != nullptr) {
        NativeRdb::RdbPredicates rdbPredicates = ConvertPredicates(absRdbPredicates->GetTableName(), predicates);
        std::shared_ptr<NativeRdb::ResultSet> resultSet;

        if (messageUriType == MessageUriType::MMS_PDU_LT || messageUriType == MessageUriType::SMS_SUBSECTION) {
            DATA_STORAGE_LOGI("SmsMmsDB QueryLT");
            resultSet = helper_.QueryLT(rdbPredicates, columns);
        } else {
            DATA_STORAGE_LOGI("SmsMmsDB Query");
            StartAsyncTrace(HITRACE_TAG_OHOS, "Query", getpid());
            resultSet = helper_.Query(rdbPredicates, columns);
            FinishAsyncTrace(HITRACE_TAG_OHOS, "Query", getpid());
        }

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

    DATA_STORAGE_LOGI("SmsMmsDB Query End | uri: %{public}s  : ", uri.ToString().c_str());
    return sharedPtrResult;
}

int SmsMmsAbility::Update(const Uri &uri, const DataShare::DataSharePredicates &predicates,
    const DataShare::DataShareValuesBucket &value)
{
    int64_t startMs = GetSystemTime();
    int32_t uid = IPCSkeleton::GetCallingUid();
    std::string callingBundleName;
    GetBundleNameByUid(uid, callingBundleName);
    OHOS::Uri tempUri = uri;
    DATA_STORAGE_LOGI("uri = %{public}s, callingBundleName = %{public}s, callingUid = %{public}d",
        tempUri.ToString().c_str(), callingBundleName.c_str(), uid);
    if (!PermissionUtil::CheckPermission(Permission::READ_MESSAGES)) {
        return DATA_STORAGE_ERR_PERMISSION_ERR;
    }
    if (!IsInitOk()) {
        return DATA_STORAGE_ERROR;
    }
    std::string opt = "UPDATE ";
    opt.append(tempUri.ToString().c_str());
    OHOS::HiAudit::GetInstance().Write(MakeAuditLog(opt));
    MessageUriType messageUriType = ParseUriType(tempUri);
    if (messageUriType == MessageUriType::BATCH_MARK_READ) {
        int result = BatchProcessMarkRead(predicates);
        CheckAndReportTimeoutError(RDBOperation::UPDATE, startMs, tempUri);
        return result;
    }
    if (messageUriType == MessageUriType::SINGLE_MARK_READ) {
        int result = UpdateSessionUnreadCount(predicates);
        CheckAndReportTimeoutError(RDBOperation::UPDATE, startMs, tempUri);
        return result;
    }
    return ProcessRdbUpdate(tempUri, predicates, value, startMs);
}

int SmsMmsAbility::ProcessRdbUpdate(const Uri &uri, const DataShare::DataSharePredicates &predicates,
    const DataShare::DataShareValuesBucket &value, int64_t startMs)
{
    std::lock_guard<std::mutex> guard(lock_);
    OHOS::Uri tempUri = uri;
    NativeRdb::AbsRdbPredicates *absRdbPredicates = GetPredicates(ParseUriType(tempUri), tempUri);
    if (!absRdbPredicates) {
        DATA_STORAGE_LOGE("SmsMmsAbility::Update  NativeRdb::AbsRdbPredicates is null!");
        return DATA_STORAGE_ERROR;
    }
    NativeRdb::RdbPredicates rdbPredicates = ConvertPredicates(absRdbPredicates->GetTableName(), predicates);
    OHOS::NativeRdb::ValuesBucket values = RdbDataShareAdapter::RdbUtils::ToValuesBucket(value);
    int changedRows = CHANGED_ROWS;
    int result = DATA_STORAGE_ERROR;
    int retryTimes = 0;
    do {
        if (ParseUriType(tempUri) == MessageUriType::MMS_PDU_LT ||
            ParseUriType(tempUri) == MessageUriType::SMS_SUBSECTION) {
            result = helper_.UpdateLT(changedRows, values, rdbPredicates);
        } else {
            result = helper_.Update(changedRows, values, rdbPredicates);
        }
        if (!IsRetryable(result)) {
            break;
        }
        if (++retryTimes < RETRY_TIMES) {
            std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_INTERVAL));
        }
    } while (retryTimes < RETRY_TIMES);
    delete absRdbPredicates;
    absRdbPredicates = nullptr;
    CheckAndReportTimeoutError(RDBOperation::UPDATE, startMs, tempUri);
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
        case MessageUriType::RCS: {
            DATA_STORAGE_LOGI("MessageUriType RCS");
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_RCS_INFO);
            return absRdbPredicates;
        }
        case MessageUriType::MMS: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_MMS_INFO);
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
        case MessageUriType::MMS_PDU_LT:
        case MessageUriType::MMS_PDU: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_MMS_PDU);
            return absRdbPredicates;
        }
        case MessageUriType::RISK_URL_RECORD: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_RISK_URL_RECORD);
            return absRdbPredicates;
        }
        case MessageUriType::SMS_MMS_FAVORITE: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_FAVORITE_INFO);
            return absRdbPredicates;
        }
        default:
            return GetPredicatesEx(messageUriType);
    }
}

NativeRdb::AbsRdbPredicates *SmsMmsAbility::GetPredicatesEx(MessageUriType messageUriType)
{
    NativeRdb::AbsRdbPredicates *absRdbPredicates = nullptr;
    switch (messageUriType) {
        case MessageUriType::CHATBOTS: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_CHAT_BOTS_INFO);
            return absRdbPredicates;
        }
        case MessageUriType::SPECIFICCHATBOTS: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_SPECIFIC_CHAT_BOTS_INFO);
            return absRdbPredicates;
        }
        case MessageUriType::SMC: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_SMC_INFO);
            return absRdbPredicates;
        }
        default:
            DATA_STORAGE_LOGD("GetPredicates##uri");
            return absRdbPredicates;
    }
}

std::shared_ptr<DataShare::DataShareResultSet> SmsMmsAbility::GetMsgCoordinates(
    const DataShare::DataSharePredicates &predicates)
{
    int32_t threadId = 0;
    int32_t msgId = 0;
    int32_t sessionType = 0;
    DATA_STORAGE_LOGI("GetMsgCoordinates");
    if (predicates.GetOperationList().size() <= 0) {
        DATA_STORAGE_LOGE("GetMsgCoordinates predicates.GetOperationList() is null");
        return nullptr;
    }
    for (uint32_t i = 0; i < predicates.GetOperationList().size(); i++) {
        DataShare::OperationItem operationItem = predicates.GetOperationList()[i];
        if (operationItem.operation == DataShare::OperationType::INVALID_OPERATION) {
            DATA_STORAGE_LOGI("operation is INVALID_OPERATION continue");
            continue;
        } else if (operationItem.operation == DataShare::OperationType::EQUAL_TO &&
            operationItem.singleParams.size() == 3L) {
            std::string equal = std::get<std::string>(operationItem.singleParams[0]);
            if (equal == "threadId") {
                threadId = int(std::get<double>(operationItem.singleParams[1]));
            } else if (equal == "msgId") {
                msgId = int(std::get<double>(operationItem.singleParams[1]));
            } else if (equal == "sessionType") {
                sessionType = int(std::get<double>(operationItem.singleParams[1]));
            } else {
                DATA_STORAGE_LOGE("GetMsgCoordinates unprocessed condition %{public}s", equal.c_str());
            }
        } else {
            DATA_STORAGE_LOGE("GetMsgCoordinates singleParams is Unknown");
        }
    }
    DATA_STORAGE_LOGI("threadId %{public}d , msgId %{public}d, sessionType %{public}d", threadId, msgId, sessionType);
    if (msgId > 0 && threadId > 0) {
        std::shared_ptr<NativeRdb::ResultSet> resultSet;
        resultSet = helper_.GetMsgCoordinatesWithSessionId(threadId, msgId, sessionType);
        std::shared_ptr<DataShare::DataShareResultSet> sharedPtrResult = nullptr;
        if (resultSet == nullptr) {
            DATA_STORAGE_LOGE("SmsMmsAbility::Query  NativeRdb::ResultSet is null!");
            return nullptr;
        }
        auto queryResultSet = RdbDataShareAdapter::RdbUtils::ToResultSetBridge(resultSet);
        sharedPtrResult = std::make_shared<DataShare::DataShareResultSet>(queryResultSet);
        return sharedPtrResult;
    }
    return nullptr;
}

std::shared_ptr<DataShare::DataShareResultSet> SmsMmsAbility::GetQueryMmsInfoResultSet(
    const DataShare::DataSharePredicates &predicates)
{
        std::string infoWhereSql = "";
        std::string infoLimitSql = "";
        if (predicates.GetOperationList().size() <= 0) {
            DATA_STORAGE_LOGE("SmsMmsDB GetQueryMmsInfoResult predicates.GetOperationList() is null");
            return nullptr;
        }
        for (uint32_t i = 0; i < predicates.GetOperationList().size(); i++) {
            DataShare::OperationItem operationItem = predicates.GetOperationList()[i];
            if (operationItem.operation == DataShare::OperationType::INVALID_OPERATION) {
                DATA_STORAGE_LOGI("operation is INVALID_OPERATION continue");
                continue;
            } else if (operationItem.operation == DataShare::OperationType::EQUAL_TO &&
                operationItem.singleParams.size() == 3L) {
                if (std::holds_alternative<std::string>(operationItem.singleParams[0]) &&
                    std::holds_alternative<double>(operationItem.singleParams[1])) {
                        infoWhereSql.append(std::get<std::string>(operationItem.singleParams[0]))
                        .append(" = ")
                        .append(std::to_string(int(std::get<double>(operationItem.singleParams[1]))));
                }
            } else if (operationItem.operation == DataShare::OperationType::AND &&
                !infoWhereSql.empty() && i != predicates.GetOperationList().size() -1) {
                infoWhereSql.append(" and ");
            } else if (operationItem.operation == DataShare::OperationType::LIMIT &&
                operationItem.singleParams.size() == 3L) {
                if (std::holds_alternative<int>(operationItem.singleParams[0]) &&
                    std::holds_alternative<int>(operationItem.singleParams[1])) {
                        infoLimitSql.append(" ").append(std::to_string(std::get<int>(operationItem.singleParams[1])))
                        .append(" , ")
                        .append(std::to_string(std::get<int>(operationItem.singleParams[0])));
                }
            } else {
                DATA_STORAGE_LOGE("SmsMmsDB QUERY_MMS_INFO_VIEW_CUSTOMIZED singleParams is Unknown");
            }
        }
        if (infoWhereSql.empty() || infoLimitSql.empty()) {
            DATA_STORAGE_LOGE("SmsMmsDB infoWhereSql or infoLimitSql is null");
            return nullptr;
        }
        std::shared_ptr<DataShare::DataShareResultSet> sharedPtrResult = nullptr;
        std::shared_ptr<NativeRdb::ResultSet> resultSet = nullptr;
        resultSet = helper_.QueryMMSVIEWS(infoWhereSql, infoLimitSql);
        if (resultSet == nullptr) {
            DATA_STORAGE_LOGE("ResultSet is null!");
            return nullptr;
        }
        auto queryResultSet = RdbDataShareAdapter::RdbUtils::ToResultSetBridge(resultSet);
        sharedPtrResult = std::make_shared<DataShare::DataShareResultSet>(queryResultSet);
        return sharedPtrResult;
}

std::shared_ptr<DataShare::DataShareResultSet> SmsMmsAbility::QueryMmsPathsWithSessionIdsResult(
    const DataShare::DataSharePredicates &predicates)
{
    std::shared_ptr<DataShare::DataShareResultSet> sharedPtrResult = nullptr;
    std::shared_ptr<NativeRdb::ResultSet> resultSet = nullptr;
    resultSet = helper_.QueryMmsPathsWithSessionIds(predicates);
    if (resultSet == nullptr) {
        DATA_STORAGE_LOGE("QueryMmsPathsWithSessionIdsResult resultSet is null!");
        return nullptr;
    }
    auto queryResultSet = RdbDataShareAdapter::RdbUtils::ToResultSetBridge(resultSet);
    sharedPtrResult = std::make_shared<DataShare::DataShareResultSet>(queryResultSet);
    return sharedPtrResult;
}

int SmsMmsAbility::BatchProcessMarkRead(const DataShare::DataSharePredicates &predicates)
{
    uint32_t markUnreadType = 0;
    uint32_t sessionId = 0;
    DATA_STORAGE_LOGI("BatchProcessMarkRead");
    if (predicates.GetOperationList().size() <= 0) {
        DATA_STORAGE_LOGE("BatchProcessMarkRead predicates.GetOperationList() is null");
        return NativeRdb::E_ERROR;
    }
    for (uint32_t i = 0; i < predicates.GetOperationList().size(); i++) {
        DataShare::OperationItem operationItem = predicates.GetOperationList()[i];
        if (operationItem.operation == DataShare::OperationType::INVALID_OPERATION) {
            DATA_STORAGE_LOGI("operation is INVALID_OPERATION continue");
            continue;
        } else if (operationItem.operation == DataShare::OperationType::EQUAL_TO &&
            operationItem.singleParams.size() == 3L) {
            if (!std::holds_alternative<std::string>(operationItem.singleParams[0]) ||
                !std::holds_alternative<double>(operationItem.singleParams[1])) {
                continue;
            }
            std::string equal = std::get<std::string>(operationItem.singleParams[0]);
            int equalto = int(std::get<double>(operationItem.singleParams[1]));
            if (equal == "mark_read_way") {
                markUnreadType = static_cast<uint32_t>(equalto);
            } else if (equal == "id") {
                sessionId = static_cast<uint32_t>(equalto);
            } else {
                DATA_STORAGE_LOGE("unprocessed condition %{public}s", equal.c_str());
            }
            DATA_STORAGE_LOGI("equal %{public}s", equal.c_str());
            DATA_STORAGE_LOGI("equalto %{public}d", equalto);
        } else {
            DATA_STORAGE_LOGE("BatchProcessMarkRead singleParams is Unknown");
        }
    }
    if (static_cast<uint32_t>(MarkReadType::SINGLE) == markUnreadType) {
        if (sessionId == 0) {
            DATA_STORAGE_LOGE("BatchProcessMarkRead singleParams error ");
            return NativeRdb::E_ERROR;
        }
        return helper_.MarkedSingleASRead(sessionId);
    } else if (static_cast<uint32_t>(MarkReadType::INFO) == markUnreadType) {
        return helper_.MarkedInfoASRead();
    } else if (static_cast<uint32_t>(MarkReadType::ALL) == markUnreadType) {
        return helper_.MarkedAllASRead();
    } else {
        DATA_STORAGE_LOGE("BatchProcessMarkRead invalid markUnreadType %{public}d ", markUnreadType);
    }
    return NativeRdb::E_ERROR;
}

int SmsMmsAbility::UpdateSessionUnreadCount(const DataShare::DataSharePredicates &predicates)
{
    std::string telephone = "";
    std::string msgType = "";
    std::string startTime = "";
    int32_t msgId = 0;
    DATA_STORAGE_LOGI("UpdateSessionUnreadCount");
    if (predicates.GetOperationList().size() <= 0) {
        DATA_STORAGE_LOGE("UpdateSessionUnreadCount predicates.GetOperationList() is null");
        return NativeRdb::E_ERROR;
    }
    for (uint32_t i = 0; i < predicates.GetOperationList().size(); i++) {
        DataShare::OperationItem operationItem = predicates.GetOperationList()[i];
        if (operationItem.operation == DataShare::OperationType::INVALID_OPERATION) {
            DATA_STORAGE_LOGI("operation is INVALID_OPERATION continue");
            continue;
        } else if (operationItem.operation == DataShare::OperationType::EQUAL_TO &&
            operationItem.singleParams.size() == 3L) {
            std::string equal = std::get<std::string>(operationItem.singleParams[0]);
            if (equal == "phone_number") {
                telephone = std::get<std::string>(operationItem.singleParams[1]);
            } else if (equal == "start_time") {
                startTime = std::get<std::string>(operationItem.singleParams[1]);
            } else if (equal == "msg_id") {
                msgId = int(std::get<double>(operationItem.singleParams[1]));
            } else if (equal == "msg_type") {
                msgType = std::get<std::string>(operationItem.singleParams[1]);
            } else {
                DATA_STORAGE_LOGE("UpdateSessionUnreadCount unprocessed condition %{public}s", equal.c_str());
            }
        } else {
            DATA_STORAGE_LOGE("UpdateSessionUnreadCount singleParams is Unknown");
        }
    }
    if (msgType.empty()) {
        DATA_STORAGE_LOGE("UpdateSessionUnreadCount invalid msg type, do nothing");
        return NativeRdb::E_ERROR;
    }
    if (msgId != 0) {
        return helper_.ProcessSessionUnreadWithMsgId(msgId, msgType);
    } else {
        if (telephone.empty() || startTime.empty()) {
            DATA_STORAGE_LOGE("UpdateSessionUnreadCount incomplete parameters ");
            return NativeRdb::E_ERROR;
        }
        return helper_.ProcessSessionUnreadWithTelephone(telephone, startTime, msgType);
    }
    return NativeRdb::E_ERROR;
}

std::shared_ptr<DataShare::DataShareResultSet> SmsMmsAbility::GetQueryInvalidData(const MessageUriType &messageUriType)
{
    std::shared_ptr<NativeRdb::ResultSet> resultSet;
    if (messageUriType == MessageUriType::INVALID_SESSION) {
        resultSet = helper_.StatisticsQueryInvalidSession();
    } else if (messageUriType == MessageUriType::INVALID_SMS_INFO) {
        resultSet = helper_.StatisticsQueryInvalidSmsInfo();
    } else if (messageUriType == MessageUriType::INVALID_RCS_INFO) {
        resultSet = helper_.StatisticsQueryInvalidRcsInfo();
    } else if (messageUriType == MessageUriType::INVALID_RCS_INFO_WITH_MSG) {
        resultSet = helper_.StatisticsQueryInvalidRcsInfoWithMsg();
    } else if (messageUriType == MessageUriType::INVALID_PART_INFO) {
        resultSet = helper_.StatisticsQueryInvalidPart();
    } else {
        DATA_STORAGE_LOGE("undefined messageUriType ");
        return nullptr;
    }
    auto queryResultSet = RdbDataShareAdapter::RdbUtils::ToResultSetBridge(resultSet);
    std::shared_ptr<DataShare::DataShareResultSet> sharedPtrResult = nullptr;
    sharedPtrResult = std::make_shared<DataShare::DataShareResultSet>(queryResultSet);
    return sharedPtrResult;
}

std::shared_ptr<DataShare::DataShareResultSet> SmsMmsAbility::GetQueryMatchedRecipientResult(
    const DataShare::DataSharePredicates &predicates)
{
    std::string recipientList = "";
    if (predicates.GetOperationList().size() <= 0) {
        DATA_STORAGE_LOGE("GetQueryMatchedRecipientResult predicates.GetOperationList() is null");
        return nullptr;
    }
    for (uint32_t i = 0; i < predicates.GetOperationList().size(); i++) {
        DataShare::OperationItem operationItem = predicates.GetOperationList()[i];
        if (operationItem.operation == DataShare::OperationType::EQUAL_TO &&
            operationItem.singleParams.size() == 3L) {
            if (std::holds_alternative<std::string>(operationItem.singleParams[0]) &&
                std::holds_alternative<std::string>(operationItem.singleParams[1])) {
                recipientList = std::get<std::string>(operationItem.singleParams[1]);
                DATA_STORAGE_LOGE("GetQueryMatchedRecipientResult operation is valid");
            }
        }
    }
    if (recipientList.empty() || recipientList.length() <= 0) {
        DATA_STORAGE_LOGE("GetQueryMatchedRecipientResult operation is invalid");
        return nullptr;
    }
    std::shared_ptr<DataShare::DataShareResultSet> sharedPtrResult = nullptr;
    std::shared_ptr<NativeRdb::ResultSet> resultSet = nullptr;
    resultSet = helper_.QueryMatchedRecipient(recipientList);
    if (resultSet == nullptr) {
        DATA_STORAGE_LOGE("GetQueryMatchedRecipientResult resultSet is null!");
        return nullptr;
    }
    auto queryResultSet = RdbDataShareAdapter::RdbUtils::ToResultSetBridge(resultSet);
    sharedPtrResult = std::make_shared<DataShare::DataShareResultSet>(queryResultSet);
    return sharedPtrResult;
}

std::shared_ptr<DataShare::DataShareResultSet> SmsMmsAbility::GetQueryConversionListResult(
    const DataShare::DataSharePredicates &predicates, bool isQueryNotifiyMsg)
{
    std::string infoSmsTypeSql = "";
    std::string infoLimitSql = "";
    if (predicates.GetOperationList().size() <= 0) {
        DATA_STORAGE_LOGE("GetQueryConversionListResult predicates.GetOperationList() is null");
        return nullptr;
    }
    for (uint32_t i = 0; i < predicates.GetOperationList().size(); i++) {
        DataShare::OperationItem operationItem = predicates.GetOperationList()[i];
        if (operationItem.operation == DataShare::OperationType::LIMIT &&
            operationItem.singleParams.size() == 3L) {
            if (std::holds_alternative<int>(operationItem.singleParams[0]) &&
                std::holds_alternative<int>(operationItem.singleParams[1])) {
                    infoLimitSql.append(" ").append(std::to_string(std::get<int>(operationItem.singleParams[1])))
                    .append(" , ")
                    .append(std::to_string(std::get<int>(operationItem.singleParams[0])));
            }
        } else {
            DATA_STORAGE_LOGE("operation is invalidation");
        }
    }
    if (infoLimitSql.empty()) {
        DATA_STORAGE_LOGE("SmsMmsDB infoLimitSql is null");
        return nullptr;
    }
    std::shared_ptr<DataShare::DataShareResultSet> sharedPtrResult = nullptr;
    std::shared_ptr<NativeRdb::ResultSet> resultSet = nullptr;
    resultSet = helper_.QueryConversionList(infoLimitSql, isQueryNotifiyMsg);
    if (resultSet == nullptr) {
        DATA_STORAGE_LOGE("GetQueryConversionListResultSet resultSet is null!");
        return nullptr;
    }
    auto queryResultSet = RdbDataShareAdapter::RdbUtils::ToResultSetBridge(resultSet);
    sharedPtrResult = std::make_shared<DataShare::DataShareResultSet>(queryResultSet);
    return sharedPtrResult;
}

std::shared_ptr<DataShare::DataShareResultSet> SmsMmsAbility::GetResultSet(
    MessageUriType messageUriType)
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
        case MessageUriType::SELECT_INFO_COUNT: {
            resultSet = helper_.StatisticsSelectInfoCount();
            if (resultSet == nullptr) {
                DATA_STORAGE_LOGE("ResultSet is null!");
                return nullptr;
            }
            auto queryResultSet = RdbDataShareAdapter::RdbUtils::ToResultSetBridge(resultSet);
            sharedPtrResult = std::make_shared<DataShare::DataShareResultSet>(queryResultSet);
            break;
        }
        case MessageUriType::SELECT_CONTACT_ID: {
            DATA_STORAGE_LOGE("ResultSet!");
            resultSet = helper_.StatisticsSelectContactId();
            if (resultSet == nullptr) {
                DATA_STORAGE_LOGE("ResultSet is null!");
                return nullptr;
            }
            auto queryResultSet = RdbDataShareAdapter::RdbUtils::ToResultSetBridge(resultSet);
            sharedPtrResult = std::make_shared<DataShare::DataShareResultSet>(queryResultSet);
            break;
        }
        default:
            DATA_STORAGE_LOGI("GetResultSet uri");
    }
    return sharedPtrResult;
}

int SmsMmsAbility::Delete(const Uri &uri, const DataShare::DataSharePredicates &predicates)
{
    int64_t startMs = GetSystemTime();
    int32_t uid = IPCSkeleton::GetCallingUid();
    int32_t pid = IPCSkeleton::GetCallingPid();
    std::string callingBundleName;
    OHOS::Uri tempUri = uri;
    DATA_STORAGE_LOGI("uri = %{public}s, pid = %{public}d, callingUid = %{public}d",
        tempUri.ToString().c_str(), pid, uid);
    if (!PermissionUtil::CheckPermission(Permission::READ_MESSAGES)) {
        DATA_STORAGE_LOGE("Permission denied!");
        return DATA_STORAGE_ERR_PERMISSION_ERR;
    }
    int result = DATA_STORAGE_ERROR;
    MessageUriType messageUriType = ParseUriType(tempUri);
    if (!IsInitOk()) {
        return result;
    }
    std::lock_guard<std::mutex> guard(lock_);
    std::string opt = "DELETE ";
    opt.append(tempUri.ToString().c_str());
    OHOS::HiAudit::GetInstance().Write(MakeAuditLog(opt));
    if (messageUriType == MessageUriType::CLEAN_INVALID_SESSION) {
        result = helper_.StatisticsCleanInvalidSession();
        CheckAndReportTimeoutError(RDBOperation::DELETE, startMs, uri);
        return result;
    }
    if (messageUriType == MessageUriType::DELETE_SESSION) {
        DATA_STORAGE_LOGI("batch delete");
        result = helper_.StatisticsDeleteSession(predicates);
        CheckAndReportTimeoutError(RDBOperation::DELETE, startMs, uri);
        return result;
    }
    NativeRdb::AbsRdbPredicates *absRdbPredicates = CreateAbsRdbPredicates(messageUriType, result, tempUri);
    if (absRdbPredicates != nullptr) {
        NativeRdb::RdbPredicates rdbPredicates = ConvertPredicates(absRdbPredicates->GetTableName(), predicates);
        int deletedRows = CHANGED_ROWS;
        int retryTimes = 0;
        do {
            if (messageUriType == MessageUriType::MMS_PDU_LT || messageUriType == MessageUriType::SMS_SUBSECTION) {
                result = helper_.DeleteLT(deletedRows, rdbPredicates);
            } else {
                result = helper_.Delete(deletedRows, rdbPredicates);
            }
            if (!IsRetryable(result)) {
                break;
            }
            if (++retryTimes < RETRY_TIMES) {
                std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_INTERVAL));
                DATA_STORAGE_LOGI("SmsMmsAbility::Delete rdb error");
            }
        } while (retryTimes < RETRY_TIMES);
        delete absRdbPredicates;
        absRdbPredicates = nullptr;
    } else if (result == DATA_STORAGE_ERROR) {
        DATA_STORAGE_LOGE("SmsMmsAbility::Delete NativeRdb::AbsRdbPredicates is null!");
    }
    CheckAndReportTimeoutError(RDBOperation::DELETE, startMs, uri);
    return result;
}

OHOS::AuditLog SmsMmsAbility::MakeAuditLog(std::string operationType)
{
    OHOS::AuditLog auditLog;
    auditLog.isUserBehavior = true;
    auditLog.cause = "USER BEHAVIOR";
    auditLog.operationType = operationType;
    auditLog.operationScenario = "io";
    auditLog.operationCount = 1,
    auditLog.operationStatus = "running";
    auditLog.extend = "ok";
    return auditLog;
}

NativeRdb::AbsRdbPredicates *SmsMmsAbility::CreateAbsRdbPredicates(MessageUriType messageUriType, int &result, Uri uri)
{
    NativeRdb::AbsRdbPredicates *absRdbPredicates = nullptr;
    switch (messageUriType) {
        case MessageUriType::SMS_MMS: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_SMS_MMS_INFO);
            return absRdbPredicates;
        }
        case MessageUriType::RCS: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_RCS_INFO);
            return absRdbPredicates;
        }
        case MessageUriType::MMS: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_MMS_INFO);
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
        case MessageUriType::MMS_PDU_LT:
        case MessageUriType::MMS_PDU: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_MMS_PDU);
            return absRdbPredicates;
        }
        case MessageUriType::RISK_URL_RECORD: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_RISK_URL_RECORD);
            return absRdbPredicates;
        }
        default:
            return CreateAbsRdbPredicatesEx(messageUriType);
    }
}

NativeRdb::AbsRdbPredicates *SmsMmsAbility::CreateAbsRdbPredicatesEx(MessageUriType messageUriType)
{
    NativeRdb::AbsRdbPredicates *absRdbPredicates = nullptr;
    switch (messageUriType) {
        case MessageUriType::SMS_MMS_FAVORITE: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_FAVORITE_INFO);
            return absRdbPredicates;
        }
        case MessageUriType::SMC: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_SMC_INFO);
            return absRdbPredicates;
        }
        case MessageUriType::CHATBOTS: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_CHAT_BOTS_INFO);
            return absRdbPredicates;
        }
        case MessageUriType::SPECIFICCHATBOTS: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_SPECIFIC_CHAT_BOTS_INFO);
            return absRdbPredicates;
        }
        default:
            DATA_STORAGE_LOGI("SmsMmsAbility::Delete##uri");
            return absRdbPredicates;
    }
}

bool SmsMmsAbility::IsInitOk()
{
    bool needInit;
    {
        std::lock_guard<std::mutex> lock(initMtx_);
        needInit = !initDatabaseDir || !initRdbStore;
    }
    if (needInit) {
        DATA_STORAGE_LOGW("SmsMmsAbility::IsInitOk failed!");
        DoInit();
        {
            std::lock_guard<std::mutex> lock(initMtx_);
            needInit = !initDatabaseDir || !initRdbStore;
        }
    }
    return !needInit;
}

std::string SmsMmsAbility::GetType(const Uri &uri)
{
    DATA_STORAGE_LOGI("SmsMmsAbility::GetType##uri");
    std::string retval(uri.ToString());
    return retval;
}

int SmsMmsAbility::OpenFile(const Uri &uri, const std::string &mode)
{
    DATA_STORAGE_LOGI("SmsMmsAbility::OpenFile##uri");
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
    Uri tempUri = uri;
    MessageUriType messageUriType = ParseUriType(tempUri);
    if (!IsInitOk()) {
        return result;
    }
    std::lock_guard<std::mutex> guard(lock_);
    int64_t id = DATA_STORAGE_ERROR;
    if (messageUriType == MessageUriType::SMS_MMS) {
        result = helper_.BatchInsertSmsMmsInfo(id, values);
    } else {
        DATA_STORAGE_LOGI("SmsMmsAbility::BatchInsert##uri");
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
            DATA_STORAGE_LOGD("SmsMmsAbility::ParseUriType##path");
            auto it = smsMmsUriMap_.find(path);
            if (it != smsMmsUriMap_.end()) {
                messageUriType = it->second;
                DATA_STORAGE_LOGD("SmsMmsAbility::ParseUriType##messageUriType");
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

bool SmsMmsAbility::GetBundleNameByUid(int32_t uid, std::string &bundleName)
{
    sptr<ISystemAbilityManager> smgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (smgr == nullptr) {
        DATA_STORAGE_LOGE("%{public}s GetBundleNameByUid smgr is nullptr", __func__);
        return false;
    }
    sptr<IRemoteObject> remoteObject = smgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject == nullptr) {
        DATA_STORAGE_LOGE("%{public}s GetBundleNameByUid remoteObject is nullptr", __func__);
        return false;
    }
    sptr<AppExecFwk::IBundleMgr> bundleMgr = iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    if (bundleMgr == nullptr) {
        DATA_STORAGE_LOGE("%{public}s GetBundleNameByUid bundleMgr is nullptr", __func__);
        return false;
    }
    int32_t error = bundleMgr->GetNameForUid(uid, bundleName);
    if (error != ERR_OK) {
        DATA_STORAGE_LOGE("%{public}s GetBundleNameByUid is fail", __func__);
        return false;
    }
    return true;
}


void SmsMmsAbility::ChangeStoreToEl5AndMoveData()
{
    helper_.ChangeStoreToEl5AndMoveData();
}

bool SmsMmsAbility::IsRetryable(int32_t errCode)
{
    if (errCode == NativeRdb::E_OK) {
        return false;
    }
    bool found = false;
    for (int i = 0; i < RETRY_ERROR_CODE_COUNT; i++) {
        if (RETRY_ERROR_CODE_LIST[i] == errCode) {
            found = true;
        }
    }
    if (found) {
        DATA_STORAGE_LOGI("SmsMmsAbility::IsRetryable rdb error");
        return true;
    }
    return false;
}

void SmsMmsAbility::InsertRetry(int64_t *id, const NativeRdb::ValuesBucket &values, const char *tableName)
{
    int retryTimes = 0;
    int ret = NativeRdb::E_OK;
    do {
        ret = helper_.Insert(*id, values, tableName);
        if (!IsRetryable(ret)) {
            break;
        }
        if (++retryTimes < RETRY_TIMES) {
            DATA_STORAGE_LOGI("SmsMmsAbility::InsertRetry retry");
            std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_INTERVAL));
        }
    } while (retryTimes < RETRY_TIMES);
}

} // namespace Telephony
} // namespace OHOS
