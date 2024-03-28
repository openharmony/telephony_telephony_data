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

#ifndef DATA_STORAGE_RDB_BASE_HELPER_H
#define DATA_STORAGE_RDB_BASE_HELPER_H

#include <stdint.h>

#include "data_storage_log_wrapper.h"
#include "iosfwd"
#include "memory"
#include "string"
#include "vector"

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
class RdbBaseHelper {
public:
    RdbBaseHelper() {};
    ~RdbBaseHelper() = default;
    int Insert(int64_t &id, const NativeRdb::ValuesBucket &initialValues, const std::string &table);
    int BatchInsert(int64_t &id, const NativeRdb::ValuesBucket &initialValues, const std::string &table);
    int Update(int &changedRows, const std::string &table, const NativeRdb::ValuesBucket &values,
        const std::string &whereClause = "",
        const std::vector<std::string> &whereArgs = std::vector<std::string>());
    int Update(int &changedRows, const NativeRdb::ValuesBucket &values, const NativeRdb::AbsRdbPredicates &predicates);
    int Delete(int &changedRows, const std::string &table, const std::string &whereClause = "",
        const std::vector<std::string> &whereArgs = std::vector<std::string>());
    int Delete(int &deletedRows, const NativeRdb::AbsRdbPredicates &predicates);
    std::shared_ptr<NativeRdb::ResultSet> QuerySql(
        const std::string &sql, const std::vector<std::string> &selectionArgs = std::vector<std::string>());
    std::shared_ptr<NativeRdb::ResultSet> Query(
        const NativeRdb::AbsRdbPredicates &predicates, const std::vector<std::string> columns);
    int ExecuteSql(const std::string &sql);
    int ExecuteSql(const std::string &sql, const std::vector<NativeRdb::ValueObject> &bindArgs);
    void CreateRdbStore(const NativeRdb::RdbStoreConfig &config, int version,
        NativeRdb::RdbOpenCallback &openCallback, int &errCode);
    int BeginTransaction();
    int RollBack();
    int Commit();

    void ReplaceAllStr(std::string &path, const std::string &oldStr, const std::string &newStr);

public:
    const std::string FOLDER_PATH = "/data/accounts/account_0/appdata/com.ohos.smsmmsability/database/";

protected:
    std::shared_ptr<NativeRdb::RdbStore> store_;

private:
    int IsExistStore();
};
} // namespace Telephony
} // namespace OHOS
#endif // DATA_STORAGE_RDB_BASE_HELPER_H
