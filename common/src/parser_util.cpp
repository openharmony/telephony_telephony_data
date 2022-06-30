/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "parser_util.h"

#include <climits>
#include <cstdio>
#include <cstring>

#include "data_storage_log_wrapper.h"
#include "preferences_util.h"

namespace OHOS {
namespace Telephony {
int ParserUtil::ParserPdpProfileJson(std::vector<PdpProfile> &vec)
{
    char *content = nullptr;
    int ret = LoaderJsonFile(content, PATH);
    if (ret != DATA_STORAGE_SUCCESS) {
        DATA_STORAGE_LOGE("ParserUtil::ParserPdpProfileJson LoaderJsonFile is fail!\n");
        return ret;
    }
    const int contentLength = strlen(content);
    const std::string rawJson(content);
    free(content);
    JSONCPP_STRING err;
    Json::Value root;
    Json::CharReaderBuilder builder;
    Json::CharReader* reader(builder.newCharReader());
    if (!reader->parse(rawJson.c_str(), rawJson.c_str() + contentLength, &root, &err)) {
        DATA_STORAGE_LOGE("ParserUtil::ParserPdpProfileJson reader is error!\n");
        return static_cast<int>(LoadProFileErrorType::FILE_PARSER_ERROR);
    }
    delete reader;
    Json::Value itemRoots = root[ITEM_OPERATOR_INFOS];
    if (itemRoots.size() == 0) {
        DATA_STORAGE_LOGE("ParserUtil::ParserPdpProfileJson itemRoots size == 0!\n");
        return static_cast<int>(LoadProFileErrorType::ITEM_SIZE_IS_NULL);
    }
    ParserPdpProfileInfos(vec, itemRoots);
    return DATA_STORAGE_SUCCESS;
}

void ParserUtil::ParserPdpProfileInfos(std::vector<PdpProfile> &vec, Json::Value &root)
{
    for (int32_t i = 0; i < static_cast<int32_t>(root.size()); i++) {
        Json::Value itemRoot = root[i];
        PdpProfile bean;
        bean.profileName = itemRoot[ITEM_OPERATOR_NAME].asString();
        bean.authUser = itemRoot[ITEM_AUTH_USER].asString();
        bean.authPwd = itemRoot[ITEM_AUTH_PWD].asString();
        std::string authTypeStr = itemRoot[ITEM_AUTH_TYPE].asString();
        if (authTypeStr.empty()) {
            bean.authType = 0;
        } else {
            bean.authType = atoi(authTypeStr.c_str());
        }
        bean.mcc = itemRoot[ITEM_MCC].asString();
        bean.mnc = itemRoot[ITEM_MNC].asString();
        bean.apn = itemRoot[ITEM_APN].asString();
        bean.apnTypes = itemRoot[ITEM_APN_TYPES].asString();
        bean.mmsIpAddress = itemRoot[ITEM_MMS_IP_ADDRESS].asString();
        bean.proxyIpAddress =  itemRoot[ITEM_IP_ADDRESS].asString();
        bean.homeUrl =  itemRoot[ITEM_HOME_URL].asString();
        vec.push_back(bean);
    }
}

void ParserUtil::ParserPdpProfileToValuesBucket(NativeRdb::ValuesBucket &value, const PdpProfile &bean)
{
    value.PutString(PdpProfileData::PROFILE_NAME, bean.profileName);
    value.PutString(PdpProfileData::MCC, bean.mcc);
    value.PutString(PdpProfileData::MNC, bean.mnc);
    std::string mccmnc(bean.mcc);
    mccmnc.append(bean.mnc);
    value.PutString(PdpProfileData::MCCMNC, mccmnc);
    value.PutString(PdpProfileData::APN, bean.apn);
    value.PutInt(PdpProfileData::AUTH_TYPE, bean.authType);
    value.PutString(PdpProfileData::AUTH_USER, bean.authUser);
    value.PutString(PdpProfileData::AUTH_PWD, bean.authPwd);
    value.PutString(PdpProfileData::APN_TYPES, bean.apnTypes);
    value.PutBool(PdpProfileData::IS_ROAMING_APN, bean.isRoamingApn);
    value.PutString(PdpProfileData::HOME_URL, bean.homeUrl);
    value.PutString(PdpProfileData::PROXY_IP_ADDRESS, bean.proxyIpAddress);
    value.PutString(PdpProfileData::MMS_IP_ADDRESS, bean.mmsIpAddress);
    value.PutString(PdpProfileData::APN_PROTOCOL, bean.pdpProtocol);
    value.PutString(PdpProfileData::APN_ROAM_PROTOCOL, bean.roamPdpProtocol);
}

int ParserUtil::ParserOpKeyJson(std::vector<OpKey> &vec)
{
    char *content = nullptr;
    int ret = LoaderJsonFile(content, OPKEY_INFO_PATH);
    if (ret != DATA_STORAGE_SUCCESS) {
        DATA_STORAGE_LOGE("ParserUtil::ParserOpKeyJson LoaderJsonFile is fail!\n");
        return ret;
    }
    if (content == nullptr) {
        DATA_STORAGE_LOGE("ParserUtil::content is nullptr!\n");
        return static_cast<int>(LoadProFileErrorType::FILE_PARSER_ERROR);
    }
    const int contentLength = strlen(content);
    const std::string rawJson(content);
    delete content;
    JSONCPP_STRING err;
    Json::Value root;
    Json::CharReaderBuilder builder;
    Json::CharReader* reader(builder.newCharReader());
    if (!reader->parse(rawJson.c_str(), rawJson.c_str() + contentLength, &root, &err)) {
        DATA_STORAGE_LOGE("ParserUtil::ParserOpKeyInfos reader is error!\n");
        return static_cast<int>(LoadProFileErrorType::FILE_PARSER_ERROR);
    }
    delete reader;
    int version = -1;
    std::string versionStr = root[OPKEY_VERSION].asString();
    if (versionStr.empty()) {
        version = -1;
    } else {
        version = atoi(versionStr.c_str());
    }
    int oldversion = PreferencesUtil::GetInstance()->ObtainInt(SAVE_OPKEY_VERSION, DEFAULT_OPKEY_VERSION);
    DATA_STORAGE_LOGI("ParserUtil::ParserOpKeyJson version =  %{public}d oldversion = %{public}d", version, oldversion);
    if (version == oldversion) {
        DATA_STORAGE_LOGE("ParserUtil::ParserOpKeyJson version no change");
        return VERSION_NO_CHANGE;
    }
    PreferencesUtil::GetInstance()->SaveInt(SAVE_OPKEY_VERSION, version);
    Json::Value itemRoots = root[ITEM_OPERATOR_ID];
    if (itemRoots.size() == 0) {
        DATA_STORAGE_LOGE("ParserUtil::ParserOpKeyInfos itemRoots size == 0!\n");
        return static_cast<int>(LoadProFileErrorType::ITEM_SIZE_IS_NULL);
    }
    ParserOpKeyInfos(vec, itemRoots);
    return DATA_STORAGE_SUCCESS;
}

void ParserUtil::ParserOpKeyInfos(std::vector<OpKey> &vec, Json::Value &root)
{
    for (int i = 0; i < (int)root.size(); i++) {
        Json::Value itemRoot = root[i];
        OpKey bean;
        Json::Value ruleRoot = itemRoot[ITEM_RULE];
        bean.mccmnc = ruleRoot[ITEM_MCCMNC].asString();
        bean.gid1 = ruleRoot[ITEM_GID_ONE].asString();
        bean.gid2 = ruleRoot[ITEM_GID_TWO].asString();
        bean.imsi = ruleRoot[ITEM_IMSI].asString();
        bean.spn = ruleRoot[ITEM_SPN].asString();
        bean.iccid = ruleRoot[ITEM_ICCID].asString();
        bean.operatorName = itemRoot[ITEM_OPERATOR_NAME_OPKEY].asString();
        std::string operatorKeyStr = itemRoot[ITEM_OPERATOR_KEY].asString();
        if (operatorKeyStr.empty()) {
            bean.operatorKey = DEFAULT_OPKEY_VERSION;
        } else {
            bean.operatorKey = atoi(operatorKeyStr.c_str());
        }
        bean.operatorKeyExt = itemRoot[ITEM_OPERATOR_KEY_EXT].asString();
        int ruleId = static_cast<int32_t>(RuleID::RULE_EMPTY);
        if (!bean.mccmnc.empty()) {
            ruleId += static_cast<int32_t>(RuleID::RULE_MCCMNC);
        }
        if (!bean.iccid.empty()) {
            ruleId += static_cast<int32_t>(RuleID::RULE_ICCID);
        }
        if (!bean.imsi.empty()) {
            ruleId += static_cast<int32_t>(RuleID::RULE_IMSI);
        }
        if (!bean.spn.empty()) {
            ruleId += static_cast<int32_t>(RuleID::RULE_SPN);
        }
        if (!bean.gid1.empty()) {
            ruleId += static_cast<int32_t>(RuleID::RULE_GID1);
        }
        if (!bean.gid2.empty()) {
            ruleId += static_cast<int32_t>(RuleID::RULE_GID2);
        }
        bean.ruleId = ruleId;
        vec.push_back(bean);
    }
}

void ParserUtil::ParserOpKeyToValuesBucket(NativeRdb::ValuesBucket &value, const OpKey &bean)
{
    value.PutString(OpKeyData::MCCMNC, bean.mccmnc);
    value.PutString(OpKeyData::GID1, bean.gid1);
    value.PutString(OpKeyData::GID2, bean.gid2);
    value.PutString(OpKeyData::IMSI, bean.imsi);
    value.PutString(OpKeyData::SPN, bean.spn);
    value.PutString(OpKeyData::ICCID, bean.iccid);
    value.PutString(OpKeyData::OPERATOR_NAME, bean.operatorName);
    value.PutInt(OpKeyData::OPERATOR_KEY, bean.operatorKey);
    value.PutString(OpKeyData::OPERATOR_KEY_EXT, bean.operatorKeyExt);
    value.PutInt(OpKeyData::RULE_ID, bean.ruleId);
}

int ParserUtil::LoaderJsonFile(char *&content, const char *path) const
{
    size_t len = 0;
    char realPath[PATH_MAX] = {0x00};
    if (realpath(path, realPath) == nullptr) {
        DATA_STORAGE_LOGE("ParserUtil::LoaderJsonFile realpath fail! #PATH: %{public}s", path);
        return static_cast<int>(LoadProFileErrorType::REALPATH_FAIL);
    }
    FILE *f = fopen(realPath, "rb");
    if (f == nullptr) {
        DATA_STORAGE_LOGE("ParserUtil::LoaderJsonFile file is null!");
        return static_cast<int>(LoadProFileErrorType::OPEN_FILE_ERROR);
    }
    int ret_seek_end = fseek(f, 0, SEEK_END);
    if (ret_seek_end != 0) {
        DATA_STORAGE_LOGE("ParserUtil::LoaderJsonFile ret_seek_end != 0!");
        CloseFile(f);
        return static_cast<int>(LoadProFileErrorType::LOAD_FILE_ERROR);
    }
    len = static_cast<size_t>(ftell(f));
    int ret_seek_set = fseek(f, 0, SEEK_SET);
    if (ret_seek_set != 0) {
        DATA_STORAGE_LOGE("ParserUtil::LoaderJsonFile ret_seek_set != 0!");
        CloseFile(f);
        return static_cast<int>(LoadProFileErrorType::LOAD_FILE_ERROR);
    }
    if (len == 0 || len > ULONG_MAX) {
        DATA_STORAGE_LOGE("ParserUtil::LoaderJsonFile len <= 0 or len > LONG_MAX!");
        CloseFile(f);
        return static_cast<int>(LoadProFileErrorType::LOAD_FILE_ERROR);
    }
    content = static_cast<char *>(malloc(len));
    if (content == nullptr) {
        DATA_STORAGE_LOGE("ParserUtil::LoaderJsonFile malloc content fail!");
        CloseFile(f);
        return static_cast<int>(LoadProFileErrorType::LOAD_FILE_ERROR);
    }
    size_t ret_read = fread(content, 1, len, f);
    if (ret_read != len) {
        DATA_STORAGE_LOGE("ParserUtil::LoaderJsonFile ret_read != len!");
        CloseFile(f);
        free(content);
        return static_cast<int>(LoadProFileErrorType::LOAD_FILE_ERROR);
    }
    return CloseFile(f);
}

int ParserUtil::CloseFile(FILE *f) const
{
    int ret_close = fclose(f);
    if (ret_close != 0) {
        DATA_STORAGE_LOGE("ParserUtil::LoaderJsonFile ret_close != 0!");
        return static_cast<int>(LoadProFileErrorType::CLOSE_FILE_ERROR);
    }
    return DATA_STORAGE_SUCCESS;
}
} // namespace Telephony
} // namespace OHOS