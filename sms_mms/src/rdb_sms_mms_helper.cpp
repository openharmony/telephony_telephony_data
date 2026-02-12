/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include <filesystem>
#include <fstream>
#include "preferences_util.h"
#include <climits>
#include "rdb_errno.h"
#include "rdb_helper.h"
#include "rdb_sms_mms_callback.h"
#include "rdb_store_config.h"
#include "rdb_utils.h"
#include "sms_mms_data.h"
#include "time_util.h"
#include "rdb_sql_utils.h"
#include <sstream>

#include <chrono>
#include <mutex>
#include "rdb_sms_mms_util.h"
#include "datashare_predicates_def.h"
#include <parameters.h>
#include "screenlock_manager.h"
#include "sms_mms_tables.h"
#include "hisysevent.h"

namespace OHOS {
namespace NativeRdb {
class ResultSet;
class ValuesBucket;
} // namespace NativeRdb
namespace Telephony {
using namespace std;
const int32_t SMS_SENDER_RECEIVE_TIMEOUT = 60;
const int32_t SMS_MMS_PER_SECOND = 1000;
const int32_t MOVE_TO_EL2_RESULT_DEFAULT = 2;
const int32_t MOVE_TO_EL2_RESULT_FAIL = 1;
const int32_t MOVE_TO_EL2_RESULT_SUCCESS = 0;
const int32_t DIR_JUDGE_RETURN_TRUE = 0;
const int32_t DIR_JUDGE_RETURN_FALSE = 1;
const int32_t DIR_JUDGE_RETURN_GO = 2;
const std::string MOVE_TO_EL2_RESULT = "move_to_el2_result";
constexpr char MMS_UE[] = "MMS_UE";
namespace {
std::mutex g_mutex;
bool moveToEl2Result = false;
}

RdbSmsMmsHelper::RdbSmsMmsHelper()
{
    DATA_STORAGE_LOGI("new RdbSmsMmsHelper");
}

RdbSmsMmsHelper::~RdbSmsMmsHelper()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    stopRequested_.store(true);
}

int RdbSmsMmsHelper::Init()
{
    DATA_STORAGE_LOGI("RdbSmsMmsHelper::Init");
    int errCode = OHOS::NativeRdb::E_OK;
    if (MoveDbFileToEl2()) {
        DATA_STORAGE_LOGE("RdbSmsMmsHelper Database file moved el2 successfully.");
        moveToEl2Result = true;
    } else {
        DATA_STORAGE_LOGE("RdbSmsMmsHelper Database file moved el2 failed, use el1 database.");
    }

    int el1ErrCode = OHOS::NativeRdb::E_OK;
    int el2ErrCode = OHOS::NativeRdb::E_OK;
    //不再强制使用el1下面的数据库,根据迁移结果来选择，迁移失败，只开el1的 sms_mms.db，成功，开el1的smsmms_el1.db和el5的smsmms.db
    if (!moveToEl2Result) {
        errCode = ForceCreateEl1RdbStore();
        if (errCode != OHOS::NativeRdb::E_OK) {
            DATA_STORAGE_LOGE("RdbSmsMmsHelper::ForceCreateEl1RdbStore create el1 sms_mms.db Fail");
            return errCode;
        }
        InitInfoQuerySql();
        ChangeRdbStoreToEl1(); // 指向EL1。
        return errCode;
    } else {
        if (!RdbStoreLTIsReady()) {
            el1ErrCode = CreateLtDataBase();
            if (el1ErrCode != OHOS::NativeRdb::E_OK) {
                DATA_STORAGE_LOGE("RdbSmsMmsHelper::Init Create el1/sms_mms_el1.db Fail");
            }
        }
        el2ErrCode = CreatSql();
        if (el2ErrCode != OHOS::NativeRdb::E_OK) {
            DATA_STORAGE_LOGE("RdbSmsMmsHelper::Init CreateSql el2/sms_mms.db Fail");
        }
        if (el1ErrCode == OHOS::NativeRdb::E_OK && el2ErrCode == OHOS::NativeRdb::E_OK) {
            ChangeRdbStoreToEl2();
            DATA_STORAGE_LOGI("el1 and el2 is ok");
        } else if ((el2ErrCode != OHOS::NativeRdb::E_OK) && (el1ErrCode == OHOS::NativeRdb::E_OK)) {
            ChangeRdbStoreToLT();
            DATA_STORAGE_LOGE("el1 ok, el2 is not ok");
        } else if ((el2ErrCode == OHOS::NativeRdb::E_OK) && (el1ErrCode != OHOS::NativeRdb::E_OK)) {
            ChangeRdbStoreToEl2();
            DATA_STORAGE_LOGE("el1 not ok, el2 is ok");
        }

        InitInfoQuerySql();
        // el1和el5任何一个开库成功,都返回成功
        return el1ErrCode && el2ErrCode;
    }
}

int RdbSmsMmsHelper::ForceCreateEl1RdbStore()
{
    int errCode = NativeRdb::E_OK;
    int getDataBasePathErrCode = OHOS::NativeRdb::E_OK;
    dbName_ = OHOS::NativeRdb::RdbSqlUtils::GetDefaultDatabasePath(dbPath, "sms_mms.db", getDataBasePathErrCode);
    if (getDataBasePathErrCode != OHOS::NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("GetDefaultDatabasePath err :%{public}d, dbName_ = %{public}s, rdb path = %{public}s,"
                          "ts = %{public}lld", getDataBasePathErrCode, dbName_.c_str(), dbPathLT_.c_str(),
                          (long long) time(NULL));
        return getDataBasePathErrCode;
    }
    NativeRdb::RdbStoreConfig config(dbName_);
    config.SetBundleName("com.ohos.telephonydataability");
    config.SetName("sms_mms.db");
    config.SetArea(RDB_AREA_EL1);
    config.SetSearchable(false);
    config.SetSecurityLevel(OHOS::NativeRdb::SecurityLevel::S1);
    // rebuilt if corrupt occurs
    config.SetAllowRebuild(true);
    RdbSmsMmsTables tables;
    std::vector<std::string> createTableVec = tables.InitCreateTableVec();
    RdbSmsMmsCallback callback(createTableVec);
    CreateEl1RdbStore(config, VERSION, callback, errCode);

    return errCode;
}

int RdbSmsMmsHelper::CreateLtDataBase()
{
    int errCode = NativeRdb::E_OK;
    int getDataBasePathErrCode = OHOS::NativeRdb::E_OK;
    dbName_ = OHOS::NativeRdb::RdbSqlUtils::GetDefaultDatabasePath(
        dbPathLT_, "sms_mms_el1.db", getDataBasePathErrCode);
    if (getDataBasePathErrCode != OHOS::NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("GetDefaultDatabasePath err :%{public}d, dbName_ = %{public}s, rdb path = %{public}s,"
            "ts = %{public}lld", getDataBasePathErrCode,
            dbName_.c_str(), dbPathLT_.c_str(), (long long) time(NULL));
        return getDataBasePathErrCode;
    }
    OHOS::NativeRdb::RdbStoreConfig config(dbName_);
    config.SetBundleName("com.ohos.telephonydataability");
    config.SetName("sms_mms_el1.db");
    config.SetSearchable(false);
    config.SetArea(RDB_AREA_EL1);
    config.SetSecurityLevel(OHOS::NativeRdb::SecurityLevel::S1);
    // rebuilt if corrupt occurs
    config.SetAllowRebuild(true);
    RdbSmsMmsTables tables;
    std::vector<std::string> createTableVec = tables.InitCreateTableVec();
    RdbSmsMmsCallback callback(createTableVec);
    CreateRdbStoreLT(config, VERSION, callback, errCode);
    return errCode;
}

int RdbSmsMmsHelper::CreatSql()
{
    std::string newDbPath = "/data/storage/el2/database/rdb/";
    std::string newDbFilePath = newDbPath + "sms_mms.db";
    std::string oldDbPath = "/data/storage/el1/database/rdb/";
    std::string oldDbFilePath = newDbPath + "sms_mms.db";
    char *newDbSuccessFlagPath = realpath(newDbFilePath.c_str(), NULL);
    char *oldDbSuccessFlagPath = realpath(oldDbFilePath.c_str(), NULL);

    // el1存在的情况下，不可以开el2库。继续使用el1。
    if (newDbSuccessFlagPath == NULL && oldDbSuccessFlagPath != NULL) {
        DATA_STORAGE_LOGE("el2 Database sms_mms.db is not exist, not allow open db, wait move db from el1.");
        free(newDbSuccessFlagPath);
        free(oldDbSuccessFlagPath);
        return NativeRdb::E_ERROR;
    }
    free(newDbSuccessFlagPath);
    free(oldDbSuccessFlagPath);

    std::string rdbName = "sms_mms";
    int errCode = NativeRdb::E_OK;
    if (IsExistStoreE() == NativeRdb::E_OK) {
        return NativeRdb::E_OK;
    }

    int getDataBasePathErrCode = OHOS::NativeRdb::E_OK;
    std::string databasePath = "";
    databasePath = dbPathE_;
    rdbName += ".db";
    dbName_ = OHOS::NativeRdb::RdbSqlUtils::GetDefaultDatabasePath(
        databasePath, rdbName, getDataBasePathErrCode);
    if (getDataBasePathErrCode != OHOS::NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("GetDefaultDatabasePath err :%{public}d, dbName_ = %{public}s, rdb path = %{public}s,"
            "ts = %{public}lld", getDataBasePathErrCode, dbName_.c_str(), databasePath.c_str(), (long long) time(NULL));
        return getDataBasePathErrCode;
    }
    OHOS::NativeRdb::RdbStoreConfig config(dbName_);
    config.SetBundleName("com.ohos.telephonydataability");
    config.SetName(rdbName);
    config.SetArea(RDB_AREA_EL2);
    config.SetSecurityLevel(OHOS::NativeRdb::SecurityLevel::S2);
    config.SetSearchable(true);
    // rebuilt if corrupt occurs
    config.SetAllowRebuild(true);
    RdbSmsMmsTables tables;
    std::vector<std::string> createTableVec = tables.InitCreateTableVec();
    RdbSmsMmsCallback callback(createTableVec);
    CreateRdbStoreSms(config, VERSION, callback, errCode);
    return errCode;
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

int RdbSmsMmsHelper::InsertSmsMmsInfo(int64_t &id, const NativeRdb::ValuesBucket &value)
{
    return Insert(id, value, TABLE_SMS_MMS_INFO);
}

int32_t RdbSmsMmsHelper::BatchInsertSmsMmsInfo(int64_t &id,
    const std::vector<DataShare::DataShareValuesBucket> &values)
{
    int32_t result = BeginTransaction();
    if (result != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("RdbSmsMmsHelper::BatchInsertSmsMmsInfo BeginTransaction error!");
        return result;
    }
    for (const auto &item : values) {
        OHOS::NativeRdb::ValuesBucket value = RdbDataShareAdapter::RdbUtils::ToValuesBucket(item);
        result = InsertSmsMmsInfo(id, value);
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

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsHelper::QueryMaxGroupId()
{
    std::string sql;
    std::string maxGroupId("maxGroupId");
    sql.append("select MAX(").append(SmsMmsInfo::GROUP_ID).append(") as ");
    sql.append(maxGroupId).append(" from ( ");
    sql.append("select ").append(SmsMmsInfo::GROUP_ID).append(" from ");
    sql.append(TABLE_SMS_MMS_INFO).append(" UNION ALL ");
    sql.append("select ").append(RcsInfo::GROUP_ID).append(" from ");
    sql.append(TABLE_RCS_INFO).append(")");
    return QuerySql(sql);
}

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsHelper::StatisticsUnRead()
{
    std::string sql;
    sql.append("SELECT SUM(unread_count) AS totalListCount, ");
    sql.append("SUM(CASE WHEN sms_type = 0 THEN unread_count ELSE 0 END) AS unreadCount, ");
    sql.append("SUM(CASE WHEN sms_type = 2 THEN unread_count ELSE 0 END) AS unreadInfo, ");
    sql.append("SUM(CASE WHEN sms_type = 1 THEN unread_count ELSE 0 END) AS unreadTotalOfInfo FROM session");
    return QuerySql(sql);
}

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsHelper::StatisticsSelectInfoCount()
{
    std::string sql;
    sql.append("select sum(msg_num) as msgNumTotal , sum(rcs_num) as rcsNumTotal , ");
    sql.append("sum(msg_num) + sum(rcs_num) as totalNum  from ( ");
    sql.append("select msg_id as mid , session_id , 1 as msg_num , 0 as rcs_num from ");
    sql.append(TABLE_SMS_MMS_INFO);
    sql.append(" where is_blocked = 0");
    sql.append(" union all ");
    sql.append(" select rcs_id as mid, session_id , 0 as msg_num , 1 as rcs_num from ");
    sql.append(TABLE_RCS_INFO);
    sql.append(" where is_blocked = 0");
    sql.append(" ) as combined_table");
    return QuerySql(sql);
}

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsHelper::StatisticsSelectContactId()
{
    std::string sql;
    sql.append("select telephone, contact_id from ");
    sql.append(TABLE_SESSION);
    sql.append(" where contact_id != '' ");
    return QuerySql(sql);
}

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsHelper::QueryConversionList(std::string infoLimitSql,
    bool isQueryNotifiyMsg)
{
    std::string sql;
    sql.append("select * from ");
    sql.append(TABLE_SESSION);
    if (!isQueryNotifiyMsg) {
        sql.append(" WHERE sms_type = 0");
    }
    sql.append(" ORDER BY ");
    sql.append(" CASE WHEN pinning_time = 0 THEN 1 ELSE 0 END, ");
    sql.append(" CASE WHEN time > pinning_time THEN time ELSE pinning_time END DESC ");
    sql.append(" limit ");
    sql.append(infoLimitSql);
    return QuerySql(sql);
}

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsHelper::QueryMatchedRecipient(std::string recipients) {
    std::string sql;
    std::string recipientsFormat = addQuotes(recipients);
    // 通过比较 `telephone` 字段中逗号的数量 + 1，判断电话号码数量是否一致。
    // 使用 `json_each` 将电话号码字符串转换为 JSON 数组。
    // 使用子查询对结果按 `value` 排序。
    // 使用 `GROUP_CONCAT` 按顺序拼接结果并比较。
    sql.append("SELECT * FROM session WHERE (LENGTH(telephone) - LENGTH(REPLACE(telephone, ',', '')) + 1) = ");
    sql.append("(LENGTH('");
    sql.append(recipients);
    sql.append("') - LENGTH(REPLACE('");
    sql.append(recipients);
    sql.append("', ',', '')) + 1) ");
    sql.append("AND ( SELECT GROUP_CONCAT(value, ',') FROM ( SELECT value ");
    sql.append("FROM json_each('[\"' || REPLACE(telephone, ',', '\",\"') || '\"]') ");
    sql.append("ORDER BY value)) = (SELECT GROUP_CONCAT(value, ',') FROM (SELECT value FROM json_each('[");
    sql.append(recipientsFormat);
    sql.append("]') ORDER BY value));");
    return QuerySql(sql);
}

std::string RdbSmsMmsHelper::addQuotes(const std::string& input) {
    std::string result;
    bool inNumber = false;
    for (char ch : input) {
        if (ch == ',') {
            result += "\"";
            inNumber = false;
            result += ch;
        } else {
            if (!inNumber) {
                result += "\"";
                inNumber = true;
            }
            result += ch;
        }
    }
    if (inNumber) {
        result += "\"";
    }
    return result;
}

int RdbSmsMmsHelper::StatisticsCleanInvalidSession()
{
    std::string sql;
    sql.append("DELETE from ");
    sql.append(TABLE_SESSION);
    sql.append(" WHERE (");
    sql.append("( session.id NOT IN ( SELECT sms_mms_info.session_id FROM sms_mms_info ");
    sql.append("WHERE sms_mms_info.is_blocked != 1))");
    sql.append(" OR ");
    sql.append("( session.time IS 0 )");
    sql.append(")");
    return ExecuteSql(sql);
}

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsHelper::StatisticsQueryInvalidSession()
{
    std::string sql;
    sql.append("SELECT id from session ");
    sql.append("WHERE (");
    sql.append("( session.id NOT IN ( SELECT sms_mms_info.session_id FROM sms_mms_info ");
    sql.append("WHERE sms_mms_info.is_blocked != 1))");
    sql.append(" OR ");
    sql.append("( session.time IS 0 )");
    sql.append(")");
    return QuerySql(sql);
}

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsHelper::StatisticsQueryInvalidSmsInfo()
{
    // sms_mms_info 中 session_id 不在 session.id 列表的，属于无关联会话的info;
    // 需要删除时，同时保证关联的 rcs_info（rcs信息会同时存在info和rcs_info） 和 part 表的清理
    std::string sql;
    sql.append("SELECT msg_id from sms_mms_info ");
    sql.append("WHERE sms_mms_info.session_id NOT IN ( SELECT session.id FROM session ) ");
    sql.append("AND sms_mms_info.is_blocked != 1;");
    return QuerySql(sql);
}

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsHelper::StatisticsQueryInvalidRcsInfo()
{
    // rcs_info 中 session_id 不在 session.id 列表的，属于无关联会话的info;
    // 需要删除时，同时保证关联的 part 表的清理
    std::string sql;
    sql.append("SELECT rcs_id from rcs_info ");
    sql.append("WHERE rcs_info.session_id NOT IN ( SELECT session.id FROM session ) ");
    sql.append("AND rcs_info.is_blocked != 1;");
    return QuerySql(sql);
}

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsHelper::StatisticsQueryInvalidRcsInfoWithMsg()
{
    // rcs_info 中 rcs_id 不在 sms_mms_info.rcs_id 列表里面的；
    // 这类信息可能来源于无5.0早期rcs和info未融合的数据
    std::string sql;
    sql.append("SELECT rcs_id from rcs_info ");
    sql.append("WHERE rcs_info.rcs_id NOT IN ( SELECT sms_mms_info.rcs_id FROM sms_mms_info );");
    return QuerySql(sql);
}

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsHelper::StatisticsQueryInvalidPart()
{
    // msg_id 不在 sms_mms_info.msg_id 列表的 或者 非null和非0的rcs_id 不在 rcs_info.rcs_id 列表的
    // 需要删除时，同时保证关联的资源清理
    std::string sql;
    sql.append("SELECT id FROM mms_part ");
    sql.append("WHERE ");
    sql.append("(mms_part.msg_id IS NOT NULL AND mms_part.msg_id != 0 AND ");
    sql.append("mms_part.msg_id NOT IN (SELECT msg_id FROM sms_mms_info)) ");
    sql.append("OR ");
    sql.append("(mms_part.rcs_id IS NOT NULL AND mms_part.rcs_id != 0 AND ");
    sql.append("mms_part.rcs_id NOT IN (SELECT rcs_id FROM rcs_info));");
    return QuerySql(sql);
}

int RdbSmsMmsHelper::MarkedSmsNoticeASRead()
{
    int32_t result = NativeRdb::E_ERROR;
    std::string sql;
    sql.append("UPDATE sms_mms_info ");
    sql.append("SET is_read = 1 ");
    sql.append("WHERE session_id IN ( ");
    sql.append("SELECT s.id ");
    sql.append("FROM session s ");
    sql.append("WHERE s.sms_type IN (1, 2));");
    result = ExecuteSql(sql);
    if (result != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("RdbSmsMmsHelper::MarkedSmsNoticeASRead sms part failed!");
    }
    return result;
}

int RdbSmsMmsHelper::MarkedRcsNoticeASRead()
{
    int32_t result = NativeRdb::E_ERROR;
    std::string sql;
    sql.append("UPDATE rcs_info ");
    sql.append("SET is_read = 1 ");
    sql.append("WHERE session_id IN ( ");
    sql.append("SELECT s.id ");
    sql.append("FROM session s ");
    sql.append("WHERE s.sms_type IN (1, 2));");
    result = ExecuteSql(sql);
    if (result != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("RdbSmsMmsHelper::MarkedRcsNoticeASRead rcs part failed!");
    }
    return result;
}

int RdbSmsMmsHelper::MarkedSessionNoticeASRead()
{
    int32_t result = NativeRdb::E_ERROR;
    std::string sql;
    sql.append("UPDATE session ");
    sql.append("SET unread_count = 0 ");
    sql.append("WHERE sms_type IN (1, 2);");
    result = ExecuteSql(sql);
    if (result != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("RdbSmsMmsHelper::MarkedSessionNoticeASRead session part failed!");
    }
    return result;
}

int RdbSmsMmsHelper::MarkedInfoASRead()
{
    if (MarkedSmsNoticeASRead() != NativeRdb::E_OK || MarkedRcsNoticeASRead() != NativeRdb::E_OK ||
        MarkedSessionNoticeASRead() != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("RdbSmsMmsHelper::MarkedInfoASRead failed!");
        return NativeRdb::E_ERROR;
    }
    return NativeRdb::E_OK;
}

int RdbSmsMmsHelper::MarkedSmsAllASRead()
{
    int32_t result = NativeRdb::E_ERROR;
    std::string sql;
    sql.append("UPDATE sms_mms_info ");
    sql.append("SET is_read = 1 ");
    sql.append("WHERE is_read = 0;");
    result = ExecuteSql(sql);
    if (result != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("RdbSmsMmsHelper::MarkedSmsAllASRead info part failed!");
    }
    return result;
}

int RdbSmsMmsHelper::MarkedRcsAllASRead()
{
    int32_t result = NativeRdb::E_ERROR;
    std::string sql;
    sql.append("UPDATE rcs_info ");
    sql.append("SET is_read = 1 ");
    sql.append("WHERE is_read = 0;");
    result = ExecuteSql(sql);
    if (result != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("RdbSmsMmsHelper::MarkedRcsAllASRead rcs part failed!");
    }
    return result;
}

int RdbSmsMmsHelper::MarkedSessionAllASRead()
{
    int32_t result = NativeRdb::E_ERROR;
    std::string sql;
    sql.append("UPDATE session ");
    sql.append("SET unread_count = 0 ");
    sql.append("WHERE unread_count != 0");
    result = ExecuteSql(sql);
    if (result != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("RdbSmsMmsHelper::MarkedSessionAllASRead session part failed!");
    }
    return result;
}

int RdbSmsMmsHelper::MarkedAllASRead()
{
    if (MarkedSmsAllASRead() != NativeRdb::E_OK || MarkedRcsAllASRead() != NativeRdb::E_OK ||
        MarkedSessionAllASRead() != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("RdbSmsMmsHelper::MarkedAllASRead failed!");
        return NativeRdb::E_ERROR;
    }
    return NativeRdb::E_OK;
}

int RdbSmsMmsHelper::MarkedSmsSingleASRead(int32_t sessionId)
{
    int32_t result = NativeRdb::E_ERROR;
    std::string sql;
    sql.append("UPDATE sms_mms_info ");
    sql.append("SET is_read = 1 ");
    sql.append("WHERE session_id = ");
    sql.append(std::to_string(sessionId));
    sql.append(";");
    result = ExecuteSql(sql);
    if (result != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("RdbSmsMmsHelper::MarkedSmsSingleASRead info part failed!");
    }
    return result;
}

int RdbSmsMmsHelper::MarkedRcsSingleASRead(int32_t sessionId)
{
    int32_t result = NativeRdb::E_ERROR;
    std::string sql;
    sql.append("UPDATE rcs_info ");
    sql.append("SET is_read = 1 ");
    sql.append("WHERE session_id = ");
    sql.append(std::to_string(sessionId));
    sql.append(";");
    result = ExecuteSql(sql);
    if (result != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("RdbSmsMmsHelper::MarkedRcsSingleASRead rcs part failed!");
    }
    return result;
}

int RdbSmsMmsHelper::MarkedSessionSingleASRead(int32_t sessionId)
{
    int32_t result = NativeRdb::E_ERROR;
    std::string sql;
    sql.append("UPDATE session ");
    sql.append("SET unread_count = 0 ");
    sql.append("WHERE id = ");
    sql.append(std::to_string(sessionId));
    sql.append(";");
    result = ExecuteSql(sql);
    if (result != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("RdbSmsMmsHelper::MarkedSessionSingleASRead session part failed!");
    }
    return result;
}

int RdbSmsMmsHelper::MarkedSingleASRead(int32_t sessionId)
{
    if (MarkedSmsSingleASRead(sessionId) != NativeRdb::E_OK || MarkedRcsSingleASRead(sessionId) != NativeRdb::E_OK ||
        MarkedSessionSingleASRead(sessionId) != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("RdbSmsMmsHelper::MarkedSingleASRead failed!");
        return NativeRdb::E_ERROR;
    }
    return NativeRdb::E_OK;
}

int RdbSmsMmsHelper::MarKReadWithTelephone(std::string &telephone, std::string &time, std::string &msgType)
{
    int32_t result = NativeRdb::E_ERROR;
    std::string sql;
    sql.append("UPDATE ");
    sql.append(msgType);
    sql.append(" SET is_read = 1 WHERE phone_number = '");
    sql.append(telephone);
    sql.append("' AND start_time = '");
    sql.append(time);
    sql.append("';");
    result = ExecuteSql(sql);
    if (result != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("MarKReadWithTelephone execute failed!");
    }
    return result;
}

int RdbSmsMmsHelper::MarKReadWithMsgId(int32_t &msgId, std::string &msgType)
{
    int32_t result = NativeRdb::E_ERROR;
    std::string sql;
    sql.append("UPDATE ");
    sql.append(msgType);
    sql.append(" SET is_read = 1 WHERE ");
    sql.append(msgType == "sms_mms_info" ? "msg_id" : "rcs_id");
    sql.append(" = ");
    sql.append(std::to_string(msgId));
    result = ExecuteSql(sql);
    if (result != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("MarKReadWithMsgId execute failed!");
    }
    return result;
}

int RdbSmsMmsHelper::UpdateUnreadCountWithTelephone(std::string &telephone, std::string &time, std::string &msgType)
{
    int32_t result = NativeRdb::E_ERROR;
    std::string sql;
    sql.append("WITH TargetSession AS ( ");
    sql.append("SELECT session_id FROM ");
    sql.append(msgType);
    sql.append(" WHERE phone_number = '");
    sql.append(telephone);
    sql.append("' AND start_time = '");
    sql.append(time);
    sql.append("' ), UnreadCount AS ( ");
    sql.append("SELECT COUNT(*) AS total_unread ");
    sql.append("FROM ( ");
    sql.append("SELECT 1 ");
    sql.append("FROM sms_mms_info ");
    sql.append("WHERE session_id = (SELECT session_id FROM TargetSession) ");
    sql.append("AND is_read = 0 ");
    sql.append("UNION ALL ");
    sql.append("SELECT 1 ");
    sql.append("FROM rcs_info ");
    sql.append("WHERE session_id = (SELECT session_id FROM TargetSession) ");
    sql.append("AND is_read = 0 ");
    sql.append(") AS combined)UPDATE session ");
    sql.append("SET unread_count = (SELECT total_unread FROM UnreadCount) ");
    sql.append("WHERE id = (SELECT session_id FROM TargetSession); ");
    result = ExecuteSql(sql);
    if (result != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("UpdateUnreadCountWithTelephone execute failed!");
    }
    return result;
}

int RdbSmsMmsHelper::UpdateUnreadCountWithMsgId(int32_t &msgId, std::string &msgType)
{
    int32_t result = NativeRdb::E_ERROR;
    std::string sql;
    std::string idStr = (msgType == "sms_mms_info") ? "msg_id" : "rcs_id";
    sql.append("WITH TargetSession AS ( ");
    sql.append("SELECT session_id FROM ");
    sql.append(msgType);
    sql.append(" WHERE ");
    sql.append(idStr);
    sql.append(" = ");
    sql.append(std::to_string(msgId));
    sql.append(" ), UnreadCount AS ( ");
    sql.append("SELECT COUNT(*) AS total_unread ");
    sql.append("FROM ( ");
    sql.append("SELECT 1 ");
    sql.append("FROM sms_mms_info ");
    sql.append("WHERE session_id = (SELECT session_id FROM TargetSession) ");
    sql.append("AND is_read = 0 ");
    sql.append("UNION ALL ");
    sql.append("SELECT 1 ");
    sql.append("FROM rcs_info ");
    sql.append("WHERE session_id = (SELECT session_id FROM TargetSession) ");
    sql.append("AND is_read = 0 ");
    sql.append(") AS combined)UPDATE session ");
    sql.append("SET unread_count = (SELECT total_unread FROM UnreadCount) ");
    sql.append("WHERE id = (SELECT session_id FROM TargetSession); ");
    result = ExecuteSql(sql);
    if (result != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("UpdateUnreadCountWithMsgId execute failed!");
    }
    return result;
}

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsHelper::GetMsgCoordinatesWithSessionId(int32_t &threadId, int32_t &msgId,
                                                                                      int32_t &sessionType)
{
    int32_t rcsId = 0;
    std::string rcsSql;
    rcsSql.append("SELECT rcs_id from sms_mms_info WHERE sms_type = 99 AND msg_id = ");
    rcsSql.append(std::to_string(msgId));
    std::string msgType = "msg_id";
    std::shared_ptr<NativeRdb::ResultSet> resultSet = QuerySql(rcsSql);
    if (resultSet != nullptr && resultSet->GoToFirstRow() == NativeRdb::E_OK) {
        int32_t columnIndex = 0;
        resultSet->GetColumnIndex(RcsInfo::RCS_ID, columnIndex);
        resultSet->GetInt(columnIndex, rcsId);
        resultSet->Close();
    }
    if (rcsId > 0) {
        msgType = "rcs_id";
    }
    std::string sql;
    sql.append("WITH FirstInGroup AS (SELECT session_id, ");
    sql.append(msgType);
    sql.append(", start_time FROM ( SELECT session_id, ");
    sql.append(msgType);
    sql.append(", start_time");
    // session 类型，0：单会话；1：群发会话；
    if (sessionType == 1) {
        sql.append(", ROW_NUMBER() OVER (PARTITION BY group_id) AS rn ");
    }
    sql.append(" FROM mms_info WHERE session_id = ");
    sql.append(std::to_string(threadId));
    sql.append(" AND msg_state != 3 AND is_blocked != 1) ranked ");
    if (sessionType == 1) {
        sql.append("WHERE rn = 1");
    }
    sql.append(" ), RankedSessions AS ( SELECT session_id, ");
    sql.append(msgType);
    sql.append(", ROW_NUMBER() OVER (ORDER BY start_time) AS final_rank FROM FirstInGroup )");
    sql.append("SELECT final_rank FROM RankedSessions WHERE ");
    sql.append(msgType);
    sql.append(" = ");
    sql.append(std::to_string(rcsId > 0 ? rcsId : msgId));
    sql.append(";");
    return QuerySql(sql);
}

int RdbSmsMmsHelper::ProcessSessionUnreadWithTelephone(std::string &telephone, std::string &time, std::string &msgType)
{
    if (MarKReadWithTelephone(telephone, time, msgType) != NativeRdb::E_OK ||
        UpdateUnreadCountWithTelephone(telephone, time, msgType) != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("RdbSmsMmsHelper::ProcessSessionUnreadWithTelephone failed!");
        return NativeRdb::E_ERROR;
    }
    return NativeRdb::E_OK;
}

int RdbSmsMmsHelper::ProcessSessionUnreadWithMsgId(int32_t &msgId, std::string &msgType)
{
    if (MarKReadWithMsgId(msgId, msgType) != NativeRdb::E_OK ||
        UpdateUnreadCountWithMsgId(msgId, msgType) != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("RdbSmsMmsHelper::ProcessSessionUnreadWithMsgId failed!");
        return NativeRdb::E_ERROR;
    }
    return NativeRdb::E_OK;
}

int RdbSmsMmsHelper::StatisticsDeleteSessionExe(std::string arr, std::string limit, std::string equals)
{
    int ret = NativeRdb::E_ERROR;
    ret = RdbSmsMmsHelper::StatisticsDeleteMmsPartWithSessionID(arr, limit, equals);
    if (ret != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("Delete MmsPart With SessionID failed ret = %{public}d", ret);
        return ret;
    }
    ret = RdbSmsMmsHelper::StatisticsUpdateMmsPartWithSessionID(arr, limit, equals);
    if (ret != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("update MmsPart reference_count With SessionID failed ret = %{public}d", ret);
        return ret;
    }
    ret = RdbSmsMmsHelper::StatisticsDeleteRcsPartWithSessionID(arr, limit, equals);
    if (ret != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("Delete RcsPart With SessionID failed ret = %{public}d", ret);
        return ret;
    }
    ret = RdbSmsMmsHelper::StatisticsUpdateRcsPartWithSessionID(arr, limit, equals);
    if (ret != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("update RcsPart reference_count With SessionID failed ret = %{public}d", ret);
        return ret;
    }
    ret = RdbSmsMmsHelper::StatisticsDeleteRCSInfoWithSessionID(arr, limit, equals);
    if (ret != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("Delete RCSInfo With SessionID failed ret = %{public}d", ret);
        return ret;
    }
    ret = RdbSmsMmsHelper::StatisticsDeleteInfoWithSessionID(arr, limit, equals);
    if (ret != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("Delete Info With SessionID failed ret = %{public}d", ret);
        return ret;
    }
    ret = RdbSmsMmsHelper::StatisticsDeleteSessionTableExe(arr, limit, equals);
    if (ret != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("Delete Session Table failed ret = %{public}d", ret);
        return ret;
    }
    return NativeRdb::E_OK;
}

int RdbSmsMmsHelper::StatisticsDeleteMmsPartWithSessionID(std::string arr, std::string limit, std::string equals)
{
    std::string sql;
    if (limit.empty()) {
        DATA_STORAGE_LOGE("The deletion scope is not limited.");
        return NativeRdb::E_ERROR;
    }
    DATA_STORAGE_LOGI("delete mms part %{public}s ids %{public}s", limit.c_str(), arr.c_str());
    sql = "DELETE FROM " + std::string(TABLE_MMS_PART) + GetDeleteMmsPartWhereSql(arr, limit, equals);
    return ExecuteSql(sql);
}

int RdbSmsMmsHelper::StatisticsUpdateMmsPartWithSessionID(std::string arr, std::string limit, std::string equals)
{
    std::string sql;
    if (limit.empty()) {
        DATA_STORAGE_LOGE("The deletion scope is not limited.");
        return NativeRdb::E_ERROR;
    }
    DATA_STORAGE_LOGI("update mms part %{public}s ids %{public}s", limit.c_str(), arr.c_str());
    if (equals.empty()) {
        sql = "UPDATE " + std::string(TABLE_MMS_PART) + " SET reference_count = reference_count -1 " +
           " WHERE reference_count > 1 " +
           " AND msg_id IN (" +
           " SELECT " + std::string(TABLE_SMS_MMS_INFO) + ".msg_id FROM " + std::string(TABLE_SMS_MMS_INFO) +
           " WHERE " + std::string(TABLE_SMS_MMS_INFO) + ".session_id " +
           std::string(limit) + " (" + std::string(arr) + "))";
    } else {
        sql = "UPDATE " + std::string(TABLE_MMS_PART) + " SET reference_count = reference_count -1 " +
           " WHERE reference_count > 1 " +
           " AND msg_id IN (" +
           " SELECT " + std::string(TABLE_SMS_MMS_INFO) + ".msg_id FROM " + std::string(TABLE_SMS_MMS_INFO) +
           " JOIN " + std::string(TABLE_SESSION) + " ON " +
           std::string(TABLE_SMS_MMS_INFO) + ".session_id = " + std::string(TABLE_SESSION) + ".id" +
           " WHERE " + std::string(TABLE_SMS_MMS_INFO) + ".session_id " +
           std::string(limit) + " (" + std::string(arr) + ")" +
           " AND " + std::string(TABLE_SESSION) + "." + std::string(equals) + ")";
    }
    return ExecuteSql(sql);
}

int RdbSmsMmsHelper::StatisticsUpdateRcsPartWithSessionID(std::string arr, std::string limit, std::string equals)
{
    std::string sql;
    if (limit.empty()) {
        DATA_STORAGE_LOGE("The deletion scope is not limited.");
        return NativeRdb::E_ERROR;
    }
    DATA_STORAGE_LOGI("update rcs part %{public}s ids %{public}s", limit.c_str(), arr.c_str());
    if (equals.empty()) {
        sql = "UPDATE " + std::string(TABLE_MMS_PART) + " SET reference_count = reference_count -1 " +
            " WHERE reference_count > 1 " +
            " AND rcs_id IN (" +
            " SELECT " + std::string(TABLE_SMS_MMS_INFO) + ".rcs_id" +
            " FROM " + std::string(TABLE_SMS_MMS_INFO) +
            " WHERE " + std::string(TABLE_SMS_MMS_INFO) + ".session_id " +
            std::string(limit) + " (" + std::string(arr) + ")" +
            " AND " + std::string(TABLE_SMS_MMS_INFO) + ".rcs_id > 0 " +
            " AND " + std::string(TABLE_SMS_MMS_INFO) + ".is_blocked != 1)";
    } else {
        sql = "UPDATE " + std::string(TABLE_MMS_PART) + " SET reference_count = reference_count -1 " +
            " WHERE reference_count > 1 " + 
            " AND rcs_id IN (" +
            " SELECT " + std::string(TABLE_SMS_MMS_INFO) + ".rcs_id" +
            " FROM " + std::string(TABLE_SMS_MMS_INFO) +
            " JOIN " + std::string(TABLE_SESSION) + " ON " + std::string(TABLE_SMS_MMS_INFO) +
            ".session_id = " + std::string(TABLE_SESSION) + ".id" +
            " WHERE " + std::string(TABLE_SMS_MMS_INFO) + ".session_id " +
            std::string(limit) + " (" + std::string(arr) + ")" +
            " AND " + std::string(TABLE_SESSION) + "." + std::string(equals) +
            " AND " + std::string(TABLE_SMS_MMS_INFO) + ".rcs_id > 0 "+
            " AND " + std::string(TABLE_SMS_MMS_INFO) + ".is_blocked != 1)";
    }
    return ExecuteSql(sql);
}

int RdbSmsMmsHelper::StatisticsDeleteRcsPartWithSessionID(std::string arr, std::string limit, std::string equals)
{
    std::string sql;
    if (limit.empty()) {
        DATA_STORAGE_LOGE("The deletion scope is not limited.");
        return NativeRdb::E_ERROR;
    }
    DATA_STORAGE_LOGI("delete rcs part %{public}s ids %{public}s", limit.c_str(), arr.c_str());
    sql = "DELETE FROM " + std::string(TABLE_MMS_PART) + GetDeleteRcsPartWhereSql(arr, limit, equals);
    return ExecuteSql(sql);
}

int RdbSmsMmsHelper::StatisticsDeleteRCSInfoWithSessionID(std::string arr, std::string limit, std::string equals)
{
    std::string sql;
    if (limit.empty()) {
        DATA_STORAGE_LOGE("The deletion scope is not limited.");
        return NativeRdb::E_ERROR;
    }
    DATA_STORAGE_LOGI("delete rcs info %{public}s ids %{public}s", limit.c_str(), arr.c_str());
    if (equals.empty()) {
        sql = "DELETE FROM " + std::string(TABLE_RCS_INFO) +
           " WHERE "  +  std::string(TABLE_RCS_INFO) + ".is_blocked != 1" +
           " AND rcs_id IN (" +
           " SELECT " + std::string(TABLE_SMS_MMS_INFO) + ".rcs_id" +
           " FROM " + std::string(TABLE_SMS_MMS_INFO) + "" +
           " WHERE " + std::string(TABLE_SMS_MMS_INFO) + ".session_id " +
           std::string(limit) + " (" + std::string(arr) + ")" +
           ");";
    } else {
        sql = "DELETE FROM " + std::string(TABLE_RCS_INFO) +
           " WHERE " +  std::string(TABLE_RCS_INFO) + ".is_blocked != 1" +
           " AND rcs_id IN (" +
           " SELECT " + std::string(TABLE_SMS_MMS_INFO) + ".rcs_id" +
           " FROM " + std::string(TABLE_SMS_MMS_INFO) + "" +
           " JOIN " + std::string(TABLE_SESSION) + " ON " + std::string(TABLE_SMS_MMS_INFO) +
           ".session_id = " + std::string(TABLE_SESSION) + ".id" +
           " WHERE " + std::string(TABLE_SMS_MMS_INFO) + ".session_id " +
           std::string(limit) + " (" + std::string(arr) + ")" +
           " AND " + std::string(TABLE_SESSION) + "." + std::string(equals) + "" +
           ");";
    }
    return ExecuteSql(sql);
}

int RdbSmsMmsHelper::StatisticsDeleteInfoWithSessionID(std::string arr, std::string limit, std::string equals)
{
    std::string sql;
    if (limit.empty()) {
        DATA_STORAGE_LOGE("The deletion scope is not limited.");
        return NativeRdb::E_ERROR;
    }
    DATA_STORAGE_LOGI("delete info %{public}s ids %{public}s", limit.c_str(), arr.c_str());
    if (equals.empty()) {
        sql = "DELETE FROM " + std::string(TABLE_SMS_MMS_INFO) +
            " WHERE " + std::string(TABLE_SMS_MMS_INFO) + ".session_id " +
            std::string(limit) + " (" + std::string(arr) + ")" +
            " AND " +  std::string(TABLE_SMS_MMS_INFO) + ".is_blocked != 1;";
    } else {
        sql = "DELETE FROM " + std::string(TABLE_SMS_MMS_INFO) +
           " WHERE " + std::string(TABLE_SMS_MMS_INFO) + ".session_id " +
           std::string(limit) + " (" + std::string(arr) + ") " + 
            " AND " +  std::string(TABLE_SMS_MMS_INFO) + ".is_blocked != 1" +
            " AND " + "EXISTS ( SELECT 1 FROM " + std::string(TABLE_SESSION) +
           " WHERE " + std::string(TABLE_SESSION) + ".id = " + std::string(TABLE_SMS_MMS_INFO) + ".session_id AND " +
           equals +
           ")";
    }
    return ExecuteSql(sql);
}

int RdbSmsMmsHelper::StatisticsDeleteSessionTableExe(std::string arr, std::string limit, std::string equals)
{
    std::string sql;
    std::string conditions;
    if (limit.empty()) {
        DATA_STORAGE_LOGE("The deletion scope is not limited.");
        return NativeRdb::E_ERROR;
    }
    DATA_STORAGE_LOGI("delete threads %{public}s ids %{public}s", limit.c_str(), arr.c_str());
    if (equals.empty()) {
        conditions.append("id ").append(limit).append(" (").append(arr).append(")");
    } else {
        conditions.append("((id ").append(limit).append(" (").append(arr).append(")) AND ( ").append(equals)
            .append(" ))");
    }
    sql.append("DELETE FROM ").append(TABLE_SESSION).append(" WHERE ").append(conditions);
    return ExecuteSql(sql);
}

std::string RdbSmsMmsHelper::SessionIDsParse(std::vector<std::string> sessionIDs)
{
    std::string sessionStr = "";
    for (const auto& item : sessionIDs) {
        std::string trimmed = item.substr(0, item.find('.'));
        if (!sessionStr.empty()) {
            sessionStr += ", ";
        }
        sessionStr += trimmed;
    }
    return sessionStr;
}

int RdbSmsMmsHelper::StatisticsDeleteSession(const DataShare::DataSharePredicates &predicates)
{
    if (predicates.GetOperationList().size() <= 0) {
        return 0;
    }
    std::string sessionStr;
    std::string limit = "";
    std::string equals = "";
    for (uint32_t index = 0; index < predicates.GetOperationList().size(); index++) {
        DataShare::OperationItem operationItem = predicates.GetOperationList()[index];
        if (operationItem.operation == DataShare::OperationType::INVALID_OPERATION) {
            DATA_STORAGE_LOGI("operation is INVALID_OPERATION continue");
            continue;
        } else if (operationItem.operation == DataShare::OperationType::SQL_IN) {
            limit = std::get<std::string>(operationItem.singleParams[0]);
            std::vector<std::string> sessionIDs = std::get<std::vector<std::string>>(operationItem.multiParams[0]);
            sessionStr = RdbSmsMmsHelper::SessionIDsParse(sessionIDs);
        } else if (operationItem.operation == DataShare::EQUAL_TO &&
                operationItem.singleParams.size() == 3L) {
            if (std::holds_alternative<std::string>(operationItem.singleParams[0]) &&
                    std::holds_alternative<double>(operationItem.singleParams[1])) {
                equals.append(std::get<std::string>(operationItem.singleParams[0]))
                .append(" = ")
                .append(std::to_string(int(std::get<double>(operationItem.singleParams[1]))));
            }
        } else {
            DATA_STORAGE_LOGE("singleParams is Unknown");
        }
    }
    return RdbSmsMmsHelper::StatisticsDeleteSessionExe(sessionStr, limit, equals);
}

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsHelper::QueryMmsPathsWithSessionIds(
    const DataShare::DataSharePredicates &predicates)
{
    if (predicates.GetOperationList().size() <= 0) {
        return nullptr;
    }
    std::string sessionStr;
    std::string limit = "";
    std::string equals = "";
    for (uint32_t index = 0; index < predicates.GetOperationList().size(); index++) {
        DataShare::OperationItem operationItem = predicates.GetOperationList()[index];
        if (operationItem.operation == DataShare::OperationType::INVALID_OPERATION) {
            DATA_STORAGE_LOGI("operation is INVALID_OPERATION continue");
            continue;
        } else if (operationItem.operation == DataShare::OperationType::SQL_IN) {
            limit = std::get<std::string>(operationItem.singleParams[0]);
            std::vector<std::string> sessionIDs = std::get<std::vector<std::string>>(operationItem.multiParams[0]);
            sessionStr = RdbSmsMmsHelper::SessionIDsParse(sessionIDs);
        } else if (operationItem.operation == DataShare::EQUAL_TO &&
                operationItem.singleParams.size() == 3L) {
            if (std::holds_alternative<std::string>(operationItem.singleParams[0]) &&
                    std::holds_alternative<double>(operationItem.singleParams[1])) {
                equals.append(std::get<std::string>(operationItem.singleParams[0]))
                .append(" = ")
                .append(std::to_string(int(std::get<double>(operationItem.singleParams[1]))));
            }
        } else {
            DATA_STORAGE_LOGE("singleParams is Unknown");
        }
    }
    return RdbSmsMmsHelper::QueryMmsPathsWithSessionIdsExe(sessionStr, limit, equals);
}

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsHelper::QueryMmsPathsWithSessionIdsExe(std::string arr, 
    std::string limit, std::string equals)
{
    std::string sql;
    std::string sql1;
    std::string sql2;
    if (limit.empty()) {
        DATA_STORAGE_LOGE("QueryMmsPathsWithSessionIdsExe The deletion scope is not limited.");
        return nullptr;
    }
    DATA_STORAGE_LOGI("query mms paths %{public}s ids %{public}s", limit.c_str(), arr.c_str());
    sql1 = "SELECT location_path FROM " + std::string(TABLE_MMS_PART) + GetDeleteRcsPartWhereSql(arr, limit, equals);
    sql2 = "SELECT location_path FROM " + std::string(TABLE_MMS_PART) + GetDeleteMmsPartWhereSql(arr, limit, equals);
    sql = sql1 + " UNION ALL " + sql2;
    return QuerySql(sql);
}

std::string RdbSmsMmsHelper::GetDeleteMmsPartWhereSql(std::string arr,std::string limit, std::string equals)
{
    std::string whereSql;
    if (equals.empty()) {
        whereSql = std::string(" WHERE") + " (reference_count = -1 " +
           " AND " + " msg_id IN (" +
           " SELECT " + std::string(TABLE_SMS_MMS_INFO) + ".msg_id FROM " + std::string(TABLE_SMS_MMS_INFO) +
           " WHERE " + std::string(TABLE_SMS_MMS_INFO) + ".is_collect = 0 " +
           " AND " + std::string(TABLE_SMS_MMS_INFO) + ".session_id " +
           std::string(limit) + " (" + std::string(arr) + ")))" +
            " OR (reference_count = 1 AND  msg_id IN (" +
           " SELECT " + std::string(TABLE_SMS_MMS_INFO) + ".msg_id FROM " + std::string(TABLE_SMS_MMS_INFO) +
           " WHERE " + std::string(TABLE_SMS_MMS_INFO) + ".session_id " +
           std::string(limit) + " (" + std::string(arr) + ")))";
    } else {
        whereSql = std::string(" WHERE") + " (reference_count = -1 " +
           " AND " + " msg_id IN (" +
           " SELECT " + std::string(TABLE_SMS_MMS_INFO) + ".msg_id FROM " + std::string(TABLE_SMS_MMS_INFO) +
           " JOIN " + std::string(TABLE_SESSION) + " ON " +
           std::string(TABLE_SMS_MMS_INFO) + ".session_id = " + std::string(TABLE_SESSION) + ".id" +
           " WHERE " + std::string(TABLE_SMS_MMS_INFO) + ".is_collect = 0  " +
           " AND " + std::string(TABLE_SMS_MMS_INFO) + ".session_id " +
           std::string(limit) + " (" + std::string(arr) + ")" +
           " AND " + std::string(TABLE_SESSION) + "." + std::string(equals) + "))" +
           " OR (reference_count = 1 AND msg_id IN (" +
           " SELECT " + std::string(TABLE_SMS_MMS_INFO) + ".msg_id FROM " + std::string(TABLE_SMS_MMS_INFO) +
           " JOIN " + std::string(TABLE_SESSION) + " ON " +
           std::string(TABLE_SMS_MMS_INFO) + ".session_id = " + std::string(TABLE_SESSION) + ".id" +
           " WHERE " + std::string(TABLE_SMS_MMS_INFO) + ".session_id " +
           std::string(limit) + " (" + std::string(arr) + ")" +
           " AND " + std::string(TABLE_SESSION) + "." + std::string(equals) + "))";
    }
    return whereSql;
}

std::string RdbSmsMmsHelper::GetDeleteRcsPartWhereSql(std::string arr,std::string limit, std::string equals)
{
    std::string whereSql;
    if (equals.empty()) {
        whereSql = std::string(" WHERE") + " ( reference_count = -1 " +
            " AND " + " rcs_id IN (" +
            " SELECT " + std::string(TABLE_SMS_MMS_INFO) + ".rcs_id" +
            " FROM " + std::string(TABLE_SMS_MMS_INFO) +
            " WHERE " + std::string(TABLE_SMS_MMS_INFO) + ".session_id " +
            std::string(limit) + " (" + std::string(arr) + ")" +
            " AND " + std::string(TABLE_SMS_MMS_INFO) + ".rcs_id > 0 " +
            " AND " + std::string(TABLE_SMS_MMS_INFO) + ".is_collect = 0 " +
            " AND " + std::string(TABLE_SMS_MMS_INFO) + ".is_blocked != 1))" +
            " OR ( reference_count = 1 AND  rcs_id IN (" +
            " SELECT " + std::string(TABLE_SMS_MMS_INFO) + ".rcs_id " +
            " FROM " + std::string(TABLE_SMS_MMS_INFO) +
            " WHERE " + std::string(TABLE_SMS_MMS_INFO) + ".session_id " +
            std::string(limit) + " (" + std::string(arr) + ")" +
            " AND " + std::string(TABLE_SMS_MMS_INFO) + ".rcs_id > 0 " +
            " AND " + std::string(TABLE_SMS_MMS_INFO) + ".is_blocked != 1))";
    } else {
        whereSql = std::string(" WHERE") + " (reference_count = -1 " +
            " AND " + " rcs_id IN (" +
            " SELECT " + std::string(TABLE_SMS_MMS_INFO) + ".rcs_id" +
            " FROM " + std::string(TABLE_SMS_MMS_INFO) +
            " JOIN " + std::string(TABLE_SESSION) + " ON " + std::string(TABLE_SMS_MMS_INFO) +
            ".session_id = " + std::string(TABLE_SESSION) + ".id" +
            " WHERE " + std::string(TABLE_SMS_MMS_INFO) + ".session_id " +
            std::string(limit) + " (" + std::string(arr) + ")" +
            " AND " + std::string(TABLE_SESSION) + "." + std::string(equals) +
            " AND " + std::string(TABLE_SMS_MMS_INFO) + ".rcs_id > 0 "+
            " AND " + std::string(TABLE_SMS_MMS_INFO) + ".is_collect = 0 " +
            " AND " + std::string(TABLE_SMS_MMS_INFO) + ".is_blocked != 1))" +
            " OR (reference_count = 1 AND rcs_id IN (" +
            " SELECT " + std::string(TABLE_SMS_MMS_INFO) + ".rcs_id" +
            " FROM " + std::string(TABLE_SMS_MMS_INFO) +
            " JOIN " + std::string(TABLE_SESSION) + " ON " + std::string(TABLE_SMS_MMS_INFO) +
            ".session_id = " + std::string(TABLE_SESSION) + ".id" +
            " WHERE " + std::string(TABLE_SMS_MMS_INFO) + ".session_id " +
            std::string(limit) + " (" + std::string(arr) + ")" +
            " AND " + std::string(TABLE_SESSION) + "." + std::string(equals) +
            " AND " + std::string(TABLE_SMS_MMS_INFO) + ".rcs_id > 0 " +
            " AND " + std::string(TABLE_SMS_MMS_INFO) + ".is_blocked != 1))";
    }
    return whereSql;
}

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsHelper::QueryMMSVIEWS(std::string infoWhereSql, std::string infoLimitSql)
{
    std::string queryMmsInfoSql;
    if (smsInfoQuerySql.empty() || rcsInfoQuerySql.empty()) {
        InitInfoQuerySql();
    }
    queryMmsInfoSql.append(smsInfoQuerySql);
    queryMmsInfoSql.append(" and ");
    queryMmsInfoSql.append(infoWhereSql);
    queryMmsInfoSql.append(" UNION ");
    queryMmsInfoSql.append(rcsInfoQuerySql);
    queryMmsInfoSql.append(" and ");
    queryMmsInfoSql.append(infoWhereSql);
    queryMmsInfoSql.append(" ORDER BY start_time DESC limit ");
    queryMmsInfoSql.append(infoLimitSql);
    queryMmsInfoSql.append(" ;");
    return QuerySql(queryMmsInfoSql);
}

void RdbSmsMmsHelper::InitInfoQuerySql()
{
    DATA_STORAGE_LOGI("SmsMmsDB InitInfoQuerySql");
    smsInfoQuerySql = "";
    rcsInfoQuerySql = "";

    smsInfoQuerySql.append("SELECT msg_id, rcs_id, slot_id, receiver_number, sender_number, is_sender, ");
    smsInfoQuerySql.append("sms_type, '' as rcs_type, msg_type, start_time, end_time, msg_state, ");
    smsInfoQuerySql.append("msg_title, msg_content, operator_service_number, is_lock, is_collect, is_read, ");
    smsInfoQuerySql.append("session_type, retry_number, session_id, group_id, device_id, is_subsection, ");
    smsInfoQuerySql.append("is_send_report, msg_code, '' as enriched_calling_type, '' as error_code, ");
    smsInfoQuerySql.append("'' as network_type, '' as owner_addr, '' as privacy_mode, '' as protocol, ");
    smsInfoQuerySql.append("'' as reply_path_present, '' as seen, '' as receive_state, '' as fail_receive_context, ");
    smsInfoQuerySql.append("'' as service_center, '' as service_kind, is_advanced_security, expires_time, ");
    smsInfoQuerySql.append("mms_pdu, is_blocked, blocked_reason, blocked_sources, blocked_type, blocked_type_text, ");
    smsInfoQuerySql.append("blocked_pref_number, '' as clur_size, '' as total_size, risk_url_body, ");
    smsInfoQuerySql.append("has_be_reported_as_spam, is_report, detect_res_content, phone_number, ");
    smsInfoQuerySql.append("format_phone_number, msg_code_str ");
    smsInfoQuerySql.append(" FROM sms_mms_info WHERE sms_type < 99 ");

    rcsInfoQuerySql.append("SELECT msg_id, rcs_id, slot_id, receiver_number, sender_number, is_sender, ");
    rcsInfoQuerySql.append("'' as sms_type, rcs_type, '' as msg_type, start_time, end_time, msg_state, ");
    rcsInfoQuerySql.append("msg_title, msg_content, '' as operator_service_number, is_lock, is_collect, is_read, ");
    rcsInfoQuerySql.append("'' as session_type, '' as retry_number, session_id, group_id, ");
    rcsInfoQuerySql.append("'' as device_id, '' as is_subsection, '' as is_send_report, '' as msg_code, ");
    rcsInfoQuerySql.append("enriched_calling_type, error_code, network_type, owner_addr, privacy_mode, protocol, ");
    rcsInfoQuerySql.append("reply_path_present, seen, receive_state, fail_receive_context, ");
    rcsInfoQuerySql.append("service_center, service_kind, is_advanced_security, '' as expires_time, ");
    rcsInfoQuerySql.append("'' as mms_pdu, is_blocked, blocked_reason, blocked_sources, ");
    rcsInfoQuerySql.append("blocked_type, blocked_type_text, blocked_pref_number, clur_size, ");
    rcsInfoQuerySql.append("total_size, risk_url_body, has_be_reported_as_spam, is_report, ");
    rcsInfoQuerySql.append("detect_res_content, phone_number, format_phone_number, '' as msg_code_str ");
    rcsInfoQuerySql.append(" FROM  rcs_info WHERE 1 = 1 ");
}

int RdbSmsMmsHelper::HandleDirJudgement()
{
    std::string newDbPath = "/data/storage/el2/database/rdb/";
    std::string newDbFilePath = newDbPath + "sms_mms.db";
    std::string oldDbPath = "/data/storage/el1/database/rdb/";
    std::string oldDbFilePath = oldDbPath + "sms_mms.db";

    // 如果newDbFilePath存在，不需要迁移。
    char *newDbSuccessFlagPath = realpath(newDbFilePath.c_str(), NULL);
    if (newDbSuccessFlagPath != NULL) {
        DATA_STORAGE_LOGE("el2 Database exist, notice!!");
        free(newDbSuccessFlagPath);
    }

    // 如果realOldDbPath不存在，不需要迁移。
    char *realOldDbPath = realpath(oldDbFilePath.c_str(), NULL);
    if (realOldDbPath == NULL) {
        DATA_STORAGE_LOGE("el1 Database no exist, do not need to be moved.");
        return DIR_JUDGE_RETURN_TRUE;
    }
    free(realOldDbPath);

    // 创建EL2 rdb目录
    char *realnewDbPath = realpath(newDbPath.c_str(), NULL);
    if (realnewDbPath == NULL) {
        auto mkdirOk = NativeRdb::RdbSqlUtils::CreateDirectory(newDbPath);
        if (mkdirOk != NativeRdb::E_OK) {
            DATA_STORAGE_LOGE("CreateDirectory newDbPath:%{public}s directory failed.", newDbPath.c_str());
            return DIR_JUDGE_RETURN_FALSE;
        }
    }
    free(realnewDbPath);
    return DIR_JUDGE_RETURN_GO;
}

void RdbSmsMmsHelper::HandlePreferences(std::shared_ptr<PreferencesUtil> preferencesUtil, int value)
{
    if (preferencesUtil == nullptr) {
        DATA_STORAGE_LOGE("preferences is nullptr, do not operate.");
        return;
    }
    preferencesUtil->SaveIntSms(MOVE_TO_EL2_RESULT, value);
}

bool RdbSmsMmsHelper::HandleFile(std::shared_ptr<PreferencesUtil> preferencesUtil)
{
    bool success = BackupEl1ToEl2();
    if (success) {
        HandlePreferences(preferencesUtil, MOVE_TO_EL2_RESULT_SUCCESS);
    } else {
        HandlePreferences(preferencesUtil, MOVE_TO_EL2_RESULT_FAIL);
    }
    return success;
}

bool RdbSmsMmsHelper::MoveDbFileToEl2()
{
    std::lock_guard<std::mutex> guard(lock_);
    auto preferencesUtil = DelayedSingleton<PreferencesUtil>::GetInstance();
    if (preferencesUtil == nullptr) {
        DATA_STORAGE_LOGE("preferencesUtil is nullptr!");
        int res = HandleDirJudgement();
        if (res == DIR_JUDGE_RETURN_TRUE) {
            return true;
        } else if (res == DIR_JUDGE_RETURN_FALSE) {
            return false;
        }
        return HandleFile(preferencesUtil);
    } else {
        int moveResult = preferencesUtil->ObtainIntSms(MOVE_TO_EL2_RESULT, MOVE_TO_EL2_RESULT_DEFAULT);
        switch (moveResult) {
            case MOVE_TO_EL2_RESULT_SUCCESS:
                DATA_STORAGE_LOGE("move_to_el2_result is success");
                return true;
            case MOVE_TO_EL2_RESULT_FAIL:
                DATA_STORAGE_LOGE("move_to_el2_result is failed");
                return HandleFile(preferencesUtil);
            default:
                DATA_STORAGE_LOGE("move_to_el2_result is default");
                int res = HandleDirJudgement();
                if (res == DIR_JUDGE_RETURN_TRUE) {
                    preferencesUtil->SaveIntSms(MOVE_TO_EL2_RESULT, MOVE_TO_EL2_RESULT_SUCCESS);
                    return true;
                } else if (res == DIR_JUDGE_RETURN_FALSE) {
                    // EL2异常,不做迁移，保持默认值
                    return false;
                }
                return HandleFile(preferencesUtil);
        }
    }
}

void RdbSmsMmsHelper::ResetRdbStore()
{
    FreeRdbStoreE();
}

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsHelper::QueryUncompletedMessage(bool checkPath)
{
    auto sql = RdbSmsMmsUtil::QueryUncompletedMessage();
    return QuerySql(sql, checkPath);
}

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsHelper::QueryCompletedMessage()
{
    auto sql = RdbSmsMmsUtil::QueryCompletedMessage();
    return QuerySql(sql, false);
}

bool RdbSmsMmsHelper::IsExistUncompletedMessage(bool checkPath)
{
    bool isExist = false;
    std::shared_ptr<OHOS::NativeRdb::ResultSet> resultSet = QueryUncompletedMessage(checkPath);
    if (resultSet == nullptr) {
        DATA_STORAGE_LOGE("QueryUncompletedMessage resultSet is null!");
        return false;
    }
    int32_t resultSetNum = resultSet->GoToFirstRow();
    while (resultSetNum == 0) {
        std::string startTime = GetResultSetString("start_time", resultSet);
        int64_t msgTime = std::atol(startTime.c_str());
        chrono::system_clock::duration timePoint = chrono::system_clock::now().time_since_epoch();
        int64_t timeStamp = chrono::duration_cast<chrono::seconds>(timePoint).count();
        if (msgTime > ((timeStamp - SMS_SENDER_RECEIVE_TIMEOUT) * SMS_MMS_PER_SECOND)) {
            isExist = true;
            break;
        }
        resultSetNum = resultSet->GoToNextRow();
    }
    resultSet->Close();
    return isExist;
}

bool RdbSmsMmsHelper::IsExistCompletedMessage()
{
    bool isExist = false;
    std::shared_ptr<OHOS::NativeRdb::ResultSet> resultSet;
    resultSet = QueryCompletedMessage();
    if (resultSet == nullptr) {
        DATA_STORAGE_LOGE("QueryCompletedMessage resultSet is null!");
        return false;
    }
    int32_t resultSetNum = resultSet->GoToFirstRow();
    if (resultSetNum == 0) {
        int32_t msgCount = 0;
        auto bRet = GetResultSetInt("msgCount", resultSet, msgCount);
        if (bRet && msgCount > 0) {
            isExist = true;
        }
    }
    resultSet->Close();
    return isExist;
}

// 冗余
void RdbSmsMmsHelper::ChangeStore(bool toEl5)
{
    if (toEl5) {
        DATA_STORAGE_LOGI("RdbSmsMmsHelper::ChangeStore to el2");
        auto isExist = IsExistCompletedMessage();
        if (isExist) {
            CopySqlFromCToE();
        } else {
            DATA_STORAGE_LOGI("no need to CopySqlFromCToE");
        }
    }
}

void RdbSmsMmsHelper::ChangeStoreToEl5AndMoveData()
{
    if (!moveToEl2Result) {
        DATA_STORAGE_LOGI("moveToEl2Result is false, no need to ChangeStoreToEl5AndMoveData");
        return;
    }
    std::thread thread([this]() {
        std::lock_guard<std::mutex> lock(g_mutex);
        if (stopRequested_.load()) {
            DATA_STORAGE_LOGI("RdbSmsMmsHelper stopRequested.");
            return;
        }
        auto errCode = CreatSql();
        if (errCode != OHOS::NativeRdb::E_OK) {
            DATA_STORAGE_LOGE("RdbSmsMmsHelper::ChangeStoreToEl5AndMoveData CreateSql el2 Fail");
            return;
        }
        ChangeRdbStoreToEl2();
        // 发送广播，通知mms刷新界面
        std::string action = SmsRdbEventSupport::SMS_RDB_EVENT_STORE_CHANGED;
        RdbSmsMmsUtil::NotifySmsMmsStatus(action);
    });
    thread.join();
}

// el2冗余
void RdbSmsMmsHelper::CopySqlFromCToE()
{
    std::thread thread([this]() {
        std::lock_guard<std::mutex> lock(g_mutex);
        if (stopRequested_.load()) {
            DATA_STORAGE_LOGI("RdbSmsMmsHelper stopRequested.");
            return;
        }
        DATA_STORAGE_LOGI("RdbSmsMmsHelper::CopySqlFromCToE begin!");
        bool mmsPduStatus = false;
        bool subsectionStatus = false;
        bool sessionStatus = false;
        bool smsMmsInfoStatus = false;
        bool rcsInfoStatus = false;
        auto isExist = IsExistCompletedMessage();
        if (!isExist) {
            DATA_STORAGE_LOGI("no need to CopySqlFromCToE");
            return;
        }
        DATA_STORAGE_LOGI("RdbSmsMmsHelper::CopySqlFromCToE isExist!");
        mmsPduStatus = CopyMmsPduTableFromEl1ToEl5();
        subsectionStatus = CopySmsSubsectionTableFromEl1ToEl5();
        sessionStatus = CopySessionTableFromEl1ToEl5();
        rcsInfoStatus = CopyRcsTableFromEl1ToEl5Blocked();
        smsMmsInfoStatus = CopySmsMmsInfoTableFromEl1ToEl5Blocked();
        if (!mmsPduStatus && !subsectionStatus && !sessionStatus && !rcsInfoStatus && !smsMmsInfoStatus) {
            DATA_STORAGE_LOGI("RdbSmsMmsHelper::CopySqlFromCToE failed!");
        } else {
            // 发送广播，通知mms刷新界面
            std::string action = SmsRdbEventSupport::SMS_RDB_EVENT_STORE_CHANGED;
            RdbSmsMmsUtil::NotifySmsMmsStatus(action);
            DATA_STORAGE_LOGI("RdbSmsMmsHelper::CopySqlFromCToE succ, NotifySmsMmsStatus");
        }
    });
    thread.join();
}

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsHelper::QueryMmsPartInfo(int32_t msgId, int32_t rcsId, int32_t groupId)
{
    auto sql = RdbSmsMmsUtil::QueryMmsPartInfo(msgId, rcsId, groupId);
    return QuerySql(sql, false);
}

int RdbSmsMmsHelper::DeleteMmsPartInfo(int id)
{
    auto sql = RdbSmsMmsUtil::DeleteMmsPartInfo(id);
    return ExecuteSql(sql, false);
}

void RdbSmsMmsHelper::ResultSetToBucketMmsPart(
    std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet, OHOS::NativeRdb::ValuesBucket &bucket,
    int32_t &id)
{
    int columnIndex;
    resultSet->GetColumnIndex(MmsPart::ID, columnIndex);
    if (resultSet->GetInt(columnIndex, id) != 0) {
        return;
    }

    ResultSetTobucketInt(MmsPart::PART_INDEX, resultSet, bucket);
    ResultSetTobucketString(MmsPart::PART_SIZE, resultSet, bucket);
    ResultSetTobucketString(MmsPart::RECORDING_TIME, resultSet, bucket);
    ResultSetTobucketInt(MmsPart::TYPE, resultSet, bucket);
    ResultSetTobucketString(MmsPart::CT, resultSet, bucket);
    ResultSetTobucketString(MmsPart::LOCATION_PATH, resultSet, bucket);
    ResultSetTobucketInt(MmsPart::STATE, resultSet, bucket);
    ResultSetTobucketInt(MmsPart::ENCODE, resultSet, bucket);
    ResultSetTobucketString(MmsPart::CONTENT, resultSet, bucket);
    ResultSetTobucketInt(MmsPart::RCS_ID, resultSet, bucket);
    ResultSetTobucketInt(MmsPart::IS_FRAUD, resultSet, bucket);
    ResultSetTobucketInt(MmsPart::REFERENCE_COUNT, resultSet, bucket);
}

int32_t RdbSmsMmsHelper::MmsPartCheckE(std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet)
{
    int32_t msgId = 0;
    int32_t groupId = 0;
    int32_t type = 0;
    int32_t rcsId = 0;
    int32_t partSize = 0;
    GetResultSetInt(MmsPart::MSG_ID, resultSet, msgId);
    GetResultSetInt(MmsPart::GROUP_ID, resultSet, groupId);
    GetResultSetInt(MmsPart::TYPE, resultSet, type);
    GetResultSetInt(MmsPart::RCS_ID, resultSet, rcsId);
    GetResultSetInt(MmsPart::PART_SIZE, resultSet, partSize);
    auto locationPath = GetResultSetString(MmsPart::LOCATION_PATH, resultSet);
    auto sql = RdbSmsMmsUtil::MmsPartCheckSql(msgId, groupId, type, rcsId, partSize, locationPath);
    std::shared_ptr<OHOS::NativeRdb::ResultSet> resultSetE = QuerySql(sql);
    if (resultSetE == nullptr) {
        DATA_STORAGE_LOGE("MmsPartCheckE resultSet is null!");
        return 0;
    }
    int32_t resultSetNum = resultSetE->GoToFirstRow();
    int32_t id = 0;
    if (resultSetNum == 0) {
        bool bRet = GetResultSetInt(MmsPart::ID, resultSetE, id);
        if (!bRet) {
            DATA_STORAGE_LOGE("RdbSmsMmsHelper get MmsPart::ID error!");
        }
    }
    resultSetE->Close();
    return id;
}

bool RdbSmsMmsHelper::CopyMmsPartTableFromEl1ToEl5(int32_t rcsIdC, int32_t rcsIdE, int32_t msgIdC, int32_t msgIdE,
    int32_t groupIdC, int32_t groupIdE)
{
    std::shared_ptr<OHOS::NativeRdb::ResultSet> resultSet;
    resultSet = QueryMmsPartInfo(msgIdC, rcsIdC, groupIdC);
    if (resultSet == nullptr) {
        DATA_STORAGE_LOGE("QueryMmsPartInfo resultSet is null!");
        return false;
    }
    int32_t insertCount = 0;
    int32_t resultSetNum = resultSet->GoToFirstRow();
    while (resultSetNum == 0) {
        int32_t idC = 0;
        int32_t idE = 0;
        OHOS::NativeRdb::ValuesBucket mmsPartInfo;
        ResultSetToBucketMmsPart(resultSet, mmsPartInfo, idC);
        idE = MmsPartCheckE(resultSet);
        if (idE == 0) {
            mmsPartInfo.Put(MmsPart::GROUP_ID, std::to_string(groupIdE));
            mmsPartInfo.Put(MmsPart::MSG_ID, std::to_string(msgIdE));
            mmsPartInfo.Put(MmsPart::RCS_ID, std::to_string(rcsIdE));
            idE = InsertToE(mmsPartInfo, TABLE_MMS_PART);
        }
        if (idE == 0) {
            DATA_STORAGE_LOGE("insert TABLE_RCS_INFO Fail!");
            resultSet->Close();
            return false;
        }
        auto ret = DeleteMmsPartInfo(idC);
        insertCount++;
        if (ret != NativeRdb::E_OK) {
            DATA_STORAGE_LOGE("DeleteMmsPartInfo Fail!");
        }
        resultSetNum = resultSet->GoToNextRow();
    }
    resultSet->Close();
    DATA_STORAGE_LOGI("CopyMmsPartTableFromCToE insertCount=%{public}d!", insertCount);
    return true;
}

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsHelper::QueryRiskUrlRecord(int32_t sessionId, int32_t rcsId,
    int32_t msgId)
{
    auto sql = RdbSmsMmsUtil::QueryRiskUrlRecord(sessionId, rcsId, msgId);
    return QuerySql(sql, false);
}

int RdbSmsMmsHelper::DeleteRiskUrlRecord(int id)
{
    auto sql = RdbSmsMmsUtil::DeleteRiskUrlRecord(id);
    return ExecuteSql(sql, false);
}

void RdbSmsMmsHelper::ResultSetToBucketRiskUrlRecord(
    std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet, OHOS::NativeRdb::ValuesBucket &bucket,
    int32_t &id)
{
    int columnIndex;
    resultSet->GetColumnIndex(RiskUrlRecord::ID, columnIndex);
    if (resultSet->GetInt(columnIndex, id) != 0) {
        return;
    }
    ResultSetTobucketInt(RiskUrlRecord::SESSION_ID, resultSet, bucket);
    ResultSetTobucketInt(RiskUrlRecord::MSG_ID, resultSet, bucket);
    ResultSetTobucketInt(RiskUrlRecord::RCS_ID, resultSet, bucket);
    ResultSetTobucketInt(RiskUrlRecord::MSG_TYPE, resultSet, bucket);
    ResultSetTobucketInt(RiskUrlRecord::RISK_TYPE, resultSet, bucket);
    ResultSetTobucketString(RiskUrlRecord::RISK_URL, resultSet, bucket);
    ResultSetTobucketInt(RiskUrlRecord::URL_INDEX, resultSet, bucket);
    ResultSetTobucketString(RiskUrlRecord::CREATE_TIME, resultSet, bucket);
    ResultSetTobucketString(RiskUrlRecord::LAST_UPDATE_TIME, resultSet, bucket);
    ResultSetTobucketString(RiskUrlRecord::PACKAGE_NAME, resultSet, bucket);
    ResultSetTobucketInt(RiskUrlRecord::CHECK_TYPE, resultSet, bucket);
    ResultSetTobucketInt(RiskUrlRecord::NUMBER_OF_RETRIES, resultSet, bucket);
}

bool RdbSmsMmsHelper::RiskUrlRecordCheckE(std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet)
{
    int32_t msgId = 0;
    int32_t sessionId = 0;
    int32_t rcsId = 0;
    GetResultSetInt(RiskUrlRecord::MSG_ID, resultSet, msgId);
    GetResultSetInt(RiskUrlRecord::SESSION_ID, resultSet, sessionId);
    GetResultSetInt(RiskUrlRecord::RCS_ID, resultSet, rcsId);
    auto url = GetResultSetString(RiskUrlRecord::RISK_URL, resultSet);
    auto createTime = GetResultSetString(RiskUrlRecord::CREATE_TIME, resultSet);
    auto sql = RdbSmsMmsUtil::RiskUrlRecordCheckSql(msgId, sessionId, rcsId, url, createTime);
    bool bRet = false;
    std::shared_ptr<OHOS::NativeRdb::ResultSet> resultSetE = QuerySql(sql);
    if (resultSetE == nullptr) {
        DATA_STORAGE_LOGE("query RiskUrlRecord resultSet is null!");
        return bRet;
    }
    int32_t resultSetNum = resultSetE->GoToFirstRow();
    if (resultSetNum == 0) {
        bRet = true;
    }
    resultSetE->Close();
    return bRet;
}

bool RdbSmsMmsHelper::CopyRiskUrlRecordTableFromEl1ToEl5(int32_t sessionIdC, int32_t sessionIdE,
    int32_t rcsIdC, int32_t rcsIdE, int32_t msgIdC, int32_t msgIdE)
{
    std::shared_ptr<OHOS::NativeRdb::ResultSet> resultSet;
    resultSet = QueryRiskUrlRecord(sessionIdC, rcsIdC, msgIdC);
    if (resultSet == nullptr) {
        DATA_STORAGE_LOGE("QueryRiskUrlRecord resultSet is null!");
        return false;
    }
    int32_t insertCount = 0;
    int32_t resultSetNum = resultSet->GoToFirstRow();
    while (resultSetNum == 0) {
        int32_t idC = 0;
        OHOS::NativeRdb::ValuesBucket bucket;
        ResultSetToBucketRiskUrlRecord(resultSet, bucket, idC);
        auto isExist = RiskUrlRecordCheckE(resultSet);
        if (!isExist) {
            bucket.Put(RiskUrlRecord::SESSION_ID, std::to_string(sessionIdE));
            bucket.Put(RiskUrlRecord::MSG_ID, std::to_string(msgIdE));
            if (rcsIdE > 0) {
                bucket.Put(RiskUrlRecord::RCS_ID, std::to_string(rcsIdE));
            }
            InsertToE(bucket, TABLE_RISK_URL_RECORD);
        }
        DeleteRiskUrlRecord(idC);
        insertCount++;
        resultSetNum = resultSet->GoToNextRow();
    }
    resultSet->Close();
    DATA_STORAGE_LOGI("CopyRiskUrlRecordTableFromCToE insertCount=%{public}d!", insertCount);
    return true;
}

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsHelper::QuerySmsMmsInfo(int32_t sessionId, int32_t rcsId,
    int32_t groupId)
{
    auto sql = RdbSmsMmsUtil::QuerySmsMmsInfo(sessionId, rcsId, groupId);
    return QuerySql(sql, false);
}

int RdbSmsMmsHelper::DeleteSmsMmsInfo(int msgId)
{
    auto sql = RdbSmsMmsUtil::DeleteSmsMmsInfo(msgId);
    return ExecuteSql(sql, false);
}

void RdbSmsMmsHelper::ResultSetToBucketSmsMms(
    std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet, OHOS::NativeRdb::ValuesBucket &bucket,
    int32_t &msgId, int32_t &msgType)
{
    int columnIndex;
    resultSet->GetColumnIndex(SmsMmsInfo::MSG_ID, columnIndex);
    if (resultSet->GetInt(columnIndex, msgId) != 0) {
        return;
    }
    ResultSetTobucketInt(SmsMmsInfo::SLOT_ID, resultSet, bucket);
    ResultSetTobucketString(SmsMmsInfo::RECEIVER_NUMBER, resultSet, bucket);
    ResultSetTobucketString(SmsMmsInfo::SENDER_NUMBER, resultSet, bucket);
    ResultSetTobucketInt(SmsMmsInfo::IS_SENDER, resultSet, bucket);
    ResultSetTobucketString(SmsMmsInfo::START_TIME, resultSet, bucket);
    ResultSetTobucketString(SmsMmsInfo::END_TIME, resultSet, bucket);
    msgType = ResultSetTobucketInt(SmsMmsInfo::MSG_TYPE, resultSet, bucket);
    ResultSetTobucketInt(SmsMmsInfo::SMS_TYPE, resultSet, bucket);
    ResultSetTobucketString(SmsMmsInfo::MSG_TITLE, resultSet, bucket);
    ResultSetTobucketString(SmsMmsInfo::MSG_CONTENT, resultSet, bucket);
    ResultSetTobucketInt(SmsMmsInfo::MSG_STATE, resultSet, bucket);
    ResultSetTobucketInt(SmsMmsInfo::MSG_CODE, resultSet, bucket);
    ResultSetTobucketInt(SmsMmsInfo::IS_LOCK, resultSet, bucket);
    ResultSetTobucketInt(SmsMmsInfo::IS_READ, resultSet, bucket);
    ResultSetTobucketInt(SmsMmsInfo::IS_COLLECT, resultSet, bucket);
    ResultSetTobucketInt(SmsMmsInfo::SESSION_TYPE, resultSet, bucket);
    ResultSetTobucketInt(SmsMmsInfo::RETRY_NUMBER, resultSet, bucket);
    ResultSetTobucketString(SmsMmsInfo::OPERATOR_SERVICE_NUMBER, resultSet, bucket);
    ResultSetTobucketInt(SmsMmsInfo::SESSION_ID, resultSet, bucket);
    ResultSetTobucketInt(SmsMmsInfo::GROUP_ID, resultSet, bucket);
    ResultSetTobucketInt(SmsMmsInfo::DEVICE_ID, resultSet, bucket);
    ResultSetTobucketInt(SmsMmsInfo::IS_SUBSECTION, resultSet, bucket);
    ResultSetTobucketInt(SmsMmsInfo::IS_SEND_REPORT, resultSet, bucket);
    ResultSetTobucketInt(SmsMmsInfo::IS_ADVANCED_SECURITY, resultSet, bucket);
    ResultSetTobucketInt(SmsMmsInfo::EXPIRES_TIME, resultSet, bucket);
    ResultSetTobucketString(SmsMmsInfo::MMS_PDU, resultSet, bucket);
    ResultSetTobucketInt(SmsMmsInfo::IS_BLOCKED, resultSet, bucket);
    ResultSetTobucketInt(SmsMmsInfo::BLOCKED_REASON, resultSet, bucket);
    ResultSetTobucketString(SmsMmsInfo::BLOCKED_SOURCES, resultSet, bucket);
    ResultSetTobucketInt(SmsMmsInfo::BLOCKED_TYPE, resultSet, bucket);
    ResultSetTobucketString(SmsMmsInfo::BLOCKED_TYPE_TEXT, resultSet, bucket);
    ResultSetTobucketString(SmsMmsInfo::RISK_URL_BODY, resultSet, bucket);
    ResultSetTobucketInt(SmsMmsInfo::HAS_BE_REPORTED_AS_SPAM, resultSet, bucket);
    ResultSetTobucketInt(SmsMmsInfo::IS_REPORT, resultSet, bucket);
    ResultSetTobucketInt(SmsMmsInfo::RCS_ID, resultSet, bucket);
    ResultSetTobucketString(SmsMmsInfo::DETECT_RES_CONTENT, resultSet, bucket);
    ResultSetTobucketString(SmsMmsInfo::PHONE_NUMBER, resultSet, bucket);
    ResultSetTobucketString(SmsMmsInfo::FORMAT_PHONE_NUMBER, resultSet, bucket);
    ResultSetTobucketString(SmsMmsInfo::MSG_CODE_STR, resultSet, bucket);
}

int32_t RdbSmsMmsHelper::SmsMmsInfoCheckE(std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet)
{
    auto msgTitle = GetResultSetString(SmsMmsInfo::MSG_TITLE, resultSet);
    auto msgContent = GetResultSetString(SmsMmsInfo::MSG_CONTENT, resultSet);

    int32_t msgType = 0;
    GetResultSetInt(SmsMmsInfo::MSG_TYPE, resultSet, msgType);

    auto phoneNumber = GetResultSetString(SmsMmsInfo::PHONE_NUMBER, resultSet);
    auto startTime = GetResultSetString(SmsMmsInfo::START_TIME, resultSet);
    auto sql = RdbSmsMmsUtil::SmsMmsInfoCheckSql(msgType, msgTitle, msgContent, phoneNumber, startTime);
    std::shared_ptr<OHOS::NativeRdb::ResultSet> resultSetE;
    int32_t msgId = 0;
    resultSetE = QuerySql(sql);
    if (resultSetE == nullptr) {
        DATA_STORAGE_LOGE("SmsMmsInfoCheckE resultSetE is null!");
        return msgId;
    }
    int32_t resultSetNum = resultSetE->GoToFirstRow();
    if (resultSetNum == 0) {
        auto bRet = GetResultSetInt(SmsMmsInfo::MSG_ID, resultSetE, msgId);
        if (!bRet) {
            DATA_STORAGE_LOGE("RdbSmsMmsHelper get SmsMmsInfo::MSG_ID error!");
        }
    }
    resultSetE->Close();
    return msgId;
}

int32_t RdbSmsMmsHelper::InsertSmsMmsToE(std::shared_ptr<NativeRdb::ResultSet> resultSet,
    OHOS::NativeRdb::ValuesBucket &msgInfo, int32_t &groupId, int32_t sessionId, int32_t rcsId)
{
    auto msgIdE = SmsMmsInfoCheckE(resultSet);
    if (msgIdE > 0) {
        return msgIdE;
    }
    if (groupId == 0) {
        CalcGroupId(groupId);
    }
    msgInfo.Put(SmsMmsInfo::GROUP_ID, std::to_string(groupId));
    if (sessionId != 0) {
        msgInfo.Put(SmsMmsInfo::SESSION_ID, std::to_string(sessionId));
    }
    if (rcsId != 0) {
        msgInfo.Put(SmsMmsInfo::RCS_ID, std::to_string(rcsId));
    }
    int32_t msgId = InsertToE(msgInfo, TABLE_SMS_MMS_INFO);
    return msgId;
}

bool RdbSmsMmsHelper::CopySmsMmsInfoTableFromEl1ToEl5(int32_t sessionIdC, int32_t sessionIdE, int32_t rcsIdC,
    int32_t rcsIdE, int32_t groupIdC, int32_t groupIdE)
{
    auto resultSet = QuerySmsMmsInfo(sessionIdC, rcsIdC, groupIdC);
    if (resultSet == nullptr) {
        DATA_STORAGE_LOGE("QuerySmsMmsInfo resultSet is null!");
        return false;
    }
    int32_t insertCount = 0;
    int32_t resultSetNum = resultSet->GoToFirstRow();
    while (resultSetNum == 0) {
        int32_t msgIdC = 0;
        int32_t msgType = 0;
        OHOS::NativeRdb::ValuesBucket msgInfo;
        ResultSetToBucketSmsMms(resultSet, msgInfo, msgIdC, msgType);
        GetResultSetInt(SmsMmsInfo::GROUP_ID, resultSet, groupIdC);
        auto msgIdE = InsertSmsMmsToE(resultSet, msgInfo, groupIdE, sessionIdE, rcsIdE);
        if (msgIdE == 0) {
            DATA_STORAGE_LOGE("insert InsertSmsMmsToE Fail!");
            resultSet->Close();
            return false;
        }
        auto bRet = CopyMmsPartTableFromEl1ToEl5(rcsIdC, rcsIdE, msgIdC, msgIdE, groupIdC, groupIdE);
        if (!bRet) {
            DATA_STORAGE_LOGE("CopyMmsPartTableFromCToE Fail! msgIdC=%{public}d", msgIdC);
            resultSetNum = resultSet->GoToNextRow();
            continue;
        }
        bRet = CopyRiskUrlRecordTableFromEl1ToEl5(sessionIdC, sessionIdE, rcsIdC, rcsIdE, msgIdC, msgIdE);
        if (!bRet) {
            DATA_STORAGE_LOGE("CopyRiskUrlRecordTableFromCToE Fail! msgIdC=%{public}d", msgIdC);
            resultSetNum = resultSet->GoToNextRow();
            continue;
        }
        DeleteSmsMmsInfo(msgIdC);
        insertCount++;
        resultSetNum = resultSet->GoToNextRow();
    }
    resultSet->Close();
    DATA_STORAGE_LOGI("CopySmsMmsInfoTableFromCToE insertCount=%{public}d!", insertCount);
    return true;
}

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsHelper::QuerySmsMmsInfoBlocked()
{
    auto sql = RdbSmsMmsUtil::QuerySmsMmsInfoBlocked();
    return QuerySql(sql, false);
}

bool RdbSmsMmsHelper::CopySmsMmsInfoTableFromEl1ToEl5Blocked()
{
    DATA_STORAGE_LOGI("RdbSmsMmsHelper::CopySmsMmsInfoTableFromCToEBlocked start!");
    auto resultSet = QuerySmsMmsInfoBlocked();
    if (resultSet == nullptr) {
        DATA_STORAGE_LOGE("QuerySmsMmsInfoBlocked resultSet is null!");
        return false;
    }
    int32_t insertCount = 0;
    int32_t resultSetNum = resultSet->GoToFirstRow();
    while (resultSetNum == 0) {
        int32_t msgIdC = 0;
        int32_t msgType = 0;
        int32_t msgCountE = 0;
        long timeE = 0;
        OHOS::NativeRdb::ValuesBucket msgInfo;
        ResultSetToBucketSmsMms(resultSet, msgInfo, msgIdC, msgType);
        std::string phoneNumber = GetResultSetString(SmsMmsInfo::SENDER_NUMBER, resultSet);
        int32_t groupIdC = 0;
        int32_t rcsIdC = 0;
        int32_t sessionIdC = 0;
        GetResultSetInt(SmsMmsInfo::SESSION_ID, resultSet, sessionIdC);
        GetResultSetInt(SmsMmsInfo::GROUP_ID, resultSet, groupIdC);
        GetResultSetInt(SmsMmsInfo::RCS_ID, resultSet, rcsIdC);
        int sessionIdE = SessionCheckE(phoneNumber, msgCountE, timeE);
        int32_t groupIdE = 0;
        auto msgIdE = InsertSmsMmsToE(resultSet, msgInfo, groupIdE, sessionIdE, 0);
        if (msgIdE == 0) {
            DATA_STORAGE_LOGE("insert InsertSmsMmsToE Fail!");
            resultSet->Close();
            return false;
        }
        auto bRet = CopyMmsPartTableFromEl1ToEl5(0, 0, msgIdC, msgIdE, groupIdC, groupIdE);
        if (!bRet) {
            DATA_STORAGE_LOGE("CopyMmsPartTableFromCToE Fail! msgIdC=%{public}d", msgIdC);
            resultSetNum = resultSet->GoToNextRow();
            continue;
        }
        DeleteSmsMmsInfo(msgIdC);
        insertCount++;
        resultSetNum = resultSet->GoToNextRow();
    }
    DATA_STORAGE_LOGI("CopySmsMmsInfoTableFromCToEBlocked insertCount=%{public}d!", insertCount);
    resultSet->Close();
    return true;
}

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsHelper::QueryRcsInfo(int sessionId)
{
    auto sql = RdbSmsMmsUtil::QueryRcsInfo(sessionId);
    return QuerySql(sql, false);
}

int RdbSmsMmsHelper::DeleteRcsInfo(int rcsId)
{
    auto sql = RdbSmsMmsUtil::DeleteRcsInfo(rcsId);
    return ExecuteSql(sql, false);
}

void RdbSmsMmsHelper::ResultSetToBucketRcs(
    std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet, OHOS::NativeRdb::ValuesBucket &bucket,
    int32_t &rcsId, std::string &msgID, int32_t &groupId, std::string startTime)
{
    int columnIndex;
    resultSet->GetColumnIndex(RcsInfo::RCS_ID, columnIndex);
    if (resultSet->GetInt(columnIndex, rcsId) != 0) {
        return;
    }
    ResultSetTobucketInt(RcsInfo::SLOT_ID, resultSet, bucket);
    ResultSetTobucketString(RcsInfo::RECEIVER_NUMBER, resultSet, bucket);
    ResultSetTobucketString(RcsInfo::SENDER_NUMBER, resultSet, bucket);
    startTime = ResultSetTobucketString(RcsInfo::START_TIME, resultSet, bucket);
    ResultSetTobucketString(RcsInfo::END_TIME, resultSet, bucket);
    ResultSetTobucketInt(RcsInfo::RCS_TYPE, resultSet, bucket);
    msgID = ResultSetTobucketString(RcsInfo::MSG_ID, resultSet, bucket);
    ResultSetTobucketString(RcsInfo::MSG_TITLE, resultSet, bucket);
    ResultSetTobucketString(RcsInfo::MSG_CONTENT, resultSet, bucket);
    ResultSetTobucketInt(RcsInfo::MSG_STATE, resultSet, bucket);
    ResultSetTobucketInt(RcsInfo::IS_LOCK, resultSet, bucket);
    ResultSetTobucketInt(RcsInfo::IS_READ, resultSet, bucket);
    ResultSetTobucketInt(RcsInfo::IS_SENDER, resultSet, bucket);
    ResultSetTobucketInt(RcsInfo::SESSION_ID, resultSet, bucket);
    groupId = ResultSetTobucketInt(RcsInfo::GROUP_ID, resultSet, bucket);
    ResultSetTobucketString(RcsInfo::CALLING_TYPE, resultSet, bucket);
    ResultSetTobucketInt(RcsInfo::ERROR_CODE, resultSet, bucket);
    ResultSetTobucketString(RcsInfo::NETWORK_TYPE, resultSet, bucket);
    ResultSetTobucketString(RcsInfo::OWNER_ADDR, resultSet, bucket);
    ResultSetTobucketInt(RcsInfo::PRIVACY_MODE, resultSet, bucket);
    ResultSetTobucketInt(RcsInfo::PROTOCOL, resultSet, bucket);
    ResultSetTobucketString(RcsInfo::REPLY_PATH, resultSet, bucket);
    ResultSetTobucketInt(RcsInfo::SEEN, resultSet, bucket);
    ResultSetTobucketInt(RcsInfo::IS_COLLECT, resultSet, bucket);
    ResultSetTobucketInt(RcsInfo::RECEIVE_STATE, resultSet, bucket);
    ResultSetTobucketString(RcsInfo::FAIL_RECEIVE_CONTEXT, resultSet, bucket);
    ResultSetTobucketString(RcsInfo::SERVICE_CENTER, resultSet, bucket);
    ResultSetTobucketString(RcsInfo::SERVICE_KIND, resultSet, bucket);
    ResultSetTobucketInt(RcsInfo::IS_ADVANCED_SECURITY, resultSet, bucket);
    ResultSetTobucketInt(RcsInfo::IS_BLOCKED, resultSet, bucket);
    ResultSetTobucketInt(RcsInfo::BLOCKED_REASON, resultSet, bucket);
    ResultSetTobucketString(RcsInfo::BLOCKED_SOURCES, resultSet, bucket);
    ResultSetTobucketInt(RcsInfo::BLOCKED_TYPE, resultSet, bucket);
    ResultSetTobucketString(RcsInfo::BLOCKED_TYPE_TEXT, resultSet, bucket);
    ResultSetTobucketInt(RcsInfo::CLUR_SIZE, resultSet, bucket);
    ResultSetTobucketInt(RcsInfo::TOTAL_SIZE, resultSet, bucket);
    ResultSetTobucketString(RcsInfo::RISK_URL_BODY, resultSet, bucket);
    ResultSetTobucketInt(RcsInfo::HAS_BE_REPORTED_AS_SPAM, resultSet, bucket);
    ResultSetTobucketInt(RcsInfo::IS_REPORT, resultSet, bucket);
    ResultSetTobucketString(RcsInfo::DETECT_RES_CONTENT, resultSet, bucket);
    ResultSetTobucketString(RcsInfo::PHONE_NUMBER, resultSet, bucket);
    ResultSetTobucketString(RcsInfo::FORMAT_PHONE_NUMBER, resultSet, bucket);
}

int32_t RdbSmsMmsHelper::RcsInfoChecKE(std::string msgId, std::string startTime, int32_t &groupId)
{
    auto sql = RdbSmsMmsUtil::RcsInfoChecKSql(msgId, startTime, groupId);
    std::shared_ptr<OHOS::NativeRdb::ResultSet> resultSet;
    resultSet = QuerySql(sql);
    if (resultSet == nullptr) {
        DATA_STORAGE_LOGE("RcsInfoChecKE resultSet is null!");
        return 0;
    }
    int32_t resultSetNum = resultSet->GoToFirstRow();
    int32_t newRcsId = 0;
    if (resultSetNum == 0) {
        int32_t newGroupId = 0;
        GetResultSetInt(RcsInfo::GROUP_ID, resultSet, newGroupId);
        GetResultSetInt(RcsInfo::RCS_ID, resultSet, newRcsId);
        groupId = newGroupId;
    }
    resultSet->Close();
    return newRcsId;
}

int32_t RdbSmsMmsHelper::InsertRcsInfoToE(std::string msgId, std::string startTime, int32_t sessionId,
    OHOS::NativeRdb::ValuesBucket &rcsInfo, int32_t &groupId)
{
    auto rcsId = RcsInfoChecKE(msgId, startTime, groupId);
    if (rcsId == 0) {
        CalcGroupId(groupId);
        rcsInfo.Put(RcsInfo::GROUP_ID, std::to_string(groupId));
        if (sessionId > 0) {
            rcsInfo.Put(RcsInfo::SESSION_ID, std::to_string(sessionId));
        }
        rcsId = InsertToE(rcsInfo, TABLE_RCS_INFO);
    }
    return rcsId;
}

bool RdbSmsMmsHelper::CopyRcsTableFromEl1ToEl5(int sessionIdC, int sessionIdE)
{
    std::shared_ptr<OHOS::NativeRdb::ResultSet> resultSet;
    resultSet = QueryRcsInfo(sessionIdC);
    if (resultSet == nullptr) {
        DATA_STORAGE_LOGE("QueryRcs resultSet is null!");
        return false;
    }
    int32_t insertCount = 0;
    int32_t resultSetNum = resultSet->GoToFirstRow();
    while (resultSetNum == 0) {
        int32_t rcsIdC = 0;
        OHOS::NativeRdb::ValuesBucket rcsInfo;
        std::string msgId;
        std::string startTime;
        int32_t groupIdC = 0;
        ResultSetToBucketRcs(resultSet, rcsInfo, rcsIdC, msgId, groupIdC, startTime);
        int32_t rcsIdE = 0;
        int32_t groupIdE = 0;
        rcsIdE = InsertRcsInfoToE(msgId, startTime, sessionIdE, rcsInfo, groupIdE);
        if (rcsIdE == 0) {
            DATA_STORAGE_LOGE("insert TABLE_RCS_INFO Fail!");
            resultSet->Close();
            return false;
        }
        bool bRet = CopySmsMmsInfoTableFromEl1ToEl5(sessionIdC, sessionIdE, rcsIdC, rcsIdE, groupIdC, groupIdE);
        if (!bRet) {
            DATA_STORAGE_LOGE("CopySmsMmsInfoTableFromCToE Fail!");
            resultSetNum = resultSet->GoToNextRow();
            continue;
        }
        DeleteRcsInfo(rcsIdC);
        insertCount++;
        resultSetNum = resultSet->GoToNextRow();
    }
    resultSet->Close();
    DATA_STORAGE_LOGI("CopyRcsTableFromCToE insertCount=%{public}d!", insertCount);
    return true;
}

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsHelper::QueryRcsInfoBlocked()
{
    auto sql = RdbSmsMmsUtil::QueryRcsInfoBlocked();
    return QuerySql(sql, false);
}

bool RdbSmsMmsHelper::CopyRcsTableFromEl1ToEl5Blocked()
{
    DATA_STORAGE_LOGI("RdbSmsMmsHelper::CopyRcsTableFromCToEBlocked start!");
    std::shared_ptr<OHOS::NativeRdb::ResultSet> resultSet = QueryRcsInfoBlocked();
    if (resultSet == nullptr) {
        DATA_STORAGE_LOGE("QueryRcs resultSet is null!");
        return false;
    }
    int32_t insertCount = 0;
    int32_t resultSetNum = resultSet->GoToFirstRow();
    while (resultSetNum == 0) {
        int32_t rcsIdC = 0;
        OHOS::NativeRdb::ValuesBucket rcsInfo;
        std::string msgId;
        std::string startTime;
        int32_t groupIdC = 0;
        ResultSetToBucketRcs(resultSet, rcsInfo, rcsIdC, msgId, groupIdC, startTime);
        int32_t sessionIdC = 0;
        int32_t msgCountE = 0;
        long timeE = 0;
        GetResultSetInt(RcsInfo::SESSION_ID, resultSet, sessionIdC);
        auto phoneNumber = GetResultSetString(RcsInfo::SENDER_NUMBER, resultSet);
        int sessionIdE = SessionCheckE(phoneNumber, msgCountE, timeE);
        int32_t groupIdE = 0;
        auto rcsIdE = InsertRcsInfoToE(msgId, startTime, sessionIdE, rcsInfo, groupIdE);
        if (rcsIdE == 0) {
            DATA_STORAGE_LOGE("insert TABLE_RCS_INFO Fail!");
            resultSet->Close();
            return false;
        }
        bool bRet = CopySmsMmsInfoTableFromEl1ToEl5(sessionIdC, sessionIdE, rcsIdC, rcsIdE, groupIdC, groupIdE);
        if (!bRet) {
            DATA_STORAGE_LOGE("CopySmsMmsInfoTableFromCToE Fail!");
            resultSetNum = resultSet->GoToNextRow();
            continue;
        }
        DeleteRcsInfo(rcsIdC);
        insertCount++;
        resultSetNum = resultSet->GoToNextRow();
    }
    resultSet->Close();
    DATA_STORAGE_LOGI("CopyRcsTableFromCToE insertCount=%{public}d!", insertCount);
    return true;
}

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsHelper::QuerySession()
{
    auto sql = RdbSmsMmsUtil::QuerySession();
    return QuerySql(sql, false);
}

int RdbSmsMmsHelper::DeleteSession(int sessionId)
{
    auto sql = RdbSmsMmsUtil::DeleteSession(sessionId);
    return ExecuteSql(sql, false);
}

void RdbSmsMmsHelper::ResultSetToBucketSession(
    std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet, OHOS::NativeRdb::ValuesBucket &bucket,
    int32_t &sessionId, int32_t &msgCount, std::string &phone)
{
    int columnIndex;
    resultSet->GetColumnIndex(Session::ID, columnIndex);
    if (resultSet->GetInt(columnIndex, sessionId) != 0) {
        return;
    }
    ResultSetTobucketLong(Session::TIME, resultSet, bucket);
    phone = ResultSetTobucketString(Session::TELEPHONE, resultSet, bucket);
    ResultSetTobucketString(Session::CONTENT, resultSet, bucket);
    ResultSetTobucketInt(Session::CONTACTS_NUM, resultSet, bucket);
    ResultSetTobucketString(Session::SMS_TYPE, resultSet, bucket);
    ResultSetTobucketInt(Session::UNREAD_COUNT, resultSet, bucket);
    ResultSetTobucketInt(Session::SENDING_STATUS, resultSet, bucket);
    ResultSetTobucketInt(Session::HAS_DRAFT, resultSet, bucket);
    ResultSetTobucketInt(Session::HAS_LOCK, resultSet, bucket);
    msgCount = ResultSetTobucketInt(Session::MESSAGE_COUNT, resultSet, bucket);
    ResultSetTobucketInt(Session::HAS_MMS, resultSet, bucket);
    ResultSetTobucketString(Session::CONTACT_ID, resultSet, bucket);
    ResultSetTobucketString(Session::CONTACT_NAME, resultSet, bucket);
    ResultSetTobucketInt(Session::HAS_ATTACHMENT, resultSet, bucket);
    ResultSetTobucketString(Session::YELLOW_PAGE_ID, resultSet, bucket);
    ResultSetTobucketString(Session::BLOCKED_TYPE, resultSet, bucket);
}

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsHelper::QuerySessionE(std::string phone)
{
    auto sql = RdbSmsMmsUtil::QuerySessionE(phone);
    return QuerySql(sql);
}

int32_t RdbSmsMmsHelper::SessionCheckE(std::string phone, int32_t &msgCount, long &time)
{
    auto resultSet = QuerySessionE(phone);
    if (resultSet == nullptr) {
        DATA_STORAGE_LOGE("QuerySessionE resultSet is null!");
        return 0;
    }
    int32_t resultSetNum = resultSet->GoToFirstRow();
    while (resultSetNum == 0) {
        auto phoneE = GetResultSetString(Session::TELEPHONE, resultSet);
        if (phoneE.empty()) {
            DATA_STORAGE_LOGE("RdbSmsMmsHelper get TELEPHONE error!");
            resultSetNum = resultSet->GoToNextRow();
            continue;
        }
        std::string oldPhoneFormatNum;
        if (RdbSmsMmsUtil::FormatSmsNumber(phone, "CN",
            i18n::phonenumbers::PhoneNumberUtil::PhoneNumberFormat::NATIONAL, oldPhoneFormatNum) != 0) {
            DATA_STORAGE_LOGE("RdbSmsMmsHelper FormatSmsNumber  oldPhone error!");
            oldPhoneFormatNum = phone;
        }
        std::string newPhoneFormatNum;
        if (RdbSmsMmsUtil::FormatSmsNumber(phoneE, "CN",
            i18n::phonenumbers::PhoneNumberUtil::PhoneNumberFormat::NATIONAL, newPhoneFormatNum) != 0) {
            DATA_STORAGE_LOGE("RdbSmsMmsHelper FormatSmsNumber  newPhone error!");
            newPhoneFormatNum = phoneE;
        }
        if (oldPhoneFormatNum != newPhoneFormatNum) {
            DATA_STORAGE_LOGE("SessionCheck oldPhoneFormatNum != oldPhoneFormatNum");
            resultSetNum = resultSet->GoToNextRow();
            continue;
        }
        GetResultSetInt(Session::MESSAGE_COUNT, resultSet, msgCount);
        GetResultSetLong(Session::TIME, resultSet, time);
        int32_t sessionId = 0;
        GetResultSetInt(Session::ID, resultSet, sessionId);
        resultSet->Close();
        return sessionId;
    }
    resultSet->Close();
    return 0;
}

int RdbSmsMmsHelper::UpdateSessionE(int32_t sessionIdE, int32_t msgCount, long timeE,
    std::shared_ptr<OHOS::NativeRdb::ResultSet> resultSet)
{
    long timeC = 0;
    GetResultSetLong(Session::TIME, resultSet, timeC);
    std::string sql;
    sql.append("update ");
    sql.append(TABLE_SESSION);
    sql.append(" set message_count = ");
    sql += std::to_string(msgCount);
    if (timeC > timeE) {
        sql.append(", time = ");
        sql += std::to_string(timeC);
        auto content = GetResultSetString(Session::CONTENT, resultSet);
        sql.append(", content = '");
        sql += content;
        int32_t sendingStatus = 0;
        GetResultSetInt(Session::SENDING_STATUS, resultSet, sendingStatus);
        sql.append("', sending_status = ");
        sql += std::to_string(sendingStatus);
        int32_t hasDraft = 0;
        GetResultSetInt(Session::HAS_DRAFT, resultSet, hasDraft);
        sql.append(", has_draft = ");
        sql += std::to_string(hasDraft);
        int32_t hasMms = 0;
        GetResultSetInt(Session::HAS_MMS, resultSet, hasMms);
        sql.append(", has_mms = ");
        sql += std::to_string(hasMms);
        int32_t hasAttachment = 0;
        GetResultSetInt(Session::HAS_ATTACHMENT, resultSet, hasAttachment);
        sql.append(", has_attachment = ");
        sql += std::to_string(hasAttachment);
    }
    sql.append(" where id = ");
    sql += std::to_string(sessionIdE);
    return ExecuteSql(sql);
}

bool RdbSmsMmsHelper::CopySessionTableFromEl1ToEl5()
{
    std::shared_ptr<OHOS::NativeRdb::ResultSet> resultSet = QuerySession();
    if (resultSet == nullptr) {
        DATA_STORAGE_LOGE("QuerySession resultSet is null!");
        return false;
    }
    int32_t resultSetNum = resultSet->GoToFirstRow();
    while (resultSetNum == 0) {
        OHOS::NativeRdb::ValuesBucket sessionData;
        int32_t sessionIdC = 0;
        int32_t msgCountC = 0;
        std::string phone;
        ResultSetToBucketSession(resultSet, sessionData, sessionIdC, msgCountC, phone);
        int32_t msgCountE = 0;
        long timeE = 0;
        int sessionIdE = SessionCheckE(phone, msgCountE, timeE);
        if (sessionIdE == 0) {
            sessionIdE = InsertToE(sessionData, TABLE_SESSION);
            if (sessionIdE == 0) {
                DATA_STORAGE_LOGE("insert TABLE_SESSION Fail!");
                resultSet->Close();
                return false;
            }
        } else {
            UpdateSessionE(sessionIdE, msgCountC + msgCountE, timeE, resultSet);
        }
        bool bRet = CopyRcsTableFromEl1ToEl5(sessionIdC, sessionIdE);
        if (!bRet) {
            DATA_STORAGE_LOGE("CopyRcsTableFromCToE Fail!");
            resultSet->Close();
            return false;
        }
        bRet = CopySmsMmsInfoTableFromEl1ToEl5(sessionIdC, sessionIdE, 0, 0, 0, 0);
        if (!bRet) {
            DATA_STORAGE_LOGE("CopySmsMmsInfoTableFromCToE Fail!");
            resultSet->Close();
            return false;
        }
        DeleteSession(sessionIdC);
        resultSetNum = resultSet->GoToNextRow();
    }
    resultSet->Close();
    return true;
}

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsHelper::QuerySmsSubsection()
{
    auto sql = RdbSmsMmsUtil::QuerySmsSubsection();
    return QuerySql(sql, false);
}

int RdbSmsMmsHelper::DeleteSmsSubsection(int id)
{
    auto sql = RdbSmsMmsUtil::DeleteSmsSubsection(id);
    return ExecuteSql(sql, false);
}

void RdbSmsMmsHelper::ResultSetToBucketSmsSubsection(
    std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet, OHOS::NativeRdb::ValuesBucket &bucket,
    int32_t &id)
{
    int columnIndex;
    resultSet->GetColumnIndex(SmsSubsection::ID, columnIndex);
    if (resultSet->GetInt(columnIndex, id) != 0) {
        return;
    }
    ResultSetTobucketInt(SmsSubsection::SLOT_ID, resultSet, bucket);
    ResultSetTobucketInt(SmsSubsection::SMS_SUBSECTION_ID, resultSet, bucket);
    ResultSetTobucketString(SmsSubsection::RECEIVER_NUMBER, resultSet, bucket);
    ResultSetTobucketString(SmsSubsection::SENDER_NUMBER, resultSet, bucket);
    ResultSetTobucketInt(SmsSubsection::IS_SENDER, resultSet, bucket);
    ResultSetTobucketString(SmsSubsection::START_TIME, resultSet, bucket);
    ResultSetTobucketString(SmsSubsection::END_TIME, resultSet, bucket);
    ResultSetTobucketString(SmsSubsection::REW_PUD, resultSet, bucket);
    ResultSetTobucketInt(SmsSubsection::FORMAT, resultSet, bucket);
    ResultSetTobucketInt(SmsSubsection::DEST_PORT, resultSet, bucket);
    ResultSetTobucketInt(SmsSubsection::SUBSECTION_INDEX, resultSet, bucket);
    ResultSetTobucketInt(SmsSubsection::SIZE, resultSet, bucket);
}

bool RdbSmsMmsHelper::SmsSubsectionCheckE(std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSetC)
{
    int32_t id = 0;
    int32_t slotId = 0;
    int32_t subsectionIndex = 0;
    GetResultSetInt(SmsSubsection::SMS_SUBSECTION_ID, resultSetC, id);
    GetResultSetInt(SmsSubsection::SLOT_ID, resultSetC, slotId);
    GetResultSetInt(SmsSubsection::SUBSECTION_INDEX, resultSetC, subsectionIndex);
    auto recvNumber = GetResultSetString(SmsSubsection::RECEIVER_NUMBER, resultSetC);
    auto senderNumber = GetResultSetString(SmsSubsection::RECEIVER_NUMBER, resultSetC);
    auto startTime = GetResultSetString(SmsSubsection::START_TIME, resultSetC);
    auto sql = RdbSmsMmsUtil::SmsSubsectionCheckSql(recvNumber, senderNumber, startTime,
        id, slotId, subsectionIndex);
    bool bRet = false;
    auto resultSetE = QuerySql(sql);
    if (resultSetE == nullptr) {
        DATA_STORAGE_LOGE("QuerySmsSubsectionE resultSet is null!");
        return false;
    }
    int32_t resultSetNum = resultSetE->GoToFirstRow();
    if (resultSetNum == 0) {
        bRet = true;
    }
    resultSetE->Close();
    return bRet;
}

bool RdbSmsMmsHelper::CopySmsSubsectionTableFromEl1ToEl5()
{
    std::shared_ptr<OHOS::NativeRdb::ResultSet> resultSet;
    resultSet = QuerySmsSubsection();
    if (resultSet == nullptr) {
        DATA_STORAGE_LOGE("QuerySmsSubsection resultSet is null!");
        return false;
    }
    int32_t insertCount = 0;
    int32_t resultSetNum = resultSet->GoToFirstRow();
    while (resultSetNum == 0) {
        OHOS::NativeRdb::ValuesBucket bucket;
        int32_t idC = 0;
        ResultSetToBucketSmsSubsection(resultSet, bucket, idC);
        auto isExist = SmsSubsectionCheckE(resultSet);
        if (!isExist) {
            auto idE = InsertToE(bucket, TABLE_SMS_SUBSECTION);
            if (idE == 0) {
                DATA_STORAGE_LOGE("insert TABLE_SMS_SUBSECTION Fail!");
                resultSet->Close();
                return false;
            }
            insertCount++;
        }
        DeleteSmsSubsection(idC);
        resultSetNum = resultSet->GoToNextRow();
    }
    resultSet->Close();
    DATA_STORAGE_LOGI("CopySmsSubsectionTableFromCToE insertCount=%{public}d!", insertCount);
    return true;
}

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsHelper::QueryMmsPdu()
{
    auto sql = RdbSmsMmsUtil::QueryMmsPdu();
    return QuerySql(sql, false);
}

int RdbSmsMmsHelper::DeleteMmsPdu(int id)
{
    auto sql = RdbSmsMmsUtil::DeleteMmsPdu(id);
    return ExecuteSql(sql, false);
}

void RdbSmsMmsHelper::ResultSetToBucketMmsPdu(
    std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet, OHOS::NativeRdb::ValuesBucket &bucket,
    int32_t &id)
{
    int columnIndex;
    resultSet->GetColumnIndex(MmsPdu::ID, columnIndex);
    if (resultSet->GetInt(columnIndex, id) != 0) {
        return;
    }
    ResultSetTobucketString(MmsPdu::PDU_CONTENT, resultSet, bucket);
}

bool RdbSmsMmsHelper::MmsPduCheckE(std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSetC)
{
    auto pduContent = GetResultSetString(MmsPdu::PDU_CONTENT, resultSetC);
    auto sql = RdbSmsMmsUtil::MmsPduCheckSql(pduContent);
    bool bRet = false;
    auto resultSetE = QuerySql(sql, true);
    if (resultSetE == nullptr) {
        DATA_STORAGE_LOGE("MmsPduCheckE resultSet is null!");
        return false;
    }
    int32_t resultSetNum = resultSetE->GoToFirstRow();
    if (resultSetNum == 0) {
        bRet = true;
    }
    resultSetE->Close();
    return bRet;
}

bool RdbSmsMmsHelper::CopyMmsPduTableFromEl1ToEl5()
{
    std::shared_ptr<OHOS::NativeRdb::ResultSet> resultSet;
    resultSet = QueryMmsPdu();
    if (resultSet == nullptr) {
        DATA_STORAGE_LOGE("QueryMmsPdu resultSet is null!");
        return false;
    }
    int32_t insertCount = 0;
    int32_t resultSetNum = resultSet->GoToFirstRow();
    while (resultSetNum == 0) {
        OHOS::NativeRdb::ValuesBucket bucket;
        int32_t idC = 0;
        ResultSetToBucketMmsPdu(resultSet, bucket, idC);
        auto isExist = MmsPduCheckE(resultSet);
        if (!isExist) {
            auto idE = InsertToE(bucket, TABLE_MMS_PDU);
            if (idE == 0) {
                DATA_STORAGE_LOGE("insert TABLE_MMS_PDU Fail!");
                resultSet->Close();
                return false;
            }
        }
        DeleteMmsPdu(idC);
        insertCount++;
        resultSetNum = resultSet->GoToNextRow();
    }
    resultSet->Close();
    DATA_STORAGE_LOGI("CopyMmsPduTableFromCToE insertCount=%{public}d!", insertCount);
    return true;
}

void RdbSmsMmsHelper::CalcGroupId(int32_t &groupId)
{
    std::shared_ptr<OHOS::NativeRdb::ResultSet> resultSet = QueryMaxGroupId();
    if (resultSet == nullptr) {
        DATA_STORAGE_LOGE("QueryMaxGroupId resultSet is null!");
        return;
    }
    int32_t columnInt;
    int columnIndex;
    resultSet->GoToLastRow();
    resultSet->GetColumnIndex("maxGroupId", columnIndex);
    if (resultSet->GetInt(columnIndex, columnInt) == 0) {
        groupId = columnInt + 1;
    }
    resultSet->Close();
}

int64_t RdbSmsMmsHelper::InsertToE(OHOS::NativeRdb::ValuesBucket &bucket, std::string tableName)
{
    int64_t dataBaseId = 0;
    Insert(dataBaseId, bucket, tableName);
    return dataBaseId;
}

bool RdbSmsMmsHelper::GetResultSetInt(const char* key, std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
    int32_t &value)
{
    int32_t valueInt = 0;
    int columnIndex;
    resultSet->GetColumnIndex(key, columnIndex);
    if (resultSet->GetInt(columnIndex, valueInt) == 0) {
        value = valueInt;
        return true;
    }
    return false;
}

bool RdbSmsMmsHelper::GetResultSetLong(const char* key, std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
    long &value)
{
    int64_t valueLong = 0;
    int columnIndex;
    resultSet->GetColumnIndex(key, columnIndex);
    if (resultSet->GetLong(columnIndex, valueLong) == 0) {
        value = valueLong;
        return true;
    }
    return false;
}

int32_t RdbSmsMmsHelper::ResultSetTobucketInt(const char* key, std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
    OHOS::NativeRdb::ValuesBucket &bucket)
{
    int32_t valueInt = 0;
    auto bRet = GetResultSetInt(key, resultSet, valueInt);
    if (bRet) {
        bucket.Put(key, std::to_string(valueInt));
    }
    return valueInt;
}

long RdbSmsMmsHelper::ResultSetTobucketLong(const char* key, std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
    OHOS::NativeRdb::ValuesBucket &bucket)
{
    long valueLong = 0;
    auto bRet = GetResultSetLong(key, resultSet, valueLong);
    if (bRet) {
        bucket.Put(key, std::to_string(valueLong));
    }
    return valueLong;
}

std::string RdbSmsMmsHelper::GetResultSetString(const char* key,
    std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet)
{
    std::string columnValue;
    int columnIndex;
    resultSet->GetColumnIndex(key, columnIndex);
    if (resultSet->GetString(columnIndex, columnValue) == 0) {
        return columnValue;
    }
    return columnValue;
}

std::string RdbSmsMmsHelper::ResultSetTobucketString(const char* key,
    std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet, OHOS::NativeRdb::ValuesBucket &bucket)
{
    auto columnValue = GetResultSetString(key, resultSet);
    if (!columnValue.empty()) {
        bucket.Put(key, columnValue);
    } else {
        bucket.Put(key, "");
    }
    return columnValue;
}

bool RdbSmsMmsHelper::BackupEl1ToEl2()
{
    std::string newDbPath = "/data/storage/el2/database/rdb/sms_mms.db";
    
    int err = ForceCreateEl1RdbStore();
    if (err != NativeRdb::E_OK) {
       DATA_STORAGE_LOGE("HandleFile ForceCreateEl1RdbStore failed"); 
       return false;
    }
    // 删除融合表格ddms_data_search_aux_config
    DeleteDdmstable();

    err = BackupEl1ToEL2(newDbPath);
    FreeRdbStoreEl1(); // 删库前，释放EL1句柄
    if (err != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("RdbSmsMmsHelper::BackupEl1ToEl2 failed, errCode:%{public}d", err);
        HiSysEventWrite(MMS_UE, "DATABASE_INFO", HiviewDFX::HiSysEvent::EventType::BEHAVIOR, "MIAGRATION_RESULT", err);
        return false;
    } else {
        DATA_STORAGE_LOGI("RdbSmsMmsHelper::BackupEl1ToEl2 success");
        DeleteRdb("/data/storage/el1/database/rdb/sms_mms.db.bak");
        DeleteRdb("/data/storage/el1/database/rdb/sms_mms.db");
        HiSysEventWrite(MMS_UE, "DATABASE_INFO", HiviewDFX::HiSysEvent::EventType::BEHAVIOR, "MIAGRATION_RESULT", 0);
        return true;
    }
}

int RdbSmsMmsHelper::DeleteRdb(const std::string &path)
{
    int errCode = NativeRdb::RdbHelper::DeleteRdbStore(path);
    if (errCode != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("RdbSmsMmsBaseHelper::DeleteRdb Error, path=%{public}s", path.c_str());
    } else {
        DATA_STORAGE_LOGI("RdbSmsMmsBaseHelper::DeleteRdb success, path=%{public}s", path.c_str());
    }
    return errCode;
}
} // namespace Telephony
} // namespace OHOS
