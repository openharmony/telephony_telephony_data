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

#ifndef DATA_STORAGE_PDP_PROFILE_DATA_H
#define DATA_STORAGE_PDP_PROFILE_DATA_H

namespace OHOS {
namespace Telephony {
class PdpProfileData {
public:
    /**
     * @brief Operator id
     */
    static constexpr const char *PROFILE_ID = "profile_id";
    /**
     * @brief Operator name
     */
    static constexpr const char *PROFILE_NAME = "profile_name";
    /**
     * @brief Operator key
     */
    static constexpr const char *OPKEY = "opkey";
    /**
     * @brief Mobile country code
     */
    static constexpr const char *MCC = "mcc";
    /**
     * @brief Mobile network code
     */
    static constexpr const char *MNC = "mnc";
    /**
     * @brief Mobile country code and mobile network code
     */
    static constexpr const char *MCCMNC = "mccmnc";
    /**
     * @brief APN name
     */
    static constexpr const char *APN = "apn";
    /**
     * @brief Authentication type
     */
    static constexpr const char *AUTH_TYPE = "auth_type";
    /**
     * @brief Authentication user
     */
    static constexpr const char *AUTH_USER = "auth_user";
    /**
     * @brief Authentication password
     */
    static constexpr const char *AUTH_PWD = "auth_pwd";
    /**
     * @brief APN type
     */
    static constexpr const char *APN_TYPES = "apn_types";
    /**
     * @brief Roaming or not
     */
    static constexpr const char *IS_ROAMING_APN = "is_roaming_apn";
    /**
     * @brief Operator enable or not
     */
    static constexpr const char *PROFILE_ENABLE = "profile_enable";
    /**
     * @brief Mms center url
     */
    static constexpr const char *HOME_URL = "home_url";
    /**
     * @brief Mms proxy ip address and port
     */
    static constexpr const char *PROXY_IP_ADDRESS = "proxy_ip_address";
    /**
     * @brief Mms ip address and port
     */
    static constexpr const char *MMS_IP_ADDRESS = "mms_ip_address";
    /**
     * @brief Protocol to connect to the APN
     */
    static constexpr const char *APN_PROTOCOL = "apn_protocol";
    /**
     * @brief Protocol to connect to the APN when roaming
     */
    static constexpr const char *APN_ROAM_PROTOCOL = "apn_roam_protocol";
    /**
     * @brief Radio access family bitmask
     */
    static constexpr const char *BEARING_SYSTEM_TYPE = "bearing_system_type";
    /**
     * @brief MVNO match type
     */
    static constexpr const char *MVNO_TYPE = "mvno_type";
    /**
     * @brief MVNO match data
     */
    static constexpr const char *MVNO_MATCH_DATA = "mvno_match_data";
    /**
     * @brief APN edit status
     */
    static constexpr const char *EDITED_STATUS = "edited";
    /**
     * @brief Server address
     */
    static constexpr const char *SERVER = "server";
    /**
     * @brief SIM Id of the SIM card
     */
    static constexpr const char *SIM_ID = "sim_id";
};

struct PdpProfile {
    int profileId = 0;
    std::string profileName = "";
    std::string mcc = "";
    std::string mnc = "";
    std::string apn = "";
    int authType = 0;
    std::string authUser = "";
    std::string authPwd = "";
    std::string apnTypes = ""; // see ApnType
    int isRoamingApn = 1;
    std::string homeUrl = "";
    std::string proxyIpAddress = "";
    std::string mmsIpAddress = "";
    std::string pdpProtocol = ""; // see PdpProtocol
    std::string roamPdpProtocol = "";
    int bearingSystemType = 0; // see BearingSystemType
    std::string mvnoType = "";
    std::string mvnoMatchData = "";
    int edited = 0;
    std::string server = "";
};

class MvnoType {
public:
    inline static const std::string SPN = "spn";
    inline static const std::string IMSI = "imsi";
    inline static const std::string GID1 = "gid1";
    inline static const std::string ICCID = "iccid";
};

enum class ApnType {
    DEFAULT, IMS, MMS, ALL
};

enum class ApnAuthType {
    None = 0, PAP, CHAP, PAP_OR_CHAP
};

enum class PdpProtocol {
    IPV4 = 0, IPV6, IPV4V6
};

enum class BearingSystemType {
    UNKNOWN = 0,
    LTE,
    HSPAP,
    HSPA,
    HSUPA,
    HSDPA,
    UMTS,
    EDGE,
    GPRS,
    eHRPD,
    EVDO_B,
    EVDO_A,
    EVDO_0,
    xRTT,
    IS95B,
    IS95AS
};

inline static const std::string PREFER_APN_ID = "preferapn_id";
inline static const std::string APN_CONF_CHECKSUM = "apn_conf_checksum";
constexpr const int32_t DEFAULT_SIM_ID = 0;
constexpr const int32_t INVALID_PROFILE_ID = -1;
constexpr const char *INVALID_OPKEY = "-1";
constexpr const char *TABLE_PDP_PROFILE = "pdp_profile";
constexpr const char *TEMP_TABLE_PDP_PROFILE = "temp_pdp_profile";
constexpr const char *PDP_PROFILE_URI = "datashare:///com.ohos.pdpprofileability";
} // namespace Telephony
} // namespace OHOS
#endif // DATA_STORAGE_PDP_PROFILE_DATA_H
