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

#include "rdb_sms_mms_callback.h"

#include "data_storage_log_wrapper.h"
#include "rdb_errno.h"
#include "sms_mms_tables.h"
#include <sstream>

namespace OHOS {
namespace NativeRdb {
class RdbStore;
}
namespace Telephony {
// 只有需要升级的时候，才会调用
int RdbSmsMmsCallback::OnUpgrade(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
{
    DATA_STORAGE_LOGW(
        "Data_Storage RdbSmsMmsCallback::OnUpgrade##oldVersion = %{public}d, "
        "newVersion = %{public}d\n",
        oldVersion, newVersion);
    int oriOldVersion = oldVersion;
    CreateRcsInfoTableForcely(rdbStore, oldVersion, newVersion);
    if (oldVersion < VERSION_2 && newVersion >= VERSION_2) {
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_SMS_MMS_INFO) +
                                 " ADD COLUMN " + std::string(SmsMmsInfo::IS_ADVANCED_SECURITY) +
                                 " INTEGER DEFAULT " + "0;");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_RCS_INFO) +
                                 " ADD COLUMN " + std::string(RcsInfo::IS_ADVANCED_SECURITY) + " INTEGER DEFAULT "
                                 + "0;");
        oldVersion = VERSION_2;
    }
    if (oldVersion < VERSION_3 && newVersion >= VERSION_3) {
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_SESSION) +
                                 " ADD COLUMN " + std::string(Session::CONTACT_ID) + " TEXT DEFAULT " + "'';");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_SESSION) +
                                 " ADD COLUMN " + std::string(Session::CONTACT_NAME) + " TEXT DEFAULT " + "'';");
        oldVersion = VERSION_3;
    }
    if (oldVersion < VERSION_4 && newVersion >= VERSION_4) {
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_RCS_INFO) +
                                 " ADD COLUMN " + std::string(RcsInfo::IS_COLLECT) + " INTEGER DEFAULT " + "0;");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_RCS_INFO) +
                                 " ADD COLUMN " + std::string(RcsInfo::RECEIVE_STATE) + " INTEGER DEFAULT " + "0;");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_RCS_INFO) +
                                 " ADD COLUMN " + std::string(RcsInfo::FAIL_RECEIVE_CONTEXT) + " TEXT DEFAULT " +
                                 "'';");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_SMS_MMS_INFO) +
                                 " ADD COLUMN " + std::string(SmsMmsInfo::EXPIRES_TIME) + " INTEGER DEFAULT " + "0;");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_SMS_MMS_INFO) +
                                 " ADD COLUMN " + std::string(SmsMmsInfo::MMS_PDU) + " TEXT DEFAULT " + "'';");
        oldVersion = VERSION_4;
    }
    oldVersion = VersionUp(rdbStore, oldVersion, newVersion);
    CheckColumn(rdbStore);
    if (oldVersion != newVersion) {
        needBackup = false;
        DATA_STORAGE_LOGE("upgrade error oldVersion = %{public}d, newVersion = %{public}d ", oldVersion, newVersion);
        return NativeRdb::E_ERROR;
    } else {
        needBackup = (oriOldVersion == newVersion) ? false : true;
    }
    return NativeRdb::E_OK;
}

int RdbSmsMmsCallback::VersionUpToNine(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
{
    DATA_STORAGE_LOGI("VersionUpToNine oldVersion = %{public}d, newVersion = %{public}d ", oldVersion, newVersion);
    if (oldVersion < VERSION_9 && newVersion >= VERSION_9) {
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_MMS_PART) +
                                 " ADD COLUMN " + std::string(MmsPart::RCS_ID) + " INTEGER DEFAULT " + "0;");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_RCS_INFO) +
                                 " ADD COLUMN " + std::string(RcsInfo::CLUR_SIZE) + " INTEGER DEFAULT " + "0;");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_RCS_INFO) +
                                 " ADD COLUMN " + std::string(RcsInfo::TOTAL_SIZE) + " INTEGER DEFAULT " + "0;");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "DROP VIEW IF EXISTS " +
                                 std::string(TABLE_MMS_INFO));
        std::string mms_InfoStr;
        CreateMmsInfoViewStrCb(mms_InfoStr);
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, mms_InfoStr);
        oldVersion = VERSION_9;
    }

    return oldVersion;
}

int RdbSmsMmsCallback::VersionUpToTen(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
{
    DATA_STORAGE_LOGI("VersionUpToTen oldVersion = %{public}d, newVersion = %{public}d ", oldVersion, newVersion);
    if (oldVersion < VERSION_10 && newVersion >= VERSION_10) {
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_SMS_MMS_INFO) +
                                 " ADD COLUMN " + std::string(SmsMmsInfo::RISK_URL_BODY) + " TEXT DEFAULT " + "'';");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_SMS_MMS_INFO) +
                                 " ADD COLUMN " + std::string(SmsMmsInfo::HAS_BE_REPORTED_AS_SPAM) +
                                 " INTEGER DEFAULT " + "0;");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_RCS_INFO) +
                                 " ADD COLUMN " + std::string(RcsInfo::RISK_URL_BODY) + " TEXT DEFAULT " + "'';");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_RCS_INFO) +
                                 " ADD COLUMN " + std::string(RcsInfo::HAS_BE_REPORTED_AS_SPAM) +
                                 " INTEGER DEFAULT " + "0;");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "DROP VIEW IF EXISTS " +
                                 std::string(TABLE_MMS_INFO));
        std::string mms_InfoStr;
        CreateMmsInfoViewStrCb(mms_InfoStr);
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, mms_InfoStr);

        std::string risk_url_recordStr;
        CreateRiskUrlRecordTableStrCb(risk_url_recordStr);
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, risk_url_recordStr);
        oldVersion = VERSION_10;
    }

    return oldVersion;
}
 
void RdbSmsMmsCallback::CreateFieldList(std::vector<std::string> &columnsInt, std::vector<std::string> &columnsStr)
{
    columnsInt.push_back(FavoriteInfo::MSG_ID);
    columnsInt.push_back(FavoriteInfo::MSG_TYPE);
    columnsInt.push_back(FavoriteInfo::SMS_TYPE);
    columnsInt.push_back(FavoriteInfo::SESSION_TYPE);
    columnsInt.push_back(FavoriteInfo::GROUP_ID);
    columnsInt.push_back(FavoriteInfo::IS_SENDER);
    columnsInt.push_back(FavoriteInfo::RCS_ID);
    columnsInt.push_back(FavoriteInfo::SESSION_ID);
    columnsStr.push_back(FavoriteInfo::RECEIVER_NUMBER);
    columnsStr.push_back(FavoriteInfo::SENDER_NUMBER);
    columnsStr.push_back(FavoriteInfo::START_TIME);
    columnsStr.push_back(FavoriteInfo::MSG_TITLE);
    columnsStr.push_back(FavoriteInfo::MSG_CONTENT);
    columnsStr.push_back(FavoriteInfo::DETECT_RES_CONTENT);
}

int RdbSmsMmsCallback::GetExtendedInfoFromMmsPart(NativeRdb::RdbStore &rdb, FavoriteExtendedInfo &info)
{
    int32_t columnIndex = 0;
    std::vector<std::string> columns;
    NativeRdb::AbsRdbPredicates queryPredicates(TABLE_MMS_PART);
    columns.push_back(MmsPart::RECORDING_TIME);
    columns.push_back(MmsPart::PART_SIZE);
    columns.push_back(MmsPart::TYPE);
    columns.push_back(MmsPart::LOCATION_PATH);
    if (info.rcsId != 0) {
        queryPredicates.EqualTo(SmsMmsInfo::RCS_ID, info.rcsId);
    } else if (info.msgId != 0) {
        queryPredicates.EqualTo(SmsMmsInfo::MSG_ID, info.msgId);
    } else {
        return 1;
    }
    queryPredicates.NotEqualTo(MmsPart::TYPE, 0);
    std::shared_ptr<NativeRdb::ResultSet> resultSet = rdb.Query(queryPredicates, columns);
    if (resultSet == nullptr) {
        return 1;
    }
    if (resultSet->GoToFirstRow() != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("GoToFirstRow failed");
        resultSet->Close();
        return -1;
    }
    resultSet->GetColumnIndex(MmsPart::TYPE, columnIndex);
    resultSet->GetInt(columnIndex, info.partType);
    resultSet->GetColumnIndex(MmsPart::RECORDING_TIME, columnIndex);
    resultSet->GetString(columnIndex, info.recordingTime);
    resultSet->GetColumnIndex(MmsPart::PART_SIZE, columnIndex);
    resultSet->GetString(columnIndex, info.partSize);
    resultSet->GetColumnIndex(MmsPart::LOCATION_PATH, columnIndex);
    resultSet->GetString(columnIndex, info.partLocationPath);
    resultSet->Close();
    return 0;
}

void RdbSmsMmsCallback::FavoriteReadIntData(std::shared_ptr<NativeRdb::ResultSet> &resultSet,
    NativeRdb::ValuesBucket &vb, std::vector<std::string> &columnsInt, FavoriteExtendedInfo &info, int32_t &msgType)
{
    int32_t columnIndex = 0;
    info.msgId = 0;
    info.rcsId = 0;
    for (auto &item : columnsInt) {
        int32_t valueInt = 0;
        resultSet->GetColumnIndex(item, columnIndex);
        resultSet->GetInt(columnIndex, valueInt);
        if (item == FavoriteInfo::MSG_ID) {
            info.msgId = valueInt;
        } else if (item == FavoriteInfo::RCS_ID) {
            info.rcsId = valueInt;
        } else if (item == FavoriteInfo::MSG_TYPE) {
            msgType = valueInt;
            continue;
        }
        vb.PutInt(item, valueInt);
    }
}

void RdbSmsMmsCallback::FavoriteReadStringData(std::shared_ptr<NativeRdb::ResultSet> &resultSet,
    NativeRdb::ValuesBucket &vb, std::vector<std::string> &columnsStr, std::string &msgTitle, std::string &msgContent)
{
    int32_t columnIndex = 0;
    for (auto &item : columnsStr) {
        std::string valueStr;
        resultSet->GetColumnIndex(item, columnIndex);
        resultSet->GetString(columnIndex, valueStr);
        if (item == FavoriteInfo::MSG_CONTENT) {
            msgContent = valueStr;
            continue;
        } else if (item == FavoriteInfo::MSG_TITLE) {
            msgTitle = valueStr;
            continue;
        }
        vb.PutString(item, valueStr);
    }
}

int RdbSmsMmsCallback::GetHistoryFavoriteItem(NativeRdb::RdbStore &rdb, std::vector<NativeRdb::ValuesBucket> &values)
{
    std::vector<std::string> columns;
    std::vector<std::string> columnsInt;
    std::vector<std::string> columnsStr;
    CreateFieldList(columnsInt, columnsStr);
    columns.insert(columns.end(), columnsInt.begin(), columnsInt.end());
    columns.insert(columns.end(), columnsStr.begin(), columnsStr.end());
    NativeRdb::AbsRdbPredicates queryPredicates(TABLE_SMS_MMS_INFO);
    queryPredicates.EqualTo(SmsMmsInfo::IS_COLLECT, 1);
    std::shared_ptr<NativeRdb::ResultSet> resultSet = rdb.Query(queryPredicates, columns);
    if (resultSet == nullptr) {
        return 0;
    }
    while (resultSet->GoToNextRow() == NativeRdb::E_OK) {
        int32_t msgType = 0;
        std::string msgTitle = "";
        std::string msgContent = "";
        NativeRdb::ValuesBucket vb;
        FavoriteExtendedInfo info;
        FavoriteReadIntData(resultSet, vb, columnsInt, info, msgType);
        FavoriteReadStringData(resultSet, vb, columnsStr, msgTitle, msgContent);
        vb.PutString(FavoriteInfo::KEEP_TIME, std::to_string(time(0)));
        if (GetExtendedInfoFromMmsPart(rdb, info) == 0) {
            vb.PutString(FavoriteInfo::RECORDING_TIME, info.recordingTime);
            vb.PutString(FavoriteInfo::PART_SIZE, info.partSize);
            vb.PutString(FavoriteInfo::PART_LOCATION_PATH, info.partLocationPath);
            vb.PutInt(FavoriteInfo::PART_TYPE, info.partType);
        }
        if (info.rcsId != 0) {
            msgType = 0;
            msgTitle = "";
            msgContent = "";
            GetRcsInfo(rdb, info.rcsId, msgType, msgTitle, msgContent);
        }
        vb.PutInt(FavoriteInfo::MSG_TYPE, msgType);
        vb.PutString(FavoriteInfo::MSG_TITLE, msgTitle);
        vb.PutString(FavoriteInfo::MSG_CONTENT, msgContent);
        values.push_back(vb);
    }
    resultSet->Close();
    return values.size();
}

bool RdbSmsMmsCallback::IsFavTableExist(NativeRdb::RdbStore &rdb)
{
    std::shared_ptr<NativeRdb::ResultSet> resultSet =
        rdb.QuerySql("SELECT name FROM sqlite_master WHERE type='table' AND name='favorate_info';");
    if (resultSet == nullptr) {
        DATA_STORAGE_LOGE("Query fav table exist is null!");
        return false;
    }
    int count;
    resultSet->GetRowCount(count);
    resultSet->Close();
    if (count <= 0) {
        DATA_STORAGE_LOGE("Query no Query fav table");
        return false;
    }
    return true;
}

void RdbSmsMmsCallback::CheckAndCompleteFields(NativeRdb::RdbStore &rdbStore)
{
    if (!IsFieldExistInTable(rdbStore, std::string(TABLE_SMS_MMS_INFO), SmsMmsInfo::BLOCKED_PREF_NUMBER)) {
        rdbStore.ExecuteSql("ALTER TABLE " + std::string(TABLE_SMS_MMS_INFO) + " ADD COLUMN " +
            std::string(SmsMmsInfo::BLOCKED_PREF_NUMBER) + " TEXT DEFAULT " + "'';");
        rdbStore.ExecuteSql("DROP VIEW IF EXISTS " + std::string(TABLE_MMS_INFO));
        std::string mms_InfoStr;
        CreateMmsInfoViewStrCb(mms_InfoStr);
        rdbStore.ExecuteSql(mms_InfoStr);
        needBackup = true;
    }
    if (!IsFieldExistInTable(rdbStore, std::string(TABLE_RCS_INFO), SmsMmsInfo::BLOCKED_PREF_NUMBER)) {
        rdbStore.ExecuteSql("ALTER TABLE " + std::string(TABLE_RCS_INFO) + " ADD COLUMN " +
            std::string(RcsInfo::BLOCKED_PREF_NUMBER) + " TEXT DEFAULT " + "'';");
        rdbStore.ExecuteSql("DROP VIEW IF EXISTS " + std::string(TABLE_MMS_INFO));
        std::string mms_InfoStr;
        CreateMmsInfoViewStrCb(mms_InfoStr);
        rdbStore.ExecuteSql(mms_InfoStr);
        needBackup = true;
    }
    if (!IsFieldExistInTable(rdbStore, std::string(TABLE_SESSION), Session::BLOCKED_TYPE)) {
        rdbStore.ExecuteSql("ALTER TABLE " + std::string(TABLE_SESSION) + " ADD COLUMN " +
            std::string(Session::BLOCKED_TYPE) + " INTEGER DEFAULT " + "0;");
        needBackup = true;
    }
    if (!IsFieldExistInTable(rdbStore, std::string(TABLE_MMS_PART), MmsPart::IS_FRAUD)) {
        rdbStore.ExecuteSql("ALTER TABLE " + std::string(TABLE_MMS_PART) + " ADD COLUMN " +
            std::string(MmsPart::IS_FRAUD) + " INTEGER;");
        needBackup = true;
    }
    if (!IsFieldExistInTable(rdbStore, std::string(TABLE_MMS_PART), MmsPart::REFERENCE_COUNT)) {
        rdbStore.ExecuteSql("ALTER TABLE " + std::string(TABLE_MMS_PART) + " ADD COLUMN " +
            std::string(MmsPart::REFERENCE_COUNT) + " INTEGER DEFAULT " + "-1;");
		needBackup = true;
    }
    if (!IsFieldExistInTable(rdbStore, std::string(TABLE_SESSION), Session::PINNING_TIME)) {
        rdbStore.ExecuteSql("ALTER TABLE " + std::string(TABLE_SESSION) + " ADD COLUMN " +
            std::string(Session::PINNING_TIME) + " INTEGER DEFAULT " + "0;");
        needBackup = true;
    }
}

void RdbSmsMmsCallback::CheckAndCompleteTrigger(NativeRdb::RdbStore &rdbStore)
{
    if(!IsComponentExistInDb(rdbStore, "trigger", "insertSms_unread_count")) {
        std::string createInsertSmsTriggerStr;
        CreateInsertSmsTriggerStrCb(createInsertSmsTriggerStr);
        rdbStore.ExecuteSql(createInsertSmsTriggerStr);
		needBackup = true;
    }
    if(!IsComponentExistInDb(rdbStore, "trigger", "insertRcs_unread_count")) {
        std::string createInsertRcsTriggerStr;
        CreateInsertRcsTriggerStrCb(createInsertRcsTriggerStr);
        rdbStore.ExecuteSql(createInsertRcsTriggerStr);
		needBackup = true;
    }
    if(!IsComponentExistInDb(rdbStore, "trigger", "updateBlockSms_unread_count")) {
        std::string createUpdateBlockSmsTriggerStr;
        CreateUpdateBlockSmsTriggerStrCb(createUpdateBlockSmsTriggerStr);
        rdbStore.ExecuteSql(createUpdateBlockSmsTriggerStr);
		needBackup = true;
    }
    if(!IsComponentExistInDb(rdbStore, "trigger", "updateBlockRcs_unread_count")) {
        std::string createUpdateBlockRcsTriggerStr;
        CreateUpdateBlockRcsTriggerStrCb(createUpdateBlockRcsTriggerStr);
        rdbStore.ExecuteSql(createUpdateBlockRcsTriggerStr);
		needBackup = true;
    }
    if(IsComponentExistInDb(rdbStore, "trigger", "updateSms_unread_count")) {
	    rdbStore.ExecuteSql("DROP TRIGGER IF EXISTS updateSms_unread_count");
		needBackup = true;
	}
	if(IsComponentExistInDb(rdbStore, "trigger", "updateRcs_unread_count")) {
	    rdbStore.ExecuteSql("DROP TRIGGER IF EXISTS updateRcs_unread_count");
		needBackup = true;
	}
}

void RdbSmsMmsCallback::CheckAndCompleteComponent(NativeRdb::RdbStore &rdbStore)
{
    CheckAndCompleteTrigger(rdbStore);
    if(!IsComponentExistInDb(rdbStore, "view", std::string(TABLE_MMS_INFO))) {
        std::string mms_InfoStr;
        CreateMmsInfoViewStrCb(mms_InfoStr);
        rdbStore.ExecuteSql(mms_InfoStr);
		needBackup = true;
    }
    if(!IsComponentExistInDb(rdbStore, "index", "session_time_IDX")) {
        rdbStore.ExecuteSql(VERSION_22_CREATE_INDEX_ON_SESSION_TABLE);
		needBackup = true;
    }
    if(!IsComponentExistInDb(rdbStore, "index", "sms_mms_info_sessionId_IDX")) {
        rdbStore.ExecuteSql(VERSION_22_CREATE_INDEX_ON_SMS_MMS_INFO_TABLE);
		needBackup = true;
    }
    if(!IsComponentExistInDb(rdbStore, "index", "rcs_info_sessionId_IDX")) {
        rdbStore.ExecuteSql(VERSION_22_CREATE_INDEX_ON_RCS_TABLE);
		needBackup = true;
    }
}

bool RdbSmsMmsCallback::IsFieldExistInTable(NativeRdb::RdbStore &rdb, std::string table, std::string field)
{
    std::string sql;
    sql.append("SELECT name FROM pragma_table_info('");
    sql.append(table);
    sql.append("') WHERE name = '");
    sql.append(field);
    sql.append("';");
    std::shared_ptr<NativeRdb::ResultSet> resultSet = rdb.QuerySql(sql);
    if (resultSet == nullptr) {
        DATA_STORAGE_LOGE("Query %{public}s exist in %{public}s error ", field.c_str(), table.c_str());
        return false;
    }
    int count;
    resultSet->GetRowCount(count);
    resultSet->Close();
    DATA_STORAGE_LOGD("IsFieldExistInTable, %{public}s in %{public}s : %{public}d",
        field.c_str(), table.c_str(), count > 0);
    return count > 0 ? true : false;
}

bool RdbSmsMmsCallback::IsComponentExistInDb(NativeRdb::RdbStore &rdb,
    const std::string &componentType, const std::string &componentName)
{
    std::string sql;
    sql.append("SELECT name FROM sqlite_master WHERE type = '");
    sql.append(componentType);
    sql.append("' AND name = '");
    sql.append(componentName);
    sql.append("';");
    std::shared_ptr<NativeRdb::ResultSet> resultSet = rdb.QuerySql(sql);
    if (resultSet == nullptr) {
        DATA_STORAGE_LOGE("IsComponentExistInDb check %{public}s exist error ", componentName.c_str());
        return false;
    }
    int count;
    resultSet->GetRowCount(count);
    resultSet->Close();
    DATA_STORAGE_LOGD("IsComponentExistInDb, the %{public}s %{public}s exist : %{public}d",
        componentType.c_str(), componentName.c_str(), count > 0);
    return count > 0 ? true : false;
}

int32_t RdbSmsMmsCallback::GetRcsInfo(NativeRdb::RdbStore &rdb, int32_t rcsId,
    int32_t &rcsType, std::string &msgTitle, std::string &msgContent)
{
    int32_t columnIndex = 0;
    std::vector<std::string> columns;
    NativeRdb::AbsRdbPredicates queryPredicates(TABLE_RCS_INFO);
    columns.push_back(RcsInfo::RCS_TYPE);
    columns.push_back(RcsInfo::MSG_TITLE);
    columns.push_back(RcsInfo::MSG_CONTENT);
    queryPredicates.EqualTo(RcsInfo::RCS_ID, rcsId);
    std::shared_ptr<NativeRdb::ResultSet> resultSet = rdb.Query(queryPredicates, columns);
    if (resultSet == nullptr) {
        return 1;
    }
    if (resultSet->GoToFirstRow() != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("GoToFirstRow failed");
        resultSet->Close();
        return -1;
    }
    resultSet->GetColumnIndex(RcsInfo::RCS_TYPE, columnIndex);
    resultSet->GetInt(columnIndex, rcsType);
    resultSet->GetColumnIndex(RcsInfo::MSG_TITLE, columnIndex);
    resultSet->GetString(columnIndex, msgTitle);
    resultSet->GetColumnIndex(RcsInfo::MSG_CONTENT, columnIndex);
    resultSet->GetString(columnIndex, msgContent);
    resultSet->Close();
    return 0;
}

    int RdbSmsMmsCallback::VersionUpTo25(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
    {
        DATA_STORAGE_LOGI("VersionUpTo25 oldVersion = %{public}d, newVersion = %{public}d ", oldVersion, newVersion);
        if (oldVersion < VERSION_25 && newVersion >= VERSION_25) {
            std::string TABLE_SMC_INFO;
            CreateTableSmcInfoUrlRecordTableStrCb(TABLE_SMC_INFO);
            ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, TABLE_SMC_INFO);
            oldVersion = VERSION_25;
        }
        return oldVersion;
    }

    int RdbSmsMmsCallback::VersionUpTo28(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
    {
        DATA_STORAGE_LOGI("VersionUpTo28 oldVersion = %{public}d, newVersion = %{public}d ", oldVersion, newVersion);
        if (oldVersion < VERSION_28 && newVersion >= VERSION_28) {
            ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_SESSION) +
                                     " ADD COLUMN " + std::string(Session::BLOCKED_TYPE) + " INTEGER DEFAULT " + "0;");
            oldVersion = VERSION_28;
        }
        return oldVersion;
    }

    int RdbSmsMmsCallback::VersionUpTo29(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
    {
        DATA_STORAGE_LOGI("VersionUpTo29 oldVersion = %{public}d, newVersion = %{public}d ", oldVersion, newVersion);
        if (oldVersion < VERSION_29 && newVersion >= VERSION_29) {
            ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_MMS_PART) +
                                     " ADD COLUMN " + std::string(MmsPart::IS_FRAUD) + " INTEGER;");
            oldVersion = VERSION_29;
        }
        return oldVersion;
    }

    int RdbSmsMmsCallback::VersionUpTo30(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
    {
        DATA_STORAGE_LOGI("VersionUpTo30 oldVersion = %{public}d, newVersion = %{public}d ", oldVersion, newVersion);
        if (oldVersion < VERSION_30 && newVersion >= VERSION_30) {
            ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " +
                                     std::string(TABLE_SMS_MMS_INFO) + " ADD COLUMN " +
                                     std::string(SmsMmsInfo::BLOCKED_PREF_NUMBER) + " TEXT DEFAULT " + "'';");
            ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_RCS_INFO) +
                                     " ADD COLUMN " + std::string(RcsInfo::BLOCKED_PREF_NUMBER) + " TEXT DEFAULT " +
                                     "'';");
            ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "DROP VIEW IF EXISTS " +
                                     std::string(TABLE_MMS_INFO));
            std::string mms_InfoStr;
            CreateMmsInfoViewStrCb(mms_InfoStr);
            ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, mms_InfoStr);
            oldVersion = VERSION_30;
        }
        return oldVersion;
    }

    int RdbSmsMmsCallback::VersionUpTo31(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
    {
        DATA_STORAGE_LOGI("VersionUpTo31 oldVersion = %{public}d, newVersion = %{public}d ", oldVersion, newVersion);
        if (oldVersion < VERSION_31 && newVersion >= VERSION_31) {
            // 下方逻辑在getRadStore时可能会阻塞主线程，暂时移除
            // 更新 info 表的短信发送状态为成功，条件：接收到的信息 && 信息类型为短信 && 状态为发送中或者发送失败
            // 更新 session 表
            oldVersion = VERSION_31;
        }
        return oldVersion;
    }

    int RdbSmsMmsCallback::VersionUpTo33(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
    {
        DATA_STORAGE_LOGI("VersionUpTo33 oldVersion = %{public}d, newVersion = %{public}d ", oldVersion, newVersion);
        if (oldVersion < VERSION_33 && newVersion >= VERSION_33) {
            ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "DROP TRIGGER IF EXISTS updateSms_unread_count");
            ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "DROP TRIGGER IF EXISTS updateRcs_unread_count");
            oldVersion = VERSION_33;
        }
        return oldVersion;
    }

    int RdbSmsMmsCallback::VersionUpTo34(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
    {
        DATA_STORAGE_LOGI("VersionUpTo34 oldVersion = %{public}d, newVersion = %{public}d ", oldVersion, newVersion);
        if (oldVersion < VERSION_34 && newVersion >= VERSION_34) {
            int ret = 0;
            int64_t insertNum = 0;
            std::string tableInfo;
            std::vector<NativeRdb::ValuesBucket> values;
            if (RdbSmsMmsCallback::IsFavTableExist(rdbStore)) {
                DATA_STORAGE_LOGE("FavTable exist, do not create.");
                oldVersion = VERSION_34;
                return oldVersion;
            }
            RdbSmsMmsTables tables;
            tables.CreateSmsMmsFavoriteTableStr(tableInfo);
            ret = ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, tableInfo);
            if (ret != NativeRdb::E_OK) {
                DATA_STORAGE_LOGE("create favorite table failed error(%{public}d)", ret);
                return oldVersion;
            }
            if (GetHistoryFavoriteItem(rdbStore, values) <= 0) {
                oldVersion = VERSION_34;
                return oldVersion;
            }
            ret = rdbStore.BatchInsert(insertNum, TABLE_FAVORITE_INFO, values);
            if (ret != NativeRdb::E_OK) {
                DATA_STORAGE_LOGE("BatchInsert failed error(%{public}d)", ret);
                return oldVersion;
            }
            oldVersion = VERSION_34;
        }
        return oldVersion;
    }

    void RdbSmsMmsCallback::processUpdateSessionSendState(NativeRdb::RdbStore &rdbStore)
    {
        std::shared_ptr<NativeRdb::ResultSet> resultSet = nullptr;
        // 在 session 表中找到 sending_status 为 1 或者 2 的 id 集合，然后在 sms_mms_info 表中找到 session_id
        // 与这些集合匹配的记录，取 对应 start_time 最大的记录
        std::string sql;
        sql.append("SELECT sms_mms_info.session_id, sms_mms_info.msg_state ");
        sql.append("FROM ");
        sql.append("sms_mms_info ");
        sql.append("JOIN ");
        sql.append("(");
        sql.append("SELECT id ");
        sql.append("FROM session ");
        sql.append("WHERE sending_status IN (1, 2)");
        sql.append(") ids ON sms_mms_info.session_id = ids.id ");
        sql.append("WHERE ");
        sql.append("(sms_mms_info.session_id, sms_mms_info.start_time) IN (");
        sql.append("SELECT ");
        sql.append("session_id, ");
        sql.append("MAX(start_time) as max_start_time ");
        sql.append("FROM ");
        sql.append("sms_mms_info ");
        sql.append("GROUP BY ");
        sql.append("session_id");
        sql.append(");");
        resultSet = rdbStore.QuerySql(sql);
        if (resultSet == nullptr) {
            DATA_STORAGE_LOGE("Query session update state is null!");
            return;
        }
        int count;
        resultSet->GetRowCount(count);
        DATA_STORAGE_LOGE("process update session count %{public}d", count);
        if (count <= 0) {
            DATA_STORAGE_LOGE("Query no matched session state data!");
            return;
        }
        int32_t columnIdIndex = 0;
        int32_t columnStateIndex = 0;
        resultSet->GetColumnIndex("session_id", columnIdIndex);
        resultSet->GetColumnIndex("msg_state", columnStateIndex);
        while (resultSet->GoToNextRow() == NativeRdb::E_OK) {
            int32_t id = 0;
            int32_t msgState = 0;
            resultSet->GetInt(columnIdIndex, id);
            resultSet->GetInt(columnStateIndex, msgState);
            rdbStore.ExecuteSql("UPDATE " + std::string(TABLE_SESSION) + " SET sending_status = " +
                                std::to_string(msgState) + " WHERE id = " + std::to_string(id));
        }
        resultSet->Close();
    }

    int RdbSmsMmsCallback::VersionUpTo35(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
    {
        DATA_STORAGE_LOGI("VersionUpTo35 oldVersion = %{public}d, newVersion = %{public}d ", oldVersion, newVersion);
        if (oldVersion < VERSION_35 && newVersion >= VERSION_35) {
            ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_MMS_PART) +
                                     " ADD COLUMN " + std::string(MmsPart::REFERENCE_COUNT) + " INTEGER DEFAULT " +
                                     "-1;");
            oldVersion = VERSION_35;
        }
        return oldVersion;
    }

    int RdbSmsMmsCallback::VersionUpTo32(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
    {
        DATA_STORAGE_LOGI("VersionUpTo32 oldVersion = %{public}d, newVersion = %{public}d ", oldVersion, newVersion);
        if (oldVersion < VERSION_32 && newVersion >= VERSION_32) {
            ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_SESSION) +
                                     " ADD COLUMN " + std::string(Session::PINNING_TIME) + " INTEGER DEFAULT " + "0;");
            oldVersion = VERSION_32;
        }
        return oldVersion;
    }

int RdbSmsMmsCallback::VersionUpToEleven(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
{
    DATA_STORAGE_LOGI("VersionUpToEleven oldVersion = %{public}d, newVersion = %{public}d ", oldVersion, newVersion);
    if (oldVersion < VERSION_11 && newVersion >= VERSION_11) {
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_SMS_MMS_INFO) +
                                 " ADD COLUMN " + std::string(SmsMmsInfo::IS_REPORT) + " INTEGER DEFAULT " + "0;");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_RCS_INFO) +
                                 " ADD COLUMN " + std::string(RcsInfo::IS_REPORT) + " INTEGER DEFAULT " + "0;");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "DROP VIEW IF EXISTS " +
                                 std::string(TABLE_MMS_INFO));
        std::string mms_InfoStr;
        CreateMmsInfoViewStrCb(mms_InfoStr);
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, mms_InfoStr);
        oldVersion = VERSION_11;
    }
    return oldVersion;
}

void RdbSmsMmsCallback::RcsInfoInsertSmsMmsInfo(NativeRdb::RdbStore &rdbStore)
{
    auto resultSet = QueryRcsInfo(rdbStore);
    if (resultSet == nullptr) {
        DATA_STORAGE_LOGE("get nullptr created result");
        return;
    }
    InsertSmsMmsInfoFromResultSet(rdbStore, resultSet);
    resultSet->Close();
}

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsCallback::QueryRcsInfo(NativeRdb::RdbStore &rdbStore)
{
    NativeRdb::AbsRdbPredicates createPredicates = NativeRdb::AbsRdbPredicates(TABLE_RCS_INFO);
    createPredicates.NotEqualTo(RcsInfo::RCS_ID, 0);
    return rdbStore.Query(createPredicates, RCS_INFO_COLUMNS);
}

void RdbSmsMmsCallback::InsertSmsMmsInfoFromResultSet(NativeRdb::RdbStore &rdbStore,
                                                      std::shared_ptr<NativeRdb::ResultSet> resultSet)
{
    int64_t outId = 0;
    int count;
    resultSet->GetRowCount(count);
    DATA_STORAGE_LOGI("GetRowCount count = %{public}d ", count);
    for (int row = 0; row < count; row++) {
        NativeRdb::ValuesBucket values = GetValuesFromRow(resultSet, row);
        rdbStore.Insert(outId, TABLE_SMS_MMS_INFO, values);
    }
}

NativeRdb::ValuesBucket RdbSmsMmsCallback::GetValuesFromRow(std::shared_ptr<NativeRdb::ResultSet> resultSet, int row)
{
    resultSet->GoToRow(row);
    int columnIndex;
    int rcsId;
    int isSend;
    int isRead;
    int isCollect;
    int sessionId;
    int rcsType;
    int groupId;
    std::string startTime;
    std::string msgContent;
    std::string receiverNumber;
    std::string senderNumber;
    resultSet->GetColumnIndex(RcsInfo::RCS_ID, columnIndex);
    resultSet->GetInt(columnIndex, rcsId);
    resultSet->GetColumnIndex(RcsInfo::IS_SENDER, columnIndex);
    resultSet->GetInt(columnIndex, isSend);
    resultSet->GetColumnIndex(RcsInfo::START_TIME, columnIndex);
    resultSet->GetString(columnIndex, startTime);
    resultSet->GetColumnIndex(RcsInfo::MSG_CONTENT, columnIndex);
    resultSet->GetString(columnIndex, msgContent);
    resultSet->GetColumnIndex(RcsInfo::RCS_TYPE, columnIndex);
    resultSet->GetInt(columnIndex, rcsType);
    resultSet->GetColumnIndex(RcsInfo::RECEIVER_NUMBER, columnIndex);
    resultSet->GetString(columnIndex, receiverNumber);
    resultSet->GetColumnIndex(RcsInfo::SENDER_NUMBER, columnIndex);
    resultSet->GetString(columnIndex, senderNumber);
    resultSet->GetColumnIndex(RcsInfo::GROUP_ID, columnIndex);
    resultSet->GetInt(columnIndex, groupId);
    resultSet->GetColumnIndex(RcsInfo::IS_READ, columnIndex);
    resultSet->GetInt(columnIndex, isRead);
    resultSet->GetColumnIndex(RcsInfo::SESSION_ID, columnIndex);
    resultSet->GetInt(columnIndex, sessionId);
    resultSet->GetColumnIndex(RcsInfo::IS_COLLECT, columnIndex);
    resultSet->GetInt(columnIndex, isCollect);
    NativeRdb::ValuesBucket values;
    values.PutInt(SmsMmsInfo::RCS_ID, rcsId);
    values.PutInt(SmsMmsInfo::IS_SENDER, isSend);
    values.PutString(SmsMmsInfo::START_TIME, startTime);
    values.PutString(SmsMmsInfo::MSG_CONTENT, rcsType == 0 ? msgContent : "");
    values.PutString(SmsMmsInfo::RECEIVER_NUMBER, receiverNumber);
    values.PutString(SmsMmsInfo::SENDER_NUMBER, senderNumber);
    values.PutInt(SmsMmsInfo::GROUP_ID, groupId);
    values.PutInt(SmsMmsInfo::IS_READ, isRead);
    values.PutInt(SmsMmsInfo::SESSION_ID, sessionId);
    values.PutInt(SmsMmsInfo::IS_COLLECT, isCollect);
    values.PutInt(SmsMmsInfo::SMS_TYPE, RCS_TYPE_VALUE);
    return values;
}

int RdbSmsMmsCallback::VersionUpToTwelve(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
{
    DATA_STORAGE_LOGI("VersionUpToTwelve oldVersion = %{public}d, newVersion = %{public}d ", oldVersion, newVersion);
    if (oldVersion < VERSION_12 && newVersion >= VERSION_12) {
        if (IsFieldExistInTable(rdbStore, std::string(TABLE_SMS_MMS_INFO), SmsMmsInfo::RCS_ID)) {
            DATA_STORAGE_LOGE("No need to migrate RCS data!");
            oldVersion = VERSION_12;
            return oldVersion;
        }
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_SMS_MMS_INFO) +
                                 " ADD COLUMN " + std::string(SmsMmsInfo::RCS_ID) + " INTEGER DEFAULT " + "0;");

        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "DROP VIEW IF EXISTS " +
                                 std::string(TABLE_MMS_INFO));
        std::string mms_InfoStr;
        CreateMmsInfoViewStrCb(mms_InfoStr);
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, mms_InfoStr);
        RcsInfoInsertSmsMmsInfo(rdbStore);
        oldVersion = VERSION_12;
    }
    return oldVersion;
}

int RdbSmsMmsCallback::VersionUpToThirteen(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
{
    DATA_STORAGE_LOGI("VersionUpToThirteen oldVersion = %{public}d, newVersion = %{public}d ", oldVersion, newVersion);
    if (oldVersion < VERSION_13 && newVersion >= VERSION_13) {
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "UPDATE " + std::string(TABLE_RCS_INFO) + " SET " +
            std::string(RcsInfo::IS_READ) + " = 1 WHERE IS_SENDER = 0");

        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "DROP VIEW IF EXISTS " +
                                 std::string(TABLE_MMS_INFO));
        std::string mms_InfoStr;
        CreateMmsInfoViewStrCb(mms_InfoStr);
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, mms_InfoStr);
        oldVersion = VERSION_13;
    }
    return oldVersion;
}

int RdbSmsMmsCallback::VersionUpToFourteen(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
{
    DATA_STORAGE_LOGI("VersionUpToFourteen oldVersion = %{public}d, newVersion = %{public}d ", oldVersion, newVersion);
    if (oldVersion < VERSION_14 && newVersion >= VERSION_14) {
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_SMS_MMS_INFO) +
                                 " ADD COLUMN " + std::string(SmsMmsInfo::DETECT_RES_CONTENT) +
                                 " TEXT DEFAULT " + "'';");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_RCS_INFO) +
                                 " ADD COLUMN " + std::string(RcsInfo::DETECT_RES_CONTENT) + " TEXT DEFAULT " + "'';");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "DROP VIEW IF EXISTS " +
                                 std::string(TABLE_MMS_INFO));
        std::string mms_InfoStr;
        CreateMmsInfoViewStrCb(mms_InfoStr);
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, mms_InfoStr);
        oldVersion = VERSION_14;
    }
    return oldVersion;
}

int RdbSmsMmsCallback::VersionUpToFifteen(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
{
    DATA_STORAGE_LOGI("VersionUpToFifteen oldVersion = %{public}d, newVersion = %{public}d ", oldVersion, newVersion);
    if (oldVersion < VERSION_15 && newVersion >= VERSION_15) {
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "DROP VIEW IF EXISTS " +
                                 std::string(TABLE_MMS_INFO));
        std::string mms_InfoStr;
        CreateMmsInfoViewStrCb(mms_InfoStr);
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, mms_InfoStr);
        oldVersion = VERSION_15;
    }
    return oldVersion;
}

int RdbSmsMmsCallback::VersionUpTo16(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
{
    DATA_STORAGE_LOGI("VersionUpTo16 oldVersion = %{public}d, newVersion = %{public}d ", oldVersion, newVersion);
    if (oldVersion < VERSION_16 && newVersion >= VERSION_16) {
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_SMS_MMS_INFO) +
                                 " ADD COLUMN " + std::string(SmsMmsInfo::PHONE_NUMBER) + " TEXT DEFAULT " + "'';");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_SMS_MMS_INFO) +
                                 " ADD COLUMN " + std::string(SmsMmsInfo::FORMAT_PHONE_NUMBER) + " TEXT DEFAULT " +
                                 "'';");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_RCS_INFO) +
                                 " ADD COLUMN " + std::string(RcsInfo::PHONE_NUMBER) + " TEXT DEFAULT " + "'';");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_RCS_INFO) +
                                 " ADD COLUMN " + std::string(RcsInfo::FORMAT_PHONE_NUMBER) + " TEXT DEFAULT " + "'';");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "DROP VIEW IF EXISTS " +
                                 std::string(TABLE_MMS_INFO));
        std::string mms_InfoStr;
        CreateMmsInfoViewStrCb(mms_InfoStr);
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, mms_InfoStr);
        oldVersion = VERSION_16;
    }
    return oldVersion;
}

int RdbSmsMmsCallback::VersionUpToSeventeen(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
{
    DATA_STORAGE_LOGI("VersionUpToSeventeen oldVersion = %{public}d, newVersion = %{public}d ", oldVersion, newVersion);
    if (oldVersion < VERSION_17 && newVersion >= VERSION_17) {
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_SMS_MMS_INFO) +
                                 " ADD COLUMN " + std::string(SmsMmsInfo::MSG_CODE_STR) + " TEXT DEFAULT " + "'';");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_SESSION) +
                                 " ADD COLUMN " + std::string(Session::YELLOW_PAGE_ID) + " TEXT DEFAULT " + "'';");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "DROP VIEW IF EXISTS " +
                                 std::string(TABLE_MMS_INFO));
        std::string mms_InfoStr;
        CreateMmsInfoViewStrCb(mms_InfoStr);
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, mms_InfoStr);
        oldVersion = VERSION_17;
    }
    return oldVersion;
}

int RdbSmsMmsCallback::VersionUpTo18(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
{
    DATA_STORAGE_LOGI("VersionUpToNine oldVersion = %{public}d, newVersion = %{public}d ", oldVersion, newVersion);
    if (oldVersion < VERSION_18 && newVersion >= VERSION_18) {
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "DROP TRIGGER IF EXISTS insertSms_unread_count");
        std::string createInsertSmsTriggerStr;
        CreateInsertSmsTriggerStrCb(createInsertSmsTriggerStr);
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, createInsertSmsTriggerStr);

        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "DROP TRIGGER IF EXISTS insertRcs_unread_count");
        std::string createInsertRcsTriggerStr;
        CreateInsertRcsTriggerStrCb(createInsertRcsTriggerStr);
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, createInsertRcsTriggerStr);

        oldVersion = VERSION_18;
    }
    return oldVersion;
}
    
int RdbSmsMmsCallback::VersionUpTo19(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
{
    DATA_STORAGE_LOGI("VersionUpTo16 oldVersion = %{public}d, newVersion = %{public}d ", oldVersion, newVersion);
    if (oldVersion < VERSION_19 && newVersion >= VERSION_19) {
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_MMS_PART) +
                                 " ADD COLUMN " +
        std::string(MmsPart::CT) + " TEXT " + "'';");
        oldVersion = VERSION_19;
    }
    return oldVersion;
}

int RdbSmsMmsCallback::VersionUpTo20(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
{
    DATA_STORAGE_LOGI("VersionUpToNine oldVersion = %{public}d, newVersion = %{public}d ", oldVersion, newVersion);
    if (oldVersion < VERSION_20 && newVersion >= VERSION_20) {
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion,
                                 "DROP TRIGGER IF EXISTS updateBlockSms_unread_count");
        std::string createUpdateBlockSmsTriggerStr;
        CreateUpdateBlockSmsTriggerStrCb(createUpdateBlockSmsTriggerStr);
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, createUpdateBlockSmsTriggerStr);
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion,
                                 "DROP TRIGGER IF EXISTS updateBlockRcs_unread_count");
        std::string createUpdateBlockRcsTriggerStr;
        CreateUpdateBlockRcsTriggerStrCb(createUpdateBlockRcsTriggerStr);
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, createUpdateBlockRcsTriggerStr);

        oldVersion = VERSION_20;
    }
    return oldVersion;
}

int RdbSmsMmsCallback::VersionUpTo21(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
{
    DATA_STORAGE_LOGI("VersionUpToNine oldVersion = %{public}d, newVersion = %{public}d ", oldVersion, newVersion);
    if (oldVersion < VERSION_21 && newVersion >= VERSION_21) {
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_SMS_MMS_INFO) +
                                 " ADD COLUMN " + std::string(SmsMmsInfo::BLOCKED_PREF_NUMBER) + " TEXT DEFAULT " +
                                 "'';");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_RCS_INFO) +
                                 " ADD COLUMN " + std::string(RcsInfo::BLOCKED_PREF_NUMBER) + " TEXT DEFAULT " + "'';");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "DROP VIEW IF EXISTS " +
                                 std::string(TABLE_MMS_INFO));
        std::string mms_InfoStr;
        CreateMmsInfoViewStrCb(mms_InfoStr);
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, mms_InfoStr);

        oldVersion = VERSION_21;
    }
    return oldVersion;
}

int RdbSmsMmsCallback::VersionUpTo22(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
{
    DATA_STORAGE_LOGI("VersionUpTo22 oldVersion = %{public}d, newVersion = %{public}d ", oldVersion, newVersion);
    if (oldVersion < VERSION_22 && newVersion >= VERSION_22) {
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, VERSION_22_DROP_INDEX_ON_RCS_TABLE);
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, VERSION_22_CREATE_INDEX_ON_RCS_TABLE);

        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, VERSION_22_DROP_INDEX_ON_SMS_MMS_INFO_TABLE);
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, VERSION_22_CREATE_INDEX_ON_SMS_MMS_INFO_TABLE);

        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, VERSION_22_DROP_INDEX_ON_SESSION_TABLE);
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, VERSION_22_CREATE_INDEX_ON_SESSION_TABLE);
        oldVersion = VERSION_22;
    }
    return oldVersion;
}

int RdbSmsMmsCallback::VersionUpTo23(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
{
    DATA_STORAGE_LOGI("VersionUpTo23 enter oldVersion = %{public}d, newVersion = %{public}d ", oldVersion, newVersion);
    if (oldVersion < VERSION_23 && newVersion >= VERSION_23) {
        int ret = 0;
        int64_t insertNum = 0;
        std::string tableInfo;
        std::vector<NativeRdb::ValuesBucket> values;
        if (RdbSmsMmsCallback::IsFavTableExist(rdbStore)) {
            DATA_STORAGE_LOGE("FavTable exist, do not create.");
            oldVersion = VERSION_23;
            return oldVersion;
        }
        RdbSmsMmsTables tables;
        tables.CreateSmsMmsFavoriteTableStr(tableInfo);
        ret = ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, tableInfo);
        if (ret != NativeRdb::E_OK) {
            DATA_STORAGE_LOGE("create favorite table failed error(%{public}d)", ret);
            return oldVersion;
        }
        if (GetHistoryFavoriteItem(rdbStore, values) <= 0) {
            oldVersion = VERSION_23;
            return oldVersion;
        }
        ret = rdbStore.BatchInsert(insertNum, TABLE_FAVORITE_INFO, values);
        if (ret != NativeRdb::E_OK) {
            DATA_STORAGE_LOGE("BatchInsert failed error(%{public}d)", ret);
            return oldVersion;
        }
        oldVersion = VERSION_23;
    }
    return oldVersion;
}

int RdbSmsMmsCallback::VersionUpTo24(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
{
    DATA_STORAGE_LOGI("VersionUpTo24 oldVersion = %{public}d, newVersion = %{public}d ", oldVersion, newVersion);
    if (oldVersion < VERSION_24 && newVersion >= VERSION_24) {
        std::string TABLE_SPECIFIC_CHAT_BOTS_INFO;
        CreateTableSpecificChatBotsInfoUrlRecordTableStrCb(TABLE_SPECIFIC_CHAT_BOTS_INFO);
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, TABLE_SPECIFIC_CHAT_BOTS_INFO);

        std::string TABLE_CHAT_BOTS_INFO;
        CreateTableChatBotsInfoUrlRecordTableStrCb(TABLE_CHAT_BOTS_INFO);
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, TABLE_CHAT_BOTS_INFO);
        oldVersion = VERSION_24;
    }
    return oldVersion;
}

int RdbSmsMmsCallback::VersionUpTo26(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
{
    DATA_STORAGE_LOGI("VersionUpTo26 oldVersion = %{public}d, newVersion = %{public}d ", oldVersion, newVersion);
    if (oldVersion < VERSION_26 && newVersion >= VERSION_26) {
        int ret = ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "DELETE FROM " +
                                           std::string(TABLE_MMS_PART) + " WHERE " + std::string(TABLE_MMS_PART) +
                                           ".msg_id IN (SELECT msg_id FROM " + std::string(TABLE_SMS_MMS_INFO) +
                                           " WHERE " + std::string(TABLE_SMS_MMS_INFO) +
                                           ".session_id NOT IN (SELECT session.id from session))");
        DATA_STORAGE_LOGI("delete invalid mms part :%d", ret);
        ret = ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "DELETE FROM " +
                                       std::string(TABLE_SMS_MMS_INFO) + " WHERE " + std::string(TABLE_SMS_MMS_INFO) +
                                       ".session_id NOT IN (SELECT session.id FROM session)" +
                                       " AND " + std::string(TABLE_SMS_MMS_INFO) + ".is_blocked != 1");
        DATA_STORAGE_LOGI("delete invalid info :%d", ret);
        ret = ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "DELETE FROM " + std::string(TABLE_RCS_INFO) +
            " WHERE " + std::string(TABLE_RCS_INFO) + ".session_id NOT IN (SELECT session.id FROM session)");
        DATA_STORAGE_LOGI("delete invalid rcs info :%d", ret);
        oldVersion = VERSION_26;
    }
    return oldVersion;
}

int RdbSmsMmsCallback::VersionUpTo27(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
{
    DATA_STORAGE_LOGI("VersionUpTo27 oldVersion = %{public}d, newVersion = %{public}d ", oldVersion, newVersion);
    if (oldVersion < VERSION_27 && newVersion >= VERSION_27) {
        int ret = ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " +
                                           std::string(TABLE_RCS_INFO) + " ADD COLUMN " +
                                           std::string(RcsInfo::MSG_ID) + " TEXT DEFAULT '';");
        DATA_STORAGE_LOGI("add colum rcs_info.msg_id :%d", ret);
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "DROP VIEW IF EXISTS " +
                                 std::string(TABLE_MMS_INFO));
        std::string mms_InfoStr;
        CreateMmsInfoViewStrCb(mms_InfoStr);
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, mms_InfoStr);
        oldVersion = VERSION_27;
    }
    return oldVersion;
}

int RdbSmsMmsCallback::VersionUp(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
{
    if (oldVersion < VERSION_5 && newVersion >= VERSION_5) {
        oldVersion = VERSION_5;
    }
    if (oldVersion < VERSION_6 && newVersion >= VERSION_6) {
        oldVersion = VERSION_6;
    }
    if (oldVersion < VERSION_7 && newVersion >= VERSION_7) {
        std::string mms_InfoStr;
        CreateMmsInfoViewStrCb(mms_InfoStr);
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, mms_InfoStr);
        oldVersion = VERSION_7;
    }
    oldVersion = VersionUpToEight(rdbStore, oldVersion, newVersion);
    oldVersion = VersionUpToNine(rdbStore, oldVersion, newVersion);
    oldVersion = VersionUpToTen(rdbStore, oldVersion, newVersion);
    oldVersion = VersionUpToEleven(rdbStore, oldVersion, newVersion);
    oldVersion = VersionUpToTwelve(rdbStore, oldVersion, newVersion);
    oldVersion = VersionUpToThirteen(rdbStore, oldVersion, newVersion);
    oldVersion = VersionUpToFourteen(rdbStore, oldVersion, newVersion);
    oldVersion = VersionUpToFifteen(rdbStore, oldVersion, newVersion);
    oldVersion = VersionUpTo16(rdbStore, oldVersion, newVersion);
    oldVersion = VersionUpToSeventeen(rdbStore, oldVersion, newVersion);
    oldVersion = VersionUpTo18(rdbStore, oldVersion, newVersion);
    oldVersion = VersionUpTo19(rdbStore, oldVersion, newVersion);
    oldVersion = VersionUpTo20(rdbStore, oldVersion, newVersion);
    oldVersion = VersionUpTo21(rdbStore, oldVersion, newVersion);
    oldVersion = VersionUpTo22(rdbStore, oldVersion, newVersion);
    oldVersion = VersionUpTo23(rdbStore, oldVersion, newVersion);
    oldVersion = VersionUpTo24(rdbStore, oldVersion, newVersion);
    oldVersion = VersionUpTo25(rdbStore, oldVersion, newVersion);
    oldVersion = VersionUpTo26(rdbStore, oldVersion, newVersion);
    oldVersion = VersionUpTo27(rdbStore, oldVersion, newVersion);
    oldVersion = VersionUpTo28(rdbStore, oldVersion, newVersion);
    oldVersion = VersionUpTo29(rdbStore, oldVersion, newVersion);
    oldVersion = VersionUpTo30(rdbStore, oldVersion, newVersion);
    oldVersion = VersionUpTo31(rdbStore, oldVersion, newVersion);
    oldVersion = VersionUpTo32(rdbStore, oldVersion, newVersion);
    oldVersion = VersionUpTo33(rdbStore, oldVersion, newVersion);
    oldVersion = VersionUpTo34(rdbStore, oldVersion, newVersion);
    oldVersion = VersionUpTo35(rdbStore, oldVersion, newVersion);
    return oldVersion;
}

int RdbSmsMmsCallback::ExecuteSqlAndReportEvent(NativeRdb::RdbStore &rdbStore, int oldVersion,
    int newVersion, std::string sql)
{
    std::stringstream versionPath;
    versionPath << "Upgrade from " << oldVersion << " to " << newVersion;
    int ret = rdbStore.ExecuteSql(sql);
    std::stringstream resultString;
    resultString << "Execute " << sql << " result " << ret;
    dfxAdapter_.ReportDataBaseUpdate(versionPath.str(), ret, resultString.str());
    return ret;
}

int RdbSmsMmsCallback::VersionUpToEight(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
{
    DATA_STORAGE_LOGI("VersionUpToEight oldVersion = %{public}d, newVersion = %{public}d ", oldVersion, newVersion);
    if (oldVersion < VERSION_8 && newVersion >= VERSION_8) {
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion,
            "ALTER TABLE " + std::string(TABLE_SMS_MMS_INFO) + " ADD COLUMN " +
            std::string(SmsMmsInfo::IS_BLOCKED) + " INTEGER DEFAULT " + "0;");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_SMS_MMS_INFO) +
                                 " ADD COLUMN " + std::string(SmsMmsInfo::IS_BLOCKED) + " INTEGER DEFAULT " + "0;");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_SMS_MMS_INFO) +
                                 " ADD COLUMN " + std::string(SmsMmsInfo::BLOCKED_REASON) + " INTEGER DEFAULT " + "0;");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_SMS_MMS_INFO) +
                                 " ADD COLUMN " + std::string(SmsMmsInfo::BLOCKED_SOURCES) + " TEXT DEFAULT " + "'';");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_SMS_MMS_INFO) +
                                 " ADD COLUMN " + std::string(SmsMmsInfo::BLOCKED_TYPE) + " INTEGER DEFAULT " + "0;");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_SMS_MMS_INFO) +
                                 " ADD COLUMN " + std::string(SmsMmsInfo::BLOCKED_TYPE_TEXT) + " TEXT DEFAULT " +
                                 "'';");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_RCS_INFO) +
                                 " ADD COLUMN " + std::string(RcsInfo::IS_BLOCKED) + " INTEGER DEFAULT " + "0;");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_RCS_INFO) +
                                 " ADD COLUMN " + std::string(RcsInfo::BLOCKED_REASON) + " INTEGER DEFAULT " + "0;");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_RCS_INFO) +
                                 " ADD COLUMN " + std::string(RcsInfo::BLOCKED_SOURCES) + " TEXT DEFAULT " + "'';");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_RCS_INFO) +
                                 " ADD COLUMN " + std::string(RcsInfo::BLOCKED_TYPE) + " INTEGER DEFAULT " + "0;");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_RCS_INFO) +
                                 " ADD COLUMN " + std::string(RcsInfo::BLOCKED_TYPE_TEXT) + " TEXT DEFAULT " + "'';");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "DROP VIEW IF EXISTS " +
                                 std::string(TABLE_MMS_INFO));
        std::string mms_InfoStr;
        CreateMmsInfoViewStrCb(mms_InfoStr);
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, mms_InfoStr);
        oldVersion = VERSION_8;
    }
    return oldVersion;
}

void RdbSmsMmsCallback::AlterRcsInfoTableForcely(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
{
    if (oldVersion < VERSION_7 && newVersion >= VERSION_7) {
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "DROP VIEW IF EXISTS mms_info;");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_RCS_INFO) +
                                 " ADD COLUMN " + std::string(RcsInfo::IS_ADVANCED_SECURITY) + " INTEGER DEFAULT " +
                                 "0;");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion,
                                 "ALTER TABLE rcs_info RENAME COLUMN msg_id TO msg_idTEXT;");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "ALTER TABLE " + std::string(TABLE_RCS_INFO) +
                                 " ADD COLUMN " +
        std::string(RcsInfo::MSG_ID) + " TEXT DEFAULT " + "'';");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, "UPDATE rcs_info SET msg_id = msg_idTEXT;");
    }
}

int RdbSmsMmsCallback::OnDowngrade(NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion)
{
    DATA_STORAGE_LOGI(
        "Data_Storage RdbSmsMmsCallback::OnDowngrade##currentVersion "
        "= %d, targetVersion = %d\n",
        currentVersion, targetVersion);
    return NativeRdb::E_OK;
}

void RdbSmsMmsCallback::CheckColumn(NativeRdb::RdbStore &rdbStore)
{
    int ret = rdbStore.ExecuteSql("select expires_time from sms_mms_info;");
    DATA_STORAGE_LOGI("Data_Storage sms_mms_info CheckColumn expires_time:%d", ret);
    ret = rdbStore.ExecuteSql("select mms_pdu from sms_mms_info;");
    DATA_STORAGE_LOGI("Data_Storage sms_mms_info CheckColumn mms_pdu:%d", ret);
    ret = rdbStore.ExecuteSql("select is_advanced_security from sms_mms_info;");
    DATA_STORAGE_LOGI("Data_Storage sms_mms_info CheckColumn is_advanced_security:%d", ret);
    ret = rdbStore.ExecuteSql("select is_advanced_security from rcs_info;");
    DATA_STORAGE_LOGI("Data_Storage rcs_info CheckColumn is_advanced_security:%d", ret);
    ret = rdbStore.ExecuteSql("select is_collect from rcs_info;");
    DATA_STORAGE_LOGI("Data_Storage rcs_info CheckColumn is_collect:%d", ret);
    ret = rdbStore.ExecuteSql("select receive_state from rcs_info;");
    DATA_STORAGE_LOGI("Data_Storage rcs_info CheckColumn receive_state:%d", ret);
    ret = rdbStore.ExecuteSql("select fail_receive_context from rcs_info;");
    DATA_STORAGE_LOGI("Data_Storage rcs_info CheckColumn fail_receive_context:%d", ret);
    ret = rdbStore.ExecuteSql("select msg_idTEXT from rcs_info;");
    DATA_STORAGE_LOGI("Data_Storage rcs_info CheckColumn msg_idTEXT:%d", ret);
    ret = rdbStore.ExecuteSql("select msg_id from rcs_info;");
    DATA_STORAGE_LOGI("Data_Storage rcs_info CheckColumn msg_id:%d", ret);
    ret = rdbStore.ExecuteSql("select msg_id from mms_info;");
    DATA_STORAGE_LOGI("Data_Storage mms_info CheckColumn msg_id:%d", ret);
    ret = rdbStore.ExecuteSql("select msg_id from favorate_info;");
    DATA_STORAGE_LOGI("Data_Storage favorate_info CheckColumn msg_id:%d", ret);
}

// 无论是否升级都会走到这里，增加维测，打印当前数据最新版本号。
int RdbSmsMmsCallback::OnOpen(NativeRdb::RdbStore &rdbStore)
{
    int ret = NativeRdb::E_OK;
    int version = -1;
    ret = rdbStore.GetVersion(version);
    if (ret != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("GetVersion error %{public}d", ret);
        return NativeRdb::E_OK;
    }
    std::string path = rdbStore.GetPath();
    DATA_STORAGE_LOGW("rdbStore.verson: %{public}d, rdb.path: %{public}s", version, path.c_str());
    bool isDbComplete = IsDbComplete(rdbStore);
    if (!isDbComplete) {
        RestoreAllInfo(rdbStore);
    }
    CheckAndCompleteFields(rdbStore);
    CheckAndCompleteComponent(rdbStore);
    if (isDbComplete) {
        DoBackUpDB(rdbStore);
    }
    return NativeRdb::E_OK;
}

void RdbSmsMmsCallback::RestoreAllInfo(NativeRdb::RdbStore &rdbStore)
{
    RdbSmsMmsTables tables;
    std::vector<std::string> createTableVec;

    createTableVec = tables.InitCreateTableVec();
    DATA_STORAGE_LOGI("RdbSmsMmsCallback::RestoreAllInfo");
    int32_t ret = NativeRdb::E_ERROR;
    size_t size = createTableVec.size();
    if (size == 0) {
        DATA_STORAGE_LOGE("RdbSmsMmsCallback::RestoreAllInfo size = 0");
        return;
    }
    for (size_t i = 0; i < size; i++) {
        ret = rdbStore.ExecuteSql(createTableVec[i]);
        if (ret != NativeRdb::E_OK) {
            DATA_STORAGE_LOGE("RdbSmsMmsCallback::RestoreAllInfo error code = %{public}d", ret);
        }
    }
}

bool RdbSmsMmsCallback::IsDbComplete(NativeRdb::RdbStore &rdbStore)
{
    const int targetCount = 9;
    std::string sql = "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name "
        "IN ('sms_mms_info', 'sms_subsection', 'rcs_info', 'mms_pdu', 'mms_part', 'session',"
        "'specific_chatbots', 'favorate_info', 'chatbots');";
    std::shared_ptr<NativeRdb::ResultSet> resultSet = rdbStore.QuerySql(sql);
    if (resultSet == nullptr) {
        DATA_STORAGE_LOGE("IsDbComplete resultSet is null");
        return false;
    }
    int count = 0;
    int32_t res = resultSet->GoToFirstRow();
    if (res == NativeRdb::E_OK) {
        res = resultSet->GetInt(0, count);
        resultSet->Close();
        if (res != NativeRdb::E_OK) {
            DATA_STORAGE_LOGE("IsDbComplete GetInt error %{public}d", res);
            return false;
        } else {
            DATA_STORAGE_LOGE("IsDbComplete count = %{public}d", count);
            return count == targetCount;
        }
    } else {
        DATA_STORAGE_LOGE("GoToFirstRow error %{public}d", res);
        resultSet->Close();
        return false;
    }
}

void RdbSmsMmsCallback::DoBackUpDB(NativeRdb::RdbStore &rdbStore)
{
    std::string path = rdbStore.GetPath();
    if ((!needBackup && std::filesystem::exists(DB_PATH + "rdb/" + BAK_DB_NAME)) ||
        path.substr(0, DB_PATH.length()) != DB_PATH) {
        DATA_STORAGE_LOGI("no need backup");
        return;
    }
    auto errCode = rdbStore.Backup(BAK_DB_NAME);
    if (errCode != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("DoBackUpDB failed, errCode:%{public}d", errCode);
    } else {
        DATA_STORAGE_LOGI("DoBackUpDB success");
    }
}

void RdbSmsMmsCallback::CreateRcsInfoTableForcely(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
{
    if (oldVersion < VERSION_6 && newVersion >= VERSION_6) {
        std::string createTableStr;
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
        createTableStr.append(RcsInfo::SERVICE_KIND).append(" TEXT DEFAULT '')");
        ExecuteSqlAndReportEvent(rdbStore, oldVersion, newVersion, createTableStr);
    }
}

/*
 * Related contents need to be added to the CreateMmsInfoViewStrCb function synchronously.
 * Otherwise, the mmsinfo table does not take effect in the upgrade scenario.
 */
void RdbSmsMmsCallback::CreateMmsInfoViewStrCb(std::string &createTableStr)
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
    createTableStr.append("blocked_type, blocked_type_text, blocked_pref_number, clur_size, total_size, ");
    createTableStr.append("risk_url_body, has_be_reported_as_spam, is_report, detect_res_content, ");
    createTableStr.append("phone_number, format_phone_number, msg_code_str)").append(" AS ");
    createTableStr.append("SELECT msg_id, rcs_id, slot_id, receiver_number, ");
    createTableStr.append("sender_number, is_sender, sms_type, '', msg_type, ");
    createTableStr.append("start_time, end_time, msg_state, msg_title, msg_content, ");
    createTableStr.append("operator_service_number, is_lock, is_collect, is_read, ");
    createTableStr.append("session_type, retry_number, session_id, group_id, ");
    createTableStr.append("device_id, is_subsection, is_send_report, msg_code, '', '', ");
    createTableStr.append("'', '', '', '', '', '', '', '', '', '', ");
    createTableStr.append("is_advanced_security, expires_time, mms_pdu, ");
    createTableStr.append("is_blocked, blocked_reason, blocked_sources, blocked_type,");
    createTableStr.append(" blocked_type_text, blocked_pref_number, '', '', ");
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
    createTableStr.append("is_blocked, blocked_reason, blocked_sources, blocked_type,");
    createTableStr.append(" blocked_type_text, blocked_pref_number, ");
    createTableStr.append("clur_size, total_size, risk_url_body, has_be_reported_as_spam, is_report, ");
    createTableStr.append("detect_res_content, ");
    createTableStr.append("phone_number, format_phone_number, '' ");
    createTableStr.append("FROM rcs_info ").append("ORDER BY start_time, rcs_id ASC");
}

void RdbSmsMmsCallback::CreateRiskUrlRecordTableStrCb(std::string &createTableStr)
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

void RdbSmsMmsCallback::CreateTableChatBotsInfoUrlRecordTableStrCb(std::string &createChatBotsTable)
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

    void RdbSmsMmsCallback::CreateTableSmcInfoUrlRecordTableStrCb(std::string &createSmcTable)
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

void RdbSmsMmsCallback::CreateTableSpecificChatBotsInfoUrlRecordTableStrCb(std::string &createSpecificChatBotsTable)
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

void RdbSmsMmsCallback::CreateInsertSmsTriggerStrCb(std::string &createInsertSmsTriggerStr)
{
    createInsertSmsTriggerStr.append("CREATE TRIGGER IF NOT EXISTS insertSms_unread_count ");
    createInsertSmsTriggerStr.append("AFTER INSERT ON sms_mms_info ");
    createInsertSmsTriggerStr.append("WHEN NEW.is_read = 0 ");
    createInsertSmsTriggerStr.append("BEGIN ");
    createInsertSmsTriggerStr.append("UPDATE session SET unread_count = (SELECT SUM(unread) FROM ");
    createInsertSmsTriggerStr.append("(SELECT COUNT(*) AS unread FROM sms_mms_info WHERE session_id = NEW.session_id ");
    createInsertSmsTriggerStr.append("AND is_read = 0 UNION ALL ");
    createInsertSmsTriggerStr.append("SELECT COUNT(*) AS unread FROM rcs_info ");
    createInsertSmsTriggerStr.append("WHERE session_id = NEW.session_id AND is_read = 0)) ");
    createInsertSmsTriggerStr.append("WHERE session.id = NEW.session_id; ");
    createInsertSmsTriggerStr.append("END;");
}

void RdbSmsMmsCallback::CreateInsertRcsTriggerStrCb(std::string &createInsertRcsTriggerStr)
{
    createInsertRcsTriggerStr.append("CREATE TRIGGER IF NOT EXISTS insertRcs_unread_count AFTER INSERT ON rcs_info ");
    createInsertRcsTriggerStr.append("WHEN NEW.is_read = 0 ");
    createInsertRcsTriggerStr.append("BEGIN ");
    createInsertRcsTriggerStr.append("UPDATE session SET unread_count = (SELECT SUM(unread) FROM ");
    createInsertRcsTriggerStr.append("(SELECT COUNT(*) AS unread FROM sms_mms_info WHERE session_id = NEW.session_id ");
    createInsertRcsTriggerStr.append("AND is_read = 0 UNION ALL ");
    createInsertRcsTriggerStr.append("SELECT COUNT(*) AS unread FROM rcs_info ");
    createInsertRcsTriggerStr.append("WHERE session_id = NEW.session_id AND is_read = 0)) ");
    createInsertRcsTriggerStr.append("WHERE session.id = NEW.session_id; ");
    createInsertRcsTriggerStr.append("END;");
}

    void RdbSmsMmsCallback::CreateUpdateBlockSmsTriggerStrCb(std::string &createUpdateBlockSmsTriggerStr)
    {
        createUpdateBlockSmsTriggerStr.append("CREATE TRIGGER IF NOT EXISTS updateBlockSms_unread_count ");
        createUpdateBlockSmsTriggerStr.append("AFTER UPDATE ON sms_mms_info ");
        createUpdateBlockSmsTriggerStr.append("WHEN NEW.is_read = 0 ");
        createUpdateBlockSmsTriggerStr.append("BEGIN ");
        createUpdateBlockSmsTriggerStr.append("UPDATE session SET unread_count = (SELECT SUM(unread) FROM ");
        createUpdateBlockSmsTriggerStr.append("(SELECT COUNT(*) AS unread FROM sms_mms_info ");
        createUpdateBlockSmsTriggerStr.append("WHERE session_id = NEW.session_id ");
        createUpdateBlockSmsTriggerStr.append("AND is_read = 0 UNION ALL ");
        createUpdateBlockSmsTriggerStr.append("SELECT COUNT(*) AS unread FROM rcs_info ");
        createUpdateBlockSmsTriggerStr.append("WHERE session_id = NEW.session_id AND is_read = 0)) ");
        createUpdateBlockSmsTriggerStr.append("WHERE session.id = NEW.session_id; ");
        createUpdateBlockSmsTriggerStr.append("END; ");
    }

    void RdbSmsMmsCallback::CreateUpdateBlockRcsTriggerStrCb(std::string &createUpdateBlockRcsTriggerStr)
    {
        createUpdateBlockRcsTriggerStr.append("CREATE TRIGGER IF NOT EXISTS updateBlockRcs_unread_count ");
        createUpdateBlockRcsTriggerStr.append("AFTER UPDATE ON rcs_info ");
        createUpdateBlockRcsTriggerStr.append("WHEN NEW.is_read = 0 ");
        createUpdateBlockRcsTriggerStr.append("BEGIN ");
        createUpdateBlockRcsTriggerStr.append("UPDATE session SET unread_count = (SELECT SUM(unread) FROM ");
        createUpdateBlockRcsTriggerStr.append("(SELECT COUNT(*) AS unread FROM sms_mms_info ");
        createUpdateBlockRcsTriggerStr.append("WHERE session_id = NEW.session_id ");
        createUpdateBlockRcsTriggerStr.append("AND is_read = 0 UNION ALL ");
        createUpdateBlockRcsTriggerStr.append("SELECT COUNT(*) AS unread FROM rcs_info ");
        createUpdateBlockRcsTriggerStr.append("WHERE session_id = NEW.session_id AND is_read = 0)) ");
        createUpdateBlockRcsTriggerStr.append("WHERE session.id = NEW.session_id; ");
        createUpdateBlockRcsTriggerStr.append("END; ");
    }

} // namespace Telephony
} // namespace OHOS
