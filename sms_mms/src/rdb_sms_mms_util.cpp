/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "rdb_sms_mms_util.h"
 
#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <securec.h>
#include <sstream>
#include "data_storage_log_wrapper.h"
#include "sms_mms_data.h"
#include "phonenumbers/phonenumber.pb.h"
#include "phonenumbers/phonenumberutil.h"
#include "common_event.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "permission_util.h"
 
namespace OHOS {
namespace NativeRdb {
class ResultSet;
class ValuesBucket;
} // namespace NativeRdb
namespace Telephony {
using namespace std;
const std::string PREFIX = "+86";
const std::string NUMBER_START_STR = "192";
const int32_t PHONE_CMP_LENGTH = 7;
const std::string SmsRdbEventSupport::SMS_RDB_EVENT_STORE_CHANGED = "smsrdb.event.StoreChanged";
 
void RdbSmsMmsUtil::CbnFormat(std::string &numTemp,
    const i18n::phonenumbers::PhoneNumberUtil::PhoneNumberFormat formatInfo, std::string &formatNum)
{
    uint8_t minLength = 3;
    if (numTemp.size() < minLength) {
        DATA_STORAGE_LOGE("Phonenumber is too short");
        return;
    }
    if (numTemp.substr(0, NUMBER_START_STR.size()) == NUMBER_START_STR ||
        numTemp.substr(PREFIX.size(), NUMBER_START_STR.size()) == NUMBER_START_STR) {
        if (formatInfo == i18n::phonenumbers::PhoneNumberUtil::PhoneNumberFormat::NATIONAL) {
            if (numTemp.substr(0, PREFIX.size()) == PREFIX) {
                numTemp.erase(0, PREFIX.size());
                formatNum.assign(numTemp);
                return;
            }
            formatNum.assign(numTemp);
            return;
        }
        if (formatInfo == i18n::phonenumbers::PhoneNumberUtil::PhoneNumberFormat::INTERNATIONAL) {
            if (numTemp.substr(0, PREFIX.size()) == PREFIX) {
                formatNum.assign(numTemp);
                return;
            }
            formatNum.assign(PREFIX + numTemp);
        }
    }
}
 
void RdbSmsMmsUtil::TrimSpace(std::string &num)
{
    std::string word;
    std::stringstream streamNum(num);
    std::string store;
    while (streamNum >> word) {
        store += word;
    }
    num = store;
}
 
int32_t RdbSmsMmsUtil::FormatSmsNumber(const std::string &num, std::string countryCode,
    const i18n::phonenumbers::PhoneNumberUtil::PhoneNumberFormat formatInfo, std::string &formatNum)
{
    if (num.empty()) {
        DATA_STORAGE_LOGE("num is nullptr!");
        return 1;
    }
    i18n::phonenumbers::PhoneNumberUtil *phoneUtils = i18n::phonenumbers::PhoneNumberUtil::GetInstance();
    if (phoneUtils == nullptr) {
        DATA_STORAGE_LOGE("phoneUtils is nullptr");
        return 1;
    }
    transform(countryCode.begin(), countryCode.end(), countryCode.begin(), ::toupper);
    i18n::phonenumbers::PhoneNumber parseResult;
    phoneUtils->Parse(num, countryCode, &parseResult);
    if (phoneUtils->IsValidNumber(parseResult)) {
        phoneUtils->Format(parseResult, formatInfo, &formatNum);
    } else {
        std::string numTemp = "";
        numTemp.assign(num);
        CbnFormat(numTemp, formatInfo, formatNum);
    }
    if (formatNum.empty() || formatNum == "0") {
        return 0;
    }
    TrimSpace(formatNum);
    return 0;
}
 
std::string RdbSmsMmsUtil::QuerySession()
{
    std::string sql;
    sql.append("select * from ");
    sql.append(TABLE_SESSION);
    return sql;
}
 
std::string RdbSmsMmsUtil::DeleteSession(int sessionId)
{
    std::string sql;
    sql.append("DELETE from ");
    sql.append(TABLE_SESSION);
    sql.append(" WHERE ( id = ");
    sql += std::to_string(sessionId);
    sql.append(" )");
    return sql;
}
 
std::string RdbSmsMmsUtil::QueryRcsInfo(int sessionId)
{
    std::string sql;
    sql.append("select * from ");
    sql.append(TABLE_RCS_INFO);
    sql.append(" where session_id = ");
    sql += std::to_string(sessionId);
    return sql;
}
 
std::string RdbSmsMmsUtil::DeleteRcsInfo(int rcsId)
{
    std::string sql;
    sql.append("DELETE from ");
    sql.append(TABLE_RCS_INFO);
    sql.append(" WHERE ( rcs_id = ");
    sql += std::to_string(rcsId);
    sql.append(" )");
    return sql;
}
 
std::string RdbSmsMmsUtil::QuerySmsMmsInfo(int32_t sessionId, int32_t rcsId,
    int32_t groupId)
{
    std::string sql;
    sql.append("select * from ");
    sql.append(TABLE_SMS_MMS_INFO);
    sql.append(" where ( session_id = ");
    sql += std::to_string(sessionId);
    if (rcsId != 0 && groupId != 0) {
        sql.append(" and rcs_id = ");
        sql += std::to_string(rcsId);
        sql.append(" and group_id = ");
        sql += std::to_string(groupId);
    }
    sql.append(" )");
    return sql;
}
 
std::string RdbSmsMmsUtil::DeleteSmsMmsInfo(int msgId)
{
    std::string sql;
    sql.append("delete from ");
    sql.append(TABLE_SMS_MMS_INFO);
    sql.append(" WHERE msg_id = ");
    sql += std::to_string(msgId);
    return sql;
}
 
std::string RdbSmsMmsUtil::QueryMmsPartInfo(int32_t msgId, int32_t rcsId, int32_t groupId)
{
    std::string sql;
    sql.append("select * from ");
    sql.append(TABLE_MMS_PART);
    sql.append(" where group_id = ");
    sql += std::to_string(groupId);
    if (rcsId > 0) {
        sql.append(" and rcs_id = ");
        sql += std::to_string(rcsId);
    } else {
        sql.append(" and msg_id = ");
        sql += std::to_string(msgId);
    }
    return sql;
}
 
std::string RdbSmsMmsUtil::DeleteMmsPartInfo(int id)
{
    std::string sql;
    sql.append("DELETE from ");
    sql.append(TABLE_MMS_PART);
    sql.append(" WHERE ( id = ");
    sql += std::to_string(id);
    sql.append(" )");
    return sql;
}
 
std::string RdbSmsMmsUtil::QueryRiskUrlRecord(int32_t sessionId, int32_t rcsId,
    int32_t msgId)
{
    std::string sql;
    sql.append("select * from ");
    sql.append(TABLE_RISK_URL_RECORD);
    sql.append(" where ( session_id = ");
    sql += std::to_string(sessionId);
    if (rcsId > 0) {
        sql.append(" and rcs_id = ");
        sql += std::to_string(rcsId);
    }
    sql.append(" and msg_id = ");
    sql += std::to_string(msgId);
    sql.append(" )");
    return sql;
}
 
std::string RdbSmsMmsUtil::DeleteRiskUrlRecord(int id)
{
    std::string sql;
    sql.append("DELETE from ");
    sql.append(TABLE_RISK_URL_RECORD);
    sql.append(" WHERE ( id = ");
    sql += std::to_string(id);
    sql.append(" )");
    return sql;
}
 
std::string RdbSmsMmsUtil::QuerySmsSubsection()
{
    std::string sql;
    sql.append("select * from ");
    sql.append(TABLE_SMS_SUBSECTION);
    return sql;
}
 
std::string RdbSmsMmsUtil::DeleteSmsSubsection(int id)
{
    std::string sql;
    sql.append("DELETE from ");
    sql.append(TABLE_SMS_SUBSECTION);
    sql.append(" WHERE ( id = ");
    sql += std::to_string(id);
    sql.append(" )");
    return sql;
}
 
std::string RdbSmsMmsUtil::QuerySessionE(std::string phone)
{
    std::string likePhone;
    if (phone.length() > PHONE_CMP_LENGTH) {
        likePhone = phone.substr(phone.length() - PHONE_CMP_LENGTH, PHONE_CMP_LENGTH);
    } else {
        likePhone = phone;
    }
    std::string sql;
    sql.append("select * from ");
    sql.append(TABLE_SESSION);
    sql.append(" WHERE ( telephone like '%");
    sql += likePhone;
    sql.append("' )");
    return sql;
}
 
std::string RdbSmsMmsUtil::QueryMmsPdu()
{
    std::string sql;
    sql.append("select * from ");
    sql.append(TABLE_MMS_PDU);
    return sql;
}
 
std::string RdbSmsMmsUtil::DeleteMmsPdu(int id)
{
    std::string sql;
    sql.append("delete from ");
    sql.append(TABLE_MMS_PDU);
    sql.append(" where id = ");
    sql += std::to_string(id);
    return sql;
}
 
std::string RdbSmsMmsUtil::QuerySmsMmsInfoBlocked()
{
    std::string sql;
    sql.append("select * from ");
    sql.append(TABLE_SMS_MMS_INFO);
    sql.append(" where is_blocked = 1");
    return sql;
}
 
std::string RdbSmsMmsUtil::QueryRcsInfoBlocked()
{
    std::string sql;
    sql.append("select * from ");
    sql.append(TABLE_RCS_INFO);
    sql.append(" where is_blocked = 1");
    return sql;
}
 
std::string RdbSmsMmsUtil::QueryUncompletedMessage()
{
    std::string sql;
    sql.append("select start_time from ");
    sql.append(TABLE_SMS_MMS_INFO);
    sql.append(" where sms_type!=99 and msg_state!=0 and msg_state!=2 union all ");
    sql.append("select start_time from ");
    sql.append(TABLE_RCS_INFO);
    sql.append(" where msg_state!=0 and msg_state!=2 and msg_state!=13");
    return sql;
}
 
std::string RdbSmsMmsUtil::QueryCompletedMessage()
{
    std::string sql;
    sql.append("select count(*) as msgCount from ");
    sql.append(TABLE_SMS_MMS_INFO);
    return sql;
}
 
void RdbSmsMmsUtil::NotifySmsMmsStatus(std::string action, int code)
{
    AAFwk::Want want;
    want.SetAction(action);
    EventFwk::CommonEventData data;
    data.SetWant(want);
    data.SetCode(code);
    EventFwk::CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(false);
    publishInfo.SetOrdered(false);
    std::vector<std::string> permissions;
    permissions.emplace_back(Permission::READ_MESSAGES);
    publishInfo.SetSubscriberPermissions(permissions);
    bool ret = EventFwk::CommonEventManager::PublishCommonEvent(data, publishInfo, nullptr);
    DATA_STORAGE_LOGI("Publish %{public}s result=%{public}d", action.c_str(), ret);
}
 
std::string RdbSmsMmsUtil::SmsSubsectionCheckSql(std::string recvNumber, std::string senderNumber,
    std::string startTime, int32_t id, int32_t slotId, int32_t subsectionIndex)
{
    std::string sql;
    sql.append("select * from ");
    sql.append(TABLE_SMS_SUBSECTION);
    sql.append(" WHERE ( receiver_number = '");
    sql += recvNumber;
    sql.append("' and sender_number = '");
    sql += senderNumber;
    sql.append("' and startTime = '");
    sql += startTime;
    sql.append("' and sms_subsection_id = ");
    sql += std::to_string(id);
    sql.append("' and slot_id = ");
    sql += std::to_string(slotId);
    sql.append("' and subsection_index = ");
    sql += std::to_string(subsectionIndex);
    sql.append(" )");
    return sql;
}
 
std::string RdbSmsMmsUtil::MmsPduCheckSql(std::string pduContent)
{
    std::string sql;
    sql.append("select * from ");
    sql.append(TABLE_MMS_PDU);
    sql.append(" WHERE ( pdu_content = '");
    sql += pduContent;
    sql.append("' )");
    return sql;
}
 
std::string RdbSmsMmsUtil::RcsInfoChecKSql(std::string msgId, std::string startTime, int32_t groupId)
{
    std::string sql;
    sql.append("select * from ");
    sql.append(TABLE_RCS_INFO);
    sql.append(" WHERE  msg_id = '");
    sql += msgId;
    sql.append("' and start_time = '");
    sql += startTime;
    sql.append("'");
    return sql;
}
 
std::string RdbSmsMmsUtil::SmsMmsInfoCheckSql(int32_t msgType, std::string msgTitle, std::string msgContent,
    std::string phoneNumber, std::string startTime)
{
    std::string sql;
    sql.append("select * from ");
    sql.append(TABLE_SMS_MMS_INFO);
    sql.append(" WHERE ( msg_type = ");
    sql += std::to_string(msgType);
    sql.append(" and msg_title = '");
    sql += msgTitle;
    sql.append("' and msg_content = '");
    sql += msgContent;
    sql.append("' and phone_number = '");
    sql += phoneNumber;
    sql.append("' and start_time = '");
    sql += startTime;
    sql.append("' )");
    return sql;
}
 
std::string RdbSmsMmsUtil::MmsPartCheckSql(int32_t msgId, int32_t groupId, int32_t type, int32_t rcsId,
    int32_t partSize, std::string locationPath)
{
    std::string sql;
    sql.append("select * from ");
    sql.append(TABLE_MMS_PART);
    sql.append(" WHERE ( msg_id = ");
    sql += std::to_string(msgId);
    sql.append(" and group_id = ");
    sql += std::to_string(groupId);
    sql.append(" and type = ");
    sql += std::to_string(type);
    if (rcsId > 0) {
        sql.append(" and rcs_id = ");
        sql += std::to_string(rcsId);
    }
    sql.append(" and part_size = ");
    sql += std::to_string(partSize);
    sql.append(" and location_path = '");
    sql += locationPath;
    sql.append("' )");
    return sql;
}
std::string RdbSmsMmsUtil::RiskUrlRecordCheckSql(int32_t msgId, int32_t sessionId, int32_t rcsId,
    std::string url, std::string createTime)
{
    std::string sql;
    sql.append("select * from ");
    sql.append(TABLE_RISK_URL_RECORD);
    sql.append(" WHERE ( msg_id = ");
    sql += std::to_string(msgId);
    sql.append(" and session_id = ");
    sql += std::to_string(sessionId);
    if (rcsId > 0) {
        sql.append(" and rcs_id = ");
        sql += std::to_string(rcsId);
    }
    sql.append(" and risk_url = '");
    sql += url;
    sql.append("' and create_time = '");
    sql += createTime;
    sql.append("' )");
    return sql;
}
} // namespace Telephony
} // namespace OHOS