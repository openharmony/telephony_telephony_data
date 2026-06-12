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

#include "rdb_sms_mms_dfx_adapter.h"
#include "hisysevent.h"
#include "data_storage_log_wrapper.h"

namespace OHOS {
namespace Telephony {
using namespace std;
using HiSysEventNameSpace = OHOS::HiviewDFX::HiSysEvent;

void RdbSmsMmsDFXAdapter::ReportDataBaseError(uint32_t scene, const std::string& uri, uint32_t duration)
{
    int ret = HiSysEventWrite(MMS_UE, "SMS_DATABASE_ERROR",
        HiSysEventNameSpace::EventType::BEHAVIOR,
        "PNAMEID", PNAMEID,
        "PVERSIONID",PVERSIONID,
        "SCENE", scene,
        "URI", uri,
        "DURATION", duration);
    if (ret != 0) {
        DATA_STORAGE_LOGE("HiSysEventWrite failed, ret=%{public}d", ret);
    }
}

void RdbSmsMmsDFXAdapter::ReportDataBaseUpdate(const std::string& updateRoute, uint32_t result,
                                               const std::string& failReason)
{
    int ret = HiSysEventWrite(MMS_UE, "SMS_DATA_UPDATE",
        HiSysEventNameSpace::EventType::BEHAVIOR,
        "PNAMEID", PNAMEID,
        "PVERSIONID",PVERSIONID,
        "UPDATE_ROUTE", updateRoute,
        "UPDATE_RESULT", result,
        "FAIL_REASON", failReason);
    if (ret != 0) {
        DATA_STORAGE_LOGE("HiSysEventWrite failed, ret=%{public}d", ret);
    }
}

int32_t RdbSmsMmsDFXAdapter::GetExeTimeout()
{
    return EXECUTES_TIMEOUT;
}
} // Telephony
} // OHOS