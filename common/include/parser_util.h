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

#include <iostream>
#include <vector>
#include <json/json.h>

#include "rdb_store.h"

#include "pdp_profile_data.h"
#include "data_storage_errors.h"

namespace OHOS {
namespace Telephony {
class ParserUtil {
public:
    int ParserPdpProfileJson(std::vector<PdpProfile> &vec);
    void ParserPdpProfileInfos(std::vector<PdpProfile> &vec, Json::Value &root);
    void ParserPdpProfileToValuesBucket(NativeRdb::ValuesBucket &value, const PdpProfile &bean);

private:
    int LoaderJsonFile(char *&content) const;
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
};
} // namespace Telephony
} // namespace OHOS
#endif // DATA_STORAGE_PDP_OPERATOR_PARSER_UTIL_H
