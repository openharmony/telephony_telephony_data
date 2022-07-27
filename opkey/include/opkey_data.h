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

#ifndef DATA_STORAGE_OPKEY_DATA_H
#define DATA_STORAGE_OPKEY_DATA_H

namespace OHOS {
namespace Telephony {
class OpKeyData {
public:
    static constexpr const char *ID = "id";
    static constexpr const char *MCCMNC = "mccmnc";
    static constexpr const char *GID1 = "gid1";
    static constexpr const char *GID2 = "gid2";
    static constexpr const char *IMSI = "imsi";
    static constexpr const char *SPN = "spn";
    static constexpr const char *ICCID = "iccid";
    static constexpr const char *OPERATOR_NAME = "operator_name";
    static constexpr const char *OPERATOR_KEY = "operator_key";
    static constexpr const char *OPERATOR_KEY_EXT = "operator_key_ext";
    static constexpr const char *RULE_ID = "rule_id";
};

struct OpKey {
    int opkeyId;
    std::string mccmnc;
    std::string gid1;
    std::string gid2;
    std::string imsi;
    std::string spn;
    std::string iccid;
    std::string operatorName;
    std::string operatorKey;
    std::string operatorKeyExt;
    int ruleId;
};

constexpr const char *MCCMNC_INDEX = "mccmncIndex";
constexpr const char *TABLE_OPKEY_INFO = "opkey_info";
constexpr const char *OPKEY_URI = "dataability:///com.ohos.opkeyability";
} // namespace Telephony
} // namespace OHOS
#endif // DATA_STORAGE_OPKEY_DATA_H