/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "rdb_global_ecc_helper.h"

#include "data_storage_errors.h"
#include "data_storage_log_wrapper.h"
#include "parser_util.h"
#include "global_ecc_data.h"
#include "rdb_errno.h"
#include "rdb_global_ecc_callback.h"
#include "rdb_store_config.h"
#include "values_bucket.h"
#include "vector"

namespace OHOS {
namespace Telephony {
RdbGlobalEccHelper::RdbGlobalEccHelper() {}

int RdbGlobalEccHelper::Init()
{
    int errCode = NativeRdb::E_OK;
    NativeRdb::RdbStoreConfig config(dbPath_);
    config.SetJournalMode(NativeRdb::JournalMode::MODE_TRUNCATE);
    std::string globalEccStr;
    CreateGlobalEccTableStr(globalEccStr, TABLE_GLOBAL_ECC);
    std::vector<std::string> createTableVec;
    createTableVec.push_back(globalEccStr);
    RdbGlobalEccCallback callback(createTableVec);
    CreateRdbStore(config, VERSION, callback, errCode);
    return errCode;
}

void RdbGlobalEccHelper::UpdateDbPath(const std::string &path)
{
    dbPath_ = path + DB_NAME;
}

void RdbGlobalEccHelper::CreateGlobalEccTableStr(std::string &createTableStr, const std::string &tableName)
{
    createTableStr.append("CREATE TABLE IF NOT EXISTS ").append(tableName).append("(");
    createTableStr.append(GlobalEccData::ID).append(" INTEGER PRIMARY KEY AUTOINCREMENT, ");
    createTableStr.append(GlobalEccData::NAME).append(" TEXT DEFAULT '', ");
    createTableStr.append(GlobalEccData::MCC).append(" TEXT DEFAULT '', ");
    createTableStr.append(GlobalEccData::MNC).append(" TEXT DEFAULT '', ");
    createTableStr.append(GlobalEccData::NUMERIC).append(" TEXT DEFAULT '', ");
    createTableStr.append(GlobalEccData::ECC_WITH_CARD).append(" TEXT DEFAULT '', ");
    createTableStr.append(GlobalEccData::ECC_NO_CARD).append(" TEXT DEFAULT '', ");
    createTableStr.append(GlobalEccData::ECC_FAKE).append(" TEXT DEFAULT '', ");
    createTableStr.append("UNIQUE (").append(GlobalEccData::NUMERIC).append("))");
}
} // namespace Telephony
} // namespace OHOS
