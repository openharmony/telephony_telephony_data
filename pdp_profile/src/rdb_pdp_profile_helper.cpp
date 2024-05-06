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
    createTableStr.append(PdpProfileData::OPKEY).append(" TEXT DEFAULT '', ");
    createTableStr.append("UNIQUE (").append(PdpProfileData::MCC).append(", ");
    createTableStr.append(PdpProfileData::MNC).append(", ");
    createTableStr.append(PdpProfileData::OPKEY).append(", ");
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

int RdbPdpProfileHelper::ResetApn()
{
    auto preferencesUtil = DelayedSingleton<PreferencesUtil>::GetInstance();
    if (preferencesUtil != nullptr) {
        preferencesUtil->DeleteProfiles();
    }
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

int RdbPdpProfileHelper::CommitTransactionAction()
{
    int result = Commit();
    if (result != NativeRdb::E_OK) {
        RollBack();
    }
    return result;
}

int RdbPdpProfileHelper::InitAPNDatabase(int slotId, const std::string &opKey, bool isNeedCheckFile)
{
    if (store_ == nullptr || opKey.empty() || strcmp(opKey.c_str(), INVALID_OPKEY) == 0) {
        return NativeRdb::E_ERROR;
    }
    DATA_STORAGE_LOGD("InitAPNDatabase start");
    ParserUtil util;
    std::string path;
    util.GetPdpProfilePath(slotId, path);
    if (path.empty()) {
        return NativeRdb::E_ERROR;
    }
    std::string checksum;
    util.GetFileChecksum(path.c_str(), checksum);
    if (checksum.empty()) {
        DATA_STORAGE_LOGE("InitAPNDatabase fail! checksum is null!");
        return NativeRdb::E_ERROR;
    }
    if (isNeedCheckFile && !IsApnDbUpdateNeeded(opKey, checksum)) {
        DATA_STORAGE_LOGI("The file is not changed and does not need to be loaded again.");
        return DATA_STORAGE_SUCCESS;
    }
    std::vector<PdpProfile> vec;
    int resultCode = util.ParserPdpProfileJson(vec, path.c_str());
    if (resultCode != DATA_STORAGE_SUCCESS) {
        DATA_STORAGE_LOGE("InitAPNDatabase fail");
        return DATA_STORAGE_ERROR;
    }
    int32_t result = store_->BeginTransaction();
    if (result != NativeRdb::E_OK) {
        DATA_STORAGE_LOGE("BeginTransaction error!");
        return DATA_STORAGE_ERROR;
    }
    DATA_STORAGE_LOGD("InitAPNDatabase size = %{public}zu", vec.size());
    for (size_t i = 0; i < vec.size(); i++) {
        NativeRdb::ValuesBucket value;
        util.ParserPdpProfileToValuesBucket(value, vec[i]);
        value.PutString(PdpProfileData::OPKEY, opKey);
        int64_t id;
        store_->InsertWithConflictResolution(
            id, TABLE_PDP_PROFILE, value, NativeRdb::ConflictResolution::ON_CONFLICT_REPLACE);
    }
    result = CommitTransactionAction();
    if (result == NativeRdb::E_OK) {
        SetPreferApnConfChecksum(opKey, checksum);
    }
    DATA_STORAGE_LOGD("InitAPNDatabase end");
    return result;
}

bool RdbPdpProfileHelper::IsApnDbUpdateNeeded(const std::string &opkey, std::string &checkSum)
{
    auto preferencesUtil = DelayedSingleton<PreferencesUtil>::GetInstance();
    if (preferencesUtil != nullptr) {
        std::string lastCheckSum = preferencesUtil->ObtainString(APN_CONF_CHECKSUM + opkey, "");
        if (checkSum.compare(lastCheckSum) == 0) {
            return false;
        }
    }
    return true;
}

int RdbPdpProfileHelper::SetPreferApnConfChecksum(const std::string &opkey, std::string &checkSum)
{
    auto preferencesUtil = DelayedSingleton<PreferencesUtil>::GetInstance();
    if (preferencesUtil == nullptr) {
        DATA_STORAGE_LOGE("preferencesUtil is nullptr!");
        return NativePreferences::E_ERROR;
    }
    return preferencesUtil->SaveString(APN_CONF_CHECKSUM + opkey, checkSum);
}
} // namespace Telephony
} // namespace OHOS
