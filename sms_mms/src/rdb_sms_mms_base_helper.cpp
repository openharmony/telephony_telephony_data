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

#include "rdb_sms_mms_base_helper.h"

#include <regex>

#include "abs_shared_result_set.h"
#include "data_storage_log_wrapper.h"
#include "new"
#include "rdb_errno.h"
#include "rdb_helper.h"
#include "rdb_store.h"
#include <mutex>

namespace OHOS {
namespace NativeRdb {
class AbsRdbPredicates;
class RdbOpenCallback;
class RdbStoreConfig;
class ValueObject;
class ValuesBucket;
} // namespace NativeRdb
namespace Telephony {
namespace {
std::mutex g_mutex;
}

void RdbSmsMmsBaseHelper::TryRestore(int ret, const std::string &action)
{
    if (ret == NativeRdb::E_SQLITE_CORRUPT) {
        int result = Restore(BAK_DB_NAME);
        DATA_STORAGE_LOGE("RdbSmsMmsBaseHelper::%s E_SQLITE_CORRUPT occurs, try restore %{public}d", action.c_str(),
                          result);
    }
}

int RdbSmsMmsBaseHelper::Insert(int64_t &id, const NativeRdb::ValuesBucket &initialValues, const std::string &table)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    int ret = IsExistStore();
    if (ret == NativeRdb::E_OK) {
        ret = store_->Insert(id, table, initialValues);
    }
    TryRestore(ret, "Insert");
    return ret;
}

int RdbSmsMmsBaseHelper::Update(int &changedRows, const std::string &table, const NativeRdb::ValuesBucket &values,
    const std::string &whereClause, const std::vector<std::string> &whereArgs)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    int ret = IsExistStore();
    if (ret == NativeRdb::E_OK) {
        ret = store_->Update(changedRows, table, values, whereClause, whereArgs);
    }
    TryRestore(ret, "Update");
    return ret;
}

int RdbSmsMmsBaseHelper::Update(int &changedRows, const NativeRdb::ValuesBucket &values,
    const NativeRdb::AbsRdbPredicates &predicate)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    int ret = IsExistStore();
    if (ret == NativeRdb::E_OK) {
        ret = store_->Update(changedRows, values, predicate);
        // changedRows is Number of Updates, Print more than 1
        if (changedRows > 1) {
            DATA_STORAGE_LOGI("Update changedRows = %{public}d", changedRows);
        }
    }
    TryRestore(ret, "Update");
    return ret;
}

int RdbSmsMmsBaseHelper::Delete(int &changedRows, const std::string &table, const std::string &whereClause,
    const std::vector<std::string> &whereArgs)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    int ret = IsExistStore();
    if (ret == NativeRdb::E_OK) {
        ret = store_->Delete(changedRows, table, whereClause, whereArgs);
    }
    TryRestore(ret, "Delete");
    return ret;
}

int RdbSmsMmsBaseHelper::Delete(int &deletedRows, const NativeRdb::AbsRdbPredicates &predicates)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    int ret = IsExistStore();
    if (ret == NativeRdb::E_OK) {
        ret = store_->Delete(deletedRows, predicates);
    }
    TryRestore(ret, "Delete");
    return ret;
}

int RdbSmsMmsBaseHelper::ExecuteSql(const std::string &sql)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    int ret = IsExistStore();
    if (ret == NativeRdb::E_OK) {
        ret = store_->ExecuteSql(sql);
    }
    TryRestore(ret, "ExecuteSql");
    return ret;
}

int RdbSmsMmsBaseHelper::ExecuteSql(const std::string &sql, const std::vector<NativeRdb::ValueObject> &bindArgs)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    int ret = IsExistStore();
    if (ret == NativeRdb::E_OK) {
        ret = store_->ExecuteSql(sql, bindArgs);
    }
    TryRestore(ret, "ExecuteSql");
    return ret;
}

int RdbSmsMmsBaseHelper::ExecuteSql(const std::string &sql, bool isEl5)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    int ret = NativeRdb::E_OK;
    if (isEl5) {
        ret = IsExistStoreE();
        if (ret == NativeRdb::E_OK) {
            ret = storeE_->ExecuteSql(sql);
        }
    } else {
        ret = IsExistStoreLT();
        if (ret == NativeRdb::E_OK) {
            ret = storeLT_->ExecuteSql(sql);
        }
    }
    if (ret == NativeRdb::E_SQLITE_CORRUPT) {
        int result = Restore(BAK_DB_NAME, isEl5);
        DATA_STORAGE_LOGE("RdbSmsMmsBaseHelper::ExecuteSql E_SQLITE_CORRUPT occurs, try restore %{public}d",
                          result);
    }
    return ret;
}

int RdbSmsMmsBaseHelper::IsExistStoreE()
{
    if (storeE_ == nullptr) {
        DATA_STORAGE_LOGE("RdbSmsMmsBaseHelper, storeE is null!");
        return NativeRdb::E_ERROR;
    }
    return NativeRdb::E_OK;
}

int RdbSmsMmsBaseHelper::IsExistStore()
{
    if (store_ == nullptr) {
        DATA_STORAGE_LOGE("RdbSmsMmsBaseHelper, store is null!");
        return NativeRdb::E_ERROR;
    }
    return NativeRdb::E_OK;
}

int RdbSmsMmsBaseHelper::IsExistStoreLT()
{
    if (storeLT_ == nullptr) {
        DATA_STORAGE_LOGE("RdbSmsMmsBaseHelper, storeLT is null!");
        return NativeRdb::E_ERROR;
    }
    return NativeRdb::E_OK;
}

void RdbSmsMmsBaseHelper::ReportGetRdbStoreError(const std::string &scene)
{
    dfxAdapter_.ReportDataBaseError(static_cast<int>(RDBOperation::CREATE_RDBSTORE), scene, 0);
}

// EL1往El2迁移的时候，先删除El1中的搜索水线。否则概率出现搜索不准的问题。
void RdbSmsMmsBaseHelper::DeleteDdmstable()
{
    int res = NativeRdb::E_OK;
    DATA_STORAGE_LOGI("DeleteDdmstable start");
    std::lock_guard<std::mutex> lock(g_mutex);
    if (storeEl1_ == nullptr) {
         DATA_STORAGE_LOGE("storeEl1_ is null");
         return;
    }
    std::string sql = "DROP TABLE IF EXISTS ddms_data_search_aux_config";
    res = storeEl1_->ExecuteSql(sql);
    if (res != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("RdbSmsMmsBaseHelper, DROP ddms_data_search_aux_config error");
    }
    sql = "DROP TABLE IF EXISTS ddms_data_search_filtered_data";
    res = storeEl1_->ExecuteSql(sql);
    if (res != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("RdbSmsMmsBaseHelper, DROP ddms_data_search_filtered_data error");
    }
}

void RdbSmsMmsBaseHelper::CreateEl1RdbStore(const NativeRdb::RdbStoreConfig &config, int version,
    NativeRdb::RdbOpenCallback &openCallback, int &errCode)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    storeEl1_ = NativeRdb::RdbHelper::GetRdbStore(config, version, openCallback, errCode);
    if (errCode != NativeRdb::E_OK || storeEl1_ == nullptr) {
        ReportGetRdbStoreError("EL1 GET RDBSTORE FAILED");
        DATA_STORAGE_LOGE("RdbSmsMmsBaseHelper::CreateEl1RdbStore Error %{public}d", errCode);
        return;
    }

    auto rebuiltType = NativeRdb::RebuiltType::NONE;
    int rebuiltCode = storeEl1_->GetRebuilt(rebuiltType);
    DATA_STORAGE_LOGI("CreateEl1RdbStore rebuiltCode: %{public}d, rebuiltType: %{public}d", rebuiltCode, rebuiltType);
    if (rebuiltType == NativeRdb::RebuiltType::REBUILT && rebuiltCode == NativeRdb::E_OK) {
        // if rdb rebuilt, call restore from bak file
        int restoreCode = storeEl1_->Restore(BAK_DB_NAME);
        DATA_STORAGE_LOGE("restoreCode %{public}d", restoreCode);
        // 没有备份或者备份失败，重新走开库。
        storeEl1_ = nullptr;
        storeEl1_ = NativeRdb::RdbHelper::GetRdbStore(config, version, openCallback, errCode);
        if (errCode != NativeRdb::E_OK || storeEl1_ == nullptr) {
            ReportGetRdbStoreError("EL1 REBUILD GET RDBSTORE FAILED");
            DATA_STORAGE_LOGE("RdbSmsMmsBaseHelper::CreateEl1RdbStore Error %{public}d", errCode);
            return;
        }
    }
}

void RdbSmsMmsBaseHelper::CreateRdbStoreSms(const NativeRdb::RdbStoreConfig &config, int version,
    NativeRdb::RdbOpenCallback &openCallback, int &errCode)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    storeE_ = NativeRdb::RdbHelper::GetRdbStore(config, version, openCallback, errCode);
    if (errCode != NativeRdb::E_OK || storeE_ == nullptr) {
        ReportGetRdbStoreError("EL2 GET RDBSTORE FAILED");
        std::this_thread::sleep_for(std::chrono::milliseconds(GET_RDBSTORE_RETRY_TIMEOUT));
        storeE_ = NativeRdb::RdbHelper::GetRdbStore(config, version, openCallback, errCode);
        if (errCode != NativeRdb::E_OK || storeE_ == nullptr) {
            ReportGetRdbStoreError("EL2 RETRY GET RDBSTORE FAILED");
            DATA_STORAGE_LOGE("RdbSmsMmsBaseHelper::CreateRdbStoreSms el2 Error %{public}d", errCode);
            return;
        }
    }

    auto rebuiltType = NativeRdb::RebuiltType::NONE;
    int rebuiltCode = storeE_->GetRebuilt(rebuiltType);
    DATA_STORAGE_LOGI("RdbSmsMmsBaseHelper::CreateRdbStoreSms el2 rebuiltCode: %{public}d, rebuilt: %{public}d",
        rebuiltCode, rebuiltType);
    if (rebuiltType == NativeRdb::RebuiltType::REBUILT && rebuiltCode == NativeRdb::E_OK) {
        // if rdb rebuilt, call restore from bak file
        int result = storeE_->Restore(BAK_DB_NAME);
        DATA_STORAGE_LOGE("RdbSmsMmsBaseHelper::ExecuteSql E_SQLITE_CORRUPT occurs, try restore %{public}d",
            result);
        storeE_ = nullptr; // 释放老的rdb，重新走开库流程，会触发升级流程
        storeE_ = NativeRdb::RdbHelper::GetRdbStore(config, version, openCallback, errCode);
        if (errCode != NativeRdb::E_OK || storeE_ == nullptr) {
            ReportGetRdbStoreError("EL2 REBUILD GET RDBSTORE FAILED");
            DATA_STORAGE_LOGE("RdbSmsMmsBaseHelper::CreateRdbStoreSms Error %{public}d", errCode);
            return;
        }
    }
}

int RdbSmsMmsBaseHelper::Restore(const std::string &bakDBName)
{
    int ret = IsExistStore();
    if (ret == NativeRdb::E_OK) {
        ret = store_->Restore(bakDBName);
    }
    return ret;
}

int RdbSmsMmsBaseHelper::Restore(const std::string &bakDBName, bool isEl5)
{
    int ret = NativeRdb::E_OK;
    if (isEl5) {
        ret = IsExistStoreE();
        if (ret == NativeRdb::E_OK) {
            ret = storeE_->Restore(bakDBName);
        }
    } else {
        ret = IsExistStoreLT();
        if (ret == NativeRdb::E_OK) {
            ret = storeLT_->Restore(bakDBName);
        }
    }
    return ret;
}

int RdbSmsMmsBaseHelper::GetRebuiltE(NativeRdb::RebuiltType &type)
{
    int ret = IsExistStoreE();
    if (ret == NativeRdb::E_OK) {
        ret = storeE_->GetRebuilt(type);
    }
    return ret;
}

// 迁移成功，但是只有EL1临时库可用，默认使用storeLT_
void RdbSmsMmsBaseHelper::ChangeRdbStoreToLT()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    DATA_STORAGE_LOGI("RdbSmsMmsBaseHelper::ChangeRdbStoreToLT!");
    store_ = storeLT_;
    storeEl1_ = nullptr;
}

// 迁移失败，继续使用EL1。所有句柄指向 storeEl1_
void RdbSmsMmsBaseHelper::ChangeRdbStoreToEl1()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    DATA_STORAGE_LOGI("RdbSmsMmsBaseHelper::ChangeRdbStoreToEl1!");
    store_ = storeEl1_;
    storeLT_ = storeEl1_; // 短信接收流程，协议使用storeLT_
    storeE_ = nullptr;
}

// 迁移成功，临时库和EL2都成功，默认使用EL2
void RdbSmsMmsBaseHelper::ChangeRdbStoreToEl2()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    DATA_STORAGE_LOGI("RdbSmsMmsBaseHelper::ChangeRdbStoreToEl2!");
    store_ = storeE_;
    storeEl1_ = nullptr;
}

void RdbSmsMmsBaseHelper::FreeRdbStoreE()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (storeE_ != nullptr) {
        DATA_STORAGE_LOGI("RdbSmsMmsBaseHelper::FreeRdbStoreE!");
        storeE_ = nullptr;
    }
}

void RdbSmsMmsBaseHelper::FreeRdbStoreEl1()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (storeEl1_ != nullptr) {
        DATA_STORAGE_LOGI("RdbSmsMmsBaseHelper::FreeRdbStoreEl1!");
        storeEl1_ = nullptr;
    }
}

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsBaseHelper::QuerySql(
    const std::string &sql, const std::vector<std::string> &selectionArgs)
{
    int ret = IsExistStore();
    if (ret == NativeRdb::E_OK) {
        return store_->QuerySql(sql);
    }
    return nullptr;
}

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsBaseHelper::QuerySql(
    const std::string &sql, bool isEl5, const std::vector<std::string> &selectionArgs)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (isEl5) {
        int ret = IsExistStoreE();
        if (ret == NativeRdb::E_OK) {
            DATA_STORAGE_LOGI("RdbSmsMmsBaseHelper::QuerySql storeE_->QuerySql");
            return storeE_->QuerySql(sql);
        }
    } else {
        int ret = IsExistStoreLT();
        if (ret == NativeRdb::E_OK) {
            DATA_STORAGE_LOGI("RdbSmsMmsBaseHelper::QuerySql storeLT_->QuerySql");
            return storeLT_->QuerySql(sql);
        }
    }
    return nullptr;
}

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsBaseHelper::Query(
    const NativeRdb::AbsRdbPredicates &predicates, const std::vector<std::string> columns)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    int ret = IsExistStore();
    if (ret == NativeRdb::E_OK) {
        return store_->Query(predicates, columns);
    }
    return nullptr;
}

int RdbSmsMmsBaseHelper::BeginTransaction()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    int ret = IsExistStore();
    if (ret == NativeRdb::E_OK) {
        ret = store_->BeginTransaction();
    }
    return ret;
}

int RdbSmsMmsBaseHelper::RollBack()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    int ret = IsExistStore();
    if (ret == NativeRdb::E_OK) {
        ret = store_->RollBack();
    }
    return ret;
}

int RdbSmsMmsBaseHelper::Commit()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    int ret = IsExistStore();
    if (ret == NativeRdb::E_OK) {
        ret = store_->Commit();
    }
    return ret;
}

// 开库失败要重建
void RdbSmsMmsBaseHelper::CreateRdbStoreLT(const NativeRdb::RdbStoreConfig &config,
    int version, NativeRdb::RdbOpenCallback &openCallback, int &errCode)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    storeLT_ = NativeRdb::RdbHelper::GetRdbStore(config, version, openCallback, errCode);
    if (errCode != NativeRdb::E_OK || storeLT_ == nullptr) {
        ReportGetRdbStoreError("EL1_EL1 GET RDBSTORE FAILED");
        DATA_STORAGE_LOGE("RdbSmsMmsBaseHelper::CreateRdbStoreLT Error %{public}d", errCode);
        return;
    }
    DATA_STORAGE_LOGI("RdbSmsMmsBaseHelper::CreateRdbStoreLT sms_mms_el1.db OK");
}

bool RdbSmsMmsBaseHelper::RdbStoreLTIsReady()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    return storeLT_ == nullptr ? false : true;
}

int RdbSmsMmsBaseHelper::InsertLT(int64_t &id, const NativeRdb::ValuesBucket &initialValues, const std::string &table)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (storeLT_ != nullptr) {
        DATA_STORAGE_LOGE("RdbSmsMmsBaseHelper::InsertLT NativeRdb::RdbStore not null");
        return storeLT_->Insert(id, table, initialValues);
    }
    return NativeRdb::E_SQLITE_ERROR;
}

int RdbSmsMmsBaseHelper::DeleteLT(int &deletedRows, const NativeRdb::AbsRdbPredicates &predicates)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (storeLT_ != nullptr) {
        DATA_STORAGE_LOGE("RdbSmsMmsBaseHelper::DeleteLT NativeRdb::RdbStore not null");
        return storeLT_->Delete(deletedRows, predicates);
    }
    return NativeRdb::E_SQLITE_ERROR;
}

int RdbSmsMmsBaseHelper::UpdateLT(int &changedRows, const NativeRdb::ValuesBucket &values,
    const NativeRdb::AbsRdbPredicates &predicates)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (storeLT_ != nullptr) {
        DATA_STORAGE_LOGE("RdbSmsMmsBaseHelper::UpdateLT NativeRdb::RdbStore not null");
        return storeLT_->Update(changedRows, values, predicates);
    }
    return NativeRdb::E_SQLITE_ERROR;
}

std::shared_ptr<NativeRdb::ResultSet> RdbSmsMmsBaseHelper::QueryLT(
    const NativeRdb::AbsRdbPredicates &predicates, const std::vector<std::string> columns)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (storeLT_ != nullptr) {
        DATA_STORAGE_LOGE("RdbSmsMmsBaseHelper::QueryLT NativeRdb::RdbStore not null");
        return storeLT_->Query(predicates, columns);
    }
    return nullptr;
}

int RdbSmsMmsBaseHelper::BackupEl1ToEL2(const std::string &path)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (storeEl1_ != nullptr) {
        return storeEl1_->Backup(path);
    } else {
        DATA_STORAGE_LOGE("storeEl1_ is null");
        return NativeRdb::E_SQLITE_ERROR;
    }
}
} // namespace Telephony
} // namespace OHOS
