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

#include "cJSON.h"
#include "iosfwd"
#include "rdb_store.h"
#include "string"

namespace OHOS {
namespace NativeRdb {
class ValuesBucket;
}
namespace Telephony {
struct OpKey;
struct PdpProfile;
struct NumMatch;
struct EccNum;
const std::string ECC_DATA_HASH = "ecc_data_hash";
const std::string NUM_MATCH_HASH = "num_match_hash";
class ParserUtil {
public:
    int GetPdpProfilePath(int slotId, std::string &path);
    int GetFileChecksum(const char *path, std::string &checkSum);
    int ParserPdpProfileJson(std::vector<PdpProfile> &vec);
    int ParserPdpProfileJson(std::vector<PdpProfile> &vec, const char *filePath);
    void ParserPdpProfileInfos(std::vector<PdpProfile> &vec, cJSON *itemRoots);
    void ParserPdpProfileToValuesBucket(NativeRdb::ValuesBucket &value, const PdpProfile &bean);
    int GetOpKeyFilePath(std::string &path);
    int ParserOpKeyJson(std::vector<OpKey> &vec, const char *path);
    void ParserOpKeyInfos(std::vector<OpKey> &vec, cJSON *itemRoots);
    void ParserOpKeyToValuesBucket(NativeRdb::ValuesBucket &value, const OpKey &bean);
    int ParserNumMatchJson(std::vector<NumMatch> &vec, const bool hashCheck);
    void ParserNumMatchInfos(std::vector<NumMatch> &vec, cJSON *itemRoots);
    void ParserNumMatchToValuesBucket(NativeRdb::ValuesBucket &value, const NumMatch &bean);
    int ParserEccDataJson(std::vector<EccNum> &vec, const bool hashCheck);
    void ParserEccDataInfos(std::vector<EccNum> &vec, cJSON *itemRoots);
    void ParserEccDataToValuesBucket(NativeRdb::ValuesBucket &value, const EccNum &bean);
    void RefreshDigest(const std::string &key);
    void ClearTempDigest(const std::string &key);

public:
    inline static const int MODE_SLOT_0 = 11;
    inline static const int MODE_SLOT_1 = 12;
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
    int32_t ParseInt(const cJSON *value);
    std::string ParseString(const cJSON *value);
    std::string ParseAsString(const cJSON *value);
    int LoaderJsonFile(char *&content, const char *path) const;
    int CloseFile(FILE *f) const;
    int GetRuleId(OpKey &bean);
    std::string GetCustFile(const char *&file, const char *key);
    bool IsNeedInsertToTable(cJSON *value);
    bool IsDigestChanged(const char *path, const std::string &key);
};
} // namespace Telephony
} // namespace OHOS
#endif // DATA_STORAGE_PDP_OPERATOR_PARSER_UTIL_H
