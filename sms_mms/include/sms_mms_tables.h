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

#ifndef CODE_SMS_MMS_TABLES_H
#define CODE_SMS_MMS_TABLES_H

#include <vector>
#include <string>

namespace OHOS {
namespace Telephony {
class RdbSmsMmsTables {
public:
    std::vector<std::string> InitCreateTableVec();

    void CreateSmsMmsFavoriteTableStr(std::string &createTableStr);
private:
    /**
     * Create SmsMmsInfo table
     *
     * @param createTableStr Create table statement
     */
    void CreateSmsMmsInfoTableStr(std::string &createTableStr);

    /**
     * Create RcsInfo table
     *
     * @param createTableStr Create table statement
     */
    void CreateRcsInfoTableStr(std::string &createTableStr);

    /**
     * Create ChatBots table
     *
     * @param createTableStr Create table statement
     */
    void CreateChatBotsTableStr(std::string &createTableStr);

    /**
     * Create Specific_ChatBots table
     *
     * @param createTableStr Create table statement
     */
    void CreateSpecificChatBotsTableStr(std::string &createTableStr);

    /**
     * Create Smc table
     *
     * @param createSmcTableStr Create table statement
     */
    void CreateSmcTableStr(std::string &createTableStr);

    /**
     * Create SmsSubsection table
     *
     * @param createTableStr Create table statement
     */
    void CreateSmsSubsectionTableStr(std::string &createTableStr);

    /**
     * Create MmsProtocol table
     *
     * @param createTableStr Create table statement
     */
    void CreateMmsProtocolTableStr(std::string &createTableStr);

    /**
     * Create MmsPart table
     *
     * @param createTableStr Create table statement
     */
    void CreateMmsPartTableStr(std::string &createTableStr);

    /**
     * Create Session table
     *
     * @param createTableStr Create table statement
     */
    void CreateSessionTableStr(std::string &createTableStr);

    /**
     * Create MmsPdu table
     * @param createTableStr indicates Create table statement
     */
    void CreateMmsPduTableStr(std::string &createTableStr);

    /**
     * Create RiskUrlRecord table
     * @param createTableStr indicates Create table statement
     */
    void CreateRiskUrlRecordTableStr(std::string &createTableStr);

    /**
     * Create MmsInfo view
     * @param createTableStr Create view statement
     */
    void CreateMmsInfoViewStr(std::string &createTableStr);

    /**
     * Create Trigger view
     * @param createInsertSmsTrigger Create trigger statement
     */
    void CreateInsertSmsTriggerStr(std::string &createInsertSmsTrigger);

    /**
     * Create Trigger view
     * @param createInsertRcsTrigger Create trigger statement
     */
    void CreateInsertRcsTriggerStr(std::string &createInsertRcsTrigger);

    /**
     * Create Trigger view
     * @param createUpdateBlockSmsTrigger Create trigger statement
     */
    void CreateUpdateBlockSmsTriggerStr(std::string &createUpdateBlockSmsTrigger);

    /**
     * Create Trigger view
     * @param createUpdateBlockRcsTrigger Create trigger statement
     */
    void CreateUpdateBlockRcsTriggerStr(std::string &createUpdateBlockRcsTrigger);

    /**
     * InitCreateTableIndex
     * @param InitCreateTableIndex CreateTableIndex
     */
    void InitCreateTableIndex(std::vector<std::string> &tables);

private:
    const std::string  VERSION_22_DROP_INDEX_ON_RCS_TABLE = "DROP INDEX if exists rcs_info_sessionId_IDX;";
    const std::string  VERSION_22_CREATE_INDEX_ON_RCS_TABLE =
            "CREATE INDEX if not exists rcs_info_sessionId_IDX ON rcs_info (session_id, is_blocked);";
    const std::string  VERSION_22_DROP_INDEX_ON_SMS_MMS_INFO_TABLE =
            "DROP INDEX if exists sms_mms_info_sessionId_IDX;";
    const std::string  VERSION_22_CREATE_INDEX_ON_SMS_MMS_INFO_TABLE =
            "CREATE INDEX if not exists sms_mms_info_sessionId_IDX ON sms_mms_info (session_id,is_blocked,sms_type);";
    const std::string  VERSION_22_DROP_INDEX_ON_SESSION_TABLE = "DROP INDEX if exists session_time_IDX;";
    const std::string  VERSION_22_CREATE_INDEX_ON_SESSION_TABLE =
            "CREATE INDEX if not exists session_time_IDX ON session (time desc,sms_type desc);";
};
}
}
#endif //CODE_SMS_MMS_TABLES_H
