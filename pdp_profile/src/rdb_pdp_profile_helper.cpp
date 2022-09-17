/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#include "rdb_pdp_profile_helper.h"

#include "data_storage_errors.h"
#include "data_storage_log_wrapper.h"
#include "parser_util.h"
#include "pdp_profile_data.h"
#include "rdb_errno.h"
#include "rdb_pdp_profile_callback.h"
#include "rdb_store_config.h"
#include "values_bucket.h"
#include "vector"

namespace OHOS {
namespace Telephony {
RdbPdpProfileHelper::RdbPdpProfileHelper() {}

int RdbPdpProfileHelper::Init()
{
    int errCode = NativeRdb::E_OK;
    NativeRdb::RdbStoreConfig config(dbPath_);
    config.SetJournalMode(NativeRdb::JournalMode::MODE_TRUNCATE);
    std::string pdpProfileStr;
    CreatePdpProfileTableStr(pdpProfileStr, TABLE_PDP_PROFILE);
    std::vector<std::string> createTableVec;
    createTableVec.push_back(pdpProfileStr);
    RdbPdpProfileCallback callback(createTableVec);
    CreateRdbStore(config, VERSION, callback, errCode);
    return errCode;
}

void RdbPdpProfileHelper::UpdateDbPath(const std::string &path)
{
    dbPath_ = path + DB_NAME;
}

void RdbPdpProfileHelper::CreatePdpProfileTableStr(std::string &createTableStr, const std::string &tableName)
{
    createTableStr.append("CREATE TABLE IF NOT EXISTS ").append(tableName).append("(");
    createTableStr.append(PdpProfileData::PROFILE_ID).append(" INTEGER PRIMARY KEY AUTOINCREMENT, ");
    createTableStr.append(PdpProfileData::PROFILE_NAME).append(" TEXT DEFAULT '', ");
    createTableStr.append(PdpProfileData::MCC).append(" TEXT DEFAULT '', ");
    createTableStr.append(PdpProfileData::MNC).append(" TEXT DEFAULT '', ");
    createTableStr.append(PdpProfileData::MCCMNC).append(" TEXT DEFAULT '', ");
    createTableStr.append(PdpProfileData::APN).append(" TEXT DEFAULT '', ");
    createTableStr.append(PdpProfileData::AUTH_TYPE).append(" INTEGER, ");
    createTableStr.append(PdpProfileData::AUTH_USER).append(" TEXT DEFAULT '', ");
    createTableStr.append(PdpProfileData::AUTH_PWD).append(" TEXT DEFAULT '', ");
    createTableStr.append(PdpProfileData::APN_TYPES).append(" TEXT DEFAULT '', ");
    createTableStr.append(PdpProfileData::IS_ROAMING_APN).append(" INTEGER, ");
    createTableStr.append(PdpProfileData::APN_PROTOCOL).append(" TEXT DEFAULT '', ");
    createTableStr.append(PdpProfileData::APN_ROAM_PROTOCOL).append(" TEXT DEFAULT '', ");
    createTableStr.append(PdpProfileData::HOME_URL).append(" TEXT DEFAULT '', ");
    createTableStr.append(PdpProfileData::MMS_IP_ADDRESS).append(" TEXT DEFAULT '', ");
    createTableStr.append(PdpProfileData::PROXY_IP_ADDRESS).append(" TEXT DEFAULT '', ");
    createTableStr.append(PdpProfileData::BEARING_SYSTEM_TYPE).append(" INTEGER, ");
    createTableStr.append("UNIQUE (").append(PdpProfileData::MCC).append(", ");
    createTableStr.append(PdpProfileData::MNC).append(", ");
    createTableStr.append(PdpProfileData::APN).append(", ");
    createTableStr.append(PdpProfileData::APN_TYPES).append(", ");
    createTableStr.append(PdpProfileData::IS_ROAMING_APN).append(", ");
    createTableStr.append(PdpProfileData::APN_PROTOCOL).append(", ");
    createTableStr.append(PdpProfileData::APN_ROAM_PROTOCOL).append(", ");
    createTableStr.append(PdpProfileData::HOME_URL).append(", ");
    createTableStr.append(PdpProfileData::MMS_IP_ADDRESS).append(", ");
    createTableStr.append(PdpProfileData::PROXY_IP_ADDRESS).append("))");
}

int RdbPdpProfileHelper::ResetApn()
{
    int ret = BeginTransaction();
    if (ret != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("RdbPdpProfileHelper::ResetApn BeginTransaction is error!");
        return ret;
    }
    std::string pdpProfileStr;
    CreatePdpProfileTableStr(pdpProfileStr, TEMP_TABLE_PDP_PROFILE);
    ret = ExecuteSql(pdpProfileStr);
    if (ret != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("RdbPdpProfileHelper::ResetApn create table temp_pdp_profile ret = %{public}d", ret);
        return ret;
    }
    DATA_STORAGE_LOGI("RdbPdpProfileHelper::ResetApn create table success");
    ParserUtil util;
    std::vector<PdpProfile> vec;
    ret = util.ParserPdpProfileJson(vec);
    if (ret != DATA_STORAGE_SUCCESS) {
        RollBack();
        return ret;
    }
    for (size_t i = 0; i < vec.size(); i++) {
        NativeRdb::ValuesBucket value;
        util.ParserPdpProfileToValuesBucket(value, vec[i]);
        int64_t id;
        Insert(id, value, TEMP_TABLE_PDP_PROFILE);
    }
    ret = ExecuteSql("drop table " + std::string(TABLE_PDP_PROFILE));
    if (ret != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("RdbPdpProfileHelper::ResetApn drop table ret = %{public}d", ret);
        RollBack();
        return ret;
    }
    DATA_STORAGE_LOGI("RdbPdpProfileHelper::ResetApn success");
    std::string sql;
    sql.append("alter table ").append(TEMP_TABLE_PDP_PROFILE).append(" rename to ").append(TABLE_PDP_PROFILE);
    ret = ExecuteSql(sql);
    if (ret != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("RdbPdpProfileHelper::ResetApn alter table ret = %{public}d", ret);
        RollBack();
        return ret;
    }
    DATA_STORAGE_LOGI("RdbPdpProfileHelper::ResetApn alter table success");
    ret = CommitTransactionAction();
    return ret;
}

int RdbPdpProfileHelper::EndTransactionAction()
{
    MarkAsCommit();
    return EndTransaction();
}

int RdbPdpProfileHelper::CommitTransactionAction()
{
    int result = Commit();
    if (result != NativeRdb::E_OK) {
        RollBack();
    }
    return result;
}
} // namespace Telephony
} // namespace OHOS
