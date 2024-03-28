/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef DATA_STORAGE_SIM_DATA_H
#define DATA_STORAGE_SIM_DATA_H

namespace OHOS {
namespace Telephony {
class SimData {
public:
    /**
     * @brief SIM Id of the SIM card
     */
    static constexpr const char *SIM_ID = "sim_id";
    /**
     * @brief ICCID of the SIM card
     */
    static constexpr const char *ICC_ID = "icc_id";
    /**
     * @brief ICCID of the SIM or eid of eSIM
     */
    static constexpr const char *CARD_ID = "card_id";
    /**
     * @brief Card slot index number
     */
    static constexpr const char *SLOT_INDEX = "slot_index";
    /**
     * @brief Show name of the SIM card
     */
    static constexpr const char *SHOW_NAME = "show_name";
    /**
     * @brief Phone number of the SIM card
     */
    static constexpr const char *PHONE_NUMBER = "phone_number";
    /**
     * @brief Country code of the SIM card
     */
    static constexpr const char *COUNTRY_CODE = "country_code";
    /**
     * @brief Language of the SIM card
     */
    static constexpr const char *LANGUAGE = "language";
    /**
     * @brief International mobile subscriber ID
     */
    static constexpr const char *IMSI = "imsi";
    /**
     * @brief The type of the SIM card
     */
    static constexpr const char *CARD_TYPE = "card_type";
    /**
     * @brief Wwitch of ims
     */
    static constexpr const char *IMS_SWITCH = "ims_switch";
    /**
     * @brief Active or not
     */
    static constexpr const char *IS_ACTIVE = "is_active";
    /**
     * @brief Main card or not
     */
    static constexpr const char *IS_MAIN_CARD = "is_main_card";
    /**
     * @brief Voice card or not
     */
    static constexpr const char *IS_VOICE_CARD = "is_voice_card";
    /**
     * @brief Message card or not
     */
    static constexpr const char *IS_MESSAGE_CARD = "is_message_card";
    /**
     * @brief Cellular data card or not
     */
    static constexpr const char *IS_CELLULAR_DATA_CARD = "is_cellular_data_card";
    /**
     * @brief The opkey of sim
     */
    static constexpr const char *OPKEY = "opkey";
    /**
     * @brief The mcc of sim
     */
    static constexpr const char *MCC = "mcc";
    /**
     * @brief The mnc of sim
     */
    static constexpr const char *MNC = "mnc";
};

constexpr const char *TABLE_SIM_INFO = "sim_info";
constexpr const char *SIM_URI = "datashare:///com.ohos.simability";
} // namespace Telephony
} // namespace OHOS
#endif // DATA_STORAGE_SIM_DATA_H