/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DATA_STORAGE_SMS_DATA_H
#define DATA_STORAGE_SMS_DATA_H

namespace OHOS {
namespace Telephony {
class SmsMmsInfo {
public:
    /**
     * @brief Message id
     */
    static constexpr const char *MSG_ID = "msg_id";
    /**
     * @brief Card slot index number
     */
    static constexpr const char *SLOT_ID = "slot_id";
    /**
     * @brief Recipient number
     */
    static constexpr const char *RECEIVER_NUMBER = "receiver_number";
    /**
     * @brief Sender number
     */
    static constexpr const char *SENDER_NUMBER = "sender_number";
    /**
     * @brief Sender or not 0 - Recipient 1 - Sender
     */
    static constexpr const char *IS_SENDER = "is_sender";
    /**
     * @brief Sending start time
     */
    static constexpr const char *START_TIME = "start_time";
    /**
     * @brief Sending end time
     */
    static constexpr const char *END_TIME = "end_time";
    /**
     * @brief Message type
     */
    static constexpr const char *MSG_TYPE = "msg_type";
    /**
     * @brief SMS type 0-Common，1-Notice, 2-HuaweiInfo
     */
    static constexpr const char *SMS_TYPE = "sms_type";
    /**
     * @brief Message title
     */
    static constexpr const char *MSG_TITLE = "msg_title";
    /**
     * @brief Message content
     */
    static constexpr const char *MSG_CONTENT = "msg_content";
    /**
     * @brief Message state
     */
    static constexpr const char *MSG_STATE = "msg_state";
    /**
     * @brief Message code
     */
    static constexpr const char *MSG_CODE = "msg_code";
    /**
     * @brief Locked or not
     */
    static constexpr const char *IS_LOCK = "is_lock";
    /**
     * @brief Readed or not
     */
    static constexpr const char *IS_READ = "is_read";
    /**
     * @brief Collected or not
     */
    static constexpr const char *IS_COLLECT = "is_collect";
    /**
     * @brief Session type 0 - Common 1 - broadcast 2 - Group Sending
     */
    static constexpr const char *SESSION_TYPE = "session_type";
    /**
     * @brief Retry times
     */
    static constexpr const char *RETRY_NUMBER = "retry_number";
    /**
     * @brief Operator service number
     */
    static constexpr const char *OPERATOR_SERVICE_NUMBER = "operator_service_number";
    /**
     * @brief Session id
     */
    static constexpr const char *SESSION_ID = "session_id";
    /**
     * @brief Group id
     */
    static constexpr const char *GROUP_ID = "group_id";
    /**
     * @brief Device id
     */
    static constexpr const char *DEVICE_ID = "device_id";
    /**
     * @brief Branch 0 - NO 1 - Yes
     */
    static constexpr const char *IS_SUBSECTION = "is_subsection";
    /**
     * @brief Whether to send a report
     */
    static constexpr const char *IS_SEND_REPORT = "is_send_report";
    /**
     * @brief Whether in advanced security status
     */
    static constexpr const char *IS_ADVANCED_SECURITY = "is_advanced_security";
    /**
     * @brief Whether in advanced security status
     */
    static constexpr const char *EXPIRES_TIME = "expires_time";
    /**
     * @brief Whether in advanced security status
     */
    static constexpr const char *MMS_PDU = "mms_pdu";
    /**
     * @brief Blocked or Normal, 0 - normal, 1 - Blocked
     */
    static constexpr const char *IS_BLOCKED  = "is_blocked";
    /**
     * @brief Blocked reason, 1 - blacklist, 2 - whilelist, 3...
     */
    static constexpr const char *BLOCKED_REASON  = "blocked_reason";
    /**
     * @brief Blocked sources
     */
    static constexpr const char *BLOCKED_SOURCES  = "blocked_sources";
    /**
     * @brief Blocked type
     */
    static constexpr const char *BLOCKED_TYPE  = "blocked_type";
    /**
     * @brief Blocked type text
     */
    static constexpr const char *BLOCKED_TYPE_TEXT  = "blocked_type_text";
    /**
     * @brief Blocked pref number
     */
    static constexpr const char *BLOCKED_PREF_NUMBER  = "blocked_pref_number";
    /**
     * @brief Risk url body
     */
    static constexpr const char *RISK_URL_BODY  = "risk_url_body";
    /**
     * @brief Has be reported as spam
     */
    static constexpr const char *HAS_BE_REPORTED_AS_SPAM  = "has_be_reported_as_spam";
    /**
     * @brief Has be report sms content
     */
    static constexpr const char *IS_REPORT  = "is_report";
    /**
     * @brief RCS id
     */
    static constexpr const char *RCS_ID = "rcs_id";
    /**
     * @brief Message detected content result
     */
    static constexpr const char *DETECT_RES_CONTENT = "detect_res_content";
    /**
     * @brief Peer Number
     */
    static constexpr const char *PHONE_NUMBER = "phone_number";
    /**
     * @brief Peer Number format
     */
    static constexpr const char *FORMAT_PHONE_NUMBER = "format_phone_number";
    /**
    * @brief Message code String
    */
    static constexpr const char *MSG_CODE_STR = "msg_code_str";
};

class RcsInfo {
public:
    /**
     * @brief RCS id
     */
    static constexpr const char *RCS_ID = "rcs_id";
    /**
     * @brief Card slot index number
     */
    static constexpr const char *SLOT_ID = "slot_id";
    /**
     * @brief Sender number
     */
    static constexpr const char *RECEIVER_NUMBER = "receiver_number";
    /**
     * @brief Recipient number
     */
    static constexpr const char *SENDER_NUMBER = "sender_number";
    /**
     * @brief Sending start time
     */
    static constexpr const char *START_TIME = "start_time";
    /**
     * @brief Sending end time
     */
    static constexpr const char *END_TIME = "end_time";
    /**
     * @brief Message type
     */
    static constexpr const char *RCS_TYPE = "rcs_type";
    /**
     * @brief Message id
     */
    static constexpr const char *MSG_ID = "msg_id";
    /**
     * @brief Message title
     */
    static constexpr const char *MSG_TITLE = "msg_title";
    /**
     * @brief Message content
     */
    static constexpr const char *MSG_CONTENT = "msg_content";
    /**
     * @brief Message state
     */
    static constexpr const char *MSG_STATE = "msg_state";
    /**
     * @brief Locked or not
     */
    static constexpr const char *IS_LOCK = "is_lock";
    /**
     * @brief Readed or not
     */
    static constexpr const char *IS_READ = "is_read";
    /**
     * @brief Sender or not
     */
    static constexpr const char *IS_SENDER = "is_sender";
    /**
     * @brief Session id
     */
    static constexpr const char *SESSION_ID = "session_id";
    /**
     * @brief Group id
     */
    static constexpr const char *GROUP_ID = "group_id";
    /**
     * @brief Enriched calling type
     */
    static constexpr const char *CALLING_TYPE = "enriched_calling_type";
    /**
     * @brief Error code
     */
    static constexpr const char *ERROR_CODE = "error_code";
    /**
     * @brief Network type
     */
    static constexpr const char *NETWORK_TYPE = "network_type";
    /**
     * @brief Owner addr
     */
    static constexpr const char *OWNER_ADDR = "owner_addr";
    /**
     * @brief Privacy mode
     */
    static constexpr const char *PRIVACY_MODE = "privacy_mode";
    /**
     * @brief Protocol
     */
    static constexpr const char *PROTOCOL = "protocol";
    /**
     * @brief Reply path present
     */
    static constexpr const char *REPLY_PATH = "reply_path_present";
    /**
     * @brief Seen or not
     */
    static constexpr const char *SEEN = "seen";
    /**
     * @brief Collected or not
     */
    static constexpr const char *IS_COLLECT = "is_collect";
    /**
     * @brief Receive state
     */
    static constexpr const char *RECEIVE_STATE = "receive_state";
    /**
     * @brief Unreceived context
     */
    static constexpr const char *FAIL_RECEIVE_CONTEXT = "fail_receive_context";
    /**
     * @brief Service center
     */
    static constexpr const char *SERVICE_CENTER = "service_center";
    /**
     * @brief Service kind
     */
    static constexpr const char *SERVICE_KIND = "service_kind";
    /**
     * @brief Whether in advanced security status
     */
    static constexpr const char *IS_ADVANCED_SECURITY = "is_advanced_security";
    /**
     * @brief Blocked or Normal, 0 - normal, 1 - Blocked
     */
    static constexpr const char *IS_BLOCKED  = "is_blocked";
    /**
     * @brief Blocked reason, 1 - blacklist, 2 - whilelist, 3...
     */
    static constexpr const char *BLOCKED_REASON  = "blocked_reason";
    /**
     * @brief Blocked sources
     */
    static constexpr const char *BLOCKED_SOURCES  = "blocked_sources";
    /**
     * @brief Blocked type
     */
    static constexpr const char *BLOCKED_TYPE  = "blocked_type";
    /**
     * @brief Blocked type text
     */
    static constexpr const char *BLOCKED_TYPE_TEXT  = "blocked_type_text";
    /**
     * @brief Blocked pref number
     */
    static constexpr const char *BLOCKED_PREF_NUMBER  = "blocked_pref_number";
    /**
     * @brief Blocked clur size
     */
    static constexpr const char *CLUR_SIZE = "clur_size";
    /**
     * @brief Blocked total size
     */
    static constexpr const char *TOTAL_SIZE = "total_size";
    /**
     * @brief Risk url body
     */
    static constexpr const char *RISK_URL_BODY  = "risk_url_body";
    /**
     * @brief Has be reported as spam
     */
    static constexpr const char *HAS_BE_REPORTED_AS_SPAM  = "has_be_reported_as_spam";
    /**
     * @brief Has be report sms content
     */
    static constexpr const char *IS_REPORT  = "is_report";
    /**
     * @brief Message detected content result
     */
    static constexpr const char *DETECT_RES_CONTENT = "detect_res_content";
    /**
     * @brief Peer Number
     */
    static constexpr const char *PHONE_NUMBER = "phone_number";
    /**
     * @brief Peer Number format
     */
    static constexpr const char *FORMAT_PHONE_NUMBER = "format_phone_number";
};

class SmsSubsection {
public:
    /**
     * @brief Id
     */
    static constexpr const char *ID = "id";
    /**
     * @brief Card slot index number
     */
    static constexpr const char *SLOT_ID = "slot_id";
    /**
     * @brief SMS subsection id
     */
    static constexpr const char *SMS_SUBSECTION_ID = "sms_subsection_id";
    /**
     * @brief Recipient number
     */
    static constexpr const char *RECEIVER_NUMBER = "receiver_number";
    /**
     * @brief Sender number
     */
    static constexpr const char *SENDER_NUMBER = "sender_number";
    /**
     * @brief Sender or not 0 - Recipient 1 - Sender
     */
    static constexpr const char *IS_SENDER = "is_sender";
    /**
     * @brief Sending start time
     */
    static constexpr const char *START_TIME = "start_time";
    /**
     * @brief Sending end time
     */
    static constexpr const char *END_TIME = "end_time";
    /**
     * @brief PDU string of the sms message
     */
    static constexpr const char *REW_PUD = "raw_pdu";
    /**
     * @brief Formatted data
     */
    static constexpr const char *FORMAT = "format";
    /**
     * @brief Destination port
     */
    static constexpr const char *DEST_PORT = "dest_port";
    /**
     * @brief Section index
     */
    static constexpr const char *SUBSECTION_INDEX = "subsection_index";
    /**
     * @brief Mms size
     */
    static constexpr const char *SIZE = "size";
};

class MmsProtocol {
public:
    /**
     * @brief Id
     */
    static constexpr const char *ID = "id";
    /**
     * @brief Blind carbon copy
     */
    static constexpr const char *BCC = "bcc";
    /**
     * @brief Carbon copy
     */
    static constexpr const char *CC = "cc";
    /**
     * @brief Content location
     */
    static constexpr const char *CONTENT_LOCATION = "content_location";
    /**
     * @brief Date
     */
    static constexpr const char *DATE = "date";
    /**
     * @brief Delivery report
     */
    static constexpr const char *DELIVERY_REPORT = "delivery_report";
    /**
     * @brief Delivery time
     */
    static constexpr const char *DELIVERY_TIME = "delivery_time";
    /**
     * @brief Expiration
     */
    static constexpr const char *EXPIRY = "expiry";
    /**
     * @brief Mms type
     */
    static constexpr const char *TYPE = "type";
    /**
     * @brief SerialNumber
     */
    static constexpr const char *SERIAL_NUMBER = "serial_number";
    /**
     * @brief Mms category
     */
    static constexpr const char *CATEGORY = "category";
    /**
     * @brief Mms version
     */
    static constexpr const char *VERSION = "version";
    /**
     * @brief Mms size
     */
    static constexpr const char *SIZE = "size";
    /**
     * @brief Mms priority
     */
    static constexpr const char *PRIORITY = "priority";
    /**
     * @brief Request a read receipt or not
     */
    static constexpr const char *READ_REPLY = "read_reply";
    /**
     * @brief Allow delivery reports or not
     */
    static constexpr const char *REPORT_ALLOWED = "report_allowed";
    /**
     * @brief Mms response status
     */
    static constexpr const char *RESPONSE_STATUS = "response_status";
    /**
     * @brief Mms response text
     */
    static constexpr const char *RESPONSE_TEXT = "response_text";
    /**
     * @brief Sender visibility
     */
    static constexpr const char *SENDER_VISIBILITY = "sender_visibility";
};

class MmsPart {
public:
    /**
     * @brief Id
     */
    static constexpr const char *ID = "id";
    /**
     * @brief Mms data subscript
     */
    static constexpr const char *PART_INDEX = "part_index";
    /**
     * @brief Mms size
     */
    static constexpr const char *PART_SIZE = "part_size";
    /**
     * @brief Audio/Video recording time
     */
    static constexpr const char *RECORDING_TIME = "recording_time";
    /**
     * @brief 0: theme; 1: picture; 2: video; 3: audio; 4: text; 5: business card
     */
    static constexpr const char *TYPE = "type";
    /**
     * @brief content type
     */
    static constexpr const char *CT = "ct";
    /**
     * @brief Local storage path
     */
    static constexpr const char *LOCATION_PATH = "location_path";
    /**
     * @brief Sending success status
     */
    static constexpr const char *STATE = "state";
    /**
     * @brief Encoding or not
     */
    static constexpr const char *ENCODE = "encode";
    /**
     * @brief Mms text content
     */
    static constexpr const char *CONTENT = "content";
    /**
     * @brief Rcs id
     */
    static constexpr const char *RCS_ID = "rcs_id";
    /**
     * @brief Group id
     */
    static constexpr const char *GROUP_ID = "group_id";
    /**
     * @brief Message id
     */
    static constexpr const char *MSG_ID = "msg_id";

    static constexpr const char *IS_FRAUD = "is_fraud";
    /**
     * @brief Reference count
     */
    static constexpr const char *REFERENCE_COUNT = "reference_count";
};

class Session {
public:
    static constexpr const char *ID = "id";
    static constexpr const char *TIME = "time";
    static constexpr const char *TELEPHONE = "telephone";
    static constexpr const char *CONTENT = "content";
    static constexpr const char *CONTACTS_NUM = "contacts_num";
    static constexpr const char *SMS_TYPE = "sms_type";
    static constexpr const char *UNREAD_COUNT = "unread_count";
    static constexpr const char *SENDING_STATUS = "sending_status";
    static constexpr const char *HAS_DRAFT = "has_draft";
    static constexpr const char *HAS_LOCK = "has_lock";
    static constexpr const char *MESSAGE_COUNT = "message_count";
    static constexpr const char *HAS_MMS = "has_mms";
    static constexpr const char *CONTACT_ID = "contact_id";
    static constexpr const char *CONTACT_NAME = "contact_name";
    static constexpr const char *HAS_ATTACHMENT = "has_attachment";
    static constexpr const char *YELLOW_PAGE_ID = "yellow_page_id";
    static constexpr const char *BLOCKED_TYPE = "blocked_type";
    static constexpr const char *PINNING_TIME = "pinning_time";
};

class MmsPdu {
public:
    /**
     * @brief Mms Pdu Id
     */
    static constexpr const char *ID = "id";
    /**
     * @brief Mms Pdu Content
     */
    static constexpr const char *PDU_CONTENT = "pdu_content";
};

class RiskUrlRecord {
public:
    /**
     * @brief Risk Url Record Id
     */
    static constexpr const char *ID = "id";
    /**
     * @brief Session Id
     */
    static constexpr const char *SESSION_ID = "session_id";
    /**
     * @brief Msg Id
     */
    static constexpr const char *MSG_ID = "msg_id";
    /**
     * @brief Rcs Id
     */
    static constexpr const char *RCS_ID = "rcs_id";
    /**
     * @brief Msg Type
     */
    static constexpr const char *MSG_TYPE = "msg_type";
    /**
     * @brief Risk Type
     */
    static constexpr const char *RISK_TYPE = "risk_type";
    /**
     * @brief Risk Url
     */
    static constexpr const char *RISK_URL = "risk_url";
    /**
     * @brief Url Index
     */
    static constexpr const char *URL_INDEX = "url_index";
    /**
     * @brief Create Time
     */
    static constexpr const char *CREATE_TIME = "create_time";
    /**
     * @brief Last Update Time
     */
    static constexpr const char *LAST_UPDATE_TIME = "last_update_time";
    /**
     * @brief Package Name
     */
    static constexpr const char *PACKAGE_NAME = "package_name";
    /**
     * @brief Check Type
     */
    static constexpr const char *CHECK_TYPE = "check_type";
    /**
     * @brief Number Of Retries
     */
    static constexpr const char *NUMBER_OF_RETRIES = "number_of_retries";
};

class ChatBots {
public:
    /**
     * @brief _id
     */
    static constexpr const char *ID = "_id";
    /**
     * @brief Service Id
     */
    static constexpr const char *SERVICE_ID = "service_id";
    /**
     * @brief Service Name
     */
    static constexpr const char *SERVICE_NAME = "service_name";
    /**
     * @brief Service Description
     */
    static constexpr const char *SERVICE_DESCRIPTION = "service_description";
    /**
     * @brief Callback Phone Number
     */
    static constexpr const char *CALLBACK_PHONE_NUMBER = "callback_phone_number";
    /**
     * @brief SMS
     */
    static constexpr const char *SMS = "sms";
    /**
     * @brief Service Icon
     */
    static constexpr const char *SERVICE_ICON = "service_icon";
    /**
     * @brief Category_list
     */
    static constexpr const char *CATEGORY_LIST = "category_list";
    /**
     * @brief Brief
     */
    static constexpr const char *BRIEF = "brief";
    /**
     * @brief Favorite
     */
    static constexpr const char *FAVORITE = "favorite";
    /**
     * @brief Email
     */
    static constexpr const char *EMAIL = "email";
    /**
     * @brief Website
     */
    static constexpr const char *WEBSITE = "website";
    /**
     * @brief Address
     */
    static constexpr const char *ADDRESS = "address";
    /**
     * @brief Address Lable
     */
    static constexpr const char *ADDRESS_LABLE = "address_lable";
    /**
     * @brief Last Suggested List
     */
    static constexpr const char *LAST_SUGGESTED_LIST = "last_suggested_list";
    /**
     * @brief Pinyin
     */
    static constexpr const char *PINYIN = "pinyin";
    /**
     * @brief Pinyin Short
     */
    static constexpr const char *PINYIN_SHORT = "pinyin_short";
    /**
     * @brief Colour
     */
    static constexpr const char *COLOUR = "colour";
    /**
     * @brief Background Image
     */
    static constexpr const char *BACKGROUND_IMAGE = "background_image";
    /**
     * @brief Verified
     */
    static constexpr const char *VERIFIED = "verified";
    /**
     * @brief Verified By
     */
    static constexpr const char *VERIFIED_BY = "verified_by";
    /**
     * @brief Verified Expires
     */
    static constexpr const char *VERIFIED_EXPIRES = "verified_expires";
    /**
     * @brief Timeout Interval
     */
    static constexpr const char *EXPIRES = "expires";
    /**
     * @brief Timeout Interval Control Time
     */
    static constexpr const char *CACHE_CONTROL = "cache_control";
    /**
     * @brief Carry It For The Next Update
     */
    static constexpr const char *E_TAG = "e_tag";
    /**
     * @brief Enterprise Certification Subject
     */
    static constexpr const char *TC_PAGE = "tc_page";
    /**
     * @brief Recent Use Time
     */
    static constexpr const char *RECENT_USE_TIME = "recent_use_time";
    /**
     * @brief Disturb
     */
    static constexpr const char *DISTURB = "disturb";
    /**
     * @brief Persistent Menu
     */
    static constexpr const char *PERSISTENT_MENU = "persistent_menu";
    /**
     * @brief Share Info
     */
    static constexpr const char *SHARE_INFO = "share_info";
    /**
     * @brief Service Provider
     */
    static constexpr const  char *service_provider = "service_provider";
};
 
class SpecificChatBots {
public:
    /**
     * @brief Chat Bots Id
     */
    static constexpr const char *ID = "_id";
    /**
     * @brief Service Id
     */
    static constexpr const char *SERVICE_ID = "service_id";
    /**
     * @brief Chat Bots Type
     */
    static constexpr const char *TYPE = "type";
    /**
     * @brief Carry It For The Next Update
     */
    static constexpr const char *E_TAG = "e_tag";
    /**
     * @brief Timeout Interval
     */
    static constexpr const char *EXPIRES = "expires";
    /**
     * @brief Timeout Interval Control Time
     */
    static constexpr const char *CACHE_CONTROL = "cache_control";
};

class FavoriteInfo {
public:
    /**
     * @brief Message id
     */
    static constexpr const char *MSG_ID = "msg_id";
    /**
     * @brief Recipient number
     */
    static constexpr const char *RECEIVER_NUMBER = "receiver_number";
    /**
     * @brief Sender number
     */
    static constexpr const char *SENDER_NUMBER = "sender_number";
    /**
     * @brief Keep time
     */
    static constexpr const char *KEEP_TIME = "keep_time";
    /**
     * @brief Sending start time
     */
    static constexpr const char *START_TIME = "start_time";
    /**
     * @brief Message type
     */
    static constexpr const char *MSG_TYPE = "msg_type";
    /**
     * @brief SMS type 0-Common，1-Notice, 2-HuaweiInfo
     */
    static constexpr const char *SMS_TYPE = "sms_type";
    /**
     * @brief Message title
     */
    static constexpr const char *MSG_TITLE = "msg_title";
    /**
     * @brief Message content
     */
    static constexpr const char *MSG_CONTENT = "msg_content";
    /**
     * @brief Session type 0 - Common 1 - broadcast 2 - Group Sending
     */
    static constexpr const char *SESSION_TYPE = "session_type";
    /**
     * @brief Group id
     */
    static constexpr const char *GROUP_ID = "group_id";
    /**
     * @brief MMS type
     */
    static constexpr const char *PART_TYPE = "part_type";
    /**
     * @brief MMS storage location
     */
    static constexpr const char *PART_LOCATION_PATH = "part_location_path";
    /**
     * @brief Sender or not
     */
    static constexpr const char *IS_SENDER = "is_sender";
    /**
     * @brief RCS id
     */
    static constexpr const char *RCS_ID = "rcs_id";
    /**
     * @brief Message detected content result
     */
    static constexpr const char *DETECT_RES_CONTENT = "detect_res_content";
    /**
     * @brief Session id
     */
    static constexpr const char *SESSION_ID = "session_id";
    /**
     * @brief Mms size
     */
    static constexpr const char *PART_SIZE = "part_size";
    /**
     * @brief Audio/Video recording time
     */
    static constexpr const char *RECORDING_TIME = "recording_time";
};

    class Smc {
    public:
        /**
         * @brief Smc Id
         */
        static constexpr const char *ID = "_id";
        /**
         * @brief thread id
         */
        static constexpr const char *THREAD_ID = "thread_id";
        /**
         * @brief type
         */
        static constexpr const char *TYPE = "type";
        /**
         * @brief address
         */
        static constexpr const char *ADDRESS = "address";
        /**
         * @brief date
         */
        static constexpr const char *DATE = "date";
        /**
         * @brief status
         */
        static constexpr const char *STATUS = "status";
        /**
         * @brief body
         */
        static constexpr const char *BODY = "body";
        /**
         * @brief sent
         */
        static constexpr const char *SENT = "sent";
        /**
         * @brief read
         */
        static constexpr const char *READ = "read";
        /**
         * @brief sub_id
         */
        static constexpr const char *SUB_ID = "sub_id";
        /**
         * @brief error_code
         */
        static constexpr const char *ERROR_CODE = "error_code";
        /**
         * @brief longitude
         */
        static constexpr const char *LONGITUDE = "longitude";
        /**
         * @brief latitude
         */
        static constexpr const char *LATITUDE = "latitude";
        /**
         * @brief privacy_mode
         */
        static constexpr const char *PRIVACY_MODE = "privacy_mode";
    };

constexpr const char *TABLE_SMS_MMS_INFO = "sms_mms_info";
constexpr const char *TABLE_RCS_INFO = "rcs_info";
constexpr const char *TABLE_MMS_INFO = "mms_info";
constexpr const char *TABLE_SMS_SUBSECTION = "sms_subsection";
constexpr const char *TABLE_MMS_PROTOCOL = "mms_protocol";
constexpr const char *TABLE_MMS_PART = "mms_part";
constexpr const char *TABLE_SESSION = "session";
constexpr const char *TABLE_MMS_PDU = "mms_pdu";
constexpr const char *TABLE_CHAT_BOTS_INFO = "chatbots";
constexpr const char *TABLE_SPECIFIC_CHAT_BOTS_INFO = "specific_chatbots";
constexpr const char *TABLE_RISK_URL_RECORD = "risk_url_record";
constexpr const char *SMS_MMS_URI = "datashare:///com.ohos.smsmmsability";
constexpr const char *TABLE_FAVORITE_INFO = "favorate_info";
constexpr const char *TABLE_SMC_INFO = "smc";
} // namespace Telephony
} // namespace OHOS
#endif // DATA_STORAGE_SMS_DATA_H
