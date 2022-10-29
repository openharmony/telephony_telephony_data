/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "rdb_opkey_helper.h"

#include "opkey_data.h"
#include "rdb_errno.h"
#include "rdb_opkey_callback.h"
#include "rdb_store_config.h"
#include "vector"

namespace OHOS {
namespace Telephony {
const int VERSION = 1;
RdbOpKeyHelper::RdbOpKeyHelper()
{
}

int RdbOpKeyHelper::Init()
{
    int errCode = NativeRdb::E_OK;
    NativeRdb::RdbStoreConfig config(dbPath_);
    config.SetJournalMode(NativeRdb::JournalMode::MODE_TRUNCATE);
    std::string opKeyTableStr;
    CreateOpKeyInfoTableStr(opKeyTableStr);
    std::string opKeyIndexStr;
    CreateOpKeyInfoIndexStr(opKeyIndexStr);
    std::vector<std::string> createTableVec;
    createTableVec.push_back(opKeyTableStr);
    createTableVec.push_back(opKeyIndexStr);
    RdbOpKeyCallback callback(createTableVec);
    CreateRdbStore(config, VERSION, callback, errCode);
    return errCode;
}

void RdbOpKeyHelper::UpdateDbPath(const std::string &path)
{
    dbPath_ = path + DB_NAME;
}

void RdbOpKeyHelper::CreateOpKeyInfoTableStr(std::string &createTableStr)
{
    createTableStr.append("CREATE TABLE IF NOT EXISTS ").append(TABLE_OPKEY_INFO).append("(");
    createTableStr.append(OpKeyData::ID).append(" INTEGER PRIMARY KEY AUTOINCREMENT, ");
    createTableStr.append(OpKeyData::MCCMNC).append(" TEXT NOT NULL , ");
    createTableStr.append(OpKeyData::GID1).append(" TEXT , ");
    createTableStr.append(OpKeyData::GID2).append(" TEXT , ");
    createTableStr.append(OpKeyData::IMSI).append(" TEXT , ");
    createTableStr.append(OpKeyData::SPN).append(" TEXT , ");
    createTableStr.append(OpKeyData::ICCID).append(" TEXT , ");
    createTableStr.append(OpKeyData::OPERATOR_NAME).append(" TEXT , ");
    createTableStr.append(OpKeyData::OPERATOR_KEY).append(" TEXT DEFAULT '-1', ");
    createTableStr.append(OpKeyData::OPERATOR_KEY_EXT).append(" TEXT DEFAULT '', ");
    createTableStr.append(OpKeyData::RULE_ID).append(" INTEGER DEFAULT 0, ");
    createTableStr.append("UNIQUE (").append(OpKeyData::MCCMNC).append(", ");
    createTableStr.append(OpKeyData::GID1).append(", ");
    createTableStr.append(OpKeyData::GID2).append(", ");
    createTableStr.append(OpKeyData::IMSI).append(", ");
    createTableStr.append(OpKeyData::SPN).append(", ");
    createTableStr.append(OpKeyData::ICCID).append("))");
}

void RdbOpKeyHelper::CreateOpKeyInfoIndexStr(std::string &createIndexStr)
{
    createIndexStr.append("CREATE INDEX IF NOT EXISTS [").append(MCCMNC_INDEX).append("]");
    createIndexStr.append("ON [").append(TABLE_OPKEY_INFO).append("]");
    createIndexStr.append("([").append(OpKeyData::MCCMNC).append("])");
}

int RdbOpKeyHelper::EndTransactionAction()
{
    MarkAsCommit();
    return EndTransaction();
}

int RdbOpKeyHelper::CommitTransactionAction()
{
    int result = Commit();
    if (result != NativeRdb::E_OK) {
        RollBack();
    }
    return result;
}
} // namespace Telephony
} // namespace OHOS
