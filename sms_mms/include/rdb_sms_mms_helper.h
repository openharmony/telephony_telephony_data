/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef DATA_STORAGE_RDB_SMS_MMS_HELPER_H
#define DATA_STORAGE_RDB_SMS_MMS_HELPER_H

#include <stdint.h>

#include "iosfwd"
#include "memory"
#include "datashare_values_bucket.h"
#include "rdb_base_helper.h"
#include "string"
#include "vector"

namespace OHOS {
namespace NativeRdb {
class ResultSet;
class ValuesBucket;
} // namespace NativeRdb
namespace Telephony {
class RdbSmsMmsHelper : public RdbBaseHelper {
public:
    RdbSmsMmsHelper();
    ~RdbSmsMmsHelper() = default;

    /**
     * Delete sms_mms_info table data of the previous 30 days
     *
     * @return 0 is succeed else failed
     */
    int32_t DeleteDataByThirty();

    /**
     * Insert sms_mms_info table data
     *
     * @param id id
     * @param value Table field data
     * @return 0 is succeed else failed
     */
    int32_t InsertSmsMmsInfo(int64_t &id, const NativeRdb::ValuesBucket &value);

    /**
     * Batch Insert sms_mms_info table data
     *
     * @param id id
     * @param values Table field data
     * @return 0 is succeed else failed
     */
    int32_t BatchInsertSmsMmsInfo(int64_t &id, const std::vector<DataShare::DataShareValuesBucket> &values);

    /**
     * Commit the transaction action
     *
     * @return 0 is succeed else failed
     */
    int CommitTransactionAction();

    /**
     * Statistics sms_mms_info table unread quantity
     *
     * @return unique_ptr<NativeRdb::ResultSet>
     */
    std::shared_ptr<NativeRdb::ResultSet> StatisticsUnRead();

    /**
     * Query sms_mms_info table max group_id
     *
     * @return  unique_ptr<NativeRdb::ResultSet>
     */
    std::shared_ptr<NativeRdb::ResultSet> QueryMaxGroupId();

    /**
     * Update dataBase path
     *
     * @param path path
     */
    void UpdateDbPath(const std::string &path);

    /**
     * Init dataBase
     *
     * @return 0 is succeed else failed
     */
    int Init();

private:
    /**
     * Create SmsMmsInfo table
     *
     * @param createTableStr Create table statement
     */
    void CreateSmsMmsInfoTableStr(std::string &createTableStr);

    /**
     * Create SmsSubsection table
     *
     * @param createTableStr Create table statement
     */
    void CreateSmsSubsectionTableStr(std::string &createTableStr);

    /**
     * Create MmsProtocol table
     *
     * @param createTableStr Create table statement
     */
    void CreateMmsProtocolTableStr(std::string &createTableStr);

    /**
     * Create MmsPart table
     *
     * @param createTableStr Create table statement
     */
    void CreateMmsPartTableStr(std::string &createTableStr);

    /**
     * Create Session table
     *
     * @param createTableStr Create table statement
     */
    void CreateSessionTableStr(std::string &createTableStr);

    /**
     * Create MmsPdu table
     * @param createTableStr indicates Create table statement
     */
    void CreateMmsPduTableStr(std::string &createTableStr);

private:
    const std::string DB_NAME = "sms_mms.db";
    std::string dbPath_ = FOLDER_PATH + DB_NAME;
    const int VERSION = 2;
};
} // namespace Telephony
} // namespace OHOS
#endif // DATA_STORAGE_RDB_SMS_MMS_HELPER_H
