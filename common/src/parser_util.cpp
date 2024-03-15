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

#include <cstdio>
#include <securec.h>
#include <unistd.h>

#include "climits"
#include "config_policy_utils.h"
#include "cstdint"
#include "cstdio"
#include "cstdlib"
#include "cstring"
#include "core_service_client.h"
#include "data_storage_errors.h"
#include "data_storage_log_wrapper.h"
#include "global_params_data.h"
#include "json/config.h"
#include "json/reader.h"
#include "json/value.h"
#include "memory"
#include "new"
#include "opkey_data.h"
#include "parameters.h"
#include "pdp_profile_data.h"
#include "telephony_types.h"
#include "values_bucket.h"
#include "vector"
#include <fstream>
#include <iostream>
#include <zlib.h>

namespace OHOS {
namespace Telephony {
const char *PATH = "/etc/telephony/pdp_profile.json";
const char *ITEM_OPERATOR_INFOS = "operator_infos";
const char *ITEM_OPERATOR_NAME = "operator_name";
const char *ITEM_AUTH_USER = "auth_user";
const char *ITEM_AUTH_PWD = "auth_pwd";
const char *ITEM_AUTH_TYPE = "auth_type";
const char *ITEM_MCC = "mcc";
const char *ITEM_MNC = "mnc";
const char *ITEM_APN = "apn";
const char *ITEM_APN_TYPES = "apn_types";
const char *ITEM_IP_ADDRESS = "ip_addr";
const char *ITEM_MMS_IP_ADDRESS = "mms_ip_addr";
const char *ITEM_HOME_URL = "home_url";
const char *ITEM_MVNO_TYPE = "mvno_type";
const char *ITEM_MVNO_MATCH_DATA = "mvno_match_data";
const char *ITEM_EDITED_STATUS = "edited";
const char *ITEM_SERVER = "server";
const char *ITEM_BEARER = "bearing_system_type";
const char *ITEM_IS_ROAMING_APN = "is_roaming_apn";
const char *ITEM_APN_PROTOCOL = "apn_protocol";
const char *ITEM_ROAMING_PROTOCOL = "apn_roam_protocol";
const char *APN_VERSION = "apn_version";
const char *OPKEY_INFO_PATH = "etc/telephony/OpkeyInfo.json";
const char *ITEM_OPERATOR_ID = "operator_id";
const char *ITEM_RULE = "rule";
const char *ITEM_MCCMNC = "mcc_mnc";
const char *ITEM_GID_ONE = "gid1";
const char *ITEM_GID_TWO = "gid2";
const char *ITEM_IMSI = "imsi";
const char *ITEM_SPN = "spn";
const char *ITEM_ICCID = "iccid";
const char *ITEM_OPERATOR_NAME_OPKEY = "operator_name";
const char *ITEM_OPERATOR_KEY = "operator_key";
const char *ITEM_OPERATOR_KEY_EXT = "operator_key_ext";
const char *NUM_MATCH_PATH = "etc/telephony/number_match.json";
const char *ECC_DATA_PATH = "etc/telephony/ecc_data.json";
const char *ITEM_NUM_MATCH_INFOS = "numMatchs";
const char *ITEM_NAME = "name";
const char *ITEM_NUM_MATCH = "num_match";
const char *ITEM_NUM_MATCH_SHORT = "num_match_short";
const char *ITEM_NUMERIC = "numeric";
const char *ITEM_ECC_WITH_CARD = "ecc_withcard";
const char *ITEM_ECC_NO_CARD = "ecc_nocard";
const char *ITEM_ECC_FAKE = "ecc_fake";
const int BYTE_LEN = 1024 * 1024;
const int MAX_BYTE_LEN = 10 * 1024 * 1024;
static constexpr const char *CUST_RULE_PATH_KEY = "const.telephony.rule_path";
static constexpr const char *CUST_NETWORK_PATH_KEY = "const.telephony.network_path";

int ParserUtil::ParserPdpProfileJson(std::vector<PdpProfile> &vec)
{
    char buf[MAX_PATH_LEN];
    char *path = GetOneCfgFile(PATH, buf, MAX_PATH_LEN);
    return ParserPdpProfileJson(vec, path);
}

int ParserUtil::ParserPdpProfileJson(std::vector<PdpProfile> &vec, char *path)
{
    char *content = nullptr;
    int ret = DATA_STORAGE_SUCCESS;
    if (path && *path != '\0') {
        ret = LoaderJsonFile(content, path);
    }
    if (ret != DATA_STORAGE_SUCCESS) {
        DATA_STORAGE_LOGE("ParserUtil::ParserPdpProfileJson LoaderJsonFile is fail!");
        return ret;
    }
    if (content == nullptr) {
        DATA_STORAGE_LOGE("ParserUtil::content is nullptr!");
        return static_cast<int>(LoadProFileErrorType::FILE_PARSER_ERROR);
    }
    const int contentLength = strlen(content);
    const std::string rawJson(content);
    free(content);
    content = nullptr;
    JSONCPP_STRING err;
    Json::Value root;
    Json::CharReaderBuilder builder;
    Json::CharReader *reader(builder.newCharReader());
    if (!reader->parse(rawJson.c_str(), rawJson.c_str() + contentLength, &root, &err)) {
        DATA_STORAGE_LOGE("ParserUtil::ParserPdpProfileJson reader is error!");
        delete reader;
        return static_cast<int>(LoadProFileErrorType::FILE_PARSER_ERROR);
    }
    delete reader;
    reader = nullptr;
    Json::Value itemRoots = root[ITEM_OPERATOR_INFOS];
    if (itemRoots.size() == 0) {
        DATA_STORAGE_LOGE("ParserUtil::ParserPdpProfileJson itemRoots size == 0!");
        return static_cast<int>(LoadProFileErrorType::ITEM_SIZE_IS_NULL);
    }
    ParserPdpProfileInfos(vec, itemRoots);
    return DATA_STORAGE_SUCCESS;
}

void ParserUtil::ParserPdpProfileInfos(std::vector<PdpProfile> &vec, Json::Value &root)
{
    for (int32_t i = 0; i < static_cast<int32_t>(root.size()); i++) {
        Json::Value itemRoot = root[i];
        if (!IsNeedInsertToTable(itemRoot)) {
            continue;
        }
        PdpProfile bean;
        bean.profileName = ParseString(itemRoot[ITEM_OPERATOR_NAME]);
        bean.authUser = ParseString(itemRoot[ITEM_AUTH_USER]);
        bean.authPwd = ParseString(itemRoot[ITEM_AUTH_PWD]);
        std::string authTypeStr = ParseString(itemRoot[ITEM_AUTH_TYPE]);
        bean.authType = authTypeStr.empty() ? 0 : atoi(authTypeStr.c_str());
        bean.mcc = ParseString(itemRoot[ITEM_MCC]);
        bean.mnc = ParseString(itemRoot[ITEM_MNC]);
        bean.apn = ParseString(itemRoot[ITEM_APN]);
        bean.apnTypes = ParseString(itemRoot[ITEM_APN_TYPES]);
        bean.mmsIpAddress = ParseString(itemRoot[ITEM_MMS_IP_ADDRESS]);
        bean.proxyIpAddress = ParseString(itemRoot[ITEM_IP_ADDRESS]);
        bean.homeUrl = ParseString(itemRoot[ITEM_HOME_URL]);
        bean.mvnoType = ParseString(itemRoot[ITEM_MVNO_TYPE]);
        bean.mvnoMatchData = ParseString(itemRoot[ITEM_MVNO_MATCH_DATA]);
        bean.server = ParseString(itemRoot[ITEM_SERVER]);
        std::string editedStr = ParseString(itemRoot[ITEM_EDITED_STATUS]);
        bean.edited = editedStr.empty() ? 0 : atoi(editedStr.c_str());
        std::string bearingStr = ParseString(itemRoot[ITEM_BEARER]);
        bean.bearingSystemType = bearingStr.empty() ? 0 : atoi(bearingStr.c_str());
        std::string isRoamingApnStr = ParseString(itemRoot[ITEM_IS_ROAMING_APN]);
        bean.isRoamingApn = isRoamingApnStr.empty() ? 0 : atoi(isRoamingApnStr.c_str());
        std::string pdpProtocolStr = ParseString(itemRoot[ITEM_APN_PROTOCOL]);
        bean.pdpProtocol = pdpProtocolStr.empty() ? "IP" : pdpProtocolStr;
        std::string roamPdpProtocolStr = ParseString(itemRoot[ITEM_ROAMING_PROTOCOL]);
        bean.roamPdpProtocol = roamPdpProtocolStr.empty() ? "IP" : roamPdpProtocolStr;
        vec.push_back(bean);
    }
}

std::string ParserUtil::ParseString(const Json::Value &value)
{
    return value.isString() ? value.asString() : "";
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
    value.PutString(PdpProfileData::MVNO_TYPE, bean.mvnoType);
    value.PutString(PdpProfileData::MVNO_MATCH_DATA, bean.mvnoMatchData);
    value.PutInt(PdpProfileData::EDITED_STATUS, bean.edited);
    value.PutString(PdpProfileData::SERVER, bean.server);
    value.PutInt(PdpProfileData::BEARING_SYSTEM_TYPE, bean.bearingSystemType);
}

bool ParserUtil::ParseFromCustomSystem(std::vector<OpKey> &vec)
{
    DATA_STORAGE_LOGI("ParserUtil ParseFromCustomSystem");
    std::string file = GetCustFile(OPKEY_INFO_PATH, CUST_RULE_PATH_KEY);
    CfgFiles *cfgFiles = GetCfgFiles(file.c_str());
    if (cfgFiles == nullptr) {
        DATA_STORAGE_LOGE("ParserUtil ParseFromCustomSystem cfgFiles is null");
        return false;
    }
    char *filePath = nullptr;
    int result = DATA_STORAGE_ERROR;
    for (int32_t i = MAX_CFG_POLICY_DIRS_CNT - 1; i >= 0; i--) {
        filePath = cfgFiles->paths[i];
        if (filePath && *filePath != '\0') {
            if (ParserOpKeyJson(vec, filePath) == DATA_STORAGE_SUCCESS) {
                result = DATA_STORAGE_SUCCESS;
            }
        }
    }
    FreeCfgFiles(cfgFiles);
    return result == DATA_STORAGE_SUCCESS;
}

int ParserUtil::ParserOpKeyJson(std::vector<OpKey> &vec, const char *path)
{
    char *content = nullptr;
    int ret = LoaderJsonFile(content, path);
    if (ret != DATA_STORAGE_SUCCESS) {
        DATA_STORAGE_LOGE("ParserUtil::ParserOpKeyJson LoaderJsonFile is fail!");
        return ret;
    }
    if (content == nullptr) {
        DATA_STORAGE_LOGE("ParserUtil::content is nullptr!");
        return static_cast<int>(LoadProFileErrorType::FILE_PARSER_ERROR);
    }
    const int contentLength = strlen(content);
    const std::string rawJson(content);
    free(content);
    content = nullptr;
    JSONCPP_STRING err;
    Json::Value root;
    Json::CharReaderBuilder builder;
    Json::CharReader *reader(builder.newCharReader());
    if (!reader->parse(rawJson.c_str(), rawJson.c_str() + contentLength, &root, &err)) {
        DATA_STORAGE_LOGE("ParserUtil::ParserOpKeyInfos reader is error!");
        delete reader;
        return static_cast<int>(LoadProFileErrorType::FILE_PARSER_ERROR);
    }
    delete reader;
    reader = nullptr;
    Json::Value itemRoots = root[ITEM_OPERATOR_ID];
    if (itemRoots.size() == 0) {
        DATA_STORAGE_LOGE("ParserUtil::ParserOpKeyInfos itemRoots size == 0!");
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
        if (ruleRoot[ITEM_MCCMNC].isString()) {
            bean.mccmnc = ruleRoot[ITEM_MCCMNC].asString();
        }
        if (ruleRoot[ITEM_GID_ONE].isString()) {
            bean.gid1 = ruleRoot[ITEM_GID_ONE].asString();
        }
        if (ruleRoot[ITEM_GID_TWO].isString()) {
            bean.gid2 = ruleRoot[ITEM_GID_TWO].asString();
        }
        if (ruleRoot[ITEM_IMSI].isString()) {
            bean.imsi = ruleRoot[ITEM_IMSI].asString();
        }
        if (ruleRoot[ITEM_SPN].isString()) {
            bean.spn = ruleRoot[ITEM_SPN].asString();
        }
        if (ruleRoot[ITEM_ICCID].isString()) {
            bean.iccid = ruleRoot[ITEM_ICCID].asString();
        }
        if (itemRoot[ITEM_OPERATOR_NAME_OPKEY].isString()) {
            bean.operatorName = itemRoot[ITEM_OPERATOR_NAME_OPKEY].asString();
        }
        if (itemRoot[ITEM_OPERATOR_KEY].isString()) {
            bean.operatorKey = itemRoot[ITEM_OPERATOR_KEY].asString();
        }
        if (itemRoot[ITEM_OPERATOR_KEY_EXT].isString()) {
            bean.operatorKeyExt = itemRoot[ITEM_OPERATOR_KEY_EXT].asString();
        }
        bean.ruleId = GetRuleId(bean);
        vec.push_back(bean);
    }
}

int ParserUtil::GetRuleId(OpKey &bean)
{
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
    return ruleId;
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
    value.PutString(OpKeyData::OPERATOR_KEY, bean.operatorKey);
    value.PutString(OpKeyData::OPERATOR_KEY_EXT, bean.operatorKeyExt);
    value.PutInt(OpKeyData::RULE_ID, bean.ruleId);
}

int ParserUtil::ParserNumMatchJson(std::vector<NumMatch> &vec)
{
    char *content = nullptr;
    char buf[MAX_PATH_LEN];
    std::string file = GetCustFile(NUM_MATCH_PATH, CUST_NETWORK_PATH_KEY);
    char *path = GetOneCfgFile(file.c_str(), buf, MAX_PATH_LEN);
    int ret = DATA_STORAGE_SUCCESS;
    if (path && *path != '\0') {
        ParserUtil parser;
        ret = parser.LoaderJsonFile(content, path);
    }
    if (ret != DATA_STORAGE_SUCCESS) {
        DATA_STORAGE_LOGE("ParserUtil::ParserNumMatchJson LoaderJsonFile is fail!\n");
        return ret;
    }
    if (content == nullptr) {
        DATA_STORAGE_LOGE("ParserUtil::content is nullptr!");
        return static_cast<int>(LoadProFileErrorType::FILE_PARSER_ERROR);
    }
    const int contentLength = strlen(content);
    const std::string rawJson(content);
    free(content);
    content = nullptr;
    JSONCPP_STRING err;
    Json::Value root;
    Json::CharReaderBuilder builder;
    Json::CharReader *reader(builder.newCharReader());
    if (!reader->parse(rawJson.c_str(), rawJson.c_str() +contentLength, &root, &err)) {
        DATA_STORAGE_LOGE("ParserUtil::ParserNumMatchJson reader is error!\n");
        return static_cast<int>(LoadProFileErrorType::FILE_PARSER_ERROR);
    }
    delete reader;
    reader = nullptr;
    Json::Value itemRoots = root[ITEM_NUM_MATCH_INFOS];
    if (itemRoots.size() == 0) {
        DATA_STORAGE_LOGE("ParserUtil::ParserNumMatchJson itemRoots size == 0!\n");
        return static_cast<int>(LoadProFileErrorType::ITEM_SIZE_IS_NULL);
    }
    ParserNumMatchInfos(vec, itemRoots);
    return DATA_STORAGE_SUCCESS;
}

void ParserUtil::ParserNumMatchInfos(std::vector<NumMatch> &vec, Json::Value &root)
{
    for (int32_t i = 0; i < static_cast<int32_t>(root.size()); i++) {
        Json::Value itemRoot = root[i];
        NumMatch bean;
        bean.name = itemRoot[ITEM_NAME].asString();
        bean.mcc = itemRoot[ITEM_MCC].asString();
        bean.mnc = itemRoot[ITEM_MNC].asString();
        bean.numMatch = itemRoot[ITEM_NUM_MATCH].asInt();
        bean.numMatchShort = itemRoot[ITEM_NUM_MATCH_SHORT].asInt();
        vec.push_back(bean);
    }
}

void ParserUtil::ParserNumMatchToValuesBucket(NativeRdb::ValuesBucket &value, const NumMatch &bean)
{
    value.PutString(NumMatchData::NAME, bean.name);
    value.PutString(NumMatchData::MCC, bean.mcc);
    value.PutString(NumMatchData::MNC, bean.mnc);
    std::string mccmnc(bean.mcc);
    mccmnc.append(bean.mnc);
    value.PutString(NumMatchData::MCCMNC, mccmnc);
    value.PutInt(NumMatchData::NUM_MATCH, bean.numMatch);
    value.PutInt(NumMatchData::NUM_MATCH_SHORT, bean.numMatchShort);
}

int ParserUtil::ParserEccDataJson(std::vector<EccNum> &vec)
{
    char *content = nullptr;
    char buf[MAX_PATH_LEN];
    std::string file = GetCustFile(ECC_DATA_PATH, CUST_NETWORK_PATH_KEY);
    char *path = GetOneCfgFile(file.c_str(), buf, MAX_PATH_LEN);
    int ret = DATA_STORAGE_SUCCESS;
    if (path && *path != '\0') {
        ret = LoaderJsonFile(content, path);
    }
    if (ret != DATA_STORAGE_SUCCESS) {
        DATA_STORAGE_LOGE("ParserUtil::ParserEccDataJson LoaderJsonFile is fail!");
        return ret;
    }
    if (content == nullptr) {
        DATA_STORAGE_LOGE("ParserUtil::content is nullptr!");
        return static_cast<int>(LoadProFileErrorType::FILE_PARSER_ERROR);
    }
    const int contentLength = strlen(content);
    const std::string rawJson(content);
    free(content);
    content = nullptr;
    JSONCPP_STRING err;
    Json::Value root;
    Json::CharReaderBuilder builder;
    Json::CharReader *reader(builder.newCharReader());
    if (!reader->parse(rawJson.c_str(), rawJson.c_str() + contentLength, &root, &err)) {
        DATA_STORAGE_LOGE("ParserUtil::ParserEccDataJson reader is error!");
        delete reader;
        return static_cast<int>(LoadProFileErrorType::FILE_PARSER_ERROR);
    }
    delete reader;
    reader = nullptr;
    Json::Value itemRoots = root[ITEM_OPERATOR_INFOS];
    if (itemRoots.size() == 0) {
        DATA_STORAGE_LOGE("ParserUtil::ParserEccDataJson itemRoots size == 0!");
        return static_cast<int>(LoadProFileErrorType::ITEM_SIZE_IS_NULL);
    }
    ParserEccDataInfos(vec, itemRoots);
    return DATA_STORAGE_SUCCESS;
}

void ParserUtil::ParserEccDataInfos(std::vector<EccNum> &vec, Json::Value &roots)
{
    for (int i = 0; i < static_cast<int>(roots.size()); i++) {
        Json::Value itemRoot = roots[i];
        EccNum bean;
        bean.name = itemRoot[ITEM_NAME].asString();
        bean.mcc = itemRoot[ITEM_MCC].asString();
        bean.mnc = itemRoot[ITEM_MNC].asString();
        bean.numeric = itemRoot[ITEM_NUMERIC].asString();
        bean.ecc_withcard = itemRoot[ITEM_ECC_WITH_CARD].asString();
        bean.ecc_nocard = itemRoot[ITEM_ECC_NO_CARD].asString();
        bean.ecc_fake = itemRoot[ITEM_ECC_FAKE].asString();
        vec.push_back(bean);
    }
}

void ParserUtil::ParserEccDataToValuesBucket(NativeRdb::ValuesBucket &value, const EccNum &bean)
{
    value.PutString(EccData::NAME, bean.name);
    value.PutString(EccData::MCC, bean.mcc);
    value.PutString(EccData::MNC, bean.mnc);
    value.PutString(EccData::NUMERIC, bean.numeric);
    value.PutString(EccData::ECC_WITH_CARD, bean.ecc_withcard);
    value.PutString(EccData::ECC_NO_CARD, bean.ecc_nocard);
    value.PutString(EccData::ECC_FAKE, bean.ecc_fake);
}

int ParserUtil::LoaderJsonFile(char *&content, const char *path) const
{
    long len = 0;
    char realPath[PATH_MAX] = { 0x00 };
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
    len = ftell(f);
    int ret_seek_set = fseek(f, 0, SEEK_SET);
    if (ret_seek_set != 0) {
        DATA_STORAGE_LOGE("ParserUtil::LoaderJsonFile ret_seek_set != 0!");
        CloseFile(f);
        return static_cast<int>(LoadProFileErrorType::LOAD_FILE_ERROR);
    }
    if (len == 0 || len > static_cast<long>(MAX_BYTE_LEN)) {
        DATA_STORAGE_LOGE("ParserUtil::LoaderJsonFile len <= 0 or len > LONG_MAX!");
        CloseFile(f);
        return static_cast<int>(LoadProFileErrorType::LOAD_FILE_ERROR);
    }
    content = static_cast<char *>(malloc(len + 1));
    if (content == nullptr) {
        DATA_STORAGE_LOGE("ParserUtil::LoaderJsonFile malloc content fail!");
        CloseFile(f);
        return static_cast<int>(LoadProFileErrorType::LOAD_FILE_ERROR);
    }
    if (memset_s(content, len + 1, 0, len + 1) != EOK) {
        DATA_STORAGE_LOGE("ParserUtil::LoaderJsonFile memset_s failed");
        free(content);
        content = nullptr;
        CloseFile(f);
        return static_cast<int>(LoadProFileErrorType::LOAD_FILE_ERROR);
    }
    size_t ret_read = fread(content, 1, len, f);
    if (ret_read != static_cast<size_t>(len)) {
        DATA_STORAGE_LOGE("ParserUtil::LoaderJsonFile ret_read != len!");
        free(content);
        content = nullptr;
        CloseFile(f);
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

std::string ParserUtil::GetCustFile(const char *&file, const char *key)
{
    std::string custFile = system::GetParameter(key, "");
    if (!custFile.empty()) {
        custFile.append(file);
    } else {
        custFile = file;
    }
    return custFile;
}

int ParserUtil::GetPdpProfilePath(int slotId, std::string &path)
{
    int mode = MODE_SLOT_0;
    if (slotId == SimSlotId::SIM_SLOT_1) {
        mode = MODE_SLOT_1;
    }
    char buf[MAX_PATH_LEN];
    char *ret = GetOneCfgFileEx(PATH, buf, MAX_PATH_LEN, mode, nullptr);
    if (ret && *ret != '\0') {
        path = ret;
        return OPERATION_OK;
    }
    DATA_STORAGE_LOGE("ParserUtil::GetPdpProfilePath fail");
    return OPERATION_ERROR;
}

int ParserUtil::GetFileChecksum(const char *path, std::string &checkSum)
{
    char realPath[PATH_MAX] = {0x00};
    if (realpath(path, realPath) == nullptr) {
        DATA_STORAGE_LOGE("ParserUtil::GetFileChecksum Failed to get realPath!");
        return OPERATION_ERROR;
    }
    std::ifstream file(realPath, std::ios::binary);
    if (!file.is_open()) {
        DATA_STORAGE_LOGE("ParserUtil::GetFileChecksum Failed to open file!");
        return OPERATION_ERROR;
    }
    std::vector<char> buffer(BYTE_LEN);
    uint32_t crc32 = crc32_z(0L, Z_NULL, 0);
    while (file) {
        file.read(buffer.data(), buffer.size());
        auto bytesRead = file.gcount();
        if (bytesRead > 0) {
            crc32 = crc32_z(crc32, reinterpret_cast<const Bytef *>(buffer.data()), static_cast<uInt>(bytesRead));
        }
    }
    checkSum = std::to_string(crc32);
    return OPERATION_OK;
}

bool ParserUtil::IsNeedInsertToTable(Json::Value &content)
{
    if (content.empty()) {
        return false;
    }
    Json::FastWriter fastWriter;
    const Json::String &string = fastWriter.write(content);
    std::string res(string.c_str());
    DATA_STORAGE_LOGI("ParserUtil::IsNeedInsertToTable res: %{public}s", res.c_str());
    return DelayedRefSingleton<CoreServiceClient>::GetInstance().IsAllowedInsertApn(res);
}
} // namespace Telephony
} // namespace OHOS
