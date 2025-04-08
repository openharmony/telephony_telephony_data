/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef PDP_RESULT_SET_BRIDGE_H
#define PDP_RESULT_SET_BRIDGE_H

#include <condition_variable>
#include <mutex>
#include <functional>
#include <cstddef>
#include <cstdint>

#include "kvstore_result_set.h"
#include "result_set_bridge.h"
#include "pdp_profile_data.h"

namespace OHOS {
namespace Telephony {
using NativeData = std::variant<std::monostate, int64_t, double_t, std::string, std::vector<uint8_t>>;
using NativeRecord = std::vector<NativeData>;
struct NativeDataSet {
    std::vector<std::string> columnNames;
    std::vector<NativeRecord> records;
};

class PdpResultSetBridge : public DataShare::ResultSetBridge {
public:
    using WriteFunc = std::function<int(Writer &, const NativeRecord &record, uint32_t)>;
    struct ColumnDefinition {
        std::string name;
        WriteFunc write;
    };
    PdpResultSetBridge(const NativeDataSet &dataSet);
    ~PdpResultSetBridge() = default;
    int GetRowCount(int32_t &count) override;
    int GetAllColumnNames(std::vector<std::string> &columnNames) override;
    int OnGo(int32_t startRowIndex, int32_t targetRowIndex, DataShare::ResultSetBridge::Writer &writer) override;
    int FillBlocks(int32_t startRowIndex, int32_t targetRowIndex, DataShare::ResultSetBridge::Writer &Writer);

private:
    std::vector<NativeRecord> data_;
    uint32_t count_ = 0;
    std::vector<std::string> columnsNames_;
};
} // namespace Telephony
} // namespace OHOS
#endif // PDP_RESULT_SET_BRIDGE_H