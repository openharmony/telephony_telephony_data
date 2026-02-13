/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "sms_mms_tables.h"
#include "sms_mms_data.h"

namespace OHOS {
namespace Telephony {

std::vector<std::string> RdbSmsMmsTables::InitCreateTableVec()
{
    std::vector<std::string> createTableVec;
    {
        std::string messageInfoStr;
        CreateSmsMmsInfoTableStr(messageInfoStr);
        std::string mmsProtocolStr;
        CreateMmsProtocolTableStr(mmsProtocolStr);
        std::string rcsInfoStr;
        CreateRcsInfoTableStr(rcsInfoStr);
        std::string smsSubsectionStr;
        CreateSmsSubsectionTableStr(smsSubsectionStr);
        std::string mmsPartStr;
        CreateMmsPartTableStr(mmsPartStr);
        std::string sessionStr;
        CreateSessionTableStr(sessionStr);
        std::string mmsPduStr;
        CreateMmsPduTableStr(mmsPduStr);
        std::string riskUrlRecordStr;
        CreateRiskUrlRecordTableStr(riskUrlRecordStr);

        createTableVec.push_back(messageInfoStr);
        createTableVec.push_back(mmsProtocolStr);
        createTableVec.push_back(rcsInfoStr);
        createTableVec.push_back(smsSubsectionStr);
        createTableVec.push_back(mmsPartStr);
        createTableVec.push_back(sessionStr);
        createTableVec.push_back(mmsPduStr);
        createTableVec.push_back(riskUrlRecordStr);
    }

    {
        std::string mmsInfoStr;
        CreateMmsInfoViewStr(mmsInfoStr);
        createTableVec.push_back(mmsInfoStr);
    }

    {
        std::string createInsertSmsTrigger;
        CreateInsertSmsTriggerStr(createInsertSmsTrigger);
        std::string createInsertRcsTrigger;
        CreateInsertRcsTriggerStr(createInsertRcsTrigger);
        std::string createUpdateBlockSmsTrigger;
        CreateUpdateBlockSmsTriggerStr(createUpdateBlockSmsTrigger);
        std::string createUpdateBlockRcsTrigger;
        CreateUpdateBlockRcsTriggerStr(createUpdateBlockRcsTrigger);

        createTableVec.push_back(createInsertSmsTrigger);
        createTableVec.push_back(createInsertRcsTrigger);
        createTableVec.push_back(createUpdateBlockSmsTrigger);
        createTableVec.push_back(createUpdateBlockRcsTrigger);
    }

    InitCreateTableIndex(createTableVec);
    return createTableVec;
}

void RdbSmsMmsTables::InitCreateTableIndex(std::vector<std::string> &tables)
{
    tables.push_back(VERSION_22_DROP_INDEX_ON_RCS_TABLE);
    tables.push_back(VERSION_22_CREATE_INDEX_ON_RCS_TABLE);

    tables.push_back(VERSION_22_DROP_INDEX_ON_SMS_MMS_INFO_TABLE);
    tables.push_back(VERSION_22_CREATE_INDEX_ON_SMS_MMS_INFO_TABLE);

    tables.push_back(VERSION_22_DROP_INDEX_ON_SESSION_TABLE);
    tables.push_back(VERSION_22_CREATE_INDEX_ON_SESSION_TABLE);

    std::string favoriteInfoStr;
    CreateSmsMmsFavoriteTableStr(favoriteInfoStr);
    tables.push_back(favoriteInfoStr);

    std::string createChatBotsTable;
    CreateChatBotsTableStr(createChatBotsTable);
    std::string createSpecificChatBotsTable;
    CreateSpecificChatBotsTableStr(createSpecificChatBotsTable);
    tables.push_back(createChatBotsTable);
    tables.push_back(createSpecificChatBotsTable);

    std::string createSmcTable;
    CreateSmcTableStr(createSmcTable);
    tables.push_back(createSmcTable);
}

void RdbSmsMmsTables::CreateSmsMmsInfoTableStr(std::string &createTableStr)
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
    createTableStr.append(SmsMmsInfo::IS_ADVANCED_SECURITY).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(SmsMmsInfo::EXPIRES_TIME).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(SmsMmsInfo::MMS_PDU).append(" TEXT DEFAULT '', ");
    createTableStr.append(SmsMmsInfo::MSG_CODE).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(SmsMmsInfo::IS_BLOCKED).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(SmsMmsInfo::BLOCKED_REASON).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(SmsMmsInfo::BLOCKED_SOURCES).append(" TEXT DEFAULT '', ");
    createTableStr.append(SmsMmsInfo::BLOCKED_TYPE).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(SmsMmsInfo::BLOCKED_TYPE_TEXT).append(" TEXT DEFAULT '', ");
    createTableStr.append(SmsMmsInfo::BLOCKED_PREF_NUMBER).append(" TEXT DEFAULT '', ");
    createTableStr.append(SmsMmsInfo::RISK_URL_BODY).append(" TEXT DEFAULT '', ");
    createTableStr.append(SmsMmsInfo::HAS_BE_REPORTED_AS_SPAM).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(SmsMmsInfo::IS_REPORT).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(SmsMmsInfo::RCS_ID).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(SmsMmsInfo::DETECT_RES_CONTENT).append(" TEXT DEFAULT '', ");
    createTableStr.append(SmsMmsInfo::PHONE_NUMBER).append(" TEXT , ");
    createTableStr.append(SmsMmsInfo::FORMAT_PHONE_NUMBER).append(" TEXT , ");
    createTableStr.append(SmsMmsInfo::MSG_CODE_STR).append(" TEXT DEFAULT '')");
}

void RdbSmsMmsTables::CreateRcsInfoTableStr(std::string &createTableStr)
{
    createTableStr.append("CREATE TABLE IF NOT EXISTS ").append(TABLE_RCS_INFO);
    createTableStr.append("(").append(RcsInfo::RCS_ID).append(" INTEGER PRIMARY KEY AUTOINCREMENT, ");
    createTableStr.append(RcsInfo::SLOT_ID).append(" INTEGER DEFAULT 0,");
    createTableStr.append(RcsInfo::RECEIVER_NUMBER).append(" TEXT , ");
    createTableStr.append(RcsInfo::SENDER_NUMBER).append(" TEXT , ");
    createTableStr.append(RcsInfo::START_TIME).append(" TEXT DEFAULT '', ");
    createTableStr.append(RcsInfo::END_TIME).append(" TEXT DEFAULT '', ");
    createTableStr.append(RcsInfo::RCS_TYPE).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(RcsInfo::MSG_STATE).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(RcsInfo::MSG_TITLE).append(" TEXT DEFAULT '', ");
    createTableStr.append(RcsInfo::MSG_CONTENT).append(" TEXT DEFAULT '', ");
    createTableStr.append(RcsInfo::IS_LOCK).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(RcsInfo::IS_READ).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(RcsInfo::IS_SENDER).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(RcsInfo::SESSION_ID).append(" INTEGER DEFAULT -1, ");
    createTableStr.append(RcsInfo::GROUP_ID).append(" INTEGER DEFAULT -1, ");
    createTableStr.append(RcsInfo::MSG_ID).append(" TEXT DEFAULT '', ");
    createTableStr.append(RcsInfo::CALLING_TYPE).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(RcsInfo::ERROR_CODE).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(RcsInfo::NETWORK_TYPE).append(" TEXT DEFAULT '', ");
    createTableStr.append(RcsInfo::OWNER_ADDR).append(" TEXT DEFAULT '', ");
    createTableStr.append(RcsInfo::PRIVACY_MODE).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(RcsInfo::IS_ADVANCED_SECURITY).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(RcsInfo::IS_COLLECT).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(RcsInfo::RECEIVE_STATE).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(RcsInfo::FAIL_RECEIVE_CONTEXT).append(" TEXT DEFAULT '', ");
    createTableStr.append(RcsInfo::PROTOCOL).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(RcsInfo::REPLY_PATH).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(RcsInfo::SEEN).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(RcsInfo::SERVICE_CENTER).append(" TEXT DEFAULT '', ");
    createTableStr.append(RcsInfo::SERVICE_KIND).append(" TEXT DEFAULT '', ");
    createTableStr.append(RcsInfo::IS_BLOCKED).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(RcsInfo::BLOCKED_REASON).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(RcsInfo::BLOCKED_SOURCES).append(" TEXT DEFAULT '', ");
    createTableStr.append(RcsInfo::BLOCKED_TYPE).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(RcsInfo::BLOCKED_TYPE_TEXT).append(" TEXT DEFAULT '', ");
    createTableStr.append(RcsInfo::BLOCKED_PREF_NUMBER).append(" TEXT DEFAULT '', ");
    createTableStr.append(RcsInfo::CLUR_SIZE).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(RcsInfo::TOTAL_SIZE).append(" INTEGER DEFAULT 0 , ");
    createTableStr.append(SmsMmsInfo::RISK_URL_BODY).append(" TEXT DEFAULT '', ");
    createTableStr.append(SmsMmsInfo::HAS_BE_REPORTED_AS_SPAM).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(RcsInfo::IS_REPORT).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(SmsMmsInfo::DETECT_RES_CONTENT).append(" TEXT DEFAULT '', ");
    createTableStr.append(RcsInfo::PHONE_NUMBER).append(" TEXT , ");
    createTableStr.append(RcsInfo::FORMAT_PHONE_NUMBER).append(" TEXT)");
}

void RdbSmsMmsTables::CreateMmsProtocolTableStr(std::string &createTableStr)
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

void RdbSmsMmsTables::CreateSmsSubsectionTableStr(std::string &createTableStr)
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

void RdbSmsMmsTables::CreateMmsPartTableStr(std::string &createTableStr)
{
    createTableStr.append("CREATE TABLE IF NOT EXISTS ").append(TABLE_MMS_PART);
    createTableStr.append("(").append(MmsPart::ID).append(" INTEGER PRIMARY KEY AUTOINCREMENT, ");
    createTableStr.append(SmsMmsInfo::MSG_ID).append(" INTEGER NOT NULL, ");
    createTableStr.append(SmsMmsInfo::GROUP_ID).append(" INTEGER , ");
    createTableStr.append(MmsPart::PART_INDEX).append(" INTEGER , ");
    createTableStr.append(MmsPart::PART_SIZE).append(" TEXT , ");
    createTableStr.append(MmsPart::RECORDING_TIME).append(" TEXT , ");
    createTableStr.append(MmsPart::TYPE).append(" INTEGER , ");
    createTableStr.append(MmsPart::CT).append(" TEXT , ");
    createTableStr.append(MmsPart::LOCATION_PATH).append(" TEXT DEFAULT '', ");
    createTableStr.append(MmsPart::ENCODE).append(" INTEGER , ");
    createTableStr.append(MmsPart::STATE).append(" INTEGER , ");
    createTableStr.append(MmsPart::CONTENT).append(" TEXT , ");
    createTableStr.append(MmsPart::RCS_ID).append(" INTEGER , ");
    createTableStr.append(MmsPart::IS_FRAUD).append(" INTEGER , ");
    createTableStr.append(MmsPart::REFERENCE_COUNT).append(" INTEGER DEFAULT -1, ");
    createTableStr.append("foreign key(").append(SmsMmsInfo::MSG_ID).append(") references ");
    createTableStr.append(TABLE_SMS_MMS_INFO).append("(").append(SmsMmsInfo::MSG_ID);
    createTableStr.append(") on delete cascade on update cascade )");
}

void RdbSmsMmsTables::CreateSessionTableStr(std::string &createTableStr)
{
    createTableStr.append("CREATE TABLE IF NOT EXISTS ").append(TABLE_SESSION);
    createTableStr.append("(").append(Session::ID).append(" INTEGER PRIMARY KEY AUTOINCREMENT, ");
    createTableStr.append(Session::TIME).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(Session::TELEPHONE).append(" TEXT , ");
    createTableStr.append(Session::CONTENT).append(" TEXT , ");
    createTableStr.append(Session::CONTACTS_NUM).append(" INTEGER DEFAULT 0 , ");
    createTableStr.append(Session::SMS_TYPE).append(" INTEGER DEFAULT 0 , ");
    createTableStr.append(Session::UNREAD_COUNT).append(" INTEGER DEFAULT 0 , ");
    createTableStr.append(Session::SENDING_STATUS).append(" INTEGER DEFAULT 0 , ");
    createTableStr.append(Session::HAS_DRAFT).append(" INTEGER DEFAULT 0 , ");
    createTableStr.append(Session::HAS_LOCK).append(" INTEGER DEFAULT 0 , ");
    createTableStr.append(Session::MESSAGE_COUNT).append(" INTEGER DEFAULT 0 , ");
    createTableStr.append(Session::HAS_MMS).append(" INTEGER DEFAULT 0 , ");
    createTableStr.append(Session::CONTACT_ID).append(" TEXT DEFAULT '', ");
    createTableStr.append(Session::CONTACT_NAME).append(" TEXT DEFAULT '', ");
    createTableStr.append(Session::HAS_ATTACHMENT).append(" INTEGER DEFAULT 0 , ");
    createTableStr.append(Session::YELLOW_PAGE_ID).append(" TEXT DEFAULT '', ");
    createTableStr.append(Session::BLOCKED_TYPE).append(" INTEGER DEFAULT 0 , ");
    createTableStr.append(Session::PINNING_TIME).append(" INTEGER DEFAULT 0)");
}

void RdbSmsMmsTables::CreateMmsPduTableStr(std::string &createTableStr)
{
    createTableStr.append("CREATE TABLE IF NOT EXISTS ").append(TABLE_MMS_PDU);
    createTableStr.append("(").append(MmsPdu::ID).append(" INTEGER PRIMARY KEY AUTOINCREMENT, ");
    createTableStr.append(MmsPdu::PDU_CONTENT).append(" TEXT )");
}

void RdbSmsMmsTables::CreateRiskUrlRecordTableStr(std::string &createTableStr)
{
    createTableStr.append("CREATE TABLE IF NOT EXISTS ").append(TABLE_RISK_URL_RECORD);
    createTableStr.append("(").append(RiskUrlRecord::ID).append(" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, ");
    createTableStr.append(RiskUrlRecord::SESSION_ID).append(" INTEGER NOT NULL, ");
    createTableStr.append(RiskUrlRecord::MSG_ID).append(" INTEGER NOT NULL, ");
    createTableStr.append(RiskUrlRecord::RCS_ID).append(" INTEGER NOT NULL, ");
    createTableStr.append(RiskUrlRecord::MSG_TYPE).append(" INTEGER NOT NULL, ");
    createTableStr.append(RiskUrlRecord::RISK_TYPE).append(" INTEGER , ");
    createTableStr.append(RiskUrlRecord::RISK_URL).append(" TEXT , ");
    createTableStr.append(RiskUrlRecord::URL_INDEX).append(" INTEGER NOT NULL, ");
    createTableStr.append(RiskUrlRecord::CREATE_TIME).append(" TEXT NOT NULL, ");
    createTableStr.append(RiskUrlRecord::LAST_UPDATE_TIME).append(" TEXT NOT NULL, ");
    createTableStr.append(RiskUrlRecord::PACKAGE_NAME).append(" TEXT , ");
    createTableStr.append(RiskUrlRecord::CHECK_TYPE).append(" INTEGER NOT NULL, ");
    createTableStr.append(RiskUrlRecord::NUMBER_OF_RETRIES).append(" INTEGER DEFAULT (0) NOT NULL )");
}

/*
* Related contents need to be added to the CreateMmsInfoViewStr function.
* Otherwise, the mmsinfo table does not take effect in the new installation scenario.
*/
void RdbSmsMmsTables::CreateMmsInfoViewStr(std::string &createTableStr)
{
    createTableStr.append("CREATE VIEW IF NOT EXISTS ").append("mms_info(");
    createTableStr.append("msg_id, rcs_id, slot_id, receiver_number, ");
    createTableStr.append("sender_number, is_sender, sms_type, rcs_type, ");
    createTableStr.append("msg_type, start_time, end_time, msg_state, msg_title, ");
    createTableStr.append("msg_content, operator_service_number, is_lock, is_collect, ");
    createTableStr.append("is_read, session_type, retry_number, session_id, group_id, ");
    createTableStr.append("device_id, is_subsection, is_send_report, msg_code, ");
    createTableStr.append("enriched_calling_type, error_code, network_type, owner_addr, ");
    createTableStr.append("privacy_mode, protocol, reply_path_present, seen, receive_state, ");
    createTableStr.append("fail_receive_context, service_center, service_kind, ");
    createTableStr.append("is_advanced_security, expires_time, mms_pdu, ");
    createTableStr.append("is_blocked, blocked_reason, blocked_sources, ");
    createTableStr.append("blocked_type, blocked_type_text, blocked_pref_number,clur_size, total_size, ");
    createTableStr.append("risk_url_body, has_be_reported_as_spam, is_report, detect_res_content, ");
    createTableStr.append("phone_number, format_phone_number, msg_code_str )").append(" AS ");
    createTableStr.append("SELECT msg_id, rcs_id, slot_id, receiver_number, ");
    createTableStr.append("sender_number, is_sender, sms_type, '', msg_type, ");
    createTableStr.append("start_time, end_time, msg_state, msg_title, msg_content, ");
    createTableStr.append("operator_service_number, is_lock, is_collect, is_read, ");
    createTableStr.append("session_type, retry_number, session_id, group_id, ");
    createTableStr.append("device_id, is_subsection, is_send_report, msg_code, '', '', ");
    createTableStr.append("'', '', '', '', '', '', '', '', '', '', ");
    createTableStr.append("is_advanced_security, expires_time, mms_pdu, ");
    createTableStr.append("is_blocked, blocked_reason, blocked_sources, blocked_type,");
    createTableStr.append(" blocked_type_text, blocked_pref_number,'', '', ");
    createTableStr.append("risk_url_body, has_be_reported_as_spam, is_report, detect_res_content, ");
    createTableStr.append("phone_number, format_phone_number, msg_code_str ");
    createTableStr.append("FROM sms_mms_info ");
    createTableStr.append("WHERE sms_type != 99 ");
    createTableStr.append(" UNION ");
    createTableStr.append("SELECT msg_id, rcs_id, slot_id, receiver_number, sender_number, ");
    createTableStr.append("is_sender, '', rcs_type, '', start_time, end_time, msg_state, ");
    createTableStr.append("msg_title, msg_content, '', is_lock, is_collect, ");
    createTableStr.append("is_read, '', '', session_id, group_id, '', '', '', '', ");
    createTableStr.append("enriched_calling_type, error_code, network_type, owner_addr, ");
    createTableStr.append("privacy_mode, protocol, reply_path_present, seen, receive_state, ");
    createTableStr.append("fail_receive_context, service_center, service_kind, ");
    createTableStr.append("is_advanced_security, '', '', ");
    createTableStr.append("is_blocked, blocked_reason, blocked_sources,");
    createTableStr.append("blocked_type, blocked_type_text, blocked_pref_number, ");
    createTableStr.append("clur_size, total_size, risk_url_body, has_be_reported_as_spam, is_report, ");
    createTableStr.append("detect_res_content, ");
    createTableStr.append("phone_number, format_phone_number, '' ");
    createTableStr.append("FROM rcs_info ").append("ORDER BY start_time, rcs_id ASC");
}

void RdbSmsMmsTables::CreateInsertSmsTriggerStr(std::string &createInsertSmsTrigger)
{
    createInsertSmsTrigger.append("CREATE TRIGGER IF NOT EXISTS insertSms_unread_count AFTER INSERT ON sms_mms_info ");
    createInsertSmsTrigger.append("WHEN NEW.is_read = 0 ");
    createInsertSmsTrigger.append("BEGIN ");
    createInsertSmsTrigger.append("UPDATE session SET unread_count = (SELECT SUM(unread) FROM ");
    createInsertSmsTrigger.append("(SELECT COUNT(*) AS unread FROM sms_mms_info WHERE session_id = NEW.session_id ");
    createInsertSmsTrigger.append("AND is_read = 0 UNION ALL ");
    createInsertSmsTrigger.append("SELECT COUNT(*) AS unread FROM rcs_info ");
    createInsertSmsTrigger.append("WHERE session_id = NEW.session_id AND is_read = 0)) ");
    createInsertSmsTrigger.append("WHERE session.id = NEW.session_id; ");
    createInsertSmsTrigger.append("END;");
}

void RdbSmsMmsTables::CreateInsertRcsTriggerStr(std::string &createInsertRcsTrigger)
{
    createInsertRcsTrigger.append("CREATE TRIGGER IF NOT EXISTS insertRcs_unread_count AFTER INSERT ON rcs_info ");
    createInsertRcsTrigger.append("WHEN NEW.is_read = 0 ");
    createInsertRcsTrigger.append("BEGIN ");
    createInsertRcsTrigger.append("UPDATE session SET unread_count = (SELECT SUM(unread) FROM ");
    createInsertRcsTrigger.append("(SELECT COUNT(*) AS unread FROM sms_mms_info WHERE session_id = NEW.session_id ");
    createInsertRcsTrigger.append("AND is_read = 0 UNION ALL ");
    createInsertRcsTrigger.append("SELECT COUNT(*) AS unread FROM rcs_info ");
    createInsertRcsTrigger.append("WHERE session_id = NEW.session_id AND is_read = 0)) ");
    createInsertRcsTrigger.append("WHERE session.id = NEW.session_id; ");
    createInsertRcsTrigger.append("END;");
}

void RdbSmsMmsTables::CreateUpdateBlockSmsTriggerStr(std::string &createUpdateBlockSmsTrigger)
{
    createUpdateBlockSmsTrigger.append("CREATE TRIGGER IF NOT EXISTS updateBlockSms_unread_count ");
    createUpdateBlockSmsTrigger.append("AFTER UPDATE ON sms_mms_info ");
    createUpdateBlockSmsTrigger.append("WHEN NEW.is_read = 0 ");
    createUpdateBlockSmsTrigger.append("BEGIN ");
    createUpdateBlockSmsTrigger.append("UPDATE session SET unread_count = (SELECT SUM(unread) FROM ");
    createUpdateBlockSmsTrigger.append("(SELECT COUNT(*) AS unread FROM sms_mms_info WHERE ");
    createUpdateBlockSmsTrigger.append("session_id = NEW.session_id ");
    createUpdateBlockSmsTrigger.append("AND is_read = 0 UNION ALL ");
    createUpdateBlockSmsTrigger.append("SELECT COUNT(*) AS unread FROM rcs_info ");
    createUpdateBlockSmsTrigger.append("WHERE session_id = NEW.session_id AND is_read = 0)) ");
    createUpdateBlockSmsTrigger.append("WHERE session.id = NEW.session_id; ");
    createUpdateBlockSmsTrigger.append("END; ");
}

void RdbSmsMmsTables::CreateUpdateBlockRcsTriggerStr(std::string &createBlockUpdateBlockRcsTrigger)
{
    createBlockUpdateBlockRcsTrigger.append("CREATE TRIGGER IF NOT EXISTS updateBlockRcs_unread_count ");
    createBlockUpdateBlockRcsTrigger.append("AFTER UPDATE ON rcs_info ");
    createBlockUpdateBlockRcsTrigger.append("WHEN NEW.is_read = 0 ");
    createBlockUpdateBlockRcsTrigger.append("BEGIN ");
    createBlockUpdateBlockRcsTrigger.append("UPDATE session SET unread_count = (SELECT SUM(unread) FROM ");
    createBlockUpdateBlockRcsTrigger.append("(SELECT COUNT(*) AS unread FROM sms_mms_info ");
    createBlockUpdateBlockRcsTrigger.append("WHERE session_id = NEW.session_id ");
    createBlockUpdateBlockRcsTrigger.append("AND is_read = 0 UNION ALL ");
    createBlockUpdateBlockRcsTrigger.append("SELECT COUNT(*) AS unread FROM rcs_info ");
    createBlockUpdateBlockRcsTrigger.append("WHERE session_id = NEW.session_id AND is_read = 0)) ");
    createBlockUpdateBlockRcsTrigger.append("WHERE session.id = NEW.session_id; ");
    createBlockUpdateBlockRcsTrigger.append("END; ");
}

void RdbSmsMmsTables::CreateChatBotsTableStr(std::string &createChatBotsTable)
{
    createChatBotsTable.append("CREATE TABLE IF NOT EXISTS ").append(TABLE_CHAT_BOTS_INFO);
    createChatBotsTable.append("(").append(ChatBots::ID).append(" INTEGER PRIMARY KEY AUTOINCREMENT, ");
    createChatBotsTable.append(ChatBots::SERVICE_ID).append(" TEXT DEFAULT '' ,");
    createChatBotsTable.append(ChatBots::SERVICE_NAME).append(" TEXT DEFAULT '',");
    createChatBotsTable.append(ChatBots::SERVICE_DESCRIPTION).append(" TEXT DEFAULT '' ,");
    createChatBotsTable.append(ChatBots::CALLBACK_PHONE_NUMBER).append(" TEXT DEFAULT '' ,");
    createChatBotsTable.append(ChatBots::SMS).append(" TEXT DEFAULT '' ,");
    createChatBotsTable.append(ChatBots::SERVICE_ICON).append(" TEXT DEFAULT '' ,");
    createChatBotsTable.append(ChatBots::CATEGORY_LIST).append(" TEXT DEFAULT '' ,");
    createChatBotsTable.append(ChatBots::BRIEF).append(" INTEGER DEFAULT 0, ");
    createChatBotsTable.append(ChatBots::FAVORITE).append(" INTEGER DEFAULT 0, ");
    createChatBotsTable.append(ChatBots::EMAIL).append(" TEXT DEFAULT '' ,");
    createChatBotsTable.append(ChatBots::WEBSITE).append(" TEXT DEFAULT '' ,");
    createChatBotsTable.append(ChatBots::ADDRESS).append(" TEXT DEFAULT '' ,");
    createChatBotsTable.append(ChatBots::ADDRESS_LABLE).append(" TEXT DEFAULT '' ,");
    createChatBotsTable.append(ChatBots::LAST_SUGGESTED_LIST).append(" TEXT DEFAULT '' ,");
    createChatBotsTable.append(ChatBots::PINYIN).append(" TEXT DEFAULT '' ,");
    createChatBotsTable.append(ChatBots::PINYIN_SHORT).append(" TEXT DEFAULT '' ,");
    createChatBotsTable.append(ChatBots::COLOUR).append(" TEXT DEFAULT '' ,");
    createChatBotsTable.append(ChatBots::BACKGROUND_IMAGE).append(" TEXT DEFAULT '' ,");
    createChatBotsTable.append(ChatBots::VERIFIED).append(" INTEGER DEFAULT 0, ");
    createChatBotsTable.append(ChatBots::VERIFIED_BY).append(" TEXT DEFAULT '' ,");
    createChatBotsTable.append(ChatBots::VERIFIED_EXPIRES).append(" TEXT DEFAULT '' ,");
    createChatBotsTable.append(ChatBots::EXPIRES).append(" INTEGER DEFAULT 0, ");
    createChatBotsTable.append(ChatBots::CACHE_CONTROL).append(" TEXT DEFAULT '' ,");
    createChatBotsTable.append(ChatBots::E_TAG).append(" TEXT DEFAULT '' ,");
    createChatBotsTable.append(ChatBots::TC_PAGE).append(" TEXT DEFAULT '' ,");
    createChatBotsTable.append(ChatBots::RECENT_USE_TIME).append(" TEXT DEFAULT '' ,");
    createChatBotsTable.append(ChatBots::DISTURB).append(" INTEGER DEFAULT 0, ");
    createChatBotsTable.append(ChatBots::PERSISTENT_MENU).append(" TEXT DEFAULT '' ,");
    createChatBotsTable.append(ChatBots::SHARE_INFO).append(" INTEGER DEFAULT 0, ");
    createChatBotsTable.append(ChatBots::service_provider).append(" TEXT DEFAULT ''");
    createChatBotsTable.append(")");
}

void RdbSmsMmsTables::CreateSmcTableStr(std::string &createSmcTable)
{
    createSmcTable.append("CREATE TABLE IF NOT EXISTS ").append(TABLE_SMC_INFO);
    createSmcTable.append("(").append(Smc::ID).append(" INTEGER PRIMARY KEY AUTOINCREMENT, ");
    createSmcTable.append(Smc::THREAD_ID).append(" INTEGER DEFAULT 0 ,");
    createSmcTable.append(Smc::TYPE).append(" INTEGER DEFAULT 2 ,");
    createSmcTable.append(Smc::ADDRESS).append(" TEXT DEFAULT '' ,");
    createSmcTable.append(Smc::DATE).append(" INTEGER DEFAULT 0 ,");
    createSmcTable.append(Smc::STATUS).append(" INTEGER DEFAULT -1 ,");
    createSmcTable.append(Smc::BODY).append(" TEXT DEFAULT '' ,");
    createSmcTable.append(Smc::SENT).append(" INTEGER DEFAULT 0 ,");
    createSmcTable.append(Smc::READ).append(" INTEGER DEFAULT 0 , ");
    createSmcTable.append(Smc::SUB_ID).append(" INTEGER DEFAULT 0 , ");
    createSmcTable.append(Smc::ERROR_CODE).append(" INTEGER DEFAULT 0 ,");
    createSmcTable.append(Smc::LONGITUDE).append(" DOUBLE ,");
    createSmcTable.append(Smc::LATITUDE).append(" DOUBLE ,");
    createSmcTable.append(Smc::PRIVACY_MODE).append(" INTEGER DEFAULT 0 ");
    createSmcTable.append(")");
}

void RdbSmsMmsTables::CreateSpecificChatBotsTableStr(std::string &createSpecificChatBotsTable)
{
    createSpecificChatBotsTable.append("CREATE TABLE IF NOT EXISTS ").append(TABLE_SPECIFIC_CHAT_BOTS_INFO);
    createSpecificChatBotsTable.append("(").append(SpecificChatBots::ID);
    createSpecificChatBotsTable.append(" INTEGER PRIMARY KEY AUTOINCREMENT, ");
    createSpecificChatBotsTable.append(SpecificChatBots::SERVICE_ID).append(" TEXT DEFAULT '' ,");
    createSpecificChatBotsTable.append(SpecificChatBots::TYPE).append(" INTEGER DEFAULT 0, ");
    createSpecificChatBotsTable.append(SpecificChatBots::E_TAG).append(" TEXT DEFAULT '' ,");
    createSpecificChatBotsTable.append(SpecificChatBots::EXPIRES).append(" INTEGER DEFAULT 0, ");
    createSpecificChatBotsTable.append(SpecificChatBots::CACHE_CONTROL).append(" TEXT DEFAULT ''");
    createSpecificChatBotsTable.append(")");
}

void RdbSmsMmsTables::CreateSmsMmsFavoriteTableStr(std::string &createTableStr)
{
    createTableStr.append("CREATE TABLE IF NOT EXISTS ").append(TABLE_FAVORITE_INFO);
    createTableStr.append("(").append(FavoriteInfo::MSG_ID).append(" INTEGER , ");
    createTableStr.append(FavoriteInfo::RECEIVER_NUMBER).append(" TEXT DEFAULT '' , ");
    createTableStr.append(FavoriteInfo::SENDER_NUMBER).append(" TEXT DEFAULT '' , ");
    createTableStr.append(FavoriteInfo::KEEP_TIME).append(" TEXT DEFAULT '' , ");
    createTableStr.append(FavoriteInfo::START_TIME).append(" TEXT DEFAULT '' , ");
    createTableStr.append(FavoriteInfo::MSG_TYPE).append(" INTEGER , ");
    createTableStr.append(FavoriteInfo::SMS_TYPE).append(" INTEGER , ");
    createTableStr.append(FavoriteInfo::MSG_TITLE).append(" TEXT DEFAULT '' , ");
    createTableStr.append(FavoriteInfo::MSG_CONTENT).append(" TEXT DEFAULT '' , ");
    createTableStr.append(FavoriteInfo::SESSION_TYPE).append(" INTEGER , ");
    createTableStr.append(FavoriteInfo::GROUP_ID).append(" INTEGER , ");
    createTableStr.append(FavoriteInfo::PART_TYPE).append(" INTEGER , ");
    createTableStr.append(FavoriteInfo::PART_LOCATION_PATH).append(" TEXT DEFAULT '' , ");
    createTableStr.append(FavoriteInfo::IS_SENDER).append(" INTEGER , ");
    createTableStr.append(FavoriteInfo::RCS_ID).append(" INTEGER , ");
    createTableStr.append(FavoriteInfo::DETECT_RES_CONTENT).append(" TEXT DEFAULT '' , ");
    createTableStr.append(FavoriteInfo::SESSION_ID).append(" INTEGER , ");
    createTableStr.append(FavoriteInfo::RECORDING_TIME).append(" TEXT DEFAULT '' , ");
    createTableStr.append(FavoriteInfo::PART_SIZE).append(" TEXT DEFAULT '' , ");
    createTableStr.append("UNIQUE (").append(FavoriteInfo::MSG_ID).append(", ");
    createTableStr.append(FavoriteInfo::RCS_ID).append("))");
}
}
}

