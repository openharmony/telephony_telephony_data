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
#include "preferences_util.h"
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
    createTableStr.append(PdpProfileData::OPKEY).append(" TEXT DEFAULT '', ");
    createTableStr.append(PdpProfileData::MCC).append(" TEXT DEFAULT '', ");
    createTableStr.append(PdpProfileData::MNC).append(" TEXT DEFAULT '', ");
    createTableStr.append(PdpProfileData::MCCMNC).append(" TEXT DEFAULT '', ");
    createTableStr.append(PdpProfileData::APN).append(" TEXT DEFAULT '', ");
    createTableStr.append(PdpProfileData::AUTH_TYPE).append(" INTEGER, ");
    createTableStr.append(PdpProfileData::AUTH_USER).append(" TEXT DEFAULT '', ");
    createTableStr.append(PdpProfileData::AUTH_PWD).append(" TEXT DEFAULT '', ");
    createTableStr.append(PdpProfileData::APN_TYPES).append(" TEXT DEFAULT '', ");
    createTableStr.append(PdpProfileData::IS_ROAMING_APN).append(" INTEGER DEFAULT 1, ");
    createTableStr.append(PdpProfileData::APN_PROTOCOL).append(" TEXT DEFAULT '', ");
    createTableStr.append(PdpProfileData::APN_ROAM_PROTOCOL).append(" TEXT DEFAULT '', ");
    createTableStr.append(PdpProfileData::HOME_URL).append(" TEXT DEFAULT '', ");
    createTableStr.append(PdpProfileData::MMS_IP_ADDRESS).append(" TEXT DEFAULT '', ");
    createTableStr.append(PdpProfileData::PROXY_IP_ADDRESS).append(" TEXT DEFAULT '', ");
    createTableStr.append(PdpProfileData::BEARING_SYSTEM_TYPE).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(PdpProfileData::MVNO_TYPE).append(" TEXT DEFAULT '', ");
    createTableStr.append(PdpProfileData::MVNO_MATCH_DATA).append(" TEXT DEFAULT '', ");
    createTableStr.append(PdpProfileData::EDITED_STATUS).append(" INTEGER DEFAULT 0, ");
    createTableStr.append(PdpProfileData::SERVER).append(" TEXT DEFAULT '', ");
    createTableStr.append("UNIQUE (").append(PdpProfileData::MCC).append(", ");
    createTableStr.append(PdpProfileData::MNC).append(", ");
    createTableStr.append(PdpProfileData::MVNO_TYPE).append(", ");
    createTableStr.append(PdpProfileData::MVNO_MATCH_DATA).append(", ");
    createTableStr.append(PdpProfileData::APN).append(", ");
    createTableStr.append(PdpProfileData::APN_TYPES).append(", ");
    createTableStr.append(PdpProfileData::IS_ROAMING_APN).append(", ");
    createTableStr.append(PdpProfileData::APN_PROTOCOL).append(", ");
    createTableStr.append(PdpProfileData::APN_ROAM_PROTOCOL).append(", ");
    createTableStr.append(PdpProfileData::HOME_URL).append(", ");
    createTableStr.append(PdpProfileData::MMS_IP_ADDRESS).append(", ");
    createTableStr.append(PdpProfileData::PROFILE_NAME).append(", ");
    createTableStr.append(PdpProfileData::BEARING_SYSTEM_TYPE).append(", ");
    createTableStr.append(PdpProfileData::AUTH_USER).append(", ");
    createTableStr.append(PdpProfileData::AUTH_PWD).append(", ");
    createTableStr.append(PdpProfileData::EDITED_STATUS).append(", ");
    createTableStr.append(PdpProfileData::PROXY_IP_ADDRESS).append("))");
}

int RdbPdpProfileHelper::initAPNDatabase(const std::string &opKey)
{
    if (store_ == nullptr) {
        return NativeRdb::E_ERROR;
    }
    DATA_STORAGE_LOGD("initAPNDatabase start");
    ParserUtil util;
    std::vector<PdpProfile> vec;
    int resultCode = util.ParserPdpProfile(vec, opKey.c_str());
    if (resultCode != DATA_STORAGE_SUCCESS) {
        DATA_STORAGE_LOGE("initAPNDatabase fail");
        return DATA_STORAGE_ERROR;
    }
    int32_t result = store_->BeginTransaction();
    if (result != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("BeginTransaction error!");
        return DATA_STORAGE_ERROR;
    }
    DATA_STORAGE_LOGD("initAPNDatabase size = %{public}zu", vec.size());

    for (size_t i = 0; i < vec.size(); i++) {
        NativeRdb::ValuesBucket value;
        util.ParserPdpProfileToValuesBucket(value, vec[i]);
        value.PutString(PdpProfileData::OPKEY, opKey);
        int64_t id;
        result = store_->InsertWithConflictResolution(
            id, TABLE_PDP_PROFILE, value, NativeRdb::ConflictResolution::ON_CONFLICT_REPLACE);
        if (result != DATA_STORAGE_SUCCESS) {
            DATA_STORAGE_LOGI("initAPNDatabase Conflict.");
            continue;
        }
    }
    store_->Commit();
    DATA_STORAGE_LOGD("initAPNDatabase end");
    return NativeRdb::E_OK;
}
} // namespace Telephony
} // namespace OHOS
