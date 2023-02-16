/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
    static constexpr const char *MSG_ID = "msg_id";
    static constexpr const char *SLOT_ID = "slot_id";
    static constexpr const char *RECEIVER_NUMBER = "receiver_number";
    static constexpr const char *SENDER_NUMBER = "sender_number";
    static constexpr const char *IS_SENDER = "is_sender";
    static constexpr const char *START_TIME = "start_time";
    static constexpr const char *END_TIME = "end_time";
    static constexpr const char *MSG_TYPE = "msg_type";
    static constexpr const char *SMS_TYPE = "sms_type";
    static constexpr const char *MSG_TITLE = "msg_title";
    static constexpr const char *MSG_CONTENT = "msg_content";
    static constexpr const char *MSG_STATE = "msg_state";
    static constexpr const char *MSG_CODE = "msg_code";
    static constexpr const char *IS_LOCK = "is_lock";
    static constexpr const char *IS_READ = "is_read";
    static constexpr const char *IS_COLLECT = "is_collect";
    static constexpr const char *SESSION_TYPE = "session_type";
    static constexpr const char *RETRY_NUMBER = "retry_number";
    static constexpr const char *OPERATOR_SERVICE_NUMBER = "operator_service_number";
    static constexpr const char *SESSION_ID = "session_id";
    static constexpr const char *GROUP_ID = "group_id";
    static constexpr const char *DEVICE_ID = "device_id";
    static constexpr const char *IS_SUBSECTION = "is_subsection";
    static constexpr const char *IS_SEND_REPORT = "is_send_report";
};

class SmsSubsection {
public:
    static constexpr const char *ID = "id";
    static constexpr const char *SLOT_ID = "slot_id";
    static constexpr const char *SMS_SUBSECTION_ID = "sms_subsection_id";
    static constexpr const char *RECEIVER_NUMBER = "receiver_number";
    static constexpr const char *SENDER_NUMBER = "sender_number";
    static constexpr const char *IS_SENDER = "is_sender";
    static constexpr const char *START_TIME = "start_time";
    static constexpr const char *END_TIME = "end_time";
    static constexpr const char *REW_PUD = "raw_pdu";
    static constexpr const char *FORMAT = "format";
    static constexpr const char *DEST_PORT = "dest_port";
    static constexpr const char *SUBSECTION_INDEX = "subsection_index";
    static constexpr const char *SIZE = "size";
};

class MmsProtocol {
public:
    static constexpr const char *ID = "id";
    static constexpr const char *BCC = "bcc";
    static constexpr const char *CC = "cc";
    static constexpr const char *CONTENT_LOCATION = "content_location";
    static constexpr const char *DATE = "date";
    static constexpr const char *DELIVERY_REPORT = "delivery_report";
    static constexpr const char *DELIVERY_TIME = "delivery_time";
    static constexpr const char *EXPIRY = "expiry";
    static constexpr const char *TYPE = "type";
    static constexpr const char *SERIAL_NUMBER = "serial_number";
    static constexpr const char *CATEGORY = "category";
    static constexpr const char *VERSION = "version";
    static constexpr const char *SIZE = "size";
    static constexpr const char *PRIORITY = "priority";
    static constexpr const char *READ_REPLY = "read_reply";
    static constexpr const char *REPORT_ALLOWED = "report_allowed";
    static constexpr const char *RESPONSE_STATUS = "response_status";
    static constexpr const char *RESPONSE_TEXT = "response_text";
    static constexpr const char *SENDER_VISIBILITY = "sender_visibility";
};

class MmsPart {
public:
    static constexpr const char *ID = "id";
    static constexpr const char *PART_INDEX = "part_index";
    static constexpr const char *PART_SIZE = "part_size";
    static constexpr const char *RECORDING_TIME = "recording_time";
    static constexpr const char *TYPE = "type";
    static constexpr const char *LOCATION_PATH = "location_path";
    static constexpr const char *STATE = "state";
    static constexpr const char *ENCODE = "encode";
    static constexpr const char *CONTENT = "content";
};

constexpr const char *TABLE_SMS_MMS_INFO = "sms_mms_info";
constexpr const char *TABLE_SMS_SUBSECTION = "sms_subsection";
constexpr const char *TABLE_MMS_PROTOCOL = "mms_protocol";
constexpr const char *TABLE_MMS_PART = "mms_part";
constexpr const char *SMS_MMS_URI = "dataability:///com.ohos.smsmmsability";
} // namespace Telephony
} // namespace OHOS
#endif // DATA_STORAGE_SMS_DATA_H
