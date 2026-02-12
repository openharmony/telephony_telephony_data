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

#ifndef DATA_STORAGE_RDB_SMS_MMS_CALLBACK_H
#define DATA_STORAGE_RDB_SMS_MMS_CALLBACK_H

#include "iosfwd"
#include "rdb_base_callback.h"
#include "vector"
#include "sms_mms_data.h"
#include <fstream>
#include "../../common/include/rdb_sms_mms_dfx_adapter.h"

namespace OHOS {
namespace NativeRdb {
class RdbStore;
}
namespace Telephony {
class RdbSmsMmsCallback : public RdbBaseCallBack {
public:
    RdbSmsMmsCallback(const std::vector<std::string> &createTableVec) : RdbBaseCallBack(createTableVec) {}
    ~RdbSmsMmsCallback() = default;

    /**
     * @brief Called when the database associate whit this RdbStore needs to be upgrade.
     *
     * @param rdbStore Indicates the RdbStore object.
     * @param currentVersion Indicates the old database version.
     * @param targetVersion Indicates the new database version.
     */
    int OnUpgrade(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion) override;

    /**
     * @brief Called when the database associate whit this RdbStore needs to be downgrade.
     *
     * @param rdbStore Indicates the RdbStore object.
     * @param currentVersion Indicates the old database version.
     * @param targetVersion Indicates the new database version.
     */
    int OnDowngrade(NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion) override;

    /**
     * @brief Called when the RdbStore has been opened.
     *
     * @param rdbStore Indicates the RdbStore object.
     */
    int OnOpen(NativeRdb::RdbStore &rdbStore) override;

    void CreateMmsInfoViewStrCb(std::string &createTableStr);
    void CreateTableSmcInfoUrlRecordTableStrCb(std::string &createTableStr);
    void CreateTableSpecificChatBotsInfoUrlRecordTableStrCb(std::string &createTableStr);
    void CreateTableChatBotsInfoUrlRecordTableStrCb(std::string &createTableStr);
    void CreateRcsInfoTableForcely(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion);
    void CreateRiskUrlRecordTableStrCb(std::string &createTableStr);

    void AlterRcsInfoTableForcely(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion);
    int VersionUp(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion);
    void CheckColumn(NativeRdb::RdbStore &rdbStore);
    void CreateUpdateSmsTriggerStrCb(std::string &createUpdateSmsTriggerStr);
    void CreateInsertSmsTriggerStrCb(std::string &createInsertSmsTriggerStr);
    void CreateUpdateRcsTriggerStrCb(std::string &createUpdateRcsTriggerStr);
    void CreateInsertRcsTriggerStrCb(std::string &createInsertRcsTriggerStr);
    void CreateUpdateBlockSmsTriggerStrCb(std::string &createUpdateBlockSmsTriggerStr);
    void CreateUpdateBlockRcsTriggerStrCb(std::string &createUpdateBlockRcsTriggerStr);
private:
    int VersionUpToEight(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion);
    int VersionUpToNine(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion);
    int VersionUpToTen(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion);
    int VersionUpToEleven(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion);
    int VersionUpToTwelve(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion);
    int VersionUpToThirteen(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion);
    int VersionUpToFourteen(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion);
    int VersionUpToFifteen(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion);
    int VersionUpTo16(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion);
    int VersionUpToSeventeen(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion);
    int VersionUpTo18(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion);
    int VersionUpTo19(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion);
    int VersionUpTo20(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion);
    int VersionUpTo21(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion);
    void RcsInfoInsertSmsMmsInfo(NativeRdb::RdbStore &rdbStore);
	std::shared_ptr<NativeRdb::ResultSet> QueryRcsInfo(NativeRdb::RdbStore &rdbStore);
    void InsertSmsMmsInfoFromResultSet(NativeRdb::RdbStore &rdbStore, std::shared_ptr<NativeRdb::ResultSet> resultSet);
    NativeRdb::ValuesBucket GetValuesFromRow(std::shared_ptr<NativeRdb::ResultSet> resultSet, int row);
    void ProcessRcsInfoToSmsInfo(NativeRdb::RdbStore &rdbStore, NativeRdb::ResultSet);
    int VersionUpTo22(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion);
    int VersionUpTo23(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion);
    int VersionUpTo24(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion);
    int VersionUpTo25(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion);
    int VersionUpTo26(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion);
    int VersionUpTo27(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion);
    int VersionUpTo28(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion);
    int VersionUpTo29(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion);
    int VersionUpTo30(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion);
    int VersionUpTo31(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion);
    int VersionUpTo32(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion);
    int VersionUpTo33(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion);
    int VersionUpTo34(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion);
    int VersionUpTo35(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion);
private:
    struct FavoriteExtendedInfo {
        int32_t msgId;
        int32_t rcsId;
        int32_t partType;
        std::string partLocationPath;
        std::string recordingTime;
        std::string partSize;
    };
    void CreateFieldList(std::vector<std::string> &columnsInt, std::vector<std::string> &columnsStr);
    int GetExtendedInfoFromMmsPart(NativeRdb::RdbStore &rdb, FavoriteExtendedInfo &info);
    int GetHistoryFavoriteItem(NativeRdb::RdbStore &rdb, std::vector<NativeRdb::ValuesBucket> &values);
    int32_t GetRcsInfo(NativeRdb::RdbStore &rdb, int32_t rcsId, int32_t &rcsType,
        std::string &msgTitle, std::string &msgContent);
    void FavoriteReadIntData(std::shared_ptr<NativeRdb::ResultSet> &resultSet, NativeRdb::ValuesBucket &vb,
        std::vector<std::string> &columnsInt, FavoriteExtendedInfo &info, int32_t &msgType);
    void FavoriteReadStringData(std::shared_ptr<NativeRdb::ResultSet> &resultSet, NativeRdb::ValuesBucket &vb,
        std::vector<std::string> &columnsStr, std::string &msgTitle, std::string &msgContent);
    bool IsFavTableExist(NativeRdb::RdbStore &rdb);
    void processUpdateSessionSendState(NativeRdb::RdbStore &rdbStore);
    bool IsFieldExistInTable(NativeRdb::RdbStore &rdb, std::string table, std::string field);
    void CheckAndCompleteFields(NativeRdb::RdbStore &rdbStore);
    bool IsComponentExistInDb(NativeRdb::RdbStore &rdb, const std::string &componentType,
        const std::string &componentName);
    void CheckAndCompleteComponent(NativeRdb::RdbStore &rdbStore);
	void DoBackUpDB(NativeRdb::RdbStore &rdbStore);
    void CheckAndCompleteTrigger(NativeRdb::RdbStore &rdbStore);
    int ExecuteSqlAndReportEvent(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion,
        std::string sql);
    bool IsTableExist(NativeRdb::RdbStore &rdb, const std::string &table);
    void CheckIncompleteTable(NativeRdb::RdbStore &rdbStore);
    bool IsDbComplete(NativeRdb::RdbStore &rdbStore);
    void RestoreAllInfo(NativeRdb::RdbStore &rdbStore);

private:
    const int VERSION_2 = 2;
    const int VERSION_3 = 3;
    const int VERSION_4 = 4;
    const int VERSION_5 = 5;
    const int VERSION_6 = 6;
    const int VERSION_7 = 7;
    const int VERSION_8 = 8;
    const int VERSION_9 = 9;
    const int VERSION_10 = 10;
    const int VERSION_11 = 11;
    const int VERSION_12 = 12;
    const int VERSION_13 = 13;
    const int VERSION_14 = 14;
    const int VERSION_15 = 15;
    const int VERSION_16 = 16;
    const int VERSION_17 = 17;
    const int VERSION_18 = 18;
    const int VERSION_19 = 19;
    const int VERSION_20 = 20;
    const int VERSION_21 = 21;
    const int VERSION_22 = 22;
    const int VERSION_23 = 23;
    const int VERSION_24 = 24;
    const int VERSION_25 = 25;
    const int VERSION_26 = 26;
    const int VERSION_27 = 27;
    const int VERSION_28 = 28;
    const int VERSION_29 = 29;
    const int VERSION_30 = 30;
    const int VERSION_31 = 31;
    const int VERSION_32 = 32;
    const int VERSION_33 = 33;
    const int VERSION_34 = 34;
    const int VERSION_35 = 35;
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

    const std::vector<std::string> RCS_INFO_COLUMNS = {
        RcsInfo::RCS_ID,
        RcsInfo::IS_SENDER,
        RcsInfo::START_TIME,
        RcsInfo::MSG_CONTENT,
        RcsInfo::IS_COLLECT,
        RcsInfo::IS_READ,
        RcsInfo::SESSION_ID
    };
    RdbSmsMmsDFXAdapter dfxAdapter_;
    const int RCS_TYPE_VALUE = 99;
    bool needBackup = false;
    const std::string BAK_DB_NAME = "sms_mms.db.bak";
    const std::string DB_PATH = "/data/storage/el2/database/";
};
} // namespace Telephony
} // namespace OHOS
#endif // DATA_STORAGE_RDB_SMS_MMS_CALLBACK_H
