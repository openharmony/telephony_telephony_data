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

#ifndef DATA_STORAGE_PDP_OPERATOR_PARSER_UTIL_H
#define DATA_STORAGE_PDP_OPERATOR_PARSER_UTIL_H

#include <json/json.h>

#include "iosfwd"
#include "rdb_store.h"
#include "string"

namespace Json {
class Value;
}

namespace OHOS {
namespace NativeRdb {
class ValuesBucket;
}
namespace Telephony {
struct OpKey;
struct PdpProfile;
class ParserUtil {
public:
    int ParserPdpProfileJson(std::vector<PdpProfile> &vec);
    void ParserPdpProfileInfos(std::vector<PdpProfile> &vec, Json::Value &root);
    void ParserPdpProfileToValuesBucket(NativeRdb::ValuesBucket &value, const PdpProfile &bean);
    bool ParseFromCustomSystem(std::vector<OpKey> &vec);
    int ParserOpKeyJson(std::vector<OpKey> &vec, const char *path);
    void ParserOpKeyInfos(std::vector<OpKey> &vec, Json::Value &root);
    void ParserOpKeyToValuesBucket(NativeRdb::ValuesBucket &value, const OpKey &bean);

public:
    enum class RuleID {
        RULE_EMPTY = 0x0,
        RULE_MCCMNC = 0x1,
        RULE_ICCID = 0x2,
        RULE_IMSI = 0x4,
        RULE_SPN = 0x8,
        RULE_GID1 = 0x10,
        RULE_GID2 = 0x20,
    };

private:
    int LoaderJsonFile(char *&content, const char *path) const;
    int CloseFile(FILE *f) const;

private:
    const char *PATH = "/system/etc/telephony/pdp_profile.json";
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
    const std::string APN_VERSION = "apn_version";
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
    const int MAX_BYTE_LEN = 10 * 1024 * 1024;
};
} // namespace Telephony
} // namespace OHOS
#endif // DATA_STORAGE_PDP_OPERATOR_PARSER_UTIL_H
