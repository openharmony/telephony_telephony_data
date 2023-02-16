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

#include "rdb_sms_mms_helper.h"

#include "rdb_errno.h"
#include "rdb_sms_mms_callback.h"
#include "rdb_store_config.h"
#include "sms_mms_data.h"
#include "time_util.h"

namespace OHOS {
namespace NativeRdb {
class AbsSharedResultSet;
class ValuesBucket;
} // namespace NativeRdb
namespace Telephony {
RdbSmsMmsHelper::RdbSmsMmsHelper()
{
}

int RdbSmsMmsHelper::Init()
{
    int errCode = NativeRdb::E_OK;
    NativeRdb::RdbStoreConfig config(dbPath_);
    config.SetJournalMode(NativeRdb::JournalMode::MODE_TRUNCATE);
    std::string messageInfoStr;
    CreateSmsMmsInfoTableStr(messageInfoStr);
    std::string mmsProtocolStr;
    CreateMmsProtocolTableStr(mmsProtocolStr);
    std::string smsSubsectionStr;
    CreateSmsSubsectionTableStr(smsSubsectionStr);
    std::string mmsPartStr;
    CreateMmsPartTableStr(mmsPartStr);
    std::vector<std::string> createTableVec;
    createTableVec.push_back(messageInfoStr);
    createTableVec.push_back(mmsProtocolStr);
    createTableVec.push_back(smsSubsectionStr);
    createTableVec.push_back(mmsPartStr);
    RdbSmsMmsCallback callback(createTableVec);
    CreateRdbStore(config, VERSION, callback, errCode);
    return errCode;
}

void RdbSmsMmsHelper::CreateSmsMmsInfoTableStr(std::string &createTableStr)
{
    createTableStr.append("CREATE TABLE IF NOT EXISTS ").append(TABLE_SMS_MMS_INFO);
    createTableStr.append("(").append(SmsMmsInfo::MSG_ID).append(" INTEGER PRIMARY KEY AUTOINCREMENT, ");
    createTableStr.append(SmsMmsInfo::SLOT_ID).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(SmsMmsInfo::RECEIVER_NUMBER).append(" TEXT , ");
    createTableStr.append(SmsMmsInfo::SENDER_NUMBER).append(" TEXT , ");
    createTableStr.append(SmsMmsInfo::IS_SENDER).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(SmsMmsInfo::MSG_TYPE).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(SmsMmsInfo::SMS_TYPE).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(SmsMmsInfo::START_TIME).append(" TEXT DEFAULT '', ");
    createTableStr.append(SmsMmsInfo::END_TIME).append(" TEXT DEFAULT '', ");
    createTableStr.append(SmsMmsInfo::MSG_STATE).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(SmsMmsInfo::MSG_TITLE).append(" TEXT DEFAULT '', ");
    createTableStr.append(SmsMmsInfo::MSG_CONTENT).append(" TEXT DEFAULT '', ");
    createTableStr.append(SmsMmsInfo::OPERATOR_SERVICE_NUMBER).append(" TEXT DEFAULT '', ");
    createTableStr.append(SmsMmsInfo::IS_LOCK).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(SmsMmsInfo::IS_COLLECT).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(SmsMmsInfo::IS_READ).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(SmsMmsInfo::SESSION_TYPE).append(" INTEGER DEFAULT 3, ");
    createTableStr.append(SmsMmsInfo::RETRY_NUMBER).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(SmsMmsInfo::SESSION_ID).append(" INTEGER DEFAULT -1, ");
    createTableStr.append(SmsMmsInfo::GROUP_ID).append(" INTEGER DEFAULT -1, ");
    createTableStr.append(SmsMmsInfo::DEVICE_ID).append(" INTEGER , ");
    createTableStr.append(SmsMmsInfo::IS_SUBSECTION).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(SmsMmsInfo::IS_SEND_REPORT).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(SmsMmsInfo::MSG_CODE).append(" INTEGER DEFAULT 0)");
}

void RdbSmsMmsHelper::CreateMmsProtocolTableStr(std::string &createTableStr)
{
    createTableStr.append("CREATE TABLE IF NOT EXISTS ").append(TABLE_MMS_PROTOCOL);
    createTableStr.append("(").append(MmsProtocol::ID).append(" INTEGER PRIMARY KEY AUTOINCREMENT, ");
    createTableStr.append(SmsMmsInfo::MSG_ID).append(" INTEGER NOT NULL, ");
    createTableStr.append(MmsProtocol::BCC).append(" TEXT DEFAULT '', ");
    createTableStr.append(MmsProtocol::CC).append(" TEXT DEFAULT '', ");
    createTableStr.append(MmsProtocol::CONTENT_LOCATION).append(" TEXT DEFAULT '', ");
    createTableStr.append(MmsProtocol::DATE).append(" TEXT DEFAULT '', ");
    createTableStr.append(MmsProtocol::DELIVERY_REPORT).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(MmsProtocol::DELIVERY_TIME).append(" TEXT DEFAULT '', ");
    createTableStr.append(MmsProtocol::EXPIRY).append(" INTEGER , ");
    createTableStr.append(MmsProtocol::TYPE).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(MmsProtocol::SERIAL_NUMBER).append(" TEXT DEFAULT '', ");
    createTableStr.append(MmsProtocol::CATEGORY).append(" TEXT DEFAULT '', ");
    createTableStr.append(MmsProtocol::VERSION).append(" INTEGER , ");
    createTableStr.append(MmsProtocol::SIZE).append(" INTEGER , ");
    createTableStr.append(MmsProtocol::PRIORITY).append(" INTEGER , ");
    createTableStr.append(MmsProtocol::READ_REPLY).append(" INTEGER , ");
    createTableStr.append(MmsProtocol::REPORT_ALLOWED).append(" INTEGER , ");
    createTableStr.append(MmsProtocol::RESPONSE_STATUS).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(MmsProtocol::RESPONSE_TEXT).append(" TEXT DEFAULT '', ");
    createTableStr.append(MmsProtocol::SENDER_VISIBILITY).append(" INTEGER DEFAULT 0,");
    createTableStr.append("foreign key(").append(SmsMmsInfo::MSG_ID).append(") references ");
    createTableStr.append(TABLE_SMS_MMS_INFO).append("(").append(SmsMmsInfo::MSG_ID);
    createTableStr.append(") on delete cascade on update cascade )");
}

void RdbSmsMmsHelper::CreateSmsSubsectionTableStr(std::string &createTableStr)
{
    createTableStr.append("CREATE TABLE IF NOT EXISTS ").append(TABLE_SMS_SUBSECTION);
    createTableStr.append("(").append(SmsSubsection::ID).append(" INTEGER PRIMARY KEY AUTOINCREMENT, ");
    createTableStr.append(SmsMmsInfo::SLOT_ID).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(SmsSubsection::SMS_SUBSECTION_ID).append(" INTEGER , ");
    createTableStr.append(SmsSubsection::RECEIVER_NUMBER).append(" TEXT NOT NULL, ");
    createTableStr.append(SmsSubsection::SENDER_NUMBER).append(" TEXT NOT NULL, ");
    createTableStr.append(SmsSubsection::IS_SENDER).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(SmsSubsection::START_TIME).append(" TEXT DEFAULT '', ");
    createTableStr.append(SmsSubsection::END_TIME).append(" TEXT DEFAULT '', ");
    createTableStr.append(SmsSubsection::REW_PUD).append(" TEXT NOT NULL, ");
    createTableStr.append(SmsSubsection::FORMAT).append(" INTEGER , ");
    createTableStr.append(SmsSubsection::DEST_PORT).append(" INTEGER , ");
    createTableStr.append(SmsSubsection::SUBSECTION_INDEX).append(" INTEGER , ");
    createTableStr.append(SmsSubsection::SIZE).append(" INTEGER )");
}

void RdbSmsMmsHelper::CreateMmsPartTableStr(std::string &createTableStr)
{
    createTableStr.append("CREATE TABLE IF NOT EXISTS ").append(TABLE_MMS_PART);
    createTableStr.append("(").append(MmsPart::ID).append(" INTEGER PRIMARY KEY AUTOINCREMENT, ");
    createTableStr.append(SmsMmsInfo::MSG_ID).append(" INTEGER NOT NULL, ");
    createTableStr.append(SmsMmsInfo::GROUP_ID).append(" INTEGER , ");
    createTableStr.append(MmsPart::PART_INDEX).append(" INTEGER , ");
    createTableStr.append(MmsPart::PART_SIZE).append(" TEXT , ");
    createTableStr.append(MmsPart::RECORDING_TIME).append(" TEXT , ");
    createTableStr.append(MmsPart::TYPE).append(" INTEGER , ");
    createTableStr.append(MmsPart::LOCATION_PATH).append(" TEXT DEFAULT '', ");
    createTableStr.append(MmsPart::ENCODE).append(" INTEGER , ");
    createTableStr.append(MmsPart::STATE).append(" INTEGER , ");
    createTableStr.append(MmsPart::CONTENT).append(" TEXT , ");
    createTableStr.append("foreign key(").append(SmsMmsInfo::MSG_ID).append(") references ");
    createTableStr.append(TABLE_SMS_MMS_INFO).append("(").append(SmsMmsInfo::MSG_ID);
    createTableStr.append(") on delete cascade on update cascade )");
}

void RdbSmsMmsHelper::UpdateDbPath(const std::string &path)
{
    dbPath_ = path + DB_NAME;
}

int32_t RdbSmsMmsHelper::DeleteDataByThirty()
{
    int changedRows = 0;
    int32_t result;
    std::string values;
    std::string date;
    GetTimeOfThirty(date);
    values.append(SmsMmsInfo::START_TIME).append("< '").append(date).append("'");
    result = Delete(changedRows, TABLE_SMS_MMS_INFO, values);
    return result;
}

int RdbSmsMmsHelper::InsertSmsMmsInfo(int64_t &id, const NativeRdb::ValuesBucket &values)
{
    return Insert(id, values, TABLE_SMS_MMS_INFO);
}

int32_t RdbSmsMmsHelper::BatchInsertSmsMmsInfo(int64_t &id, const std::vector<NativeRdb::ValuesBucket> &values)
{
    int32_t result = BeginTransaction();
    if (result != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("RdbSmsMmsHelper::BatchInsertSmsMmsInfo BeginTransaction is error!");
        return result;
    }
    for (const NativeRdb::ValuesBucket &item : values) {
        result = InsertSmsMmsInfo(id, item);
        if (result != NativeRdb::E_OK) {
            DATA_STORAGE_LOGE("RdbSmsMmsHelper::InsertSmsMmsInfo error result = %{public}d", result);
            RollBack();
            return result;
        }
    }
    result = CommitTransactionAction();
    return result;
}

int RdbSmsMmsHelper::CommitTransactionAction()
{
    int result = Commit();
    if (result != NativeRdb::E_OK) {
        RollBack();
    }
    return result;
}

std::unique_ptr<NativeRdb::AbsSharedResultSet> RdbSmsMmsHelper::QueryMaxGroupId()
{
    std::string sql;
    std::string maxGroupId("maxGroupId");
    sql.append("select MAX(").append(SmsMmsInfo::GROUP_ID).append(") as ");
    sql.append(maxGroupId).append(" from ") .append(TABLE_SMS_MMS_INFO);
    return QuerySql(sql);
}

std::unique_ptr<NativeRdb::AbsSharedResultSet> RdbSmsMmsHelper::StatisticsUnRead()
{
    std::string sql;
    sql.append("select count(*) as totalListCount, ");
    sql.append("count(CASE WHEN ").append(SmsMmsInfo::MSG_TYPE).append("=0 THEN 1 ELSE null END) as unreadCount, ");
    sql.append("count(CASE WHEN ").append(SmsMmsInfo::MSG_TYPE).append("=1 THEN 1 ELSE null END) as unreadTotalOfInfo");
    sql.append(" from ").append(TABLE_SMS_MMS_INFO).append(" WHERE ").append(SmsMmsInfo::IS_READ).append("=0");
    return QuerySql(sql);
}
} // namespace Telephony
} // namespace OHOS
