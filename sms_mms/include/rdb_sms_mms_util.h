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
#ifndef RDB_SMS_MMS_UTIL_H
#define RDB_SMS_MMS_UTIL_H
 
#include <string>
#include "phonenumbers/phonenumber.pb.h"
#include "phonenumbers/phonenumberutil.h"
namespace OHOS {
namespace Telephony {
class SmsRdbEventSupport {
public:
    static const std::string SMS_RDB_EVENT_STORE_CHANGED;
};
 
class RdbSmsMmsUtil {
public:
    static void CbnFormat(std::string &numTemp,
    const i18n::phonenumbers::PhoneNumberUtil::PhoneNumberFormat formatInfo,
        std::string &formatNum);
    static void TrimSpace(std::string &num);
    static int32_t FormatSmsNumber(const std::string &num, std::string countryCode,
        const i18n::phonenumbers::PhoneNumberUtil::PhoneNumberFormat formatInfo, std::string &formatNum);
    static std::string QuerySession();
    static std::string DeleteSession(int sessionId);
    static std::string QueryRcsInfo(int sessionId);
    static std::string DeleteRcsInfo(int rcsId);
    static std::string QuerySmsMmsInfo(int32_t sessionId, int32_t rcsId, int32_t groupId);
    static std::string DeleteSmsMmsInfo(int msgId);
    static std::string QueryMmsPartInfo(int32_t msgId, int32_t rcsId, int32_t groupId);
    static std::string DeleteMmsPartInfo(int id);
    static std::string QueryRiskUrlRecord(int32_t sessionId, int32_t rcsId, int32_t msgId);
    static std::string DeleteRiskUrlRecord(int id);
    static std::string QuerySmsSubsection();
    static std::string DeleteSmsSubsection(int id);
    static std::string QueryMmsPdu();
    static std::string DeleteMmsPdu(int id);
    static std::string QuerySessionE(std::string phone);
    static std::string QuerySmsMmsInfoBlocked();
    static std::string QueryRcsInfoBlocked();
    static std::string QueryCompletedMessage();
    static std::string QueryUncompletedMessage();
    static void NotifySmsMmsStatus(std::string action, int code = 0);
    static std::string SmsSubsectionCheckSql(std::string recvNumber, std::string senderNumber,
        std::string startTime, int32_t id, int32_t slotId, int32_t subsectionIndex);
    static std::string MmsPduCheckSql(std::string pduContent);
    static std::string RcsInfoChecKSql(std::string msgId, std::string startTime, int32_t groupId);
    static std::string SmsMmsInfoCheckSql(int32_t msgType, std::string msgTitle, std::string msgContent,
        std::string phoneNumber, std::string startTime);
    static std::string MmsPartCheckSql(int32_t msgId, int32_t groupId, int32_t type, int32_t rcsId,
        int32_t partSize, std::string locationPath);
    static std::string RiskUrlRecordCheckSql(int32_t msgId, int32_t sessionId, int32_t rcsId,
        std::string url, std::string createTime);
};
}
}
#endif