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

#include "rdb_base_callback.h"

#include "data_storage_log_wrapper.h"
#include "rdb_errno.h"
#include "rdb_store.h"

namespace OHOS {
namespace Telephony {
int RdbBaseCallBack::OnCreate(NativeRdb::RdbStore &rdbStore)
{
    DATA_STORAGE_LOGI("Data_Storage RdbBaseCallBack::OnCreate\n");
    int32_t ret = NativeRdb::E_ERROR;
    size_t size = createTableVec_.size();
    if (size == 0) {
        return ret;
    }
    for (size_t i = 0; i < createTableVec_.size(); i++) {
        ret = rdbStore.ExecuteSql(createTableVec_[i]);
    }
    return ret;
}
} // namespace Telephony
} // namespace OHOS
