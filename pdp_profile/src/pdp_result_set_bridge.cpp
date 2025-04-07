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

#include "pdp_result_set_bridge.h"
#include "data_storage_log_wrapper.h"
#include "rdb_errno.h"
#include "rdb_store_config.h"
#include "rdb_utils.h"

namespace OHOS {
namespace Telephony {
using namespace DataShare;
using namespace DistributedKv;

PdpResultSetBridge::PdpResultSetBridge(const NativeDataSet &dataSet) : data_(dataSet.records)
{
    columnsNames_ = dataSet.columnNames;
    count_ = static_cast<uint32_t>(dataSet.records.size());
}

int PdpResultSetBridge::GetRowCount(int32_t &count)
{
    count = static_cast<int32_t>(this->data_.size());
    return NativeRdb::E_OK;
}

inline int WriteString(ResultSetBridge::Writer &writer, uint32_t columnIndex, const string &value)
{
    DistributedKv::Value insertValue(value);
    auto converted = insertValue.ToString();
    auto chars = converted.c_str();
    auto size = insertValue.Size() + 1;
    auto errCode = writer.Write(columnIndex, chars, size);
    if (errCode != 0) {
        DATA_STORAGE_LOGE("WriteString error:%{public}d", errCode);
        return errCode;
    }
    return errCode;
}

inline int WriteOptionalString(ResultSetBridge::Writer &writer, uint32_t columnIndex, const std::optional<string> &opt)
{
    return opt.has_value() ? WriteString(writer, columnIndex, opt.value()) : ERR_OK;
}

inline int WriteInt64(ResultSetBridge::Writer &writer, uint32_t columnIndex, int64_t value)
{
    auto errCode = writer.Write(columnIndex, value);
    if (errCode != 0) {
        DATA_STORAGE_LOGE("WriteInt64 error:%{public}d", errCode);
        return errCode;
    }
    return errCode;
}

inline int WriteDouble64(ResultSetBridge::Writer &writer, uint32_t columnIndex, double_t value)
{
    auto errCode = writer.Write(columnIndex, value);
    if (errCode != 0) {
        DATA_STORAGE_LOGE("WriteDouble64 error:%{public}d", errCode);
        return errCode;
    }
    return errCode;
}

inline int WriteBlob(ResultSetBridge::Writer &writer, uint32_t columnIndex, const vector<uint8_t> &value)
{
    auto errCode = writer.Write(columnIndex, value.data(), value.size());
    if (errCode != 0) {
        DATA_STORAGE_LOGE("WriteBlob error:%{public}d", errCode);
        return errCode;
    }
    return errCode;
}

int PdpResultSetBridge::GetAllColumnNames(vector<string> &columnNames)
{
   columnNames = columnsNames_;
   return 0;
}

int PdpResultSetBridge::OnGo(int32_t startRowIndex, int32_t targetRowIndex, DataShare::ResultSetBridge::Writer &writer)
{
    DATA_STORAGE_LOGE("PdpProfileAbility::OnGo startRowIndex = %{public}d targetRowIndex = %{public}d",
        startRowIndex, targetRowIndex);
    if ((startRowIndex < 0) || (targetRowIndex < 0) || (startRowIndex > targetRowIndex) ||
        (targetRowIndex >= static_cast<int32_t>(count_))) {
        // When failed to move, we should return -1.
        return -1;
    }
    // When called DataShareResultSet.GoToRow(index),
    // DataShareResultSet will try to call `OnGo(index, rowCount - 1, writer)`.
    // If we decide not to fill all the data at one `move`,
    // `OnGo` should return the final row index of actual filled blocks.
    // Here we fill all the data in range [index, target].
    return FillBlocks(startRowIndex, targetRowIndex, writer);
}

int PdpResultSetBridge::FillBlocks(int32_t startRowIndex, int32_t targetRowIndex, ResultSetBridge::Writer &writer)
{
    int allocatedRows = 0;
    for (int32_t rowIndex = startRowIndex; rowIndex <= targetRowIndex; ++rowIndex) {
        auto errCode = writer.AllocRow();
        if (errCode != 0) {
            DATA_STORAGE_LOGE("PdpProfileAbility::FillBlock. ERROR 22");
            return rowIndex - 1;
        }
        ++allocatedRows;
        NativeRecord markInfo = data_[rowIndex];
        uint32_t markInfoSize = static_cast<uint32_t>(markInfo.size());
        uint32_t columnsNameSize = static_cast<uint32_t>(columnsNames_.size());
        if (markInfoSize != columnsNameSize) {
            DATA_STORAGE_LOGE("columnsNameSize:%{public}d, markInfoSize:%{public}d", columnsNameSize, markInfoSize);
            return rowIndex - 1;
        }
        for (uint32_t i = 0; i < columnsNameSize; i++) {
            auto errCode = 0;
            const NativeData *cell = &markInfo[i];
            if (auto ptr = std::get_if<int64_t>(cell); ptr != nullptr) {
                errCode = WriteInt64(writer, i, *ptr);
            } else if (auto ptr = std::get_if<double_t>(cell); ptr != nullptr) {
                errCode = WriteDouble64(writer, i, *ptr);
            } else if (auto ptr = std::get_if<string>(cell); ptr != nullptr) {
                errCode = WriteString(writer, i, *ptr);
            } else if (auto ptr = std::get_if<vector<uint8_t>>(cell); ptr != nullptr) {
                errCode = WriteBlob(writer, i, *ptr);
            } else {
                errCode = -1;
            }
            if (errCode != 0) {
                DATA_STORAGE_LOGE("FillBlocks failed errCode = %{public}d", errCode);
                return rowIndex;
            }
        }
    }
    DATA_STORAGE_LOGI("FillBlocks %{public}d rows", allocatedRows);
    return targetRowIndex;
}
} // namespace NativeRdb
} // namespace OHOS