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

void RdbOpKeyCallback::InitData(NativeRdb::RdbStore &rdbStore, const std::string &tableName)
{
    ParserUtil util;
    std::vector<OpKey> vec;
    if (!util.ParseFromCustomSystem(vec)) {
        return;
    }
    ClearData(rdbStore);
    for (size_t i = 0; i < vec.size(); i++) {
        NativeRdb::ValuesBucket value;
        util.ParserOpKeyToValuesBucket(value, vec[i]);
        int64_t id;
        rdbStore.Insert(id, tableName, value);
    }
}

int RdbOpKeyCallback::ClearData(NativeRdb::RdbStore &rdbStore)
{
    std::string sql;
    sql.append("delete from ").append(TABLE_OPKEY_INFO);
    return rdbStore.ExecuteSql(sql);
}
} // namespace Telephony
} // namespace OHOS
