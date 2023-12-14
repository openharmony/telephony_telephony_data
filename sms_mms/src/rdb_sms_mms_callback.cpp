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

#include "rdb_sms_mms_callback.h"

#include "data_storage_log_wrapper.h"
#include "rdb_errno.h"
#include "sms_mms_data.h"

namespace OHOS {
namespace NativeRdb {
class RdbStore;
}
namespace Telephony {
int RdbSmsMmsCallback::OnUpgrade(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion)
{
    DATA_STORAGE_LOGI(
        "Data_Storage RdbSmsMmsCallback::OnUpgrade##oldVersion = %d, "
        "newVersion = %d\n",
        oldVersion, newVersion);
    if (oldVersion < VERSION_2 && newVersion >= VERSION_2) {
        rdbStore.ExecuteSql("ALTER TABLE " + std::string(TABLE_SMS_MMS_INFO) + " ADD COLUMN " +
                            std::string(SmsMmsInfo::IS_ADVANCED_SECURITY) + " INTEGER DEFAULT " + "0;");
        oldVersion = VERSION_2;
    }

    if (oldVersion != newVersion) {
        DATA_STORAGE_LOGE("upgrade error oldVersion = %{public}d, newVersion = %{public}d ", oldVersion, newVersion);
        return NativeRdb::E_ERROR;
    }
    return NativeRdb::E_OK;
}

int RdbSmsMmsCallback::OnDowngrade(NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion)
{
    DATA_STORAGE_LOGI(
        "Data_Storage RdbSmsMmsCallback::OnDowngrade##currentVersion "
        "= %d, targetVersion = %d\n",
        currentVersion, targetVersion);
    return NativeRdb::E_OK;
}

int RdbSmsMmsCallback::OnOpen(NativeRdb::RdbStore &rdbStore)
{
    return NativeRdb::E_OK;
}
} // namespace Telephony
} // namespace OHOS
