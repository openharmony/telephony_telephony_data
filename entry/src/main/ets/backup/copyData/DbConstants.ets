/**
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

export default class DbConstants {
  // Clone database name
  public static readonly cloneDatabaseName = 'sms.db';
  public static readonly cloneRcsDatabaseName = 'chatSms.db';
  public static readonly nextCloneDatabaseName = 'sms_mms.db';
  public static readonly nextCloneDatabaseNameShm = 'sms_mms.db-shm';
  public static readonly nextCloneDatabaseNameWal = 'sms_mms.db-wal';
  // Clone path for clone
  public static readonly CLONE_DATABASE_PATH = '/data/storage/el2/backup/restore';
  /**
   * database path
   */
  public static readonly NEXT_DATABASE_PATH = '/data/storage/el1/database/rdb';
  public static readonly EL1_RDB = '/data/storage/el1/database/rdb/';
  public static readonly EL2_RDB = '/data/storage/el2/database/rdb/';
  /**
   * The table 'sms' name in old database
   */
  public static readonly OLD_SMS_NAME = 'sms_tb';
  public static readonly OLD_PDU_NAME = 'pdu_tb';
  public static readonly OLD_PART_NAME = 'part_tb';
  public static readonly OLD_FILES_NAME = 'files_mms_tb';
  public static readonly OLD_THREADS_NAME = 'threads_tb';
  public static readonly OLD_ADDRESSES_NAME = 'addresses_tb';
  public static readonly OLD_RCS_CHAT_TABLE_NAME = 'comand' + 'roidrcsbackuprcs_chat_tb';
  public static readonly OLD_RCS_GROUP_TABLE_NAME = 'comand' + 'roidrcsbackuprcs_group_messages_tb';

  /**
   * The table 'sms' success or fail in old database
   */
  public static readonly THREAD_ID_FAIL = 'thread_id_fail';
  public static readonly THREAD_ID_SUCCESS = 'thread_id_success';

  public static readonly SMS_ID_FAIL = 'sms_id_fail';
  public static readonly SMS_ID_SUCCESS = 'sms_id_success';

  public static readonly PDU_ID_FAIL = 'pdu_id_fail';
  public static readonly PDU_ID_SUCCESS = 'pdu_id_success';

  public static readonly PART_ID_FAIL = 'part_id_fail';
  public static readonly PART_ID_SUCCESS = 'part_id_success';

  /**
   * The table 'sms_mms' name in old database (next)
   */
  public static readonly NEXT_OLD_SESSION_NAME = 'session';
  public static readonly NEXT_OLD_SMS_NAME = 'sms_mms_info';
  public static readonly NEXT_OLD_PDU_NAME = 'mms_pdu';
  public static readonly NEXT_OLD_PART_NAME = 'mms_part';
  public static readonly NEXT_OLD_PROTOCOL_NAME = 'mms_protocol';
  public static readonly NEXT_OLD_RCS_NAME = 'rcs_info';
  public static readonly NEXT_OLD_SUBSECTION_NAME = 'sms_subsection';

  /**
   * The table 'sms_mms' success or fail in old database
   */
  public static readonly NEXT_SESSION_ID_FAIL = 'next_session_id_fail';
  public static readonly NEXT_SESSION_ID_SUCCESS = 'next_session_id_success';

  public static readonly NEXT_SMS_ID_FAIL = 'next_sms_id_fail';
  public static readonly NEXT_SMS_ID_SUCCESS = 'next_sms_id_success';

  public static readonly NEXT_PDU_ID_FAIL = 'next_pdu_id_fail';
  public static readonly NEXT_PDU_ID_SUCCESS = 'next_pdu_id_success';

  public static readonly NEXT_PART_ID_FAIL = 'next_part_id_fail';
  public static readonly NEXT_PART_ID_SUCCESS = 'next_part_id_success';

  public static readonly NEXT_PROTOCOL_ID_FAIL = 'next_protocol_id_fail';
  public static readonly NEXT_PROTOCOL_ID_SUCCESS = 'next_protocol_id_success';

  public static readonly NEXT_RCS_ID_FAIL = 'next_rcs_id_fail';
  public static readonly NEXT_RCS_ID_SUCCESS = 'next_rcs_id_success';

  public static readonly NEXT_SUBSECTION_ID_FAIL = 'next_subsection_id_fail';
  public static readonly NEXT_SUBSECTION_ID_SUCCESS = 'next_subsection_id_success';

  public static readonly NEXT_FAVORITE_ID_FAIL = 'next_favorate_id_fail';
  public static readonly NEXT_FAVORITE_ID_SUCCESS = 'next_favorate_id_success';

  public static readonly TABLE_SESSION = 'session';
  public static readonly TABLE_SMS_MMS_INFO = 'sms_mms_info';
  public static readonly TABLE_MMS_PART = 'mms_part';
  public static readonly TABLE_MMS_PDU = 'mms_pdu';
  public static readonly TABLE_MMS_PROTOCOL = 'mms_protocol';
  public static readonly TABLE_RCS_INFO = 'rcs_info';
  public static readonly TABLE_SMS_SUBSECTION = 'sms_subsection';
  public static readonly TABLE_FAVORITE_INFO = 'favorate_info';

  public static readonly CREATE_SMS_MMS_INFO =
    'CREATE TABLE IF NOT EXISTS sms_mms_info (' +
      'msg_id INTEGER PRIMARY KEY AUTOINCREMENT, ' +
      'slot_id INTEGER DEFAULT 0, ' +
      'receiver_number TEXT , ' +
      'sender_number TEXT , ' +
      'is_sender INTEGER DEFAULT 0, ' +
      'msg_type INTEGER DEFAULT 0, ' +
      'sms_type INTEGER DEFAULT 0, ' +
      'start_time TEXT DEFAULT \'\', ' +
      'end_time TEXT DEFAULT \'\', ' +
      'msg_state INTEGER DEFAULT 0, ' +
      'msg_title TEXT DEFAULT \'\', ' +
      'msg_content TEXT DEFAULT \'\', ' +
      'operator_service_number TEXT DEFAULT \'\', ' +
      'is_lock INTEGER DEFAULT 0, ' +
      'is_collect INTEGER DEFAULT 0, ' +
      'rcs_id INTEGER DEFAULT 0, ' +
      'is_read INTEGER DEFAULT 0, ' +
      'session_type INTEGER DEFAULT 3, ' +
      'retry_number INTEGER DEFAULT 0, ' +
      'session_id INTEGER DEFAULT -1, ' +
      'group_id INTEGER DEFAULT -1, ' +
      'device_id INTEGER , ' +
      'is_subsection INTEGER DEFAULT 0, ' +
      'is_send_report INTEGER DEFAULT 0, ' +
      'is_advanced_security INTEGER DEFAULT 0, ' +
      'expires_time INTEGER DEFAULT 0, ' +
      'mms_pdu TEXT DEFAULT \'\', ' +
      'msg_code INTEGER DEFAULT 0, ' +
      'is_blocked INTEGER DEFAULT 0 , ' +
      'blocked_reason INTEGER DEFAULT 0 , ' +
      'blocked_sources TEXT DEFAULT \'\', ' +
      'blocked_type INTEGER DEFAULT 0 , ' +
      'blocked_type_text TEXT DEFAULT \'\', ' +
      'risk_url_body TEXT DEFAULT \'\', ' +
      'has_be_reported_as_spam INTEGER DEFAULT 0 , ' +
      'is_report INTEGER DEFAULT 0 , ' +
      'detect_res_content TEXT DEFAULT \'\', ' +
      'msg_code_str TEXT DEFAULT \'\', ' +
      'phone_number TEXT , ' +
      'format_phone_number TEXT , ' +
      'blocked_pref_number TEXT DEFAULT \'\')';

  public static readonly CREATE_RCS_INFO =
    'CREATE TABLE IF NOT EXISTS rcs_info (' +
      'rcs_id INTEGER PRIMARY KEY AUTOINCREMENT, ' +
      'slot_id INTEGER DEFAULT 0,' +
      'receiver_number TEXT , ' +
      'sender_number TEXT , ' +
      'start_time TEXT DEFAULT \'\', ' +
      'end_time TEXT DEFAULT \'\', ' +
      'rcs_type INTEGER DEFAULT 0, ' +
      'msg_state INTEGER DEFAULT 0, ' +
      'msg_title TEXT DEFAULT \'\', ' +
      'msg_content TEXT DEFAULT \'\', ' +
      'is_lock INTEGER DEFAULT 0, ' +
      'is_read INTEGER DEFAULT 0, ' +
      'is_sender INTEGER DEFAULT 0, ' +
      'session_id INTEGER DEFAULT -1, ' +
      'group_id INTEGER DEFAULT -1, ' +
      'msg_id TEXT DEFAULT \'\', ' +
      'enriched_calling_type INTEGER DEFAULT 0, ' +
      'error_code INTEGER DEFAULT 0, ' +
      'network_type TEXT DEFAULT \'\', ' +
      'owner_addr TEXT DEFAULT \'\', ' +
      'privacy_mode INTEGER DEFAULT 0, ' +
      'is_advanced_security INTEGER DEFAULT 0, ' +
      'is_collect INTEGER DEFAULT 0, ' +
      'receive_state INTEGER DEFAULT 0, ' +
      'fail_receive_context TEXT DEFAULT \'\', ' +
      'protocol INTEGER DEFAULT 0, ' +
      'reply_path_present INTEGER DEFAULT 0, ' +
      'seen INTEGER DEFAULT 0, ' +
      'service_center TEXT DEFAULT \'\', ' +
      'service_kind TEXT DEFAULT \'\', ' +
      'is_blocked INTEGER DEFAULT 0, ' +
      'blocked_reason INTEGER DEFAULT 0, ' +
      'blocked_sources TEXT DEFAULT \'\', ' +
      'blocked_type INTEGER DEFAULT 0, ' +
      'blocked_type_text TEXT DEFAULT \'\', ' +
      'clur_size INTEGER DEFAULT 0, ' +
      'total_size INTEGER DEFAULT 0, ' +
      'risk_url_body TEXT DEFAULT \'\', ' +
      'has_be_reported_as_spam INTEGER DEFAULT 0, ' +
      'is_report INTEGER DEFAULT 0, ' +
      'detect_res_content TEXT DEFAULT \'\', ' +
      'phone_number TEXT , ' +
      'format_phone_number TEXT , ' +
      'blocked_pref_number TEXT DEFAULT \'\')';

  public static readonly CREATE_SMS_SUBSECTION =
    'CREATE TABLE IF NOT EXISTS sms_subsection (' +
      'id INTEGER PRIMARY KEY AUTOINCREMENT, ' +
      'slot_id INTEGER DEFAULT 0,' +
      'sms_subsection_id INTEGER , ' +
      'receiver_number TEXT NOT NULL, ' +
      'sender_number TEXT NOT NULL, ' +
      'is_sender INTEGER DEFAULT 0, ' +
      'start_time TEXT DEFAULT \'\', ' +
      'end_time TEXT DEFAULT \'\', ' +
      'raw_pdu TEXT NOT NULL, ' +
      'format INTEGER , ' +
      'dest_port INTEGER , ' +
      'subsection_index INTEGER , ' +
      'size INTEGER )';

  public static readonly CREATE_MMS_PROTOCOL =
    'CREATE TABLE IF NOT EXISTS mms_protocol (' +
      'id INTEGER PRIMARY KEY AUTOINCREMENT, ' +
      'msg_id INTEGER NOT NULL, ' +
      'bcc TEXT DEFAULT \'\', ' +
      'cc TEXT DEFAULT \'\', ' +
      'content_location TEXT DEFAULT \'\', ' +
      'date TEXT DEFAULT \'\', ' +
      'delivery_report INTEGER DEFAULT 0, ' +
      'delivery_time TEXT DEFAULT \'\', ' +
      'expiry INTEGER , ' +
      'type INTEGER DEFAULT 0, ' +
      'serial_number TEXT DEFAULT \'\', ' +
      'category TEXT DEFAULT \'\', ' +
      'version INTEGER , ' +
      'size INTEGER , ' +
      'priority INTEGER , ' +
      'read_reply INTEGER , ' +
      'report_allowed INTEGER , ' +
      'response_status INTEGER DEFAULT 0, ' +
      'response_text TEXT DEFAULT \'\', ' +
      'sender_visibility INTEGER DEFAULT 0,' +
      'foreign key(' +
      'msg_id) references ' +
      'sms_mms_info(msg_id' +
      ') on delete cascade on update cascade )';

  public static readonly CREATE_MMS_PART =
    'CREATE TABLE IF NOT EXISTS mms_part (' +
      'id INTEGER PRIMARY KEY AUTOINCREMENT, ' +
      'msg_id INTEGER NOT NULL, ' +
      'group_id INTEGER , ' +
      'part_index INTEGER , ' +
      'part_size TEXT , ' +
      'recording_time TEXT , ' +
      'type INTEGER , ' +
      'ct TEXT , ' +
      'location_path TEXT DEFAULT \'\', ' +
      'encode INTEGER , ' +
      'state INTEGER , ' +
      'content TEXT , ' +
      'rcs_id INTEGER , ' +
      'is_fraud INTEGER , ' +
      'reference_count INTEGER DEFAULT -1 , ' +
      'foreign key(' +
      'msg_id) references ' +
      'sms_mms_info(msg_id' +
      ') on delete cascade on update cascade )';

  public static readonly CREATE_SESSION =
    'CREATE TABLE IF NOT EXISTS session (' +
      'id INTEGER PRIMARY KEY AUTOINCREMENT, ' +
      'time INTEGER DEFAULT 0, ' +
      'telephone TEXT , ' +
      'content TEXT , ' +
      'contacts_num INTEGER DEFAULT 0 , ' +
      'sms_type INTEGER DEFAULT 0 , ' +
      'unread_count INTEGER DEFAULT 0 , ' +
      'sending_status INTEGER DEFAULT 0 , ' +
      'has_draft INTEGER DEFAULT 0 , ' +
      'has_lock INTEGER DEFAULT 0 , ' +
      'message_count INTEGER DEFAULT 0 , ' +
      'has_mms INTEGER DEFAULT 0 , ' +
      'contact_id TEXT DEFAULT \'\', ' +
      'contact_name TEXT DEFAULT \'\', ' +
      'has_attachment INTEGER DEFAULT 0 , ' +
      'yellow_page_id TEXT DEFAULT \'\', ' +
      'blocked_type INTEGER DEFAULT 0, ' +
      'pinning_time INTEGER DEFAULT 0)';

  public static readonly CREATE_MMS_PDU =
    'CREATE TABLE IF NOT EXISTS mms_pdu (' +
      'id INTEGER PRIMARY KEY AUTOINCREMENT, ' +
      'pdu_content TEXT )';

  public static readonly CREATE_CHATBOTS =
    'CREATE TABLE IF NOT EXISTS chatbots(' +
      '_id INTEGER PRIMARY KEY AUTOINCREMENT, ' +
      'service_id TEXT DEFAULT \'\', ' +
      'service_name TEXT DEFAULT \'\', ' +
      'service_description TEXT DEFAULT \'\', ' +
      'callback_phone_number TEXT DEFAULT \'\', ' +
      'sms TEXT DEFAULT \'\', ' +
      'service_icon TEXT DEFAULT \'\', ' +
      'category_list TEXT DEFAULT \'\', ' +
      'brief INTEGER DEFAULT 0, ' +
      'favorite INTEGER DEFAULT 0, ' +
      'email TEXT DEFAULT \'\', ' +
      'website TEXT DEFAULT \'\', ' +
      'address TEXT DEFAULT \'\', ' +
      'address_lable TEXT DEFAULT \'\', ' +
      'last_suggested_list TEXT DEFAULT \'\', ' +
      'pinyin TEXT DEFAULT \'\', ' +
      'pinyin_short TEXT DEFAULT \'\', ' +
      'colour TEXT DEFAULT \'\', ' +
      'background_image TEXT DEFAULT \'\', ' +
      'verified INTEGER DEFAULT 0, ' +
      'verified_by TEXT DEFAULT \'\', ' +
      'verified_expires TEXT DEFAULT \'\', ' +
      'expires INTEGER DEFAULT 0, ' +
      'cache_control TEXT DEFAULT \'\', ' +
      'e_tag TEXT DEFAULT \'\', ' +
      'tc_page TEXT DEFAULT \'\', ' +
      'recent_use_time TEXT DEFAULT \'\', ' +
      'disturb INTEGER DEFAULT 0, ' +
      'persistent_menu TEXT DEFAULT \'\', ' +
      'share_info INTEGER DEFAULT 0, ' +
      'service_provider TEXT DEFAULT \'\')';

  public static readonly CREATE_SPECIFIC_CHATBOTS =
    'CREATE TABLE IF NOT EXISTS specific_chatbots(' +
      '_id INTEGER PRIMARY KEY AUTOINCREMENT, ' +
      'service_id TEXT DEFAULT \'\', ' +
      'type INTEGER DEFAULT 0, ' +
      'e_tag TEXT DEFAULT \'\', ' +
      'expires INTEGER DEFAULT 0, ' +
      'cache_control TEXT DEFAULT \'\')';
  /**
   * The query page number limit 50
   */
  public static readonly QUERY_PAGE_NUM = 100;

  public static readonly CREATE_FAVORITE_INFO =
    'CREATE TABLE IF NOT EXISTS favorate_info (' +
      'msg_id INTEGER, ' +
      'receiver_number TEXT DEFAULT \'\', ' +
      'sender_number TEXT DEFAULT \'\', ' +
      'keep_time TEXT DEFAULT \'\', ' +
      'start_time TEXT DEFAULT \'\', ' +
      'msg_type INTEGER, ' +
      'sms_type INTEGER, ' +
      'msg_title TEXT DEFAULT \'\', ' +
      'msg_content TEXT DEFAULT \'\', ' +
      'session_type INTEGER, ' +
      'group_id INTEGER, ' +
      'part_type INTEGER, ' +
      'part_location_path TEXT DEFAULT \'\', ' +
      'is_sender INTEGER, ' +
      'rcs_id INTEGER, ' +
      'detect_res_content TEXT DEFAULT \'\', ' +
      'session_id INTEGER, ' +
      'recording_time TEXT DEFAULT \'\', ' +
      'part_size TEXT DEFAULT \'\', ' +
      'UNIQUE (msg_id, rcs_id)' +
      ')';
    public static readonly DROP_DDMS_DATA_SEARCH_AUX_CONFIG_TABLE =
      'DROP TABLE IF EXISTS ddms_data_search_aux_config'
    public static readonly DROP_DDMS_DATA_SEARCH_FILTERED_DATA_TABLE =
      'DROP TABLE IF EXISTS ddms_data_search_filtered_data'
}