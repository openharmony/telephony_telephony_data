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

#ifndef DATA_STORAGE_RDB_SMS_MMS_DFX_ADAPTER_H
#define DATA_STORAGE_RDB_SMS_MMS_DFX_ADAPTER_H

#include <string>

namespace OHOS {
namespace Telephony {

enum class RDBOperation : int {
    QUERY = 1,
    UPDATE = 2,
    INSERT = 3,
    DELETE = 4,
    CREATE_RDBSTORE = 5
};
class RdbSmsMmsDFXAdapter {
public:
    void ReportDataBaseError(uint32_t scene, const std::string& uri, uint32_t duration);
    void ReportDataBaseUpdate(const std::string& updateRoute, uint32_t result, const std::string& failReason);
    int32_t GetExeTimeout();
private:
    const std::string PNAMEID = "com.ohos.telephonydataability";
    const std::string PVERSIONID = "1.0.32";
    const int32_t EXECUTES_TIMEOUT = 500;
    static constexpr char MMS_UE[] = "MMS_UE";
};
} // Telephony
} // OHOS
#endif // DATA_STORAGE_RDB_SMS_MMS_DFX_ADAPTER_H
