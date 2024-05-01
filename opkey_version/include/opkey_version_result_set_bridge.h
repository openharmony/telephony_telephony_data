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

#ifndef OPKEY_VERSION_RESULT_SET_BRIDGE_H
#define OPKEY_VERSION_RESULT_SET_BRIDGE_H

#include "result_set_bridge.h"

namespace OHOS {
namespace Telephony {
class OpkeyVersionResultSetBridge : public DataShare::ResultSetBridge {
public:
    OpkeyVersionResultSetBridge(const std::string &info);
    ~OpkeyVersionResultSetBridge() = default;
    int GetRowCount(int32_t &count) override;
    int GetAllColumnNames(std::vector<std::string> &columnNames) override;
    int OnGo(int32_t startRowIndex, int32_t targetRowIndex, DataShare::ResultSetBridge::Writer &writer) override;

private:
    const int32_t ROW_COUNT = 1;
    const int32_t KEY_COLUMN = 0;
    const int32_t VALUE_COLUMN = 1;
    const int32_t INVALID_POSITION = -1;
    const std::string INFO_KEY = "key";

    std::string opkeyInfo_;
    bool FillBlock(DataShare::ResultSetBridge::Writer &Writer);
};
} // Telephony
} // OHOS

#endif // OPKEY_VERSION_RESULT_SET_BRIDGE_H