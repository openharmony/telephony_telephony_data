/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "rdb_opkey_callback.h"

#include "data_storage_errors.h"
#include "data_storage_log_wrapper.h"
#include "opkey_data.h"
#include "parser_util.h"
#include "preferences_util.h"
#include "rdb_errno.h"
#include "rdb_store.h"
#include "string"
#include "values_bucket.h"

namespace OHOS {
namespace Telephony {
int RdbOpKeyCallback::OnUpgrade(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
{
    DATA_STORAGE_LOGI(
        "Data_Storage RdbOpKeyCallback::OnUpgrade##oldVersion = %d, "
        "newVersion = %d\n",
        oldVersion, newVersion);
    return NativeRdb::E_OK;
}

int RdbOpKeyCallback::OnDowngrade(NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion)
{
    DATA_STORAGE_LOGI(
        "Data_Storage RdbOpKeyCallback::OnDowngrade##currentVersion = "
        "%d, targetVersion = %d\n",
        currentVersion, targetVersion);
    return NativeRdb::E_OK;
}

int RdbOpKeyCallback::OnCreate(NativeRdb::RdbStore &rdbStore)
{
    DATA_STORAGE_LOGI("RdbOpKeyCallback::OnCreate");
    RdbBaseCallBack::OnCreate(rdbStore);
    InitData(rdbStore, TABLE_OPKEY_INFO);
    return NativeRdb::E_OK;
}

int RdbOpKeyCallback::OnOpen(NativeRdb::RdbStore &rdbStore)
{
    DATA_STORAGE_LOGI("RdbOpKeyCallback::OnOpen");
    return NativeRdb::E_OK;
}

int64_t RdbOpKeyCallback::InitData(NativeRdb::RdbStore &rdbStore, const std::string &tableName)
{
    DATA_STORAGE_LOGI("InitData::start");
    ParserUtil util;
    std::vector<OpKey> vec;
    std::string path;
    util.GetOpKeyFilePath(path);
    if (path.empty()) {
        DATA_STORAGE_LOGE("InitData fail! path empty!");
        return DATA_STORAGE_ERROR;
    }
    std::string checksum;
    util.GetFileChecksum(path.c_str(), checksum);
    if (checksum.empty()) {
        DATA_STORAGE_LOGE("InitData fail! checksum is null!");
        return DATA_STORAGE_ERROR;
    }
    if (!IsOpKeyDbUpdateNeeded(checksum)) {
        DATA_STORAGE_LOGI("InitData::opkey data has not change.");
        return DATA_STORAGE_SUCCESS;
    }

    if (util.ParserOpKeyJson(vec, path.c_str()) != DATA_STORAGE_SUCCESS) {
        DATA_STORAGE_LOGE("Parse OpKey info fail");
        return DATA_STORAGE_ERROR;
    }
    ClearData(rdbStore);
    int result = rdbStore.BeginTransaction();
    if (result != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("BeginTransaction error!");
        return DATA_STORAGE_ERROR;
    }
    DATA_STORAGE_LOGD("InitData size = %{public}zu", vec.size());
    std::vector<NativeRdb::ValuesBucket> valuesBuckets;
    for (size_t i = 0; i < vec.size(); i++) {
        NativeRdb::ValuesBucket value;
        util.ParserOpKeyToValuesBucket(value, vec[i]);
        valuesBuckets.push_back(value);
    }
    int64_t outInsertNum;
    result = rdbStore.BatchInsert(outInsertNum, tableName, valuesBuckets);
    if (result == NativeRdb::E_OK) {
        result = rdbStore.Commit();
    }
    if (result != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("Commit error!");
        return DATA_STORAGE_ERROR;
    }
    SetPreferOpKeyConfChecksum(checksum);
    DATA_STORAGE_LOGI("InitData::end insert data: %{public}s", std::to_string(outInsertNum).c_str());
    return outInsertNum;
}

int RdbOpKeyCallback::ClearData(NativeRdb::RdbStore &rdbStore)
{
    std::string sql;
    sql.append("delete from ").append(TABLE_OPKEY_INFO);
    return rdbStore.ExecuteSql(sql);
}

bool RdbOpKeyCallback::IsOpKeyDbUpdateNeeded(std::string &checkSum)
{
    auto preferencesUtil = DelayedSingleton<PreferencesUtil>::GetInstance();
    if (preferencesUtil != nullptr) {
        std::string lastCheckSum = preferencesUtil->ObtainString(OPKEY_CONF_CHECKSUM, "");
        if (checkSum.compare(lastCheckSum) == 0) {
            return false;
        }
    }
    return true;
}

int RdbOpKeyCallback::SetPreferOpKeyConfChecksum(std::string &checkSum)
{
    auto preferencesUtil = DelayedSingleton<PreferencesUtil>::GetInstance();
    if (preferencesUtil == nullptr) {
        DATA_STORAGE_LOGE("preferencesUtil is nullptr!");
        return NativePreferences::E_ERROR;
    }
    return preferencesUtil->SaveString(OPKEY_CONF_CHECKSUM, checkSum);
}
} // namespace Telephony
} // namespace OHOS
