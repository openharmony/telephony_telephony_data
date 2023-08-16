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

#include "rdb_pdp_profile_callback.h"

#include "data_storage_errors.h"
#include "data_storage_log_wrapper.h"
#include "parser_util.h"
#include "pdp_profile_data.h"
#include "rdb_errno.h"
#include "rdb_store.h"
#include "values_bucket.h"

namespace OHOS {
namespace Telephony {
int RdbPdpProfileCallback::OnUpgrade(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
{
    DATA_STORAGE_LOGI("RdbPdpProfileCallback::OnUpgrade##oldVersion = %d, newVersion = %d\n", oldVersion, newVersion);
    return NativeRdb::E_OK;
}

int RdbPdpProfileCallback::OnDowngrade(NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion)
{
    DATA_STORAGE_LOGI(
        "RdbPdpProfileCallback::OnDowngrade##currentVersion = %d, "
        "targetVersion = %d\n",
        currentVersion, targetVersion);
    return NativeRdb::E_OK;
}

int RdbPdpProfileCallback::OnCreate(NativeRdb::RdbStore &rdbStore)
{
    DATA_STORAGE_LOGI("RdbPdpProfileCallback::OnCreate");
    RdbBaseCallBack::OnCreate(rdbStore);
    InitData(rdbStore, TABLE_PDP_PROFILE);
    return NativeRdb::E_OK;
}

int RdbPdpProfileCallback::OnOpen(NativeRdb::RdbStore &rdbStore)
{
    DATA_STORAGE_LOGI("RdbPdpProfileCallback::OnOpen");
    return NativeRdb::E_OK;
}

void RdbPdpProfileCallback::InitData(NativeRdb::RdbStore &rdbStore, const std::string &tableName)
{
    ParserUtil util;
    std::vector<PdpProfile> vec;
    int resultCode = util.ParserPdpProfileJson(vec);
    if (resultCode != DATA_STORAGE_SUCCESS) {
        return;
    }
    for (size_t i = 0; i < vec.size(); i++) {
        NativeRdb::ValuesBucket value;
        util.ParserPdpProfileToValuesBucket(value, vec[i]);
        int64_t id;
        rdbStore.Insert(id, tableName, value);
    }
}
} // namespace Telephony
} // namespace OHOS
