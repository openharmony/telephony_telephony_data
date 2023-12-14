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
     * @brief SMS type 0-Common，1-Notice
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
    static constexpr const char *HAS_ATTACHMENT = "has_attachment";
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

constexpr const char *TABLE_SMS_MMS_INFO = "sms_mms_info";
constexpr const char *TABLE_SMS_SUBSECTION = "sms_subsection";
constexpr const char *TABLE_MMS_PROTOCOL = "mms_protocol";
constexpr const char *TABLE_MMS_PART = "mms_part";
constexpr const char *TABLE_SESSION = "session";
constexpr const char *TABLE_MMS_PDU = "mms_pdu";
constexpr const char *SMS_MMS_URI = "datashare:///com.ohos.smsmmsability";
} // namespace Telephony
} // namespace OHOS
#endif // DATA_STORAGE_SMS_DATA_H
