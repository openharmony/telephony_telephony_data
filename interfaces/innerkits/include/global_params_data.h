/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef DATA_STORAGE_GLOBAL_PARAMS_DATA_H
#define DATA_STORAGE_GLOBAL_PARAMS_DATA_H

namespace OHOS {
namespace Telephony {
class EccData {
public:
    /**
     * @brief Id
     */
    static constexpr const char *ID = "id";
    /**
     * @brief Mobile operator name
     */
    static constexpr const char *NAME = "name";
    /**
     * @brief Country code
     */
    static constexpr const char *MCC = "mcc";
    /**
     * @brief Network code
     */
    static constexpr const char *MNC = "mnc";
    /**
     * @brief Mobile country code and network code
     */
    static constexpr const char *NUMERIC = "numeric";
    /**
     * @brief Mobile emergency number list with card
     */
    static constexpr const char *ECC_WITH_CARD = "ecc_withcard";
    /**
     * @brief Mobile emergency number list no card
     */
    static constexpr const char *ECC_NO_CARD = "ecc_nocard";
    /**
     * @brief Mobile emergency number list is fake
     */
    static constexpr const char *ECC_FAKE = "ecc_fake";
};

struct EccNum {
    int id = 0;
    std::string name = "";
    std::string mcc = "";
    std::string mnc = "";
    std::string numeric = "";
    std::string ecc_withcard = "";
    std::string ecc_nocard = "";
    std::string ecc_fake = "";
};

static constexpr const char *TABLE_ECC_DATA = "ecc_data";
static constexpr const char *GLOBAL_PARAMS_URI = "datashare:///com.ohos.globalparamsability";
} // namespace Telephony
} // namespace OHOS
#endif // DATA_STORAGE_GLOBAL_PARAMS_DATA_H
