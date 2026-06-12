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

import rdb from '@ohos.data.relationalStore';
import { BusinessError } from '@ohos.base';
import { ValuesBucket, ValueType } from '@ohos.data.ValuesBucket';
import common from '@ohos.app.ability.common';
import contextConstant from '@ohos.app.ability.contextConstant';
import DbConstants from './DbConstants';
import LogUtils from '../../common/utils/LogUtils';
import LooseObject from './LooseObject';
import HashMap from '@ohos.util.HashMap';
import SessionMapItem from './bean/SessionMapItem';
import ErrorUtils from '../../common/utils/ErrorUtils';
import { StringUtil } from '../../common/utils/StringUtil';
import fs from '@ohos.file.fs';
import BackupRestoreProgress from './BackupRestoreProcess';
import NextOldDBData from './NextOldDBData';
import BackupCountInfo from './BackupCountInfo';
import dataRdb from '@ohos.data.relationalStore';
import BackupRestoreResult from './BackupRestoreResult';
import { GskvSharedPreferencesUtils } from '../../common/utils/GskvPreferencesUtils';
import SmsMmsInfoItem from './bean/SmsMmsInfoItem';
import RcsInfoItem from './bean/RcsInfoItem';
import RdbStoreUtils from '../../common/utils/RdbStoreUtils';

const TAG = 'Copy-NewDbHelper: ';

const CURRENT_RDB_VERSION: number = 1;
const CURRENT_RDB_VERSION_32: number = 32;
const PROCESS_PAGE_SIZE: number = 50;

export default class NewDbHelper {
  protected rdbStore: rdb.RdbStore | null = null;

  public async insertSession(context: common.Context, values: ValuesBucket): Promise<number> {
    if (!values) {
      LogUtils.e(TAG, '[insertSession] The addParams of parameter is NULL');
      return Promise.resolve(-1);
    }
    let sessionRdbStore = await this.getRdbStore(context);
    if (sessionRdbStore == null) {
      return -1;
    }
    try {
      return await RdbStoreUtils.insert(sessionRdbStore, DbConstants.TABLE_SESSION, values);
    } catch (e) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `insertSession code:${e.code} msg:${e.message}`);
      LogUtils.e(TAG, 'insertSession: ' + JSON.stringify(e));
      return -1;
    }
  }

  public async resetSessionUnread(context: common.Context): Promise<number> {
    let valueBucket: ValuesBucket = {
      'unread_count': 0
    }
    let sessionRdbStore = await this.getRdbStore(context);
    if (sessionRdbStore == null) {
      return -1;
    }
    try {
      let predicates = new rdb.RdbPredicates(DbConstants.TABLE_SESSION);
      return await RdbStoreUtils.update(sessionRdbStore, valueBucket, predicates);
    } catch (e) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `resetSessionUnread code:${e.code} msg:${e.message}`);
      LogUtils.e(TAG, 'updateSession: ' + JSON.stringify(e));
      return -1;
    }
  }

  public async updateSession(context: common.Context, sessionId: number, values: ValuesBucket): Promise<number> {
    if (!values) {
      LogUtils.e(TAG, '[updateSession] The addParams of parameter is NULL');
      return Promise.resolve(-1);
    }
    let sessionRdbStore = await this.getRdbStore(context);
    if (sessionRdbStore == null) {
      LogUtils.i(TAG, `update session, getRdbStore failed`)
      return -1;
    }
    try {
      let predicates = new rdb.RdbPredicates(DbConstants.TABLE_SESSION);
      predicates.equalTo('id', sessionId);
      return await RdbStoreUtils.update(sessionRdbStore, values, predicates);
    } catch (e) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `updateSession code:${e.code} msg:${e.message}`);
      LogUtils.e(TAG, 'updateSession: ' + JSON.stringify(e));
      return -1;
    }
  }

  public async deleteSession(context: common.Context, ardtime: number): Promise<number> {
    LogUtils.i(TAG, '[deleteSession] start');
    let sessionRdbStore = await this.getRdbStore(context);
    if (sessionRdbStore == null) {
      return -1;
    }
    try {
      let predicates = new rdb.RdbPredicates(DbConstants.TABLE_SESSION);
      predicates.equalTo('time', ardtime);
      return RdbStoreUtils.delete(sessionRdbStore, predicates);
    } catch (e) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `deleteSession code:${e.code} msg:${e.message}`);
      LogUtils.e(TAG, 'deleteSession: ' + JSON.stringify(e));
      return -1;
    }
  }

  public async deleteSms(context: common.Context): Promise<void> {
    LogUtils.i(TAG, `deleteSms start`)
    let mmsPartRdbStore = await this.getRdbStore(context);
    let predicates = new rdb.RdbPredicates(DbConstants.TABLE_SMS_MMS_INFO);
    if (mmsPartRdbStore != null) {
      try {
        let data = (mmsPartRdbStore as rdb.RdbStore).executeSync(
          'DELETE FROM sms_mms_info WHERE msg_type = 1 AND sms_type != 99 AND msg_id NOT IN (SELECT DISTINCT msg_id FROM mms_part)');
        LogUtils.i(TAG, `delete result: ${data}`);
      } catch (err) {
        const backupRestoreResult = BackupRestoreResult.getInstance();
        backupRestoreResult.insertOrUpdateBackupInfo('sms',
          `deleteSms code:${err.code} msg:${err.message}`);
        LogUtils.e(TAG, `delete failed, code is ${err.code},message is ${err.message}`);
      }
    }
  }

  public async deleteSipSms(context: common.Context): Promise<void> {
    LogUtils.i(TAG, `deleteSipSms start`)
    let mmsPartRdbStore = await this.getRdbStore(context);
    let predicates = new rdb.RdbPredicates(DbConstants.TABLE_SMS_MMS_INFO);
    if (mmsPartRdbStore != undefined) {
      try {
        let data =
          (mmsPartRdbStore as rdb.RdbStore).executeSync('DELETE FROM sms_mms_info WHERE receiver_number LIKE \'sip:%\' OR sender_number LIKE \'sip:%\'');
        LogUtils.i(TAG, `deleteSipSms result: ${data}`);
      } catch (err) {
        const backupRestoreResult = BackupRestoreResult.getInstance();
        backupRestoreResult.insertOrUpdateBackupInfo('sms',
          `deleteSipSms code:${err.code} msg:${err.message}`);
        LogUtils.e(TAG, `deleteSipSms failed, code is ${err.code},message is ${err.message}`);
      }
    }
  }

  public async deleteSipRcs(context: common.Context): Promise<void> {
    LogUtils.i(TAG, `deleteSipRcs start`)
    let mmsPartRdbStore = await this.getRdbStore(context);
    let predicates = new rdb.RdbPredicates(DbConstants.TABLE_SMS_MMS_INFO);
    if (mmsPartRdbStore != null) {
      try {
        let data =
          (mmsPartRdbStore as rdb.RdbStore).executeSync('DELETE FROM rcs_info WHERE receiver_number LIKE \'sip:%\' OR sender_number LIKE \'sip:%\'');
        LogUtils.i(TAG, `deleteSipRcs result: ${data}`);
      } catch (err) {
        const backupRestoreResult = BackupRestoreResult.getInstance();
        backupRestoreResult.insertOrUpdateBackupInfo('sms',
          `deleteSipRcs code:${err.code} msg:${err.message}`);
        LogUtils.e(TAG, `deleteSipRcs failed, code is ${err.code},message is ${err.message}`);
      }
    }
  }

  public async deleteSipSession(context: common.Context): Promise<void> {
    LogUtils.i(TAG, `deleteSipSession start`)
    let mmsPartRdbStore = await this.getRdbStore(context);
    let predicates = new rdb.RdbPredicates(DbConstants.TABLE_SMS_MMS_INFO);
    if (mmsPartRdbStore != undefined) {
      try {
        let data =
          (mmsPartRdbStore as rdb.RdbStore).executeSync('DELETE FROM session WHERE telephone LIKE \'sip:%\'');
        LogUtils.i(TAG, `deleteSipSession result: ${data}`);
      } catch (err) {
        const backupRestoreResult = BackupRestoreResult.getInstance();
        backupRestoreResult.insertOrUpdateBackupInfo('sms',
          `deleteSipSession code:${err.code} msg:${err.message}`);
        LogUtils.e(TAG, `deleteSipSession failed, code is ${err.code},message is ${err.message}`);
      }
    }
  }

  // 删除数据库中多余的草稿数据, Map的key为sessionid， value为需要保留的草稿的时间
  // key: sessionId; value: 要保留的草稿的时间
  public async deleteInvalidDraft(context: common.Context, progress: BackupRestoreProgress,
    sessions: Map<number, number>):
    Promise<void> {
    LogUtils.i(TAG, `deleteInvalidDraft start: ${sessions.size}`);
    let stageCount: number = progress.getDelInvalidDraftCount();
    let historyCount: number = progress.getProcessCount();
    let rdbStore = await this.getRdbStore(context);
    if (rdbStore == null) {
      progress.updateProcessCount(historyCount + stageCount);
      LogUtils.i(TAG, `deleteInvalidDraft, getRdbStore failed`);
      return;
    }
    let processCount: number = 0;
    sessions.forEach((time, session) => {
      if (session < 0 || time == -1) {
        LogUtils.w(TAG, `deleteInvalidDraft, invalid session ${session}`);
        return;
      }
      try {
        let sql: string =
          `DELETE FROM sms_mms_info 
          WHERE sms_mms_info.session_id = ${session}
          AND sms_mms_info.msg_state = 3
          AND sms_mms_info.start_time != '${time}'
          AND EXISTS (
            SELECT 1 FROM sms_mms_info 
            WHERE sms_mms_info.session_id = ${session}
            AND sms_mms_info.msg_state = 3
            AND sms_mms_info.start_time = '${time}'
          )`;
        let data = (rdbStore as rdb.RdbStore).executeSync(sql);
        LogUtils.i(TAG, `deleteInvalidDraft result: ${session} ${data}`);
      } catch (err) {
        const backupRestoreResult = BackupRestoreResult.getInstance();
        backupRestoreResult.insertOrUpdateBackupInfo('sms', 
          `deleteInvalidDraft ${session} code:${err.code} msg:${err.message}`);
        LogUtils.e(TAG, `deleteInvalidDraft ${session} failed, ${err.code} ${err.message}`);
      }
      processCount++;
      progress.updateProcessCount(historyCount + Math.floor(stageCount * (processCount / sessions.size)));
    });
    progress.updateProcessCount(historyCount + stageCount); // 兜底异常场景
  }

  public async deleteInvalidSession(context: common.Context, progress: BackupRestoreProgress): Promise<void> {
    LogUtils.i(TAG, `deleteInvalidSession start`);
    let store = await this.getRdbStore(context);
    if (store == null) {
      LogUtils.i(TAG, `deleteInvalidSession, getRdbStore failed`);
      return;
    }
    try {
      const pageSize = PROCESS_PAGE_SIZE;
      let hasMore = true;
      while (hasMore) {
        const querySql =
          `SELECT id FROM session WHERE (( session.id NOT IN ( SELECT session_id FROM sms_mms_info ) AND ` +
            `session.id NOT IN ( SELECT session_id FROM rcs_info ) ) OR ( session.time = 0 )) ` +
            `LIMIT ${pageSize}`;
        const result = await store.querySql(querySql);
        if (result.rowCount === 0) {
          hasMore = false;
        } else {
          const idsToDelete: number[] = [];
          while (result.goToNextRow()) {
            const sessionId = result.getLong(result.getColumnIndex('id'));
            idsToDelete.push(sessionId);
          }
          const deleteSql = `DELETE FROM session WHERE id IN (${idsToDelete.join(',')})`;
          await store.executeSql(deleteSql);
          LogUtils.i(TAG, `delete ${idsToDelete.length} invalid session`);
          progress.updateProcessCount(progress.getProcessCount() + idsToDelete.length);
        }
        result.close();
      }
    } catch (err) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo(
        'sms', `delete invalid session code:${err.code} msg:${err.message}`);
      LogUtils.e(TAG, `delete invalid session failed, code is ${err.code}, message is ${err.message}`);
    }
  }

  public async deleteInvalidSmsMmsInfo(context: common.Context, progress: BackupRestoreProgress): Promise<void> {
    LogUtils.i(TAG, `deleteInvalidSmsMmsInfo start`);
    let store = await this.getRdbStore(context);
    if (store == null) {
      LogUtils.i(TAG, `deleteInvalidSmsMmsInfo, getRdbStore failed`);
      return;
    }
    try {
      const pageSize = PROCESS_PAGE_SIZE;
      let hasMore = true;
      while (hasMore) {
        const querySql =
          `SELECT msg_id FROM sms_mms_info WHERE ` +
            `((sms_mms_info.session_id NOT IN (SELECT session.id FROM session)) AND (sms_mms_info.is_blocked = 0)) ` +
            `LIMIT ${pageSize}`;
        const result = await store.querySql(querySql);
        if (result.rowCount === 0) {
          hasMore = false;
        } else {
          const idsToDelete: number[] = [];
          while (result.goToNextRow()) {
            const msgId = result.getLong(result.getColumnIndex('msg_id'));
            idsToDelete.push(msgId);
          }
          const deleteSql = `DELETE FROM sms_mms_info WHERE msg_id IN (${idsToDelete.join(',')})`;
          await store.executeSql(deleteSql);
          LogUtils.i(TAG, `delete ${idsToDelete.length} invalid info`);
          progress.updateProcessCount(progress.getProcessCount() + idsToDelete.length);
        }
        result.close();
      }
    } catch (err) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo(
        'sms', `delete invalid smsmms code:${err.code} msg:${err.message}`);
      LogUtils.e(TAG, `delete invalid info failed, code is ${err.code}, message is ${err.message}`);
    }
  }

  public async deleteInvalidRcsInfo(context: common.Context, progress: BackupRestoreProgress): Promise<void> {
    LogUtils.i(TAG, `deleteInvalidRcsInfo start`);
    let store = await this.getRdbStore(context);
    if (store == null) {
      LogUtils.i(TAG, `deleteInvalidRcsInfo, getRdbStore failed`);
      return;
    }
    try {
      const pageSize = PROCESS_PAGE_SIZE;
      let hasMore = true;
      while (hasMore) {
        const querySql =
          `SELECT rcs_id FROM rcs_info WHERE ` +
            `( rcs_info.rcs_id NOT IN ( SELECT DISTINCT sms_mms_info.rcs_id FROM sms_mms_info )) ` +
            `LIMIT ${pageSize}`;
        const result = await store.querySql(querySql);
        if (result.rowCount === 0) {
          hasMore = false;
        } else {
          const idsToDelete: number[] = [];
          while (result.goToNextRow()) {
            const rcsId = result.getLong(result.getColumnIndex('rcs_id'));
            idsToDelete.push(rcsId);
          }
          const deleteSql = `DELETE FROM rcs_info WHERE rcs_id IN (${idsToDelete.join(',')})`;
          await store.executeSql(deleteSql);
          LogUtils.i(TAG, `delete ${idsToDelete.length} invalid rcs info`);
          progress.updateProcessCount(progress.getProcessCount() + idsToDelete.length);
        }
        result.close();
      }
    } catch (err) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo(
        'sms', `delete invalid rcsinfo code:${err.code} msg:${err.message}`);
      LogUtils.e(TAG, `delete invalid rcs info failed, code is ${err.code}, message is ${err.message}`);
    }
  }

  public async insertSms(context: common.Context, values: ValuesBucket): Promise<number> {
    if (!values) {
      LogUtils.e(TAG, '[insertSms] The addParams of parameter is NULL');
      return Promise.resolve(-1);
    }
    let smsRdbStore = await this.getRdbStore(context);
    if (smsRdbStore == null) {
      return -1;
    }
    try {
      return await RdbStoreUtils.insert(smsRdbStore, DbConstants.TABLE_SMS_MMS_INFO, values);
    } catch (e) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `insertSms code:${e.code} msg:${e.message}`);
      LogUtils.e(TAG, 'insertSms: ' + JSON.stringify(e));
      return -1;
    }
  }

  public async batchInsertSms(context: common.Context, values: Array<ValuesBucket>): Promise<number> {
    if (!values) {
      LogUtils.e(TAG, '[batchInsertSms] The addParams of parameter is NULL');
      return Promise.resolve(-1);
    }
    let smsRdbStore = await this.getRdbStore(context);
    if (smsRdbStore == null) {
      return -1;
    }
    try {
      return await RdbStoreUtils.batchInsertWithConflictResolution(smsRdbStore, DbConstants.TABLE_SMS_MMS_INFO, values,
        dataRdb.ConflictResolution.ON_CONFLICT_REPLACE);
    } catch (e) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `batchInsertSms code:${e.code} msg:${e.message}`);
      LogUtils.e(TAG, 'batchInsertSms: ' + JSON.stringify(e));
      return -1;
    }
  }

  public async updateSms(context: common.Context, smsId: number, values: ValuesBucket): Promise<number> {
    if (!values) {
      LogUtils.e(TAG, '[updateSms] The addParams of parameter is NULL');
      return Promise.resolve(-1);
    }
    let smsRdbStore = await this.getRdbStore(context);
    if (smsRdbStore == null) {
       return -1;
    }
    try {
      let predicates = new rdb.RdbPredicates(DbConstants.TABLE_SMS_MMS_INFO);
      predicates.equalTo('msg_id', smsId);
      return await RdbStoreUtils.update(smsRdbStore, values, predicates);
    } catch (e) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `updateSms code:${e.code} msg:${e.message}`);
      LogUtils.e(TAG, 'updateSms: ' + JSON.stringify(e));
      return -1;
    }
  }

  public async insertPart(context: common.Context, values: ValuesBucket): Promise<number> {
    if (!values) {
      LogUtils.e(TAG, '[insertPart] The addParams of parameter is NULL');
      return Promise.resolve(-1);
    }
    let mmsPartRdbStore = await this.getRdbStore(context);
    if (mmsPartRdbStore == null) {
      return -1;
    }
    try {
      return await RdbStoreUtils.insert(mmsPartRdbStore, DbConstants.TABLE_MMS_PART, values);
    } catch (e) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `insertPart code:${e.code} msg:${e.message}`);
      LogUtils.e(TAG, 'insertPart: ' + JSON.stringify(e));
      return -1;
    }
  }

  public async batchInsertPart(context: common.Context, values: Array<ValuesBucket>): Promise<number> {
    if (!values) {
      LogUtils.e(TAG, '[batchInsertPart] The addParams of parameter is NULL');
      return Promise.resolve(-1);
    }
    let mmsPartRdbStore = await this.getRdbStore(context);
    if (mmsPartRdbStore == null) {
      return -1;
    }
    try {
      return await RdbStoreUtils.batchInsertWithConflictResolution(mmsPartRdbStore, DbConstants.TABLE_MMS_PART, values,
        dataRdb.ConflictResolution.ON_CONFLICT_REPLACE);
    } catch (e) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `batchInsertPart code:${e.code} msg:${e.message}`);
      LogUtils.e(TAG, 'batchInsertPart: ' + JSON.stringify(e));
      return -1;
    }
  }

  public async queryMaxInfoId(context: Context, isRcs: boolean): Promise<number> {
    // 此方法返回当前可用的唯一 msg_id，需要保证无info存在的场景，msg_id唯一
    if (context == null) {
      LogUtils.w(TAG, `queryMaxInfoId failed by null context`)
      return Promise.resolve(-1);
    }
    let maxId: number = 0;
    try {
      let rdbStore = await this.getRdbStore(context);
      if (rdbStore == null || rdbStore === undefined) {
        LogUtils.e(TAG, `[queryMaxInfoId] failed : rdbStore === undefined`)
        return Promise.resolve(-1);
      }
      let table = isRcs ? DbConstants.TABLE_RCS_INFO : DbConstants.TABLE_SMS_MMS_INFO;
      let idType = isRcs ? 'rcs_id' : 'msg_id';
      if (isRcs) {
        let valuesRcsInfo: ValuesBucket = new RcsInfoItem().createValuesBucket();
        maxId = await this.insertRcs(context, valuesRcsInfo);
      } else {
        let valuesMmsInfo: ValuesBucket = new SmsMmsInfoItem().createValuesBucket();
        maxId = await this.insertSms(context, valuesMmsInfo);
      }
      let predicates = new rdb.RdbPredicates(table);
      predicates.equalTo(idType, maxId);
      await RdbStoreUtils.delete(rdbStore, predicates);
      return maxId;
    } catch (err) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `queryMaxInfoId code:${err.code} msg:${err.message}`);
      LogUtils.e(TAG, `queryMaxInfoId failed, code is ${err.code},message is ${err.message}`);
      return Promise.resolve(-1);
    }
  }

  public async batchInsertFav(context: common.Context, values: Array<ValuesBucket>): Promise<number> {
    if (!values) {
      LogUtils.e(TAG, '[batchInsertFav] The addParams of parameter is NULL');
      return Promise.resolve(-1);
    }
    let rdbStore = await this.getRdbStore(context);
    if (rdbStore == null) {
      return -1;
    }
    try {
      return await RdbStoreUtils.batchInsertWithConflictResolution(rdbStore, DbConstants.TABLE_FAVORITE_INFO, values,
        dataRdb.ConflictResolution.ON_CONFLICT_REPLACE);
    } catch (e) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `batchInsertFav code:${e.code} msg:${e.message}`);
      LogUtils.e(TAG, 'batchInsertFav: ' + JSON.stringify(e));
      return -1;
    }
  }

  public async insertPdu(context: common.Context, values: ValuesBucket): Promise<number> {
    if (!values) {
      LogUtils.e(TAG, '[insertPdu] The addParams of parameter is NULL');
      return Promise.resolve(-1);
    }
    let pduPartRdbStore = await this.getRdbStore(context);
    if (pduPartRdbStore == null) {
      return -1;
    }
    try {
      return await RdbStoreUtils.insert(pduPartRdbStore, DbConstants.TABLE_MMS_PDU, values);
    } catch (e) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `insertPdu code:${e.code} msg:${e.message}`);
      LogUtils.e(TAG, 'insertPdu: ' + JSON.stringify(e));
      return -1;
    }
  }

  public async insertProtocol(context: common.Context, values: ValuesBucket): Promise<number> {
    if (!values) {
      LogUtils.e(TAG, '[insertProtocol] The addParams of parameter is NULL');
      return Promise.resolve(-1);
    }
    let protocolPartRdbStore = await this.getRdbStore(context);
    if (protocolPartRdbStore == null) {
      return -1;
    }
    try {
      return await RdbStoreUtils.insert(protocolPartRdbStore, DbConstants.TABLE_MMS_PROTOCOL, values);
    } catch (e) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `insertProtocol code:${e.code} msg:${e.message}`);
      LogUtils.e(TAG, 'insertProtocol: ' + JSON.stringify(e));
      return -1;
    }
  }

  public async insertRcs(context: common.Context, values: ValuesBucket): Promise<number> {
    if (!values) {
      LogUtils.e(TAG, '[insertRcs] The addParams of parameter is NULL');
      return Promise.resolve(-1);
    }
    let rcsInfoPartRdbStore = await this.getRdbStore(context);
    if (rcsInfoPartRdbStore == null) {
      return -1;
    }
    try {
      return await RdbStoreUtils.insert(rcsInfoPartRdbStore, DbConstants.TABLE_RCS_INFO, values);
    } catch (e) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `insertRcs code:${e.code} msg:${e.message}`);
      LogUtils.e(TAG, 'insertRcs: ' + JSON.stringify(e));
      return -1;
    }
  }

  public async insertSubsection(context: common.Context, values: ValuesBucket): Promise<number> {
    if (!values) {
      LogUtils.e(TAG, '[insertSubsection] The addParams of parameter is NULL');
      return Promise.resolve(-1);
    }
    let subsectionPartRdbStore = await this.getRdbStore(context);
    if (subsectionPartRdbStore == null) {
      return -1;
    }
    try {
      return await RdbStoreUtils.insert(subsectionPartRdbStore, DbConstants.TABLE_SMS_SUBSECTION, values);
    } catch (e) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `insertSubsection code:${e.code} msg:${e.message}`);
      LogUtils.e(TAG, 'insertSubsection: ' + JSON.stringify(e));
      return -1;
    }
  }

  public async insertFavorate(context: common.Context, values: ValuesBucket): Promise<number> {
    if (!values) {
      LogUtils.e(TAG, '[insertFavorate] The addParams of parameter is NULL');
      return Promise.resolve(-1);
    }
    let smsRdbStore = await this.getRdbStore(context);
    if (smsRdbStore == null) {
      return -1;
    }
    try {
      return await RdbStoreUtils.insert(smsRdbStore, DbConstants.TABLE_FAVORITE_INFO, values);
    } catch (e) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `insertFavorate code:${e.code} msg:${e.message}`);
      LogUtils.e(TAG, 'insertFavorate error: ' + JSON.stringify(e));
      return -1;
    }
  }

  public async queryPart(context: common.Context): Promise<rdb.ResultSet | undefined> {
    try {
      let mmsPartRdbStore = await this.getRdbStore(context);
      if (mmsPartRdbStore == null) {
        return undefined;
      }
      let predicates = new rdb.RdbPredicates(DbConstants.TABLE_MMS_PART);
      return await mmsPartRdbStore.query(predicates);
    } catch {
      LogUtils.w(TAG, 'No data exists in the part table.');
      return undefined;
    }
  }

  public async queryMmsPart(context: common.Context): Promise<HashMap<string, number>> {
    let mmsPartRdbStore = await this.getRdbStore(context);
    let predicates = new rdb.RdbPredicates(DbConstants.TABLE_MMS_PART);
    let mmsPartMap = new HashMap<string, number>();
    if (mmsPartRdbStore === undefined) {
      return mmsPartMap;
    }
    try {
      let resultSet: rdb.ResultSet =
        (mmsPartRdbStore as rdb.RdbStore).querySync(predicates,
          ['part_size', 'content', 'location_path', 'msg_id', 'rcs_id']);
        while (resultSet.goToNextRow()) {
          const msgId = resultSet.getLong(resultSet.getColumnIndex('msg_id'));
          const locationPath = resultSet.getString(resultSet.getColumnIndex('location_path'));
          const partSize = resultSet.getString(resultSet.getColumnIndex('part_size'));
          const content = resultSet.getString(resultSet.getColumnIndex('content'));
          const rcsId = resultSet.getLong(resultSet.getColumnIndex('rcs_id'));
          let key = partSize + ':' + locationPath + ':' + content + ':' + rcsId;
          mmsPartMap.set(key, msgId);
      }
      // 释放数据集的内存
      resultSet.close();
    } catch (err) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `queryMmsPart code:${err.code} msg:${err.message}`);
      LogUtils.e(TAG, `Query failed, code is ${err.code},message is ${err.message}`);
    }
    return mmsPartMap;
  }

  public async queryRcs(context: common.Context): Promise<HashMap<string, number>> {
    let rcsInfoRdbStore = await this.getRdbStore(context);
    let predicates = new rdb.RdbPredicates(DbConstants.TABLE_RCS_INFO);
    let rcsMap = new HashMap<string, number>();
    if (rcsInfoRdbStore != undefined) {
      try {
        let resultSet: rdb.ResultSet =
          (rcsInfoRdbStore as rdb.RdbStore).querySync(predicates, ['receiver_number', 'sender_number', 'start_time', 'end_time', 'rcs_id']);
        while (resultSet.goToNextRow()) {
          const receiverNumber = resultSet.getString(resultSet.getColumnIndex('receiver_number'));
          const sendNumber = resultSet.getString(resultSet.getColumnIndex('sender_number'));
          const startTime = resultSet.getString(resultSet.getColumnIndex('start_time'));
          const endTime = resultSet.getString(resultSet.getColumnIndex('end_time'));
          const rcsId = resultSet.getLong(resultSet.getColumnIndex('rcs_id'));
          rcsMap.set(receiverNumber + ':' + sendNumber + ':' + startTime, rcsId);
        }
        // 释放数据集的内存
        resultSet.close();
      } catch (err) {
        const backupRestoreResult = BackupRestoreResult.getInstance();
        backupRestoreResult.insertOrUpdateBackupInfo('sms',
          `queryRcs code:${err.code} msg:${err.message}`);
        LogUtils.e(TAG, `Query failed, code is ${err.code},message is ${err.message}`);
      }
    }
    return rcsMap;
  }

  public async querySessionIdMap(context: common.Context): Promise<HashMap<string, number>> {
    let mmsPartRdbStore = await this.getRdbStore(context);
    let predicates = new rdb.RdbPredicates(DbConstants.TABLE_SESSION);
    let sessionIdMap = new HashMap<string, number>();
    if (mmsPartRdbStore != undefined) {
      try {
        let resultSet: rdb.ResultSet =
          (mmsPartRdbStore as rdb.RdbStore).querySync(predicates, ['telephone', 'id']);
        LogUtils.i(TAG, `ResultSet column names: ${resultSet.columnNames}, column count: ${resultSet.columnCount}`);
        LogUtils.i(TAG, `ResultSet rowCount: ${resultSet.rowCount}`);
        // resultSet是一个数据集合的游标，默认指向第-1个记录，有效的数据从0开始。
        while (resultSet.goToNextRow()) {
          const telephone = resultSet.getString(resultSet.getColumnIndex('telephone'));
          const id = resultSet.getLong(resultSet.getColumnIndex('id'));
          sessionIdMap.set(telephone, id);
        }
        // 释放数据集的内存
        resultSet.close();
      } catch (err) {
        const backupRestoreResult = BackupRestoreResult.getInstance();
        backupRestoreResult.insertOrUpdateBackupInfo('sms',
          `querySessionIdMap code:${err.code} msg:${err.message}`);
        LogUtils.e(TAG, `Query failed, code is ${err.code},message is ${err.message}`);
      }
    }
    return sessionIdMap;
  }

  public async querySession(context: common.Context): Promise<HashMap<string, ValuesBucket>> {
    let mmsPartRdbStore = await this.getRdbStore(context);
    let predicates = new rdb.RdbPredicates(DbConstants.TABLE_SESSION);
    let sessionMap = new HashMap<string, ValuesBucket>();
    if (mmsPartRdbStore != undefined) {
      try {
        let queryColumn = await this.queryColumns(context, DbConstants.TABLE_SESSION);
        let resultSet: rdb.ResultSet =
          (mmsPartRdbStore as rdb.RdbStore).querySync(predicates, queryColumn);
        LogUtils.i(TAG, `ResultSet column names: ${resultSet.columnNames}, column count: ${resultSet.columnCount}`);
        LogUtils.i(TAG, `ResultSet rowCount: ${resultSet.rowCount}`);
        // resultSet是一个数据集合的游标，默认指向第-1个记录，有效的数据从0开始。
        while (resultSet.goToNextRow()) {
          const time = resultSet.getLong(resultSet.getColumnIndex('time'));
          const telephone = resultSet.getString(resultSet.getColumnIndex('telephone'));
          const content = resultSet.getString(resultSet.getColumnIndex('content'));
          const id = resultSet.getLong(resultSet.getColumnIndex('id'));
          const hasMms = resultSet.getLong(resultSet.getColumnIndex('has_mms'));
          const hasDraft = resultSet.getLong(resultSet.getColumnIndex('has_draft'));
          const pinningTime = resultSet.getLong(resultSet.getColumnIndex('pinning_time'));
          let session: ValuesBucket = new SessionMapItem().setTelephone(telephone).setId(id).setTime(time)
            .setContent(content).setHasMms(hasMms).setHasDraft(hasDraft).setPinningTime(pinningTime)
            .createValuesBucket();
          sessionMap.set(telephone, session);
        }
        // 释放数据集的内存
        resultSet.close();
      } catch (err) {
        const backupRestoreResult = BackupRestoreResult.getInstance();
        backupRestoreResult.insertOrUpdateBackupInfo('sms',
          `querySession code:${err.code} msg:${err.message}`);
        LogUtils.e(TAG, `Query failed, code is ${err.code},message is ${err.message}`);
      }
    }
    return sessionMap;
  }

  private async queryColumns(context: common.Context, table: String): Promise<string[]> {
    let rdbStore = await this.getRdbStore(context);
    if (rdbStore === undefined) {
      LogUtils.w(TAG, `queryColumns, getRdbStore failed`)
      return [];
    }
    let columns: string[] = [];
    try {
      let conditions: string = 'SELECT name FROM pragma_table_info(\'' + table + '\')';
      let resultSet: rdb.ResultSet = (rdbStore as rdb.RdbStore).querySqlSync(conditions);
      while (resultSet.goToNextRow()) {
        const name = resultSet.getString(resultSet.getColumnIndex('name'));
        columns.push(name);
      }
      return columns;
    } catch (err) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `queryColumns code:${err.code} msg:${err.message}`);
      LogUtils.e(TAG, `queryColumns failed, code is ${err.code},message is ${err.message}`);
    }
    return [];
  }

  public async querySessionTime(context: common.Context, timeMap: Map<number, number>, statusMap: Map<number, number>,
    contentMap: Map<number, string>) {
    let mmsPartRdbStore = await this.getRdbStore(context);
    let predicates = new rdb.RdbPredicates(DbConstants.TABLE_SESSION);
    if (mmsPartRdbStore != undefined) {
      try {
        let resultSet: rdb.ResultSet =
          (mmsPartRdbStore as rdb.RdbStore).querySync(predicates, ['time', 'sending_status', 'content', 'id']);
        LogUtils.i(TAG, `querySessionTime ResultSet column names:
          ${resultSet.columnNames}, column count: ${resultSet.columnCount}`);
        LogUtils.i(TAG, `querySessionTime ResultSet rowCount: ${resultSet.rowCount}`);
        // resultSet是一个数据集合的游标，默认指向第-1个记录，有效的数据从0开始。
        while (resultSet.goToNextRow()) {
          const time = resultSet.getLong(resultSet.getColumnIndex('time'));
          const content = resultSet.getString(resultSet.getColumnIndex('content'));
          const id = resultSet.getLong(resultSet.getColumnIndex('id'));
          const status = resultSet.getLong(resultSet.getColumnIndex('sending_status'));
          timeMap.set(id, time);
          statusMap.set(id, status);
          contentMap.set(id, content);
        }
        // 释放数据集的内存
        resultSet.close();
      } catch (err) {
        const backupRestoreResult = BackupRestoreResult.getInstance();
        backupRestoreResult.insertOrUpdateBackupInfo('sms',
          `querySessionTime code:${err.code} msg:${err.message}`);
        LogUtils.e(TAG, `Query failed, code is ${err.code},message is ${err.message}`);
      }
    }
  }

  public async queryCollectSmsInfo(context: common.Context, offset: number): Promise<rdb.ResultSet> {
    let rdbStore = await this.getRdbStore(context);
    let conditions: string = 'SELECT name FROM pragma_table_info(\'sms_mms_info\')';
    let resultSetColumn: rdb.ResultSet = (rdbStore as rdb.RdbStore).querySqlSync(conditions);
    let columns: string[] = [];
    while (resultSetColumn.goToNextRow()) {
      const name = resultSetColumn.getString(resultSetColumn.getColumnIndex('name'));
      columns.push(name);
    }
    resultSetColumn.close();
    let predicates = new rdb.RdbPredicates(DbConstants.TABLE_SMS_MMS_INFO);
    predicates.equalTo('is_collect', 1);
    predicates.limitAs(DbConstants.QUERY_PAGE_NUM);
    predicates.offsetAs(offset);
    let resultSet: rdb.ResultSet =
      (rdbStore as rdb.RdbStore).querySync(predicates, columns);
    return resultSet;
  }

  public async queryData(context: common.Context, predicates: rdb.RdbPredicates,
    columns: string[]): Promise<rdb.ResultSet> {
    let rdbStore = await this.getRdbStore(context);
    let resultSet: rdb.ResultSet =
      (rdbStore as rdb.RdbStore).querySync(predicates, columns);
    return resultSet;
  }

  public async querySmsInfo(context: common.Context): Promise<HashMap<string, string>> {
    LogUtils.i(TAG, `smsInfo start`);
    let mmsPartRdbStore = await this.getRdbStore(context);
    let predicates = new rdb.RdbPredicates(DbConstants.TABLE_SMS_MMS_INFO);
    let smsInfoMap = new HashMap<string, string>();
    if (mmsPartRdbStore != undefined) {
      try {
        let resultSet: rdb.ResultSet =
          (mmsPartRdbStore as rdb.RdbStore).querySync(predicates, ['msg_id', 'receiver_number', 'sender_number',
            'start_time', 'msg_content', 'is_sender']);
        LogUtils.i(TAG, `ResultSet column names: ${resultSet.columnNames}, column count: ${resultSet.columnCount}`);
        // resultSet是一个数据集合的游标，默认指向第-1个记录，有效的数据从0开始。
        while (resultSet.goToNextRow()) {
          const msgId = resultSet.getString(resultSet.getColumnIndex('msg_id'));
          const receiverNumber = resultSet.getString(resultSet.getColumnIndex('receiver_number'));
          const senderNumber = resultSet.getString(resultSet.getColumnIndex('sender_number'));
          const msgContent = resultSet.getString(resultSet.getColumnIndex('msg_content'));
          const startTime = resultSet.getString(resultSet.getColumnIndex('start_time'));
          const isSender = resultSet.getString(resultSet.getColumnIndex('is_sender'))
          let smsInfo: LooseObject = {
            'msgId': msgId,
            'receiverNumber': receiverNumber,
            'senderNumber': senderNumber,
            'msgContent': msgContent,
            'startTime': startTime,
          }
          let number = '';
          if (!StringUtil.isEmpty(receiverNumber)) {
            number = receiverNumber;
          } else if (!StringUtil.isEmpty(senderNumber)) {
            number = senderNumber;
          }
          smsInfoMap.set(number + ':' + isSender + ':' + startTime, msgId);
        }
        // 释放数据集的内存
        resultSet.close();
      } catch (err) {
        const backupRestoreResult = BackupRestoreResult.getInstance();
        backupRestoreResult.insertOrUpdateBackupInfo('sms',
          `querySmsInfo code:${err.code} msg:${err.message}`);
        LogUtils.e(TAG, `Query failed, code is ${err.code},message is ${err.message}`);
      }
    }
    return smsInfoMap;
  }

  public async queryDraftInfo(context: common.Context): Promise<rdb.ResultSet | undefined> {
    LogUtils.i(TAG, `queryDraftInfo start`);
    let mmsPartRdbStore = await this.getRdbStore(context);
    if (!mmsPartRdbStore) {
      LogUtils.e(TAG, 'queryDraftInfo Error: Invalid store!');
      return undefined;
    }
    try {
      let predicates = new rdb.RdbPredicates(DbConstants.TABLE_SMS_MMS_INFO);
      predicates.equalTo(NextOldDBData.smsMmsInfo.msgState, 3); // msgState: 3 表示草稿
      let resultSet: rdb.ResultSet = mmsPartRdbStore.querySync(predicates,
        [NextOldDBData.smsMmsInfo.startTime, NextOldDBData.smsMmsInfo.sessionId]);
      LogUtils.i(TAG, `queryDraftInfo resultSet size: ${resultSet.rowCount}`);
      return resultSet;
    } catch (err) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `queryDraftInfo code:${err.code} msg:${err.message}`);
      LogUtils.e(TAG, `queryDraftInfo failed, code is ${err.code},message is ${err.message}`);
    }
    return undefined;
  }

  public async queryFavoriteInfo(context: common.Context): Promise<HashMap<string, number>> {
    LogUtils.i(TAG, `favorateInfo start`);
    let rdbStore = await this.getRdbStore(context);
    let predicates = new rdb.RdbPredicates(DbConstants.TABLE_FAVORITE_INFO);
    let favoriteInfoMap = new HashMap<string, number>();
    if (rdbStore != undefined) {
      try {
        let resultSet: rdb.ResultSet =
          (rdbStore as rdb.RdbStore).querySync(predicates, ['msg_id', 'receiver_number', 'sender_number', 'is_sender',
            'start_time']);
        LogUtils.i(TAG, `ResultSet favorate column names: ${resultSet.columnNames}, column count: ${resultSet.columnCount}`);
        while (resultSet.goToNextRow()) {
          const msgId = resultSet.getLong(resultSet.getColumnIndex('msg_id'));
          const receiverNumber = resultSet.getString(resultSet.getColumnIndex('receiver_number'));
          const senderNumber = resultSet.getString(resultSet.getColumnIndex('sender_number'));
          const isSender = resultSet.getString(resultSet.getColumnIndex('is_sender'));
          const startTime = resultSet.getString(resultSet.getColumnIndex('start_time'));
          let number = '';
          if (!StringUtil.isEmpty(receiverNumber)) {
            number = receiverNumber;
          } else if (!StringUtil.isEmpty(senderNumber)) {
            number = senderNumber;
          }
          favoriteInfoMap.set(number + ':' + isSender + ':' + startTime, msgId);
        }
        // 释放数据集的内存
        resultSet.close();
      } catch (err) {
        const backupRestoreResult = BackupRestoreResult.getInstance();
        backupRestoreResult.insertOrUpdateBackupInfo('sms',
          `queryFavorateInfo code:${err.code} msg:${err.message}`);
        LogUtils.e(TAG, ` queryFavorateInfo, code is ${err.code},message is ${err.message}`);
      }
    }
    return favoriteInfoMap;
  }

  public async queryMaxGroupId(context: common.Context): Promise<number> {
    LogUtils.i(TAG, `queryMaxGroupId start`)
    let mmsPartRdbStore = await this.getRdbStore(context);
    let predicates = new rdb.RdbPredicates(DbConstants.TABLE_SMS_MMS_INFO);
    let maxGroupId = 0;
    if (mmsPartRdbStore != undefined) {
      try {
        let resultSet: rdb.ResultSet =
          (mmsPartRdbStore as rdb.RdbStore).querySqlSync('SELECT MAX(group_id) as maxGroupId FROM sms_mms_info');
        LogUtils.i(TAG, `ResultSet column names: ${resultSet.columnNames}, column count: ${resultSet.columnCount}`);
        // resultSet是一个数据集合的游标，默认指向第-1个记录，有效的数据从0开始。
        while (resultSet.goToNextRow()) {
          const maxGroupIdDb = resultSet.getLong(resultSet.getColumnIndex('maxGroupId'));
          maxGroupId = maxGroupIdDb > 0 ? maxGroupIdDb : 0;
          LogUtils.i(TAG, `maxGroupIdDb=${maxGroupIdDb}, maxGroupId=${maxGroupId}`);
        }
        // 释放数据集的内存
        resultSet.close();
      } catch (err) {
        const backupRestoreResult = BackupRestoreResult.getInstance();
        backupRestoreResult.insertOrUpdateBackupInfo('sms',
          `queryMaxGroupId code:${err.code} msg:${err.message}`);
        LogUtils.e(TAG, `Query failed, code is ${err.code},message is ${err.message}`);
      }
    }
    return maxGroupId;
  }

  private async updateSessionSlice(context: common.Context, progress: BackupRestoreProgress,
      batch: number[]): Promise<void> {
    try {
      let rdbStore = await this.getRdbStore(context);
      if (rdbStore === undefined) {
        LogUtils.e(TAG, `process update session failed : rdbStore === undefined`);
        return;
      }
      // 从 sms_mms_info 表中返回满足条件的消息的 session_id 和 msg_state
      // 条件：sms_mms_info.session_id 在 数组A 中
      // sms_mms_info.session_id 相同的所有记录中 start_time 最大的值
      let resultSet: rdb.ResultSet =
        (rdbStore as rdb.RdbStore).querySqlSync('SELECT \n' +
          '    session_id,\n' +
          '    msg_state\n' +
          'FROM (\n' +
          '    SELECT \n' +
          '        session_id,\n' +
          '        msg_state,\n' +
          '        ROW_NUMBER() OVER(PARTITION BY session_id ORDER BY start_time DESC) as resultNum\n' +
          '    FROM \n' +
          '        sms_mms_info\n' +
          '    WHERE \n' +
          '        session_id IN (' + batch + ')\n' +
          ') tb\n' +
          'WHERE \n' +
          '    resultNum = 1;');
      while (resultSet.goToNextRow()) {
        let id = resultSet.getLong(resultSet.getColumnIndex('session_id'));
        let msgState = resultSet.getLong(resultSet.getColumnIndex('msg_state'));
        (rdbStore as rdb.RdbStore).executeSync('UPDATE session SET sending_status = ' +
          msgState + ' WHERE id = ' + id);
        progress.updateProcessCount(progress.getProcessCount() + 1);
      }
      resultSet.close();
    } catch (err) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `updateSessionSlice code:${err.code} msg:${err.message}`);
      LogUtils.e(TAG, `Query failed, code is ${err.code},message is ${err.message}`);
    }
  }

  public async processUpdateSession(context: common.Context, progress: BackupRestoreProgress,
      sessionIds: Set<number>): Promise<void> {
    LogUtils.i(TAG, 'process update session count ' + sessionIds.size);
    if (sessionIds.size <= 0) {
      return;
    }
    let numbersArray = Array.from(sessionIds);
    let batchCount = Math.ceil(sessionIds.size / 100);
    // 分页处理
    for (let page = 0; page < batchCount; page ++) {
      let start = page * 100;
      let end = start + 100;
      let batch = numbersArray.slice(start, end);
      await this.updateSessionSlice(context, progress, batch);
    }
  }

  public async querySessionUnreadCount(context: common.Context): Promise<number> {
    LogUtils.i(TAG, 'querySessionUnreadCount');
    let mmsPartRdbStore = await this.getRdbStore(context);
    let predicates = new rdb.RdbPredicates(DbConstants.TABLE_SESSION);
    let count: number = 0;
    if (mmsPartRdbStore != undefined) {
      try {
        let resultSet: rdb.ResultSet =
          (mmsPartRdbStore as rdb.RdbStore).querySqlSync('SELECT COUNT(*) as count FROM (SELECT session_id, SUM(unread_count) AS session_unread\n' +
            'FROM (\n' +
            '    SELECT session_id, COUNT(*) AS unread_count FROM sms_mms_info WHERE is_read = 0 AND sms_type != 99 GROUP BY session_id\n' +
            '    UNION ALL\n' +
            '    SELECT session_id, COUNT(*) AS unread_count FROM rcs_info WHERE is_read = 0 GROUP BY session_id\n' +
            ') AS unread_messages\n' +
            'GROUP BY session_id)');
        LogUtils.i(TAG, `ResultSet column names: ${resultSet.columnNames}, column rowCount: ${resultSet.rowCount}`);
        // resultSet是一个数据集合的游标，默认指向第-1个记录，有效的数据从0开始。
        while (resultSet.goToNextRow()) {
           count = resultSet.getLong(resultSet.getColumnIndex('count'));
        }
        // 释放数据集的内存
        resultSet.close();
      } catch (err) {
        const backupRestoreResult = BackupRestoreResult.getInstance();
        backupRestoreResult.insertOrUpdateBackupInfo('sms',
          `querySessionUnreadCount code:${err.code} msg:${err.message}`);
        LogUtils.e(TAG, `Query failed, code is ${err.code},message is ${err.message}`);
      }
    }
    return count;
  }

  public async querySessionUnread(context: common.Context, index: number): Promise<HashMap<number, number>> {
    LogUtils.i(TAG, 'querySessionUnread');
    let mmsPartRdbStore = await this.getRdbStore(context);
    let predicates = new rdb.RdbPredicates(DbConstants.TABLE_SESSION);
    let sessionUnreadMap = new HashMap<number, number>();
    if (mmsPartRdbStore != undefined) {
      try {
        let resultSet: rdb.ResultSet =
          (mmsPartRdbStore as rdb.RdbStore).querySqlSync('SELECT session_id, SUM(unread_count) AS session_unread\n' +
            'FROM (\n' +
            '    SELECT session_id, COUNT(*) AS unread_count FROM sms_mms_info WHERE is_read = 0 AND sms_type != 99 GROUP BY session_id\n' +
            '    UNION ALL\n' +
            '    SELECT session_id, COUNT(*) AS unread_count FROM rcs_info WHERE is_read = 0 GROUP BY session_id\n' +
            ') AS unread_messages\n' +
            'GROUP BY session_id limit 1000 offset ' + index);
        LogUtils.i(TAG, `ResultSet column names: ${resultSet.columnNames}, column rowCount: ${resultSet.rowCount}`);
        // resultSet是一个数据集合的游标，默认指向第-1个记录，有效的数据从0开始。
        while (resultSet.goToNextRow()) {
          const sessionId = resultSet.getLong(resultSet.getColumnIndex('session_id'));
          const count = resultSet.getLong(resultSet.getColumnIndex('session_unread'));
          sessionUnreadMap.set(sessionId, count);
        }
        // 释放数据集的内存
        resultSet.close();
      } catch (err) {
        const backupRestoreResult = BackupRestoreResult.getInstance();
        backupRestoreResult.insertOrUpdateBackupInfo('sms',
          `querySessionUnread code:${err.code} msg:${err.message}`);
        LogUtils.e(TAG, `Query failed, code is ${err.code},message is ${err.message}`);
      }
    }
    return sessionUnreadMap;
  }

  public async updateSessionMsgCount(context: common.Context, progress: BackupRestoreProgress) : Promise<void> {
    LogUtils.i(TAG, 'updateSessionMsgCount');
    let rdbStore = await this.getRdbStore(context);
    if (rdbStore === undefined) {
      LogUtils.i(TAG, 'updateSessionMsgCount rdbStore is null');
      return;
    }
    let sql: string = 'SELECT session.id AS session_id,' +
      ' session.message_count AS session_msg_count,' +
      ' COUNT(DISTINCT sms_mms_info.group_id) AS sms_group_count' +
      ' FROM session LEFT JOIN sms_mms_info' +
      ' ON session.id = sms_mms_info.session_id AND sms_mms_info.msg_state != 3' +
      ' GROUP BY session.id;'
    try {
      let resultSet: rdb.ResultSet = await (rdbStore as rdb.RdbStore).querySql(sql);
      while (resultSet.goToNextRow()) {
        let sessionId = resultSet.getLong(resultSet.getColumnIndex('session_id'));
        let sessionMsgCount = resultSet.getLong(resultSet.getColumnIndex('session_msg_count'));
        let smsGroupCount = resultSet.getLong(resultSet.getColumnIndex('sms_group_count'));
        if (sessionMsgCount != smsGroupCount) {
          (rdbStore as rdb.RdbStore).executeSync(`update session set message_count = ${smsGroupCount}
          where id = ${sessionId};`);
          LogUtils.i(TAG, `updateSessionMsgCount sessionId: ${sessionId} count: ${smsGroupCount}}`);
        }
        progress.updateProcessCount(progress.getProcessCount() + 1);
      }
      resultSet.close();
    } catch (err) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `updateSessionMsgCount code:${err.code} msg:${err.message}`);
      LogUtils.e(TAG, `update session count failed, code is ${err.code}, message is ${err.message}`);
    }
  }

  public async checkSmsInfoIsValid(context: common.Context, progress: BackupRestoreProgress) : Promise<void> {
    try {
      let rdbStore = await this.getRdbStore(context);
      if (rdbStore === undefined) {
        LogUtils.e(TAG, `check session invalid failed : rdbStore === undefined`);
        progress.updateProcessCount(progress.getProcessCount() + progress.getCheckSmsCount());
        return;
      }
      let resultSet: rdb.ResultSet =
        (rdbStore as rdb.RdbStore).querySqlSync('SELECT msg_id\n' +
          'FROM sms_mms_info WHERE ((session_id NOT IN ( SELECT id FROM session )) AND (is_blocked = 0));\n');
      if (resultSet !== undefined && resultSet.rowCount > 0) {
        const backupRestoreResult = BackupRestoreResult.getInstance();
        backupRestoreResult.insertOrUpdateBackupInfo('sms',
          `checkNewDb invalid msgid :${resultSet.rowCount}`);
      }
      while (resultSet.goToNextRow()) {
        let msgId = resultSet.getLong(resultSet.getColumnIndex('msg_id'));
        LogUtils.e(TAG, `invalid msg_id ${msgId}`);
      }
      resultSet.close();
    } catch (err) {
      LogUtils.e(TAG, `check session invalid failed, code is ${err.code},message is ${err.message}`);
    }
    progress.updateProcessCount(progress.getProcessCount() + progress.getCheckSmsCount());
  }

  public async queryBlockedMsgCount(context: Context): Promise<number> {
    if (context == null) {
      LogUtils.w(TAG, `queryBlockedMsgCount failed by null context`)
      return Promise.resolve(0);
    }
    let totalBlockedCount: number = 0;
    try {
      let rdbStore = await this.getRdbStore(context);
      if (rdbStore == null) {
        LogUtils.e(TAG, `check session invalid failed : rdbStore === undefined`);
        return 0;
      }
      let sql: string = 'SELECT \n' +
        '(SELECT COUNT(*) FROM sms_mms_info WHERE is_blocked = 1) AS total_blocked;'
      let resultSet: rdb.ResultSet = (rdbStore as rdb.RdbStore).querySqlSync(sql);
      if (resultSet === undefined) {
        LogUtils.w(TAG, 'resultSet === undefined');
        return Promise.resolve(0);
      }
      while (resultSet.goToNextRow()) {
        totalBlockedCount = resultSet.getLong(resultSet.getColumnIndex('total_blocked'));
      }
      resultSet.close();
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `clone_blocked_count :${totalBlockedCount}`);
      return totalBlockedCount;
    } catch (err) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `queryBlockedMsgCount code:${err.code} msg:${err.message}`);
      LogUtils.e(TAG, `queryBlockedMsgCount failed, code is ${err.code},message is ${err.message}`);
      return Promise.resolve(0);
    }
  }

  public async checkSessionIsValid(context: common.Context, progress: BackupRestoreProgress) : Promise<void> {
    try {
      let rdbStore = await this.getRdbStore(context);
      if (rdbStore === undefined) {
        LogUtils.e(TAG, `check session invalid failed : rdbStore === undefined`);
        progress.updateProcessCount(progress.getProcessCount() + progress.getCheckSessionCount());
        return;
      }
      let resultSet: rdb.ResultSet =
        (rdbStore as rdb.RdbStore).querySqlSync('SELECT id\n' +
          'FROM session\n' +
          'WHERE id NOT IN (\n' +
          '    SELECT session_id\n' +
          '    FROM sms_mms_info\n' +
          ');');
      if (resultSet !== undefined && resultSet.rowCount > 0) {
        const backupRestoreResult = BackupRestoreResult.getInstance();
        backupRestoreResult.insertOrUpdateBackupInfo('sms',
          `checkNewDb invalid sessionId :${resultSet.rowCount}`);
      }
      while (resultSet.goToNextRow()) {
        let id = resultSet.getLong(resultSet.getColumnIndex('id'));
        LogUtils.e(TAG, `invalid ses id ${id}`);
      }
      resultSet.close();
    } catch (err) {
      LogUtils.e(TAG, `check session invalid failed, code is ${err.code},message is ${err.message}`);
    }
    progress.updateProcessCount(progress.getProcessCount() + progress.getCheckSessionCount());
  }

  public async deleteSip(context: common.Context) {
    let promise1 = await this.deleteSipSms(context);
    let promise2 = await this.deleteSipRcs(context);
    let promise3 = await this.deleteSipSession(context);
    await Promise.allSettled([promise1, promise2, promise3]).then(() => {
      LogUtils.i(TAG, `deleteSip success`);
    });
  }

  public async recountAndUpdateUnread(context: common.Context, progress: BackupRestoreProgress): Promise<void> {
    let unreadCount: number = 0;
    await this.resetSessionUnread(context);
    let count = await this.querySessionUnreadCount(context);
    LogUtils.i(TAG, `unreadCount: ${count}`)
    let threadsPage = Math.ceil(count / 1000);
    for (let curPage = 1; curPage <= threadsPage; curPage++) {
      unreadCount = unreadCount + await this.processPage(context, curPage, progress);
    }
    const backupRestoreResult = BackupRestoreResult.getInstance();
    backupRestoreResult.insertOrUpdateBackupInfo('sms',
      `clone_unread_count :${unreadCount}`);
  }

  private async processPage(context: common.Context, curPage: number,
    progress: BackupRestoreProgress): Promise<number> {
    let sessionUnreadMap = await this.querySessionUnread(context, 1000 * (curPage - 1));
    let unreadCount: number = 0;
    for (let key of sessionUnreadMap.keys()) {
      let value = sessionUnreadMap.get(key);
      unreadCount = unreadCount + value;
      let valueBucket: ValuesBucket = {
        "unread_count": value
      };
      await this.updateSession(context, key, valueBucket);
      progress.updateProcessCount(progress.getProcessCount() + 1);
    }
    return unreadCount;
  }

  public async updatePart(context: common.Context, partId: number, values: ValuesBucket): Promise<number> {
    if (!values) {
      LogUtils.e(TAG, '[updatePart] The values is NULL');
      return Promise.resolve(-1);
    }
    let mmsPartRdbStore = await this.getRdbStore(context);
    if (mmsPartRdbStore == null) {
      return -1;
    }
    try {
      let predicates = new rdb.RdbPredicates(DbConstants.TABLE_MMS_PART);
      predicates.equalTo('id', partId);
      return await RdbStoreUtils.update(mmsPartRdbStore, values, predicates);
    } catch (e) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `updatePart code:${e.code} msg:${e.message}`);
      LogUtils.e(TAG, 'updatePart: ' + JSON.stringify(e));
      return -1;
    }
  }

  public async batchUpdatePart(context: common.Context, partIds: number[], values: ValuesBucket): Promise<number> {
    if (!values) {
      LogUtils.e(TAG, '[batchUpdatePart] The values is NULL');
      return Promise.resolve(-1);
    }
    let mmsPartRdbStore = await this.getRdbStore(context);
    if (mmsPartRdbStore == null) {
      LogUtils.e(TAG, '[batchUpdatePart] rdbStore is NULL');
      return -1;
    }
    try {
      let predicates = new rdb.RdbPredicates(DbConstants.TABLE_MMS_PART);
      predicates.in('id', partIds);
      return await RdbStoreUtils.update(mmsPartRdbStore, values, predicates);
    } catch (e) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `batchUpdatePart code:${e.code} msg:${e.message}`);
      LogUtils.e(TAG, 'batchUpdatePart: ' + JSON.stringify(e));
      return -1;
    }
  }

  public async querySmsMmsDataCount(context: common.Context, countInfo: BackupCountInfo): Promise<number> {
    let count: number = await this.queryDataCount(context, DbConstants.NEXT_OLD_SMS_NAME,
      NextOldDBData.smsMmsInfo.msgId);
    countInfo.setTotalCount(count);
    return count;
  }

  public async queryDataCount(context: common.Context, tableName:string,
    queryKey:string, callBack?: Function): Promise<number> {
    if (context == null) {
      LogUtils.w(TAG, `queryDataCount failed by null context`)
      return Promise.resolve(0);
    }
    let count = 0;
    try {
      let predicatesInfo = new rdb.RdbPredicates(tableName);
      let rdbStore = await this.getRdbStore(context);
      if (rdbStore === undefined || rdbStore == null) {
        LogUtils.e(TAG, `[queryDataCount] failed : rdbStore === undefined`)
        if (callBack) {
          callBack(new ErrorUtils(400, 'rdbStore === undefined'));
        }
        return Promise.resolve(0);
      }
      let queryColumn = [`count(${queryKey})`];
      let resultSet = await rdbStore.query(predicatesInfo, queryColumn);
      if (resultSet === undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'query no threads found.');
        return Promise.resolve(0);
      }
      count = resultSet.getLong(0);
      LogUtils.i(TAG, '[queryDataCount] count : ' + count);
      resultSet.close();
    } catch (error) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `queryDataCount code:${error.code} msg:${error.message}`);
      LogUtils.e(TAG, '[queryDataCount] error : ' + JSON.stringify(error))
      if (callBack) {
        callBack(new ErrorUtils(400, JSON.stringify(error)));
      }
    }
    return Promise.resolve(count);
  }

  protected async getRdbStore(baseContext: common.Context): Promise<rdb.RdbStore | null> {
    if (!baseContext) {
      LogUtils.e(TAG, 'getRdbStore baseContext is null');
      return null;
    }
    if (this.rdbStore) {
      return this.rdbStore;
    }
    // backup 到EL2，需要补齐可能缺失的字段
    // 判断EL1是否存在并且EL2不存在。再备份。
    try {
      await this.BackupEl1ToEl2(baseContext);
    } catch (e) {
      LogUtils.e(TAG, `BackupEl1ToEl2 failed ${e?.message}`);
    }
    try {
      LogUtils.i(TAG, 'getRdbStore Start');
      if (baseContext.area !== contextConstant.AreaMode.EL2) {
        baseContext.area = contextConstant.AreaMode.EL2;
      }
      this.rdbStore = await rdb.getRdbStore(baseContext,
        {
          name: DbConstants.nextCloneDatabaseName,
          securityLevel: rdb.SecurityLevel.S2,
          isSearchable: true,
          customDir: '../../rdb'
        });
    } catch (e) {
      LogUtils.e(TAG, `getRdbStore error ${e?.message}`);
      return null;
    }
    if (!this.rdbStore) {
      LogUtils.e(TAG, 'initRdb failed');
      return null;
    }
    let rdbVersion = this.rdbStore.version
    LogUtils.i(TAG, 'getRdbStore rdbVersion = ' + rdbVersion);
    if (rdbVersion <= 0) {
      await this.initRdbStore();
    } else {
      await this.checkAndCompleteFields();
    }
    return this.rdbStore;
  }

  private async initRdbStore(): Promise<void> {
    if (!this.rdbStore) {
      LogUtils.e(TAG, 'initRdbStore rdb is null')
      return
    }
    LogUtils.i(TAG, 'initRdbStore1 Start');
    try {
      await this.executeSql(DbConstants.CREATE_SMS_MMS_INFO,
        'initRdbStore createSMSMMSTable succeed');
      await this.executeSql(DbConstants.CREATE_RCS_INFO,
        'initRdbStore createRCSInfoTable succeed');
      await this.executeSql(DbConstants.CREATE_SMS_SUBSECTION,
        'initRdbStore createSMSSubsectionTable succeed');
      await this.executeSql(DbConstants.CREATE_MMS_PROTOCOL,
        'initRdbStore createMMSProtocolTable succeed');
      await this.executeSql(DbConstants.CREATE_MMS_PART,
        'initRdbStore createMMSPartTable succeed');
      await this.executeSql(DbConstants.CREATE_SESSION,
        'initRdbStore createSessionTable succeed');
      await this.executeSql(DbConstants.CREATE_MMS_PDU,
        'initRdbStore createMMSPDUTable succeed');
      await this.executeSql(DbConstants.CREATE_FAVORITE_INFO,
        'initRdbStore createFavorateInfoTable succeed');
      await this.executeSql(DbConstants.CREATE_CHATBOTS,
        'initRdbStore createChatbotTable succeed');
      await this.executeSql(DbConstants.CREATE_SPECIFIC_CHATBOTS,
        'initRdbStore createSpChatbotTable succeed');
      this.rdbStore.version = CURRENT_RDB_VERSION
    } catch (e) {
      LogUtils.e(TAG, 'initRdbStore failed, code:' + (e as BusinessError)?.code +
        ', message: ' + (e as BusinessError)?.message);
    }
  }

  private async executeSql(sql: string, logSuccess?: string, bindArgs?: Array<ValueType>) {
    LogUtils.i(TAG, 'executeSql Start');
    await this.rdbStore?.executeSql(sql, bindArgs).then(() => {
      if (logSuccess != undefined) {
        LogUtils.i(TAG, logSuccess);
      }
    }).catch((err: BusinessError) => {
      LogUtils.e(TAG, 'Execute sql failed, err code: ' + err?.code + ', err message: ' + err?.message);
    });
  }

  private async checkAndCompleteFields() {
    if (this.rdbStore === undefined) {
      LogUtils.w(TAG, `checkAndCompleteFields, getRdbStore failed`)
      return;
    }
    try {
      if (!await this.isFieldExistInTable('sms_mms_info', 'blocked_pref_number')) {
        (this.rdbStore as rdb.RdbStore).executeSync('ALTER TABLE sms_mms_info ADD COLUMN blocked_pref_number ' +
          'TEXT DEFAULT \'\';');
      }
      if (!await this.isFieldExistInTable('rcs_info', 'blocked_pref_number')) {
        (this.rdbStore as rdb.RdbStore).executeSync('ALTER TABLE rcs_info ADD COLUMN blocked_pref_number ' +
          'TEXT DEFAULT \'\';');
      }
      if (!await this.isFieldExistInTable('session', 'blocked_type')) {
        (this.rdbStore as rdb.RdbStore).executeSync('ALTER TABLE session ADD COLUMN blocked_type INTEGER DEFAULT 0;');
      }
      if (!await this.isFieldExistInTable('mms_part', 'is_fraud')) {
        (this.rdbStore as rdb.RdbStore).executeSync('ALTER TABLE mms_part ADD COLUMN is_fraud INTEGER;');
      }
      if (!await this.isFieldExistInTable('mms_part', 'reference_count')) {
        (this.rdbStore as rdb.RdbStore).executeSync('ALTER TABLE mms_part ADD COLUMN reference_count INTEGER DEFAULT -1;');
      }
      if (!await this.isFieldExistInTable('session', 'pinning_time')) {
        (this.rdbStore as rdb.RdbStore).executeSync('ALTER TABLE session ADD COLUMN pinning_time INTEGER DEFAULT 0;');
      }
    } catch (error) {
      let err: BusinessError = error as BusinessError;
      LogUtils.e(TAG, 'checkAndCompleteFields failed, code:' + err.code + ', message: ' + err.message);
    }
  }

  private async isFieldExistInTable(table: String, field: String): Promise<boolean> {
    if (this.rdbStore === undefined) {
      LogUtils.w(TAG, `isFieldExistInTable, getRdbStore failed`)
      return false;
    }
    try {
      let conditions: string = 'SELECT name FROM pragma_table_info(\'' + table + '\') WHERE name = \'' + field + '\';';
      let resultSet: rdb.ResultSet = (this.rdbStore as rdb.RdbStore).querySqlSync(conditions);
      LogUtils.e(TAG, 'isFieldExistInTable, ' + resultSet.rowCount);
      if (resultSet === undefined || resultSet.rowCount == -1) {
        LogUtils.e(TAG, 'isFieldExistInTable, resultSet === undefined');
        throw new ErrorUtils(-1, 'isFieldExistInTable returned undefined result set.');
      }
      if (resultSet.rowCount > 0) {
        resultSet.close();
        return true;
      }
      LogUtils.i(TAG, `isFieldExistInTable, ${table} does not have column ${field}`);
      return false;
    } catch (err) {
      LogUtils.e(TAG, `isFieldExistInTable failed, code is ${err.code},message is ${err.message}`);
      throw new ErrorUtils(err.code, err.message);
    }
  }

  public getColumnIntegerValue(resultSet: rdb.ResultSet, columnName: string): number | undefined {
    if (resultSet.columnNames.indexOf(columnName) == -1) {
      return undefined;
    }
    try {
      let value = resultSet.getLong(resultSet.getColumnIndex(columnName));
      return value;
    } catch (error) {
      LogUtils.e(TAG, `get Column value err: ${error.code}`);
    }
    return undefined;
  }

  public getColumnStringValue(resultSet: rdb.ResultSet, columnName: string): string | undefined {
    if (resultSet.columnNames.indexOf(columnName) == -1) {
      return undefined;
    }
    try {
      let value = resultSet.getString(resultSet.getColumnIndex(columnName));
      return value;
    } catch (error) {
      LogUtils.e(TAG, `get Column value err: ${error.code}`);
    }
    return undefined;
  }

  public getColumnStringValueWithDefault(resultSet: rdb.ResultSet,
    columnName: string, defaultStr: string): string {
    let result = this.getColumnStringValue(resultSet, columnName);
    if (result === undefined) {
      return defaultStr;
    }
    return result;
  }

  public getColumnIntegerValueWithDefault(resultSet: rdb.ResultSet,
    columnName: string, defaultStr: number): number {
    let result = this.getColumnIntegerValue(resultSet, columnName);
    if (result === undefined) {
      return defaultStr;
    }
    return result;
  }

  public deleteAllData(): boolean {
    if (fs.accessSync(DbConstants.NEXT_DATABASE_PATH)) {
      try {
        fs.rmdirSync(DbConstants.NEXT_DATABASE_PATH);
      } catch (error) {
        LogUtils.i(TAG, `rm database dir failed : ${error.code} error msg: ${error.message}`);
        return false;
      }
      try {
        fs.mkdirSync(DbConstants.NEXT_DATABASE_PATH);
      } catch (error) {
        LogUtils.i(TAG, `mk database dir failed : ${error.code} error msg: ${error.message}`);
        return false;
      }
    }
    return true;
  }

  private async DeleteDdmsTable(rdbStore: rdb.RdbStore) {
    try {
      LogUtils.e(TAG, `DeleteDdmsTable`);
      await rdbStore.executeSql(DbConstants.DROP_DDMS_DATA_SEARCH_AUX_CONFIG_TABLE)
      await rdbStore.executeSql(DbConstants.DROP_DDMS_DATA_SEARCH_FILTERED_DATA_TABLE)
    } catch (error) {
      LogUtils.e(TAG, `DeleteDdmsTable error ${error.code} error msg: ${error.message}`);
    }
  }

  public async DoBackupEl1ToEl2(baseContext: common.Context) {
    let el1RdbStore: rdb.RdbStore | null = null;
    let el1Config : rdb.StoreConfig = {
      name: DbConstants.nextCloneDatabaseName,
      securityLevel: rdb.SecurityLevel.S1,
      customDir: '../../rdb'
    }
    try {
      LogUtils.i(TAG, 'DoBackupEl1ToEl2 Start');
      if (baseContext.area !== contextConstant.AreaMode.EL1) {
        baseContext.area = contextConstant.AreaMode.EL1;
      }
      el1RdbStore = await rdb.getRdbStore(baseContext, el1Config);
    } catch (error) {
      LogUtils.e(TAG, `DoBackupEl1ToEl2 InitEl1Rdb error ${error.code} error msg: ${error.message}`);
      return;
    }
    try {
      // 首次迁移的用户，只存在El2目录，不存在rdb目录。此时需要创建一个El2/rdb目录
      if (!fs.accessSync(DbConstants.EL2_RDB)) {
        fs.mkdirSync(DbConstants.EL2_RDB)
        LogUtils.i(TAG, `DoBackupEl1ToEl2 mkdir el2/rdb ok`);
      }
    } catch (error) {
      LogUtils.e(TAG, `DoBackupEl1ToEl2 mkdirSync error ${error.code} error msg: ${error.message}`);
      return;
    }

    try {
      await this.DeleteDdmsTable(el1RdbStore)
      await el1RdbStore.backup(DbConstants.EL2_RDB + DbConstants.nextCloneDatabaseName);
      el1RdbStore = null;
      LogUtils.e(TAG, `DoBackupEl1ToEl2 el1RdbStore.backup el1->el2 ok`);
      if (await this.isEL2dbComplete(baseContext)) {
        await rdb.deleteRdbStore(baseContext, el1Config)
        LogUtils.w(TAG, `DoBackupEl1ToEl2 el1RdbStore.deleteRdbStore el1 ok`);
      } else {
        LogUtils.w(TAG, `DoBackupEl1ToEl2 isEL2dbComplete false, can not delete el1`);
      }
    } catch (error) {
      LogUtils.e(TAG, `DoBackupEl1ToEl2 el1RdbStore.backup error ${error.code} error msg: ${error.message}`);
    }
  }

  private checkEl2dbTable(rdbStoreHelper: rdb.RdbStore) : boolean {
    const targetCount = 3;
    const sql = 'SELECT COUNT(*) FROM sqlite_master WHERE type=\'table\' AND name IN (\'session\', \'sms_mms_info\', ' +
      '\'rcs_info\');'
    let resultSet: rdb.ResultSet | undefined = undefined;
    try {
      resultSet = rdbStoreHelper.querySqlSync(sql);
      if (resultSet.goToFirstRow()) {
        let count = resultSet.getLong(0);
        if (count === targetCount) {
          LogUtils.w(TAG, `checkEl2dbTable is true, count = ${count}`);
          resultSet?.close();
          return true;
        } else {
          LogUtils.e(TAG, `checkEl2dbTable is false, count = ${count}`);
          resultSet?.close();
          return false;
        }
      } else {
        LogUtils.e(TAG, `checkEl2dbTable goToFirstRow return false`);
        resultSet?.close();
        return false;
      }
    } catch (error) {
      LogUtils.e(TAG, `checkEl2dbTable error ${error.code} error msg: ${error.message}`);
      return false;
    }
  }

  private async isEL2dbComplete(baseContext: common.Context) : Promise<boolean> {
    let areaMode: contextConstant.AreaMode = contextConstant.AreaMode.EL2;
    let areaModeContext: common.Context;
    try {
      areaModeContext = baseContext.createAreaModeContext(areaMode);
    } catch (error) {
      LogUtils.e(TAG, `isEL2dbComplete createAreaModeContext error ${error.code} error msg: ${error.message}`);
      return false;
    }
    let rdbStoreHelper: rdb.RdbStore | undefined = undefined;
    try {
      rdbStoreHelper = await rdb.getRdbStore(areaModeContext, {
        name: DbConstants.nextCloneDatabaseName,
        securityLevel: rdb.SecurityLevel.S2,
        customDir: '../../rdb',
      });
      if (rdbStoreHelper == null) {
        LogUtils.w(TAG, `isEL2dbComplete rdbStoreHelper is null`);
        return false;
      }

      if (this.checkEl2dbTable(rdbStoreHelper)) {
        rdbStoreHelper?.close();
        LogUtils.w(TAG, 'isEL2dbComplete true');
        return true;
      } else {
        rdbStoreHelper?.close();
        return false;
      }
    } catch (error) {
      LogUtils.e(TAG, `isEL2dbComplete error ${error.code} error msg: ${error.message}`);
      return false;
    }
  }

  private async deleteEL2Rdb(baseContext: common.Context) {
    let el2Config : rdb.StoreConfig = {
      name: DbConstants.nextCloneDatabaseName,
      securityLevel: rdb.SecurityLevel.S2,
      customDir: '../../rdb'
    }
    try {
      LogUtils.i(TAG, 'deleteEL2Rdb Start');
      if (baseContext.area !== contextConstant.AreaMode.EL2) {
        baseContext.area = contextConstant.AreaMode.EL2;
      }
      await rdb.deleteRdbStore(baseContext, el2Config);
      LogUtils.i(TAG, 'deleteEL2Rdb success');
    } catch (error) {
      LogUtils.e(TAG, `deleteEL2Rdb getRdbStore error ${error.code} error msg: ${error.message}`);
      return;
    }
  }

  // 双克单、单克单场景，开EL2库之前，识别El1存在，El2不存在场景，提前迁移。
  public async BackupEl1ToEl2(baseContext: common.Context) {
    let existEl1Db : boolean = false;
    let existEl2Db : boolean = false;
    // 迁移成功，直接返回。
    if (GskvSharedPreferencesUtils.isDbMigrationSuccess(baseContext)) {
      LogUtils.i(TAG, `el1->el2 is already success`);
      return;
    }
    // 可能存在迁移失败，由于backup中断，el2存在不完整的库的场景。
    try {
      existEl2Db = fs.accessSync(DbConstants.EL2_RDB + DbConstants.nextCloneDatabaseName);
      if (existEl2Db) {
        LogUtils.i(TAG, `BackupEl1ToEl2 : el2/sms_mms.db exists`);
      } else {
        LogUtils.i(TAG, `BackupEl1ToEl2: el2/sms_mms.db not exists`);
      }
    } catch (error) {
      LogUtils.i(TAG, `BackupEl1ToEl2 fs.accessSync(el2dbPath) failed : ${error.code} error msg: ${error.message}`);
    }

    try {
      existEl1Db = fs.accessSync(DbConstants.EL1_RDB + DbConstants.nextCloneDatabaseName);
      if (!existEl1Db) {
        LogUtils.i(TAG, `BackupEl1ToEl2 done : el1/sms_mms.db not exists, no need to backup, return`);
        return;
      } else {
        LogUtils.i(TAG, `BackupEl1ToEl2: el1/sms_mms.db exists, need to backup, continue`);
      }
    } catch (error) {
      LogUtils.i(TAG, `BackupEl1ToEl2 fs.accessSync(el1dbPath) failed : ${error.code} error msg: ${error.message}`);
      return;
    }

    await this.DoBackupEl1ToEl2(baseContext)
  }
}

export const newDbHelper: NewDbHelper = new NewDbHelper();
