/*
 * Copyright (C) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef DATA_STORAGE_RDB_SMS_MMS_BASE_HELPER_H
#define DATA_STORAGE_RDB_SMS_MMS_BASE_HELPER_H

#include <stdint.h>

#include "data_storage_log_wrapper.h"
#include "rdb_base_helper.h"
#include "iosfwd"
#include "memory"
#include "string"
#include "vector"
#include "../../common/include/rdb_sms_mms_dfx_adapter.h"

namespace OHOS {
namespace NativeRdb {
class AbsRdbPredicates;
class ResultSet;
class RdbOpenCallback;
class RdbStore;
class RdbStoreConfig;
class ValueObject;
class ValuesBucket;
} // namespace NativeRdb
namespace Telephony {
class RdbSmsMmsBaseHelper : public RdbBaseHelper {

public:

    /**
     * Inserts a new record into the specified table with the provided values.
     *
     * @param id The unique identifier for the inserted record, set by the method upon successful insertion.
     * @param initialValues The values to be inserted into the table, encapsulated in a ValuesBucket.
     * @param table The name of the table where the record will be inserted.
     *
     * @return An integer indicating the result of the operation (e.g., success or error code).
     */
    int Insert(int64_t &id, const NativeRdb::ValuesBucket &initialValues, const std::string &table);

    /**
     * Updates records in the specified table based on the provided values and conditions.
     *
     * @param changedRows The number of rows affected by the update operation, set by the method.
     * @param table The name of the table where records will be updated.
     * @param values The new values to update in the table, encapsulated in a ValuesBucket.
     * @param whereClause The optional SQL WHERE clause to filter which records to update (default is empty).
     * @param whereArgs The optional arguments to replace placeholders in the whereClause (default is empty vector).
     *
     * @return An integer indicating the result of the operation (e.g., success or error code).
     */
    int Update(int &changedRows, const std::string &table, const NativeRdb::ValuesBucket &values,
        const std::string &whereClause = "",
        const std::vector<std::string> &whereArgs = std::vector<std::string>());

    /**
     * Updates records in the table based on the provided values and predicates.
     *
     * @param changedRows The number of rows affected by the update operation, set by the method.
     * @param values The new values to update in the table, encapsulated in a ValuesBucket.
     * @param predicates The conditions to filter which records to update, specified using AbsRdbPredicates.
     *
     * @return An integer indicating the result of the operation (e.g., success or error code).
     */
    int Update(int &changedRows, const NativeRdb::ValuesBucket &values,
        const NativeRdb::AbsRdbPredicates &predicates);

    /**
     * Deletes records from the specified table based on the provided conditions.
     *
     * @param changedRows The number of rows affected by the delete operation, set by the method.
     * @param table The name of the table from which records will be deleted.
     * @param whereClause The optional SQL WHERE clause to filter which records to delete (default is empty).
     * @param whereArgs The optional arguments to replace placeholders in the whereClause (default is empty vector).
     *
     * @return An integer indicating the result of the operation (e.g., success or error code).
     */
    int Delete(int &changedRows, const std::string &table, const std::string &whereClause = "",
        const std::vector<std::string> &whereArgs = std::vector<std::string>());

    /**
     * Deletes records from the table based on the provided predicates.
     *
     * @param deletedRows The number of rows affected by the delete operation, set by the method.
     * @param predicates The conditions to filter which records to delete, specified using AbsRdbPredicates.
     *
     * @return An integer indicating the result of the operation (e.g., success or error code).
     */
    int Delete(int &deletedRows, const NativeRdb::AbsRdbPredicates &predicates);

    /**
     * Executes an SQL statement.
     *
     * @param sql The SQL statement to be executed.
     *
     * @return An integer indicating the execution status.
     */
    int ExecuteSql(const std::string &sql);

    /**
     * Executes an SQL statement with bind arguments.
     *
     * @param sql The SQL statement to be executed.
     * @param bindArgs A vector containing the bind arguments for the SQL statement.
     *
     * @return An integer indicating the execution status.
     */
    int ExecuteSql(const std::string &sql, const std::vector<NativeRdb::ValueObject> &bindArgs);

    /**
     * Creates an RDB store for SMS data.
     *
     * @param config Configuration settings for the RDB store.
     * @param version Version number of the RDB store.
     * @param openCallback Callback function for handling the result of opening the RDB store.
     *
     * @param errCode Reference parameter that will hold the error code upon return.
     */
    void CreateRdbStoreSms(const NativeRdb::RdbStoreConfig &config, int version,
        NativeRdb::RdbOpenCallback &openCallback, int &errCode);

    /**
     * Creates an RDB store for EL1 (Extended Logging Level 1) data.
     *
     * @param config Configuration settings for the RDB store.
     * @param version Version number of the RDB store.
     * @param openCallback Callback function for handling the result of opening the RDB store.
     * @param errCode Reference parameter that will hold the error code upon return.
     */
    void CreateEl1RdbStore(const NativeRdb::RdbStoreConfig &config, int version,
                NativeRdb::RdbOpenCallback &openCallback, int &errCode);

    /**
     * Executes an SQL statement.
     *
     * @param sql The SQL statement to be executed.
     * @param isEl5 Flag indicating whether the operation is related to EL5.
     * @return An integer indicating the execution status.
     */
    int ExecuteSql(const std::string &sql, bool isEl5);

    /**
     * Executes a query SQL statement and returns a ResultSet.
     *
     * @param sql The SQL query statement to be executed.
     * @param isEl5 Flag indicating whether the operation is related to EL5.
     * @param selectionArgs Optional vector of selection arguments for the SQL query.
     *
     * @return A shared pointer to the ResultSet containing the query results.
     */
    std::shared_ptr<NativeRdb::ResultSet> QuerySql(
        const std::string &sql, bool isEl5,
        const std::vector<std::string> &selectionArgs = std::vector<std::string>());

    /**
     * Executes a query based on the provided predicates and columns.
     *
     * @param predicates The predicates for the query.
     * @param columns The columns to be selected in the query.
     *
     * @return A shared pointer to the ResultSet containing the query results.
     */
    std::shared_ptr<NativeRdb::ResultSet> Query(
        const NativeRdb::AbsRdbPredicates &predicates, const std::vector<std::string> columns);

    /**
     * Gets the rebuild type information.
     *
     * @param type Reference to a RebuiltType variable where the rebuild type will be stored.
     *
     * @return An integer indicating the operation result. 0 for success, non-zero for failure.
     */
    int GetRebuiltE(NativeRdb::RebuiltType &type);

    /**
     * Restores the database from a backup file.
     *
     * @param bakDBName The name of the backup database file.
     * @param isEl5 Flag indicating whether the operation is related to EL5.
     *
     * @return An integer indicating the operation result. 0 for success, non-zero for failure.
     */
    int Restore(const std::string &bakDBName, bool isEl5);

    /**
     * Restores the database from a backup file without specifying the EL5 flag.
     *
     * @param bakDBName The name of the backup database file.
     *
     * @return An integer indicating the operation result. 0 for success, non-zero for failure.
     */
    int Restore(const std::string &bakDBName);

    /**
     * Executes a SQL query and returns the result as a ResultSet object.
     *
     * @param sql The SQL query string to execute.
     * @param selectionArgs Optional parameters used in the SQL query.
     *
     * @return A shared pointer to a ResultSet object containing the query results.
     */
    std::shared_ptr<NativeRdb::ResultSet> QuerySql(
        const std::string &sql, const std::vector<std::string> &selectionArgs = std::vector<std::string>());

    /**
     * Creates an RDB store with the specified configuration and version.
     *
     * @param config The configuration for the RDB store.
     * @param version The version of the RDB store.
     * @param openCallback A callback function to handle opening the store.
     *
     * @param errCode A reference to an integer that will hold the error code.
     */
    void CreateRdbStoreLT(const NativeRdb::RdbStoreConfig &config, int version,
        NativeRdb::RdbOpenCallback &openCallback, int &errCode);

    /**
     * Checks if the RDB store is ready.
     *
     * @return A boolean value indicating whether the store is ready.
     */
    bool RdbStoreLTIsReady();

    /**
     * Inserts a new record into the specified table.
     *
     * @param id A reference to an integer that will hold the ID of the inserted record.
     * @param initialValues The initial values for the new record.
     * @param table The name of the table to insert the record into.
     *
     * @return An integer indicating the operation result. 0 for success, non-zero for failure.
     */
    int InsertLT(int64_t &id, const NativeRdb::ValuesBucket &initialValues, const std::string &table);

    /**
     * Deletes records based on the specified predicates.
     *
     * @param deletedRows A reference to an integer that will hold the number of deleted rows.
     * @param predicates The conditions used to identify the records to delete.
     *
     * @return An integer indicating the operation result. 0 for success, non-zero for failure.
     */
    int DeleteLT(int &deletedRows, const NativeRdb::AbsRdbPredicates &predicates);

    /**
     * Updates records based on the specified predicates.
     *
     * @param changedRows A reference to an integer that will hold the number of updated rows.
     * @param values The new values for the records to update.
     * @param predicates The conditions used to identify the records to update.
     *
     * @return An integer indicating the operation result. 0 for success, non-zero for failure.
     */
    int UpdateLT(int &changedRows, const NativeRdb::ValuesBucket &values,
        const NativeRdb::AbsRdbPredicates &predicates);

    /**
     * Queries records based on the specified predicates.
     *
     * @param predicates The conditions used to identify the records to query.
     * @param columns The list of columns to retrieve.
     *
     * @return A shared pointer to a ResultSet object containing the query results.
     */
    std::shared_ptr<NativeRdb::ResultSet> QueryLT(
        const NativeRdb::AbsRdbPredicates &predicates, const std::vector<std::string> columns);

    /**
     * Frees the RDB store resources.
     */
    void FreeRdbStoreE();

    /**
     * Begins a transaction.
     *
     * @return An integer indicating the operation result. 0 for success, non-zero for failure.
     */
    int BeginTransaction();

    /**
     * Rolls back the current transaction.
     *
     * @return An integer indicating the operation result. 0 for success, non-zero for failure.
     */
    int RollBack();

    /**
     * Commits the current transaction.
     *
     * @return An integer indicating the operation result. 0 for success, non-zero for failure.
     */
    int Commit();

    /**
     * Checks if the store exists.
     *
     * @return An integer indicating the operation result. 0 for success, non-zero for failure.
     */
    int IsExistStoreE();
    /**
     * Checks if the LT store exists.
     *
     * @return An integer indicating the operation result. 0 for success, non-zero for failure.
     */
    int IsExistStoreLT();

    bool BackupEl1ToEl2();
    void ChangeRdbStoreToEl1();
    void ChangeRdbStoreToLT();
    void ChangeRdbStoreToEl2();
    int BackupEl1ToEL2(const std::string &path);
    void FreeRdbStoreEl1();
    void DeleteDdmstable();

private:
    int IsExistStore();

    void TryRestore(int ret, const std::string &action);
    void ReportGetRdbStoreError(const std::string &scene);
private:
    const std::string BAK_DB_NAME = "sms_mms.db.bak";
    const int32_t GET_RDBSTORE_RETRY_TIMEOUT = 200;
    std::shared_ptr<NativeRdb::RdbStore> storeE_ = nullptr; // EL2
    std::shared_ptr<NativeRdb::RdbStore> storeLT_ = nullptr; // EL1临时库
    std::shared_ptr<NativeRdb::RdbStore> storeEl1_ = nullptr; // EL1
    RdbSmsMmsDFXAdapter dfxAdapter_;
};
} // namespace Telephony
} // namespace OHOS
#endif // DATA_STORAGE_RDB_SMS_MMS_BASE_HELPER_H
