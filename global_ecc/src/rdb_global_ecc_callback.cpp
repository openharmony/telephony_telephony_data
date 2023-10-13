/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "rdb_global_ecc_callback.h"

#include "data_storage_errors.h"
#include "data_storage_log_wrapper.h"
#include "parser_util.h"
#include "global_ecc_data.h"
#include "rdb_errno.h"
#include "rdb_store.h"
#include "values_bucket.h"

namespace OHOS {
namespace Telephony {
RdbGlobalEccCallback::RdbGlobalEccCallback(
    const std::vector<std::string> &createTableVec) : RdbBaseCallBack(createTableVec) {}

int RdbGlobalEccCallback::OnUpgrade(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
{
    DATA_STORAGE_LOGI("RdbGlobalEccCallback::OnUpgrade##oldVersion = %d, newVersion = %d\n", oldVersion, newVersion);
    return NativeRdb::E_OK;
}

int RdbGlobalEccCallback::OnDowngrade(NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion)
{
    DATA_STORAGE_LOGI(
        "RdbGlobalEccCallback::OnDowngrade##currentVersion = %d, "
        "targetVersion = %d\n",
        currentVersion, targetVersion);
    return NativeRdb::E_OK;
}

int RdbGlobalEccCallback::OnCreate(NativeRdb::RdbStore &rdbStore)
{
    DATA_STORAGE_LOGI("RdbGlobalEccCallback::OnCreate");
    RdbBaseCallBack::OnCreate(rdbStore);
    InitData(rdbStore, TABLE_GLOBAL_ECC);
    return NativeRdb::E_OK;
}

int RdbGlobalEccCallback::OnOpen(NativeRdb::RdbStore &rdbStore)
{
    DATA_STORAGE_LOGI("RdbGlobalEccCallback::OnOpen");
    return NativeRdb::E_OK;
}

void RdbGlobalEccCallback::InitData(NativeRdb::RdbStore &rdbStore, const std::string &tableName)
{
    DATA_STORAGE_LOGD("InitData start");
    ParserUtil util;
    std::vector<GlobalEcc> vec;
    int ret = util.ParserGlobalEccJson(vec);
    if (ret != DATA_STORAGE_SUCCESS) {
        DATA_STORAGE_LOGE("ParserGlobalEccJson fail!");
        return;
    }
    ret = rdbStore.BeginTransaction();
    if (ret != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("BeginTransaction error!");
        return;
    }
    DATA_STORAGE_LOGD("InitData size = %{public}zu", vec.size());
    for (size_t i = 0; i < vec.size(); i++) {
        NativeRdb::ValuesBucket value;
        util.ParserGlobalEccToValuesBucket(value, vec[i]);
        int64_t id;
        rdbStore.Insert(id, tableName, value);
    }
    rdbStore.Commit();
    DATA_STORAGE_LOGD("InitData end");
}
} // namespace Telephony
} // namespace OHOS
