/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#include "opkey_version_result_set_bridge.h"

#include "data_storage_log_wrapper.h"
#include "rdb_errno.h"

namespace OHOS {
namespace Telephony {
OpkeyVersionResultSetBridge::OpkeyVersionResultSetBridge(const std::string &info) : opkeyInfo_(info) {}

int OpkeyVersionResultSetBridge::GetRowCount(int32_t &count)
{
    count = ROW_COUNT;
    return NativeRdb::E_OK;
}

int OpkeyVersionResultSetBridge::GetAllColumnNames(std::vector<std::string> &columnsName)
{
    columnsName = {"key", "value"};
    return NativeRdb::E_OK;
}

bool OpkeyVersionResultSetBridge::FillBlock(DataShare::ResultSetBridge::Writer &writer)
{
    int statusAlloc = writer.AllocRow();
    if (statusAlloc != NativeRdb::E_OK) {
        return false;
    }
    int keyStatus = writer.Write(KEY_COLUMN, INFO_KEY.c_str(), INFO_KEY.size() + 1);
    if (keyStatus != NativeRdb::E_OK) {
        return false;
    }
    int valueStatus = writer.Write(VALUE_COLUMN, opkeyInfo_.c_str(), opkeyInfo_.size() + 1);
    if (valueStatus != NativeRdb::E_OK) {
        return false;
    }
    return true;
}

int OpkeyVersionResultSetBridge::OnGo(int32_t start, int32_t target, DataShare::ResultSetBridge::Writer &writer)
{
    DATA_STORAGE_LOGD("start");
    if ((start < 0) || (target < 0) || (start > target)) {
        return INVALID_POSITION;
    }
    return FillBlock(writer);
}

} // Telephony
} // OHOS