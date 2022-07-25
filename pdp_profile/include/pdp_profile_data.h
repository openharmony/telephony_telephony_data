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

#ifndef DATA_STORAGE_PDP_PROFILE_DATA_H
#define DATA_STORAGE_PDP_PROFILE_DATA_H

namespace OHOS {
namespace Telephony {
class PdpProfileData {
public:
    static constexpr const char *PROFILE_ID = "profile_id";
    static constexpr const char *PROFILE_NAME = "profile_name";
    static constexpr const char *MCC = "mcc";
    static constexpr const char *MNC = "mnc";
    static constexpr const char *MCCMNC = "mccmnc";
    static constexpr const char *APN = "apn";
    static constexpr const char *AUTH_TYPE = "auth_type";
    static constexpr const char *AUTH_USER = "auth_user";
    static constexpr const char *AUTH_PWD = "auth_pwd";
    static constexpr const char *APN_TYPES = "apn_types";
    static constexpr const char *IS_ROAMING_APN = "is_roaming_apn";
    static constexpr const char *PROFILE_ENABLE = "profile_enable";
    static constexpr const char *HOME_URL = "home_url";
    static constexpr const char *PROXY_IP_ADDRESS = "proxy_ip_address";
    static constexpr const char *MMS_IP_ADDRESS = "mms_ip_address";
    static constexpr const char *APN_PROTOCOL = "apn_protocol";
    static constexpr const char *APN_ROAM_PROTOCOL = "apn_roam_protocol";
    static constexpr const char *BEARING_SYSTEM_TYPE = "bearing_system_type";
};

struct PdpProfile {
    int profileId;
    std::string profileName;
    std::string mcc;
    std::string mnc;
    std::string apn;
    int authType;
    std::string authUser;
    std::string authPwd;
    std::string apnTypes; // see ApnType
    int isRoamingApn;
    std::string homeUrl;
    std::string proxyIpAddress;
    std::string mmsIpAddress;
    std::string pdpProtocol; // see PdpProtocol
    std::string roamPdpProtocol;
    int bearingSystemType; // see BearingSystemType
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

constexpr const char *TABLE_PDP_PROFILE = "pdp_profile";
constexpr const char *TEMP_TABLE_PDP_PROFILE = "temp_pdp_profile";
constexpr const char *PDP_PROFILE_URI = "dataability:///com.ohos.pdpprofileability";
} // namespace Telephony
} // namespace OHOS
#endif // DATA_STORAGE_PDP_PROFILE_DATA_H
