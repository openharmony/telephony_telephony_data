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

#include <fstream>
#include <iostream>
#include <zlib.h>
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
#include "memory"
#include "new"
#include "opkey_data.h"
#include "parameters.h"
#include "pdp_profile_data.h"
#include "telephony_types.h"
#include "values_bucket.h"
#include "vector"
#include "preferences_util.h"

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
const std::string DEFAULT_PREFERENCES_STRING_VALUE = "default_value";
const std::string TEMP_SUFFIX = "_temp";

int ParserUtil::ParserPdpProfileJson(std::vector<PdpProfile> &vec)
{
    char buf[MAX_PATH_LEN];
    char *path = GetOneCfgFile(PATH, buf, MAX_PATH_LEN);
    return ParserPdpProfileJson(vec, path);
}

int ParserUtil::ParserPdpProfileJson(std::vector<PdpProfile> &vec, const char *path)
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

    cJSON *root = cJSON_Parse(content);
    free(content);
    content = nullptr;
    if (root == nullptr) {
        DATA_STORAGE_LOGE("ParserUtil::ParserPdpProfileJson root is error!");
        return static_cast<int>(LoadProFileErrorType::FILE_PARSER_ERROR);
    }

    cJSON *itemRoots = cJSON_GetObjectItem(root, ITEM_OPERATOR_INFOS);
    if (itemRoots == nullptr || !cJSON_IsArray(itemRoots) || cJSON_GetArraySize(itemRoots) <= 0) {
        DATA_STORAGE_LOGE("ParserUtil::ParserPdpProfileJson itemRoots size == 0!");
        cJSON_Delete(root);
        itemRoots = nullptr;
        root = nullptr;
        return static_cast<int>(LoadProFileErrorType::ITEM_SIZE_IS_NULL);
    }
    ParserPdpProfileInfos(vec, itemRoots);
    cJSON_Delete(root);
    itemRoots = nullptr;
    root = nullptr;
    return DATA_STORAGE_SUCCESS;
}

void ParserUtil::ParserPdpProfileInfos(std::vector<PdpProfile> &vec, cJSON *itemRoots)
{
    cJSON *itemRoot = nullptr;
    for (int32_t i = 0; i < cJSON_GetArraySize(itemRoots); i++) {
        itemRoot = cJSON_GetArrayItem(itemRoots, i);
        if (itemRoot == nullptr || !IsNeedInsertToTable(itemRoot)) {
            continue;
        }
        PdpProfile bean;
        bean.profileName = ParseString(cJSON_GetObjectItem(itemRoot, ITEM_OPERATOR_NAME));
        bean.authUser = ParseString(cJSON_GetObjectItem(itemRoot, ITEM_AUTH_USER));
        bean.authPwd = ParseString(cJSON_GetObjectItem(itemRoot, ITEM_AUTH_PWD));
        std::string authTypeStr = ParseString(cJSON_GetObjectItem(itemRoot, ITEM_AUTH_TYPE));
        bean.authType = authTypeStr.empty() ? 0 : atoi(authTypeStr.c_str());
        bean.mcc = ParseString(cJSON_GetObjectItem(itemRoot, ITEM_MCC));
        bean.mnc = ParseString(cJSON_GetObjectItem(itemRoot, ITEM_MNC));
        bean.apn = ParseString(cJSON_GetObjectItem(itemRoot, ITEM_APN));
        bean.apnTypes = ParseString(cJSON_GetObjectItem(itemRoot, ITEM_APN_TYPES));
        bean.mmsIpAddress = ParseString(cJSON_GetObjectItem(itemRoot, ITEM_MMS_IP_ADDRESS));
        bean.proxyIpAddress = ParseString(cJSON_GetObjectItem(itemRoot, ITEM_IP_ADDRESS));
        bean.homeUrl = ParseString(cJSON_GetObjectItem(itemRoot, ITEM_HOME_URL));
        bean.mvnoType = ParseString(cJSON_GetObjectItem(itemRoot, ITEM_MVNO_TYPE));
        bean.mvnoMatchData = ParseString(cJSON_GetObjectItem(itemRoot, ITEM_MVNO_MATCH_DATA));
        bean.server = ParseString(cJSON_GetObjectItem(itemRoot, ITEM_SERVER));
        std::string editedStr = ParseString(cJSON_GetObjectItem(itemRoot, ITEM_EDITED_STATUS));
        bean.edited = editedStr.empty() ? 0 : atoi(editedStr.c_str());
        std::string bearingStr = ParseString(cJSON_GetObjectItem(itemRoot, ITEM_BEARER));
        bean.bearingSystemType = bearingStr.empty() ? 0 : atoi(bearingStr.c_str());
        std::string isRoamingApnStr = ParseString(cJSON_GetObjectItem(itemRoot, ITEM_IS_ROAMING_APN));
        bean.isRoamingApn = isRoamingApnStr.empty() ? 0 : atoi(isRoamingApnStr.c_str());
        std::string pdpProtocolStr = ParseString(cJSON_GetObjectItem(itemRoot, ITEM_APN_PROTOCOL));
        bean.pdpProtocol = pdpProtocolStr.empty() ? "IP" : pdpProtocolStr;
        std::string roamPdpProtocolStr = ParseString(cJSON_GetObjectItem(itemRoot, ITEM_ROAMING_PROTOCOL));
        bean.roamPdpProtocol = roamPdpProtocolStr.empty() ? "IP" : roamPdpProtocolStr;
        vec.push_back(bean);
    }
    itemRoot = nullptr;
}

std::string ParserUtil::ParseString(const cJSON *value)
{
    if (value != nullptr && value->type == cJSON_String && value->valuestring != nullptr) {
        return value->valuestring;
    }
    return "";
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

int ParserUtil::GetOpKeyFilePath(std::string &path)
{
    char buf[MAX_PATH_LEN];
    std::string file = GetCustFile(OPKEY_INFO_PATH, CUST_RULE_PATH_KEY);
    char *ret = GetOneCfgFile(file.c_str(), buf, MAX_PATH_LEN);
    if (ret && *ret != '\0') {
        path = ret;
        return OPERATION_OK;
    }
    DATA_STORAGE_LOGE("ParserUtil::GetOpKeyFilePath fail");
    return OPERATION_ERROR;
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

    cJSON *root = cJSON_Parse(content);
    free(content);
    content = nullptr;
    if (root == nullptr) {
        DATA_STORAGE_LOGE("ParserUtil::ParserOpKeyInfos root is error!");
        return static_cast<int>(LoadProFileErrorType::FILE_PARSER_ERROR);
    }
    cJSON *itemRoots = cJSON_GetObjectItem(root, ITEM_OPERATOR_ID);
    if (itemRoots == nullptr || !cJSON_IsArray(itemRoots) || cJSON_GetArraySize(itemRoots) <= 0) {
        DATA_STORAGE_LOGE("ParserUtil::ParserOpKeyInfos itemRoots size == 0!");
        cJSON_Delete(root);
        itemRoots = nullptr;
        root = nullptr;
        return static_cast<int>(LoadProFileErrorType::ITEM_SIZE_IS_NULL);
    }
    ParserOpKeyInfos(vec, itemRoots);
    cJSON_Delete(root);
    itemRoots = nullptr;
    root = nullptr;
    return DATA_STORAGE_SUCCESS;
}

void ParserUtil::ParserOpKeyInfos(std::vector<OpKey> &vec, cJSON *itemRoots)
{
    cJSON *itemRoot = nullptr;
    cJSON *ruleRoot = nullptr;
    for (int i = 0; i < cJSON_GetArraySize(itemRoots); i++) {
        itemRoot = cJSON_GetArrayItem(itemRoots, i);
        if (itemRoot == nullptr) {
            continue;
        }
        OpKey bean;
        ruleRoot = cJSON_GetObjectItem(itemRoot, ITEM_RULE);
        if (ruleRoot != nullptr) {
            bean.mccmnc = ParseString(cJSON_GetObjectItem(ruleRoot, ITEM_MCCMNC));
            bean.gid1 = ParseString(cJSON_GetObjectItem(ruleRoot, ITEM_GID_ONE));
            bean.gid2 = ParseString(cJSON_GetObjectItem(ruleRoot, ITEM_GID_TWO));
            bean.imsi = ParseString(cJSON_GetObjectItem(ruleRoot, ITEM_IMSI));
            bean.spn = ParseString(cJSON_GetObjectItem(ruleRoot, ITEM_SPN));
            bean.iccid = ParseString(cJSON_GetObjectItem(ruleRoot, ITEM_ICCID));
        }
        bean.operatorName = ParseString(cJSON_GetObjectItem(itemRoot, ITEM_OPERATOR_NAME_OPKEY));
        if (bean.operatorName.empty()) {
            bean.operatorName = "COMMON";
        }
        bean.operatorKey = ParseString(cJSON_GetObjectItem(itemRoot, ITEM_OPERATOR_KEY));
        bean.operatorKeyExt = ParseString(cJSON_GetObjectItem(itemRoot, ITEM_OPERATOR_KEY_EXT));
        bean.ruleId = GetRuleId(bean);
        vec.push_back(bean);
    }
    itemRoot = nullptr;
    ruleRoot = nullptr;
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

int ParserUtil::ParserNumMatchJson(std::vector<NumMatch> &vec, const bool hashCheck)
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
    if (hashCheck && !IsDigestChanged(path, NUM_MATCH_HASH)) {
        free(content);
        return FILE_HASH_NO_CHANGE;
    }
    cJSON *root = cJSON_Parse(content);
    free(content);
    content = nullptr;
    if (root == nullptr) {
        DATA_STORAGE_LOGE("ParserUtil::ParserNumMatchJson root is error!\n");
        return static_cast<int>(LoadProFileErrorType::FILE_PARSER_ERROR);
    }
    cJSON *itemRoots = cJSON_GetObjectItem(root, ITEM_NUM_MATCH_INFOS);
    if (itemRoots == nullptr || !cJSON_IsArray(itemRoots) || cJSON_GetArraySize(itemRoots) <= 0) {
        DATA_STORAGE_LOGE("ParserUtil::ParserNumMatchJson itemRoots size == 0!\n");
        cJSON_Delete(root);
        itemRoots = nullptr;
        root = nullptr;
        return static_cast<int>(LoadProFileErrorType::ITEM_SIZE_IS_NULL);
    }
    ParserNumMatchInfos(vec, itemRoots);
    cJSON_Delete(root);
    itemRoots = nullptr;
    root = nullptr;
    return DATA_STORAGE_SUCCESS;
}

void ParserUtil::ParserNumMatchInfos(std::vector<NumMatch> &vec, cJSON *itemRoots)
{
    cJSON *itemRoot = nullptr;
    for (int32_t i = 0; i < cJSON_GetArraySize(itemRoots); i++) {
        itemRoot = cJSON_GetArrayItem(itemRoots, i);
        if (itemRoot == nullptr) {
            continue;
        }
        NumMatch bean;
        bean.name = ParseAsString(cJSON_GetObjectItem(itemRoot, ITEM_NAME));
        bean.mcc = ParseAsString(cJSON_GetObjectItem(itemRoot, ITEM_MCC));
        bean.mnc = ParseAsString(cJSON_GetObjectItem(itemRoot, ITEM_MNC));
        bean.numMatch = ParseInt(cJSON_GetObjectItem(itemRoot, ITEM_NUM_MATCH));
        bean.numMatchShort = ParseInt(cJSON_GetObjectItem(itemRoot, ITEM_NUM_MATCH_SHORT));
        vec.push_back(bean);
    }
    itemRoot = nullptr;
}

std::string ParserUtil::ParseAsString(const cJSON *value)
{
    if (value != nullptr && value->type == cJSON_String && value->valuestring != nullptr) {
        return value->valuestring;
    } else if (value != nullptr && value->type == cJSON_Number) {
        return std::to_string(static_cast<int64_t>(cJSON_GetNumberValue(value)));
    }
    return "";
}

int32_t ParserUtil::ParseInt(const cJSON *value)
{
    if (value != nullptr && value->type == cJSON_Number) {
        return value->valueint;
    }
    return 0;
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

int ParserUtil::ParserEccDataJson(std::vector<EccNum> &vec, const bool hashCheck)
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
    if (hashCheck && !IsDigestChanged(path, ECC_DATA_HASH)) {
        free(content);
        return FILE_HASH_NO_CHANGE;
    }
    cJSON *root = cJSON_Parse(content);
    free(content);
    content = nullptr;
    if (root == nullptr) {
        DATA_STORAGE_LOGE("ParserUtil::ParserEccDataJson root is error!");
        return static_cast<int>(LoadProFileErrorType::FILE_PARSER_ERROR);
    }

    cJSON *itemRoots = cJSON_GetObjectItem(root, ITEM_OPERATOR_INFOS);
    if (itemRoots == nullptr || !cJSON_IsArray(itemRoots) || cJSON_GetArraySize(itemRoots) <= 0) {
        DATA_STORAGE_LOGE("ParserUtil::ParserEccDataJson itemRoots size == 0!");
        cJSON_Delete(root);
        itemRoots = nullptr;
        root = nullptr;
        return static_cast<int>(LoadProFileErrorType::ITEM_SIZE_IS_NULL);
    }
    ParserEccDataInfos(vec, itemRoots);
    cJSON_Delete(root);
    itemRoots = nullptr;
    root = nullptr;
    return DATA_STORAGE_SUCCESS;
}

void ParserUtil::ParserEccDataInfos(std::vector<EccNum> &vec, cJSON *itemRoots)
{
    cJSON *itemRoot = nullptr;
    for (int i = 0; i < cJSON_GetArraySize(itemRoots); i++) {
        cJSON *itemRoot = cJSON_GetArrayItem(itemRoots, i);
        if (itemRoot == nullptr) {
            continue;
        }
        EccNum bean;
        bean.name = ParseAsString(cJSON_GetObjectItem(itemRoot, ITEM_NAME));
        bean.mcc = ParseAsString(cJSON_GetObjectItem(itemRoot, ITEM_MCC));
        bean.mnc = ParseAsString(cJSON_GetObjectItem(itemRoot, ITEM_MNC));
        bean.numeric = ParseAsString(cJSON_GetObjectItem(itemRoot, ITEM_NUMERIC));
        bean.ecc_withcard = ParseAsString(cJSON_GetObjectItem(itemRoot, ITEM_ECC_WITH_CARD));
        bean.ecc_nocard = ParseAsString(cJSON_GetObjectItem(itemRoot, ITEM_ECC_NO_CARD));
        bean.ecc_fake = ParseAsString(cJSON_GetObjectItem(itemRoot, ITEM_ECC_FAKE));
        vec.push_back(bean);
    }
    itemRoot = nullptr;
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

bool ParserUtil::IsNeedInsertToTable(cJSON *value)
{
    if (value == nullptr || cJSON_GetObjectItem(value, ITEM_APN) == nullptr) {
        return false;
    }
    char *tempChar = cJSON_PrintUnformatted(value);
    if (tempChar == nullptr) {
        return false;
    }
    std::string res = tempChar;
    free(tempChar);
    tempChar = nullptr;
    return DelayedRefSingleton<CoreServiceClient>::GetInstance().IsAllowedInsertApn(res);
}

bool ParserUtil::IsDigestChanged(const char *path, const std::string &key)
{
    std::string newHash;
    ParserUtil util;
    util.GetFileChecksum(path, newHash);
    auto preferencesUtil = DelayedSingleton<PreferencesUtil>::GetInstance();
    if (preferencesUtil == nullptr) {
        DATA_STORAGE_LOGE("ParserUtil::IsDigestChanged preferencesUtil is nullptr!");
        return true;
    }
    std::string oldHash = preferencesUtil->ObtainString(key, DEFAULT_PREFERENCES_STRING_VALUE);
    if (oldHash.compare(newHash) == 0) {
        DATA_STORAGE_LOGI("ParserUtil::IsDigestChanged file not changed");
        return false;
    }
    DATA_STORAGE_LOGI("ParserUtil::IsDigestChanged file is changed");
    preferencesUtil->SaveString(key + TEMP_SUFFIX, newHash);
    return true;
}

void ParserUtil::RefreshDigest(const std::string &key)
{
    auto preferencesUtil = DelayedSingleton<PreferencesUtil>::GetInstance();
    if (preferencesUtil == nullptr) {
        DATA_STORAGE_LOGE("ParserUtil::RefreshDigest preferencesUtil is nullptr!");
        return;
    }
    std::string tempHash = preferencesUtil->ObtainString(key + TEMP_SUFFIX, DEFAULT_PREFERENCES_STRING_VALUE);
    if (tempHash != DEFAULT_PREFERENCES_STRING_VALUE) {
        preferencesUtil->SaveString(key, tempHash);
        preferencesUtil->RemoveKey(key + TEMP_SUFFIX);
        preferencesUtil->Refresh();
    }
}

void ParserUtil::ClearTempDigest(const std::string &key)
{
    auto preferencesUtil = DelayedSingleton<PreferencesUtil>::GetInstance();
    if (preferencesUtil == nullptr) {
        DATA_STORAGE_LOGE("ParserUtil::ClearTempDigest preferencesUtil is nullptr!");
        return;
    }
    preferencesUtil->RemoveKey(key + TEMP_SUFFIX);
    preferencesUtil->Refresh();
}
} // namespace Telephony
} // namespace OHOS
