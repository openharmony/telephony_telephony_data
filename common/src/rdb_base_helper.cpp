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

#include "rdb_base_helper.h"

#include <regex>

#include "abs_shared_result_set.h"
#include "algorithm"
#include "data_storage_log_wrapper.h"
#include "new"
#include "rdb_errno.h"
#include "rdb_helper.h"
#include "rdb_store.h"
#include "type_traits"

namespace OHOS {
namespace NativeRdb {
class AbsRdbPredicates;
class RdbOpenCallback;
class RdbStoreConfig;
class ValueObject;
class ValuesBucket;
} // namespace NativeRdb
namespace Telephony {
int RdbBaseHelper::Insert(int64_t &id, const NativeRdb::ValuesBucket &initialValues, const std::string &table)
{
    int ret = IsExistStore();
    if (ret == NativeRdb::E_OK) {
        ret = store_->Insert(id, table, initialValues);
    }
    return ret;
}

int RdbBaseHelper::Update(int &changedRows, const std::string &table, const NativeRdb::ValuesBucket &values,
    const std::string &whereClause, const std::vector<std::string> &whereArgs)
{
    int ret = IsExistStore();
    if (ret == NativeRdb::E_OK) {
        ret = store_->Update(changedRows, table, values, whereClause, whereArgs);
    }
    return ret;
}

int RdbBaseHelper::Update(int &changedRows, const NativeRdb::ValuesBucket &values,
    const NativeRdb::AbsRdbPredicates &predicate)
{
    int ret = IsExistStore();
    if (ret == NativeRdb::E_OK) {
        ret = store_->Update(changedRows, values, predicate);
    }
    return ret;
}

int RdbBaseHelper::Delete(int &changedRows, const std::string &table, const std::string &whereClause,
    const std::vector<std::string> &whereArgs)
{
    int ret = IsExistStore();
    if (ret == NativeRdb::E_OK) {
        ret = store_->Delete(changedRows, table, whereClause, whereArgs);
    }
    return ret;
}

int RdbBaseHelper::Delete(int &deletedRows, const NativeRdb::AbsRdbPredicates &predicates)
{
    int ret = IsExistStore();
    if (ret == NativeRdb::E_OK) {
        ret = store_->Delete(deletedRows, predicates);
    }
    return ret;
}

int RdbBaseHelper::ExecuteSql(const std::string &sql)
{
    int ret = IsExistStore();
    if (ret == NativeRdb::E_OK) {
        ret = store_->ExecuteSql(sql);
    }
    return ret;
}

int RdbBaseHelper::ExecuteSql(const std::string &sql, const std::vector<NativeRdb::ValueObject> &bindArgs)
{
    int ret = IsExistStore();
    if (ret == NativeRdb::E_OK) {
        ret = store_->ExecuteSql(sql, bindArgs);
    }
    return ret;
}

std::shared_ptr<NativeRdb::ResultSet> RdbBaseHelper::QuerySql(
    const std::string &sql, const std::vector<std::string> &selectionArgs)
{
    int ret = IsExistStore();
    if (ret == NativeRdb::E_OK) {
        return store_->QuerySql(sql);
    }
    return nullptr;
}

std::shared_ptr<NativeRdb::ResultSet> RdbBaseHelper::Query(
    const NativeRdb::AbsRdbPredicates &predicates, const std::vector<std::string> columns)
{
    int ret = IsExistStore();
    if (ret == NativeRdb::E_OK) {
        return store_->Query(predicates, columns);
    }
    return nullptr;
}

int RdbBaseHelper::IsExistStore()
{
    if (store_ == nullptr) {
        DATA_STORAGE_LOGE("RdbBaseHelper::IsExistStore NativeRdb::RdbStore is null!");
        return NativeRdb::E_ERROR;
    }
    return NativeRdb::E_OK;
}

void RdbBaseHelper::CreateRdbStore(
    const NativeRdb::RdbStoreConfig &config, int version, NativeRdb::RdbOpenCallback &openCallback, int &errCode)
{
    DATA_STORAGE_LOGD("RdbBaseHelper::CreateRdbStore");
    store_ = NativeRdb::RdbHelper::GetRdbStore(config, version, openCallback, errCode);
}

int RdbBaseHelper::BeginTransaction()
{
    int ret = IsExistStore();
    if (ret == NativeRdb::E_OK) {
        ret = store_->BeginTransaction();
    }
    return ret;
}

int RdbBaseHelper::RollBack()
{
    int ret = IsExistStore();
    if (ret == NativeRdb::E_OK) {
        ret = store_->RollBack();
    }
    return ret;
}

int RdbBaseHelper::Commit()
{
    int ret = IsExistStore();
    if (ret == NativeRdb::E_OK) {
        ret = store_->Commit();
    }
    return ret;
}

int RdbBaseHelper::BatchInsert(int64_t &id, const NativeRdb::ValuesBucket &initialValues, const std::string &table)
{
    return NativeRdb::E_OK;
}

void RdbBaseHelper::ReplaceAllStr(std::string &path, const std::string &oldStr, const std::string &newStr)
{
    path = std::regex_replace(path, std::regex(oldStr), newStr);
}
} // namespace Telephony
} // namespace OHOS
