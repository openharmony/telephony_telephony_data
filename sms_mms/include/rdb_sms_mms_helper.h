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

#ifndef DATA_STORAGE_RDB_SMS_MMS_HELPER_H
#define DATA_STORAGE_RDB_SMS_MMS_HELPER_H

#include <stdint.h>

#include "iosfwd"
#include "memory"
#include "datashare_values_bucket.h"
#include "rdb_sms_mms_base_helper.h"
#include "string"
#include "vector"
#include "datashare_predicates.h"
#include "preferences_util.h"
#include <mutex>

namespace OHOS {
namespace NativeRdb {
class ResultSet;
class ValuesBucket;
class DataAbilityPredicates;
} // namespace NativeRdb
namespace Telephony {
  enum class RdbType {
    /**
     * The APP_KEY2 is destroyed due to screen lock. The database directory is moved to class C.
     */
    APP_KEY2_DESTROY,
    /**
     * Screen unlock causes APP_KEY2 creation, move database directory to class E
     */
    APP_KEY2_CREAT
};
 
const int32_t RDB_AREA_EL2 = 1;
const int32_t RDB_AREA_EL1 = 0;
class RdbSmsMmsHelper : public RdbSmsMmsBaseHelper {
public:
    RdbSmsMmsHelper();
    ~RdbSmsMmsHelper();

    /**
     * Delete sms_mms_info table data of the previous 30 days
     *
     * @return 0 is succeed else failed
     */
    int32_t DeleteDataByThirty();

    /**
     * Insert sms_mms_info table data
     *
     * @param id id
     * @param value Table field data
     * @return 0 is succeed else failed
     */
    int32_t InsertSmsMmsInfo(int64_t &id, const NativeRdb::ValuesBucket &value);

    /**
     * Batch Insert sms_mms_info table data
     *
     * @param id id
     * @param values Table field data
     * @return 0 is succeed else failed
     */
    int32_t BatchInsertSmsMmsInfo(int64_t &id, const std::vector<DataShare::DataShareValuesBucket> &values);

    /**
     * Commit the transaction action
     *
     * @return 0 is succeed else failed
     */
    int CommitTransactionAction();

    /**
     * Statistics sms_mms_info table unread quantity
     *
     * @return unique_ptr<NativeRdb::ResultSet>
     */
    std::shared_ptr<NativeRdb::ResultSet> StatisticsUnRead();

    /**
     * Statistics sms_mms_info table unread quantity
     *
     * @return unique_ptr<NativeRdb::ResultSet>
     */
    std::shared_ptr<NativeRdb::ResultSet> QueryMMSVIEWS(std::string infoWhereSql, std::string infoLimitSql);

    /**
     * Query sms_mms_info table max group_id
     *
     * @return  unique_ptr<NativeRdb::ResultSet>
     */
    std::shared_ptr<NativeRdb::ResultSet> QueryMaxGroupId();

    /**
     * Update dataBase path
     *
     * @param path path
     */
    void UpdateDbPath(const std::string &path);

    /**
     * Init dataBase
     *
     * @return 0 is succeed else failed
     */
    int Init();

    /**
    * Statistics sms_mms_info table select count
    *
    * @return unique_ptr<NativeRdb::ResultSet>
    */
    std::shared_ptr<NativeRdb::ResultSet> StatisticsSelectInfoCount();

    /**
    * Statistics sms_mms_info table select count
    *
    * @return unique_ptr<NativeRdb::ResultSet>
    */
    std::shared_ptr<NativeRdb::ResultSet> StatisticsSelectContactId();

    /**
    * Statistics session table delete invalid session
    *
    * @return int
    */
    int StatisticsCleanInvalidSession();

    /**
    * Statistics session table query invalid data
    *
    * @return unique_ptr<NativeRdb::ResultSet>
    */
    std::shared_ptr<NativeRdb::ResultSet> StatisticsQueryInvalidSession();

    /**
    * Statistics sms_mms_info table query invalid data
    *
    * @return unique_ptr<NativeRdb::ResultSet>
    */
    std::shared_ptr<NativeRdb::ResultSet> StatisticsQueryInvalidSmsInfo();

    /**
    * Statistics rcs_info table query invalid data
    *
    * @return unique_ptr<NativeRdb::ResultSet>
    */
    std::shared_ptr<NativeRdb::ResultSet> StatisticsQueryInvalidRcsInfo();

    /**
    * Statistics rcs_info table query invalid data
    *
    * @return unique_ptr<NativeRdb::ResultSet>
    */
    std::shared_ptr<NativeRdb::ResultSet> StatisticsQueryInvalidRcsInfoWithMsg();

    /**
    * Statistics mms_part table query invalid data
    *
    * @return unique_ptr<NativeRdb::ResultSet>
    */
    std::shared_ptr<NativeRdb::ResultSet> StatisticsQueryInvalidPart();

    int StatisticsDeleteSession(const DataShare::DataSharePredicates &predicates);
    int StatisticsDeleteSessionExe(std::string arr, std::string limit, std::string equals);
    int StatisticsDeleteMmsPartWithSessionID(std::string session, std::string limit, std::string equals);
    int StatisticsUpdateMmsPartWithSessionID(std::string session, std::string limit, std::string equals);
    int StatisticsUpdateRcsPartWithSessionID(std::string session, std::string limit, std::string equals);
    int StatisticsDeleteRcsPartWithSessionID(std::string session, std::string limit, std::string equals);
    int StatisticsDeleteRCSInfoWithSessionID(std::string session, std::string limit, std::string equals);
    int StatisticsDeleteInfoWithSessionID(std::string session, std::string limit, std::string equals);
    int StatisticsDeleteSessionTableExe(std::string session, std::string limit, std::string equals);
    std::string GetDeleteMmsPartWhereSql(std::string session, std::string limit, std::string equals);
    std::string GetDeleteRcsPartWhereSql(std::string session, std::string limit, std::string equals);
    
    /**
     * query mms paths with sessionIds
     * 
     * @param session
     * @param limit
     * @param equals
     * @return mms paths
     */
    std::shared_ptr<NativeRdb::ResultSet> QueryMmsPathsWithSessionIds(const DataShare::DataSharePredicates &predicates);
    
    /**
     * query mms paths with sessionIds
     * 
     * @param session
     * @param limit
     * @param equals
     * @return mms paths
     */
    std::shared_ptr<NativeRdb::ResultSet> QueryMmsPathsWithSessionIdsExe(std::string arr, std::string limit, 
                                                                         std::string equals);

    /**
     * change store and copy data from el1 to el5 when el5 access unlocked
     */
    void ChangeStoreToEl5AndMoveData();

    /**
     * change rdb store
     */
    void ChangeStore(bool toEl5);
 
    /**
     * Copy sql Data from C to E view
     */
    void CopySqlFromCToE();

    /**
    * query session info
    *
    * @return unique_ptr<NativeRdb::ResultSet>
    */
    std::shared_ptr<NativeRdb::ResultSet> QueryConversionList(std::string infoLimitSql, bool isQueryNotifiyMsg);

    /**
    * query recipient matched result
    *
    * @return unique_ptr<NativeRdb::ResultSet>
    */
    std::shared_ptr<NativeRdb::ResultSet> QueryMatchedRecipient(std::string recipients);

    int MarkedInfoASRead();
    int MarkedAllASRead();
    int MarkedSingleASRead(int32_t sessionId);
    int UpdateUnreadCountWithTelephone(std::string &telephone, std::string &time, std::string &msgType);
    int UpdateUnreadCountWithMsgId(int32_t &msgId, std::string &msgType);
    int MarKReadWithTelephone(std::string &telephone, std::string &time, std::string &msgType);
    int MarKReadWithMsgId(int32_t &msgId, std::string &msgType);
    int ProcessSessionUnreadWithTelephone(std::string &telephone, std::string &time, std::string &msgType);
    int ProcessSessionUnreadWithMsgId(int32_t &msgId, std::string &msgType);
    std::shared_ptr<NativeRdb::ResultSet> GetMsgCoordinatesWithSessionId(int32_t &threadId, int32_t &msgId,
                                                                         int32_t &sessionType);
private:
    int DeleteRdb(const std::string &path);
    bool MoveDbFileToEl2();
    int ForceCreateEl1RdbStore();
    bool BackupEl1ToEl2();

    /**
    * InitInfoQuerySql
    * @param InitInfoQuerySql init sql
     */
    void InitInfoQuerySql();

   /**
     * Create Sql view
     * @param rdbType Create trigger type
     */
    int CreatSql();
 
    /**
     * Insert bucket to table view
     * @param bucket insert data
     * @param tableName table name
     */
    int64_t InsertToE(OHOS::NativeRdb::ValuesBucket &bucket, std::string tableName);
 
    /**
     * Get int Value from resultSet view
     * @param key Table field name
     * @param resultSet Result Set
     * @param value Result Set value
     * @return true or false
     */
    bool GetResultSetInt(const char* key, std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet, int32_t &value);
 
    /**
     * Get int Value from resultSet view
     * @param key Table field name
     * @param resultSet Result Set
     * @param value Result Set value
     * @return true or false
     */
    bool GetResultSetLong(const char* key, std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
        long &value);
 
    /**
     *  Get string Value from resultSet view
     * @param key Table field name
     * @param resultSet Result Set
     * @return Table field string data
     */
    std::string GetResultSetString(const char* key, std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet);
 
    /**
     * parse resultSet to bucket view
     * @param key Table field name
     * @param resultSet Result Set
     * @param bucket Values Bucket
     * @return int value of table field name
     */
    int32_t ResultSetTobucketInt(const char* key, std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
        OHOS::NativeRdb::ValuesBucket &bucket);
 
    /**
     * parse resultSet to bucket view
     * @param key Table field name
     * @param resultSet Result Set
     * @param bucket Values Bucket
     * @return long value of table field name
     */
    long ResultSetTobucketLong(const char* key, std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
        OHOS::NativeRdb::ValuesBucket &bucket);
 
    /**
     * parse resultSet to bucket view
     * @param key Table field name
     * @param resultSet Result Set
     * @param bucket Values Bucket
     * @return string value of table field name
     */
    std::string ResultSetTobucketString(const char* key, std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
        OHOS::NativeRdb::ValuesBucket &bucket);
 
    /**
     * query session info from table session el2 view
     * @return std::shared_ptr<NativeRdb::ResultSet>
     */
    std::shared_ptr<NativeRdb::ResultSet> QuerySession();
 
    /**
     * delete session from table session by session_id view
     * @param sessionId session id
     */
    int DeleteSession(int32_t sessionId);
 
    /**
     * parse resultSet to bucket view
     * @param resultSet Result Set
     * @param bucket Values Bucket
     * @param sessionId the value of table field session_id
     * @param msgCount the value of table field msg_count
     * @param phone the value of table field phone
     */
    void ResultSetToBucketSession(std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
        OHOS::NativeRdb::ValuesBucket &bucket, int32_t &sessionId, int32_t &msgCount, std::string &phone);
 
    /**
     * query session info from table session el5 view
     * @param phone phone number
     * @return std::shared_ptr<NativeRdb::ResultSet>
     */
    std::shared_ptr<NativeRdb::ResultSet> QuerySessionE(std::string phone);
 
    /**
     * check session info el5 view
     * @param phone telephone
     * @param msgCount message_count
     * @param time time
     * @return session id el5
     */
    int32_t SessionCheckE(std::string phone, int32_t &msgCount, long &time);
 
    /**
     * update session info el5 view
     * @return NativeRdb::E_OK
     */
    int UpdateSessionE(int32_t sessionIdE, int32_t msgCount, long timeE,
        std::shared_ptr<OHOS::NativeRdb::ResultSet> resultSet);
 
    /**
     * copy session table from el2 to el5 view
     * @return true/false
     */
    bool CopySessionTableFromEl1ToEl5();
 
    /**
     * query rcs info from table rcs_info el2 view
     * @return std::shared_ptr<NativeRdb::ResultSet>
     */
    std::shared_ptr<NativeRdb::ResultSet> QueryRcsInfo(int32_t sessionId);

     /**
     * query rcs info from table rcs_info el2 view (is_blocked = 1)
     * @return std::shared_ptr<NativeRdb::ResultSet>
     */
    std::shared_ptr<NativeRdb::ResultSet> QueryRcsInfoBlocked();

    /**
     * delete rcs info from table rcs_info by rcs_id el2 view
     * @param rcsId rcs_id
     */
    int DeleteRcsInfo(int32_t rcsId);
 
    /**
     * parse resultSet to bucket view
     * @param resultSet Result Set
     * @param bucket Values Bucket
     * @param rcsId the value of table field rcs_id
     * @param msgID the value of table field msg_id
     * @param groupId the value of table field group_id
     * @param startTime the value of table field start_time
     */
    void ResultSetToBucketRcs(std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
        OHOS::NativeRdb::ValuesBucket &bucket, int32_t &rcsId, std::string &msgID,
        int32_t &groupId, std::string startTime);
 
    /**
     * check rcs info el5 view
     * @param msgId msg_id
     * @param startTime start_time
     * @param groupId group_id
     * @return rcs_id el5
     */
    int32_t RcsInfoChecKE(std::string msgId, std::string startTime, int32_t &groupId);
 
    /**
     * insert rcs info to el5 view
     * @param msgId msg_id
     * @param startTime start_time
     * @param sessionId session_id
     * @param rcsInfo ValuesBucket
     * @return rcs_id el5
     */
    int32_t InsertRcsInfoToE(std::string msgId, std::string startTime, int32_t sessionId,
        OHOS::NativeRdb::ValuesBucket &rcsInfo, int32_t &groupId);
 
    /**
     * copy table rec_info from el2 to el5 view
     * @param sessionIdC session_id el2
     * @param sessionIdE session_id el5
     * @return true/false
     */
    bool CopyRcsTableFromEl1ToEl5(int32_t sessionIdC, int32_t sessionIdE);

    /**
     * copy table rec_info from el2 to el5 view (is_blocked = 1)
     * @return true/false
     */
    bool CopyRcsTableFromEl1ToEl5Blocked();
 
    /**
     * query sms mms info from table sms_mms_info el2 view (is_blocked = 1)
     * @return std::shared_ptr<NativeRdb::ResultSet>
     */
    std::shared_ptr<NativeRdb::ResultSet> QuerySmsMmsInfoBlocked();

    /**
     * query sms mms info from table sms_mms_info el2 view
     * @param sessionId session_id
     * @param rcsId rcs_id
     * @param groupId group_id
     * @return std::shared_ptr<NativeRdb::ResultSet>
     */
    std::shared_ptr<NativeRdb::ResultSet> QuerySmsMmsInfo(int32_t sessionId, int32_t rcsId, int32_t groupId);
 
    /**
     * delete sms mms info from sms_mms_info by msg_id el2 view
     * @param msgId msg_id
     */
    int DeleteSmsMmsInfo(int32_t msgId);
 
    /**
     * parse resultSet to bucket view
     * @param resultSet Result Set
     * @param bucket Values Bucket
     * @param msgID the value of table field msg_id
     * @param msgType the value of table field msg_type
     */
    void ResultSetToBucketSmsMms(
        std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet, OHOS::NativeRdb::ValuesBucket &bucket,
        int32_t &msgId, int32_t &msgType);
 
    /**
     * check sms mms info is exist el5 view
     * @return msg_id el5
     */
    int32_t SmsMmsInfoCheckE(std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet);
 
    /**
     * Insert bucket to table view
     * @param bucket insert data
     * @param groupId the value of table field groud_id
     * @param sessionId the value of table field session_id
     * @param rcsId the value of table field rcs_id
     */
    int32_t InsertSmsMmsToE(std::shared_ptr<NativeRdb::ResultSet> resultSet, OHOS::NativeRdb::ValuesBucket &bucket,
        int32_t &groupId, int32_t sessionId, int32_t rcsId);
 
    /**
     * copy sms_mms_info table from el2 to el5 view
     * @param sessionIdC the value of table field sesson_id el2
     * @param sessionIdE the value of table field sesson_id el5
     * @param rcsIdC the value of table field rcs_id el2
     * @param rcsIdE the value of table field rcs_id el5
     * @param groupIdC the value of table field group_id el2
     * @param groupIdE the value of table field group_id el5
     * @return true/false
     */
    bool CopySmsMmsInfoTableFromEl1ToEl5(int32_t sessionIdC, int32_t sessionIdE, int32_t rcsIdC, int32_t rcsIdE,
        int32_t groupIdC, int32_t groupIdE);

    /**
     * copy sms_mms_info from el2 to el5 view (is_block = 1)
     * @return true/false
     */
    bool CopySmsMmsInfoTableFromEl1ToEl5Blocked();
 
    /**
     * query mms part info from table mms_part el2 view
     * @param msgId the value of table field msg_id
     * @param rcsId the value of table field rcs_id
     * @param groupId the value of table field group_id
     * @return std::shared_ptr<NativeRdb::ResultSet>
     */
    std::shared_ptr<NativeRdb::ResultSet> QueryMmsPartInfo(int32_t msgId, int32_t rcsId, int32_t groupId);
 
    /**
     * delete mms part from smms_part by id el2 view
     * @param id the value of table field id
     */
    int DeleteMmsPartInfo(int32_t id);
 
    /**
     * parse resultSet to bucket view
     * @param resultSet Result Set
     * @param bucket Values Bucket
     * @param id the value of table field id
     */
    void ResultSetToBucketMmsPart(std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
        OHOS::NativeRdb::ValuesBucket &bucket, int32_t &id);
 
    /**
     * check mms part is exist el5 view
     * @return id el5
     */
    int32_t MmsPartCheckE(std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet);
 
    /**
     * copy mms_part table from el2 to el5 view
     * @param rcsIdC the value of table field rcs_id el2
     * @param rcsIdE the value of table field rcs_id el5
     * @param msgIdC the value of table field msg_id el2
     * @param msgIdE the value of table field msg_id el5
     * @param groupIdC the value of table field group_id el2
     * @param groupIdE the value of table field group_id el5
     * @return true/false
     */
    bool CopyMmsPartTableFromEl1ToEl5(int32_t rcsIdC, int32_t rcsIdE, int32_t msgIdC, int32_t msgIdE,
        int32_t groupIdC, int32_t groupIdE);
 
    /**
     * query max group id el5 view
     * @param groupId max group id el5
     */
    void CalcGroupId(int32_t &groupId);
 
    /**
     * query sms subsection from table sms_subsection el2 view
     * @return std::shared_ptr<NativeRdb::ResultSet>
     */
    std::shared_ptr<NativeRdb::ResultSet> QuerySmsSubsection();
 
    /**
     * delete sms subsection from sms_subsection by id el2 view
     * @param id the value of table field id
     */
    int DeleteSmsSubsection(int32_t id);
 
    /**
     * parse resultSet to bucket view
     * @param resultSet Result Set
     * @param bucket Values Bucket
     * @param id the value of table field id
     */
    void ResultSetToBucketSmsSubsection(std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
        OHOS::NativeRdb::ValuesBucket &bucket, int32_t &id);
 
    /**
     * check sms subsection is exist el5 view
     * @return id el5
     */
    bool SmsSubsectionCheckE(std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet);
 
    /**
     * copy sms_subsection table from el2 to el5 view
     * @return true/false
     */
    bool CopySmsSubsectionTableFromEl1ToEl5();
 
    /**
     * query mms pdu from table sms_pdu el2 view
     * @return std::shared_ptr<NativeRdb::ResultSet>
     */
    std::shared_ptr<NativeRdb::ResultSet> QueryMmsPdu();
 
    /**
     * delete mms pdu from sms_pdu by id el2 view
     * @param id the value of table field id
     */
    int DeleteMmsPdu(int32_t id);
 
    /**
     * parse resultSet to bucket view
     * @param resultSet Result Set
     * @param bucket Values Bucket
     * @param id the value of table field id
     */
    void ResultSetToBucketMmsPdu(
        std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet, OHOS::NativeRdb::ValuesBucket &bucket,
        int32_t &id);
 
    /**
     * check mms pdu is exist el5 view
     * @return true/false
     */
    bool MmsPduCheckE(std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet);
 
    /**
     * copy mms_pdu table from el2 to el5 view
     * @return true/false
     */
    bool CopyMmsPduTableFromEl1ToEl5();
 
    /**
     * query risk url record from table risk_url_record el2 view
     * @return std::shared_ptr<NativeRdb::ResultSet>
     */
    std::shared_ptr<NativeRdb::ResultSet> QueryRiskUrlRecord(int32_t sessionId, int32_t rcsId, int32_t msgId);
 
    /**
     * delete risk url record from risk_url_record by id el2 view
     * @param id the value of table field id
     */
    int DeleteRiskUrlRecord(int32_t id);
 
    /**
     * parse resultSet to bucket view
     * @param resultSet Result Set
     * @param bucket Values Bucket
     * @param id the value of table field id
     */
    void ResultSetToBucketRiskUrlRecord(
        std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet, OHOS::NativeRdb::ValuesBucket &bucket,
        int32_t &id);
 
    /**
     * check risk url recordis exist el5 view
     * @return true/false
     */
    bool RiskUrlRecordCheckE(std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet);
 
    /**
     * copy risk_url_record table from el2 to el5 view
     * @param sessionIdC the value of table field session_id el2
     * @param sessionIdE the value of table field session_id el5
     * @param rcsIdC the value of table field rcs_id el2
     * @param rcsIdE the value of table field rcs_id el5
     * @param msgIdC the value of table field msg_id el2
     * @param msgIdE the value of table field msg_id el5
     * @return true/false
     */
    bool CopyRiskUrlRecordTableFromEl1ToEl5(int32_t sessionIdC, int32_t sessionIdE, int32_t rcsIdC, int32_t rcsIdE,
        int32_t msgIdC, int32_t msgIdE);
 
    /**
     * query location path from table mms_part el5 view
     * @return std::shared_ptr<NativeRdb::ResultSet>
     */
    std::shared_ptr<NativeRdb::ResultSet> QueryLocationPath();
 
    /**
     * notify sms mms change view
     */
    void NotifySmsMmsStatus(std::string action);
 
    /**
     * check is uncompleted message exist el2 or el5 view
     */
    bool IsExistUncompletedMessage(bool checkPath);
 
    /**
     * query uncompleted message from table sms_mms_info el2 or el5 view
     * @return std::shared_ptr<NativeRdb::ResultSet>
     */
    std::shared_ptr<NativeRdb::ResultSet> QueryUncompletedMessage(bool checkPath);
 
    /**
     * check completed message is exist in table sms_mms_info el2 view
     * @return true or false
     */
    bool IsExistCompletedMessage();
 
    /**
     * query completed message from table sms_mms_info el2 or el5 view
     * @return std::shared_ptr<NativeRdb::ResultSet>
     */
    std::shared_ptr<NativeRdb::ResultSet> QueryCompletedMessage();
 
    /**
     * reset rdb store el5 only view
     */
    void ResetRdbStore();

    int CreateLtDataBase();

    std::string SessionIDsParse(std::vector<std::string> sessionIDs);

    int HandleDirJudgement();

    void HandlePreferences(std::shared_ptr<PreferencesUtil> preferencesUtil, int value);

    bool HandleFile(std::shared_ptr<PreferencesUtil> preferencesUtil);
    /**
     * format telephone xxx,xxx,xxx to "xxx","xxx","xxx"
     */
    std::string addQuotes(const std::string& input);
    int MarkedSmsNoticeASRead();
    int MarkedRcsNoticeASRead();
    int MarkedSessionNoticeASRead();
    int MarkedSmsAllASRead();
    int MarkedRcsAllASRead();
    int MarkedSessionAllASRead();
    int MarkedSmsSingleASRead(int32_t sessionId);
    int MarkedRcsSingleASRead(int32_t sessionId);
    int MarkedSessionSingleASRead(int32_t sessionId);

private:
    const std::string DB_NAME = "sms_mms.db";
    std::string dbPath = "/data/storage/el1/database/";
    const int VERSION = 35;
    std::string dbPathE_ = "/data/storage/el2/database/";
    std::string dbPathLT_ = "/data/storage/el1/database/";
    std::string dbName_;
    std::atomic_bool stopRequested_ = false;
    std::atomic_bool needMigrationEl2 = true;
    std::mutex lock_;
    std::string smsInfoQuerySql;
    std::string rcsInfoQuerySql;
};
} // namespace Telephony
} // namespace OHOS
#endif // DATA_STORAGE_RDB_SMS_MMS_HELPER_H
