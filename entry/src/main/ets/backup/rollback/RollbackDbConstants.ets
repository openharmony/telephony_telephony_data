/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
import DbConstants from '../copyData/DbConstants';

export interface ErrorInfo {
  type: string,
  errorCode: number,
  errorInfo: string
}

export default class RollbackDbConstants {
  public static readonly SMS_DATABASE_NAME = DbConstants.cloneDatabaseName;
  public static readonly CHAT_SMS_DATABASE_NAME = DbConstants.cloneRcsDatabaseName;
  public static readonly THREADS_TB_NAME = 'threads_tb';
  public static readonly SMS_TB_NAME = 'sms_tb';
  public static readonly ADDRESSES_TB_NAME = 'addresses_tb';
  public static readonly BACKUP_SUBTYPE = 'data_compatibility';
  public static readonly BACKUP_DETAIL = 'next2ohos'; // for backupInfo check, not same with 'COMPATIBLE_PATH'
  public static readonly NEXT2OHOS_TEMP_FOLDER = 'next2ohos';
  public static readonly NEXT_DATABASE_FOLDER = 'rdb';
  public static readonly NEXT_DATABASE_PATH = '/data/storage/el1/database'; // database path for single
  public static readonly BACKUP_DATABASE_PATH = '/data/storage/el2/base/files'; // OHOS compatible database path

  public static readonly CREATE_ADDRESSES_TB =
    'CREATE TABLE IF NOT EXISTS addresses_tb (' +
      '_id INTEGER PRIMARY KEY AUTOINCREMENT, ' +
      'address TEXT NOT NULL); ';

  public static readonly CREATE_THREADS_TB =
    'CREATE TABLE IF NOT EXISTS threads_tb (' +
      'thread_id INTEGER PRIMARY KEY AUTOINCREMENT, ' +
      'address TEXT, ' +
      'recipient_ids TEXT );';

  public static readonly CREATE_SMS_TB =
    'CREATE TABLE IF NOT EXISTS sms_tb (' +
      'subject TEXT, ' +
      'thread_id INTEGER, ' +
      'address TEXT, ' +
      'locked INTEGER DEFAULT 0, ' +
      'protocol INTEGER DEFAULT 0, ' +
      'status INTEGER DEFAULT -1, ' +
      'sub_id INTEGER, ' +
      'date_sent INTEGER, ' +
      'network_type INTEGER DEFAULT 0, ' +
      'body TEXT, ' +
      'date INTEGER, ' +
      'read INTEGER, ' +
      'seen INTEGER DEFAULT 0, ' +
      'type INTEGER, ' +
      'time_body TEXT DEFAULT \'\', ' +
      'group_id INTEGER, ' +
      'addr_body TEXT TEXT DEFAULT \'\', ' +
      'creator TEXT TEXT DEFAULT \'\', ' +
      'is_satellite INTEGER DEFAULT 0, ' +
      'error_code INTEGER DEFAULT -1, ' +
      'reply_path_present INTEGER DEFAULT 0, ' +
      'service_center TEXT TEXT DEFAULT \'\' );';

  public static readonly CREATE_ADDR_TB =
    'CREATE TABLE IF NOT EXISTS addr_tb (' +
      'address TEXT, ' +
      'msg_id INTEGER, ' +
      'type INTEGER, ' +
      'charset INTEGER );'

  public static readonly CREATE_PDU_TB =
    'CREATE TABLE IF NOT EXISTS pdu_tb (' +
      'retr_txt_cs TEXT, ' +
      'thread_id INTEGER, ' +
      'ct_cls TEXT, ' +
      'locked INTEGER, ' +
      'm_size INTEGER, ' +
      'm_type INTEGER, ' +
      'sub_cs INTEGER, ' +
      'sub_id INTEGER, ' +
      'read_status TEXT, ' +
      'resp_txt TEXT, ' +
      'retr_txt TEXT, ' +
      'date_sent INTEGER, ' +
      'network_type INTEGER, ' +
      'v INTEGER, ' +
      'rr TEXT, ' +
      'st TEXT, ' +
      '_id INTEGER, ' +
      'exp INTEGER, ' +
      'pri INTEGER, ' +
      'sub TEXT, ' +
      'ct_l TEXT, ' +
      'ct_t TEXT, ' +
      'd_tm TEXT, ' +
      'date INTEGER, ' +
      'm_id TEXT, ' +
      'read INTEGER, ' +
      'seen INTEGER, ' +
      'd_rpt INTEGER, ' +
      'm_cls TEXT, ' +
      'rpt_a TEXT, ' +
      'tr_id TEXT, ' +
      'creator TEXT, ' +
      'esp_st TEXT, ' +
      'retr_st TEXT, ' +
      'msg_box INTEGER );';

  public static readonly CREATE_PART_TB =
    'CREATE TABLE IF NOT EXISTS part_tb (' +
      'cl TEXT, ' +
      'ct TEXT, ' +
      'fn TEXT, ' +
      '_id INTEGER, ' +
      'cid TEXT, ' +
      'mid INTEGER, ' +
      'seq INTEGER, ' +
      'name TEXT, ' +
      'text TEXT, ' +
      '_data TEXT, ' +
      'chset INTEGER );';

  public static readonly CREATE_FILES_MMS_TB =
    'CREATE TABLE IF NOT EXISTS files_mms_tb (' +
      'file_data BLOB, ' +
      '_id INTEGER, ' +
      'data_index INTEGER );';
}