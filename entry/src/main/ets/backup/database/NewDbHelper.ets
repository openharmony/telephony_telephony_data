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
import contextConstant from '@ohos.app.ability.contextConstant';
import { BusinessError } from '@ohos.base';
import { ValuesBucket, ValueType } from '@ohos.data.ValuesBucket';
import common from '@ohos.app.ability.common';
import DbConstants from './DbConstants';
import LogUtils from '../../common/utils/LogUtils';
import OldDBData from './OldDBData';
import NextOldDBData from '../../backup/copyData/NextOldDBData';
import FavorateInfoItem from '../../backup/model/bean/FavorateInfoItem';
import BackupRestoreProgress from '../copyData/BackupRestoreProcess';
import BackupRestoreResult from '../copyData/BackupRestoreResult';
import dataRdb from '@ohos.data.relationalStore';
import SmsMmsInfoItem from '../copyData/bean/SmsMmsInfoItem';
import RdbStoreUtils from '../../common/utils/RdbStoreUtils';

const TAG = 'NewDbHelper: ';

const CURRENT_RDB_VERSION: number = 1;

export default class NewDbHelper {
  private rdbStore: rdb.RdbStore | null = null;

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

  public async updateSession(context: common.Context, sessionId: number, values: ValuesBucket): Promise<number> {
    if (!values) {
      LogUtils.e(TAG, '[updateSession] The addParams of parameter is NULL');
      return Promise.resolve(-1);
    }
    let sessionRdbStore = await this.getRdbStore(context);
    if (sessionRdbStore == null) {
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

  public async insertSpChatbots(context: common.Context, values: ValuesBucket): Promise<number> {
    if (!values) {
      LogUtils.e(TAG, '[insertSpChatbots] The addParams of parameter is NULL');
      return Promise.resolve(-1);
    }
    let serviceID = values.service_id?.toString() + ''
    let curType = values.type?.toString() + ''
    let curMaapSpType = await this.getMaapSpType(serviceID, context)
    let chatbotRdbStore = await this.getRdbStore(context);
    if (chatbotRdbStore == null) {
      return -1;
    }
    if (curMaapSpType === '-1') {
      try {
        return await RdbStoreUtils.insert(chatbotRdbStore, 'specific_chatbots', values);
      } catch (e) {
        const backupRestoreResult = BackupRestoreResult.getInstance();
        backupRestoreResult.insertOrUpdateBackupInfo('sms',
          `insertSpChatbots code:${e.code} msg:${e.message}`);
        LogUtils.e(TAG, 'insertSpChatbots: ' + JSON.stringify(e));
        return -1;
      }
    } else if (curType !== curMaapSpType) {
      try {
        LogUtils.e(TAG, 'update spChatbots');
        let predicates = new rdb.RdbPredicates('specific_chatbots');
        predicates.equalTo('service_id', serviceID);
        return await RdbStoreUtils.update(chatbotRdbStore, values, predicates);
      } catch (e) {
        const backupRestoreResult = BackupRestoreResult.getInstance();
        backupRestoreResult.insertOrUpdateBackupInfo('sms',
          `insertSpChatbots code:${e.code} msg:${e.message}`);
        LogUtils.e(TAG, 'update spChatbots error : ' + JSON.stringify(e));
        return -1;
      }
    } else {
      LogUtils.i(TAG, 'no need insert or update spChatbots');
      return 0;
    }
  }

  public async getMaapSpType(serviceId: string, context: common.Context) {
    LogUtils.i(TAG, 'getMaapSpType start : ');
    let sessionRdbStore = await this.getRdbStore(context);
    if (sessionRdbStore == null) {
      return '-1';
    }
    try {
      let canonicalPredicatesInfo = new rdb.RdbPredicates('specific_chatbots');
      canonicalPredicatesInfo.equalTo('service_id', serviceId);
      let canResultSet = await sessionRdbStore.query(canonicalPredicatesInfo, ['type']);
      if (canResultSet != undefined && canResultSet.goToFirstRow()) {
        let type = canResultSet.getString(canResultSet.getColumnIndex('type'));
        LogUtils.i(TAG, 'type : ' + type);
        canResultSet.close();
        return type;
      }
    } catch (e) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `getMaapSpType code:${e.code} msg:${e.message}`);
      LogUtils.w(TAG, 'getMaapSpType error : ' + JSON.stringify(e));
      return '-1';
    }
    return '-1';
  }

  public async insertChatbots(context: common.Context, values: ValuesBucket): Promise<number> {
    if (!values) {
      LogUtils.e(TAG, '[insertChatbots] The addParams of parameter is NULL');
      return Promise.resolve(-1);
    }
    let serviceID = values.service_id?.toString() + ''
    let recentUseTime = values.recent_use_time?.toString() + ''
    let curMaapTime = await this.getMaapTime(serviceID, context)
    let chatbotRdbStore = await this.getRdbStore(context);
    if (chatbotRdbStore == null) {
      return -1;
    }
    if (curMaapTime === '-1') {
      try {
        return await RdbStoreUtils.insert(chatbotRdbStore, 'chatbots', values);
      } catch (e) {
        const backupRestoreResult = BackupRestoreResult.getInstance();
        backupRestoreResult.insertOrUpdateBackupInfo('sms',
          `insertChatbots code:${e.code} msg:${e.message}`);
        LogUtils.e(TAG, 'insertChatbots: ' + JSON.stringify(e));
        return -1;
      }
    } else if (recentUseTime > curMaapTime) {
      try {
        LogUtils.e(TAG, 'update Chatbots');
        let predicates = new rdb.RdbPredicates('chatbots');
        predicates.equalTo('service_id', serviceID);
        return await RdbStoreUtils.update(chatbotRdbStore, values, predicates);
      } catch (e) {
        const backupRestoreResult = BackupRestoreResult.getInstance();
        backupRestoreResult.insertOrUpdateBackupInfo('sms',
          `insertChatbots code:${e.code} msg:${e.message}`);
        LogUtils.e(TAG, 'update Chatbots error : ' + JSON.stringify(e));
        return -1;
      }
    } else {
      LogUtils.i(TAG, 'no need insert or update Chatbots');
      return 0;
    }
  }

  public async getMaapTime(serviceId: string, context: common.Context) {
    LogUtils.i(TAG, 'getMaapTime start : ');
    let sessionRdbStore = await this.getRdbStore(context);
    if (sessionRdbStore == null) {
      return '-1';
    }
    try {
      let canonicalPredicatesInfo = new rdb.RdbPredicates('chatbots');
      canonicalPredicatesInfo.equalTo('service_id', serviceId);
      let canResultSet = await sessionRdbStore.query(canonicalPredicatesInfo, ['recent_use_time']);
      if (canResultSet != undefined && canResultSet.goToFirstRow()) {
        let maapTime = canResultSet.getString(canResultSet.getColumnIndex('recent_use_time'));
        LogUtils.i(TAG, 'getMaapTime : ' + maapTime);
        canResultSet.close();
        return maapTime;
      }
    } catch (e) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `getMaapTime code:${e.code} msg:${e.message}`);
      LogUtils.w(TAG, 'getMaapTime error : ' + JSON.stringify(e));
      return '-1';
    }
    return '-1';
  }


  public async getSessionTime(sessionId: string, context: common.Context) {
    let sessionRdbStore = await this.getRdbStore(context);
    if (sessionRdbStore == null) {
      return 0;
    }
    try {
      let canonicalPredicatesInfo = new rdb.RdbPredicates(DbConstants.TABLE_SESSION);
      canonicalPredicatesInfo.equalTo('id', sessionId);
      let canResultSet = await sessionRdbStore.query(canonicalPredicatesInfo, ['time']);
      if (canResultSet != undefined && canResultSet.goToFirstRow()) {
        let sessionTime = canResultSet.getLong(canResultSet.getColumnIndex('time'));
        LogUtils.i(TAG, 'SessionTime : ' + sessionTime);
        return sessionTime;
      }
      canResultSet.close();
    } catch (e) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `getSessionTime code:${e.code} msg:${e.message}`);
      LogUtils.w(TAG, 'query address error : ' + JSON.stringify(e));
      return 0;
    }
    return 0;
  }

  public async getSessionNumber(sessionId: string, context: common.Context) {
    LogUtils.i(TAG, 'getSessionNumber start : ');
    let sessionRdbStore = await this.getRdbStore(context);
    if (sessionRdbStore == null) {
      return '';
    }
    try {
      let canonicalPredicatesInfo = new rdb.RdbPredicates(DbConstants.TABLE_SESSION);
      canonicalPredicatesInfo.equalTo('id', sessionId);
      let canResultSet = await sessionRdbStore.query(canonicalPredicatesInfo, ['telephone']);
      if (canResultSet != undefined && canResultSet.goToFirstRow()) {
        let telephone = canResultSet.getString(canResultSet.getColumnIndex('telephone'));
        return telephone;
      }
      canResultSet.close();
    } catch (e) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `getSessionNumber code:${e.code} msg:${e.message}`);
      LogUtils.w(TAG, 'query telephone error : ' + JSON.stringify(e));
      return '';
    }
    return '';
  }

  private async getSessionUnreadCount(sessionId: string, context: common.Context) {
    LogUtils.i(TAG, 'getSessionUnreadCount start : ');
    let sessionRdbStore = await this.getRdbStore(context);
    if (sessionRdbStore == null) {
      return 0;
    }
    try {
      let canonicalPredicatesInfo = new rdb.RdbPredicates(DbConstants.TABLE_SESSION);
      canonicalPredicatesInfo.equalTo('id', sessionId);
      let canResultSet = await sessionRdbStore.query(canonicalPredicatesInfo, ['unread_count']);
      if (canResultSet != undefined && canResultSet.goToFirstRow()) {
        let unreadCount = canResultSet.getLong(canResultSet.getColumnIndex('unread_count'));
        LogUtils.i(TAG, 'UnreadCount : ' + unreadCount);
        return unreadCount;
      }
      canResultSet.close();
    } catch (e) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `getSessionUnreadCount code:${e.code} msg:${e.message}`);
      LogUtils.w(TAG, 'query address error : ' + JSON.stringify(e));
    }
    return 0;
  }

  public async querySms(context: common.Context): Promise<rdb.ResultSet | undefined> {
    let smsRdbStore = await this.getRdbStore(context);
    if (smsRdbStore == null) {
      return undefined;
    }
    let predicates = new rdb.RdbPredicates(DbConstants.TABLE_SMS_MMS_INFO);
    return await smsRdbStore.query(predicates);
  }

  public async queryFavorate(context: common.Context, predicatesInfo: rdb.RdbPredicates, columns?: Array<string>):
    Promise<rdb.ResultSet | undefined> {
    let smsRdbStore = await this.getRdbStore(context);
    if (smsRdbStore == null) {
      return undefined;
    }
    return await smsRdbStore.query(predicatesInfo, columns);
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

  public async queryPart(context: common.Context): Promise<rdb.ResultSet | undefined> {
    let mmsPartRdbStore = await this.getRdbStore(context);
    if (mmsPartRdbStore == null) {
      return undefined;
    }
    let predicates = new rdb.RdbPredicates(DbConstants.TABLE_MMS_PART);
    return await mmsPartRdbStore.query(predicates);
  }

  public async updatePartByRecordTime(context: common.Context, msgId: number, type: number, values: ValuesBucket):
    Promise<number> {
    if (!values) {
      LogUtils.e(TAG, '[updatePartByRecordTime] The addParams of parameter is NULL');
      return Promise.resolve(-1);
    }
    let mmsPartRdbStore = await this.getRdbStore(context);
    if (mmsPartRdbStore == null) {
      return -1;
    }
    try {
      let predicates = new rdb.RdbPredicates(DbConstants.TABLE_MMS_PART);
      predicates.equalTo('msg_id', msgId);
      predicates.equalTo('type', type);
      return await RdbStoreUtils.update(mmsPartRdbStore, values, predicates);
    } catch (e) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `updatePartByRecordTime code:${e.code} msg:${e.message}`);
      LogUtils.e(TAG, 'updatePartByRecordTime: ' + JSON.stringify(e));
      return -1;
    }
  }

  public async updateSmsMmsInfoByContent(context: common.Context, msgId: number, values: ValuesBucket):
    Promise<number> {
    if (!values) {
      LogUtils.e(TAG, '[updateSmsMmsInfoByContent] The addParams of parameter is NULL');
      return Promise.resolve(-1);
    }
    let smsRdbStore = await this.getRdbStore(context);
    if (smsRdbStore == null) {
      return -1;
    }
    try {
      let predicates = new rdb.RdbPredicates(DbConstants.TABLE_SMS_MMS_INFO);
      predicates.equalTo('msg_id', msgId);
      return await RdbStoreUtils.update(smsRdbStore, values, predicates);
    } catch (e) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `updateSmsMmsInfoByContent code:${e.code} msg:${e.message}`);
      LogUtils.e(TAG, 'updateSmsMmsInfoByContent: ' + JSON.stringify(e));
      return -1;
    }
  }

  public async insertRcsInfo(context: common.Context, values: ValuesBucket): Promise<number> {
    if (!values) {
      LogUtils.e(TAG, '[insertRcsInfo] The addParams of parameter is NULL');
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
        `insertRcsInfo code:${e.code} msg:${e.message}`);
      LogUtils.e(TAG, 'insertRcsInfo: ' + JSON.stringify(e));
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

  public async queryMaxMsgIdFromInfo(context: Context): Promise<number> {
    // 已存在的 msg_id 关联的情况，sms_mms_info.msg_id -> favorate_info.msg_id ->mms_part.msg_id
    // 已存在的 rcs_id 关联的情况，rcs_info.rcs_id -> favorate_info.rcs_id ->mms_part.rcs_id
    // 此方法返回当前可用的唯一 msg_id，需要保证无info存在的场景，msg_id唯一
    if (context == null) {
      LogUtils.w(TAG, `queryMaxInfoId failed by null context`)
      return Promise.resolve(-1);
    }
    let maxMsgId: number = 0;
    try {
      let rdbStore = await this.getRdbStore(context);
      if (rdbStore == null || rdbStore === undefined) {
        LogUtils.e(TAG, `[queryMaxInfoId] failed : rdbStore === undefined`)
        return Promise.resolve(-1);
      }
      let valuesMmsInfo: ValuesBucket = new SmsMmsInfoItem().createValuesBucket();
      maxMsgId = await this.insertSms(context, valuesMmsInfo);
      let predicates = new rdb.RdbPredicates(DbConstants.TABLE_SMS_MMS_INFO);
      predicates.equalTo('msg_id', maxMsgId);
      await RdbStoreUtils.delete(rdbStore, predicates);
      return maxMsgId;
    } catch (err) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `queryMaxInfoId code:${err.code} msg:${err.message}`);
      LogUtils.e(TAG, `queryMaxInfoId failed, code is ${err.code},message is ${err.message}`);
      return Promise.resolve(-1);
    }
  }

  public async batchInsertFavorate(context: common.Context, values: Array<ValuesBucket>): Promise<number> {
    if (!values) {
      LogUtils.e(TAG, '[batchInsertFavorate] The addParams of parameter is NULL');
      return Promise.resolve(-1);
    }
    let favorateRdbStore = await this.getRdbStore(context);
    if (favorateRdbStore == null) {
      return -1;
    }
    try {
      return await RdbStoreUtils.batchInsertWithConflictResolution(favorateRdbStore, DbConstants.TABLE_FAVORATE_INFO,
        values, dataRdb.ConflictResolution.ON_CONFLICT_REPLACE);
    } catch (e) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `batchInsertFavorate code:${e.code} msg:${e.message}`);
      LogUtils.e(TAG, 'batchInsertFavorate: ' + JSON.stringify(e));
      return -1;
    }
  }

  public async insertFavorate(context: common.Context, values: ValuesBucket): Promise<number> {
    if (!values) {
      LogUtils.e(TAG, '[insertFavorate] The addParams of parameter is NULL');
      return Promise.resolve(-1);
    }
    let rdbStore = await this.getRdbStore(context);
    if (rdbStore == null) {
      return -1;
    }
    try {
      return await RdbStoreUtils.insert(rdbStore, DbConstants.TABLE_FAVORATE_INFO, values);
    } catch (e) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `insertFavorate code:${e.code} msg:${e.message}`);
      LogUtils.e(TAG, 'insertFavorate: ' + JSON.stringify(e));
      return -1;
    }
  }

  public async updateSmsCollect(context: common.Context, isRcs: boolean, smsId: number): Promise<number> {
    let rdbStore = await this.getRdbStore(context);
    if (rdbStore == null) {
      return -1;
    }
    try {
      let upInfoValues: ValuesBucket = {
        'is_collect': 1
      };
      let predicates = new rdb.RdbPredicates(DbConstants.TABLE_SMS_MMS_INFO);
      predicates.equalTo(isRcs ? 'rcs_id' : 'msg_id', smsId);
      return await RdbStoreUtils.update(rdbStore, upInfoValues, predicates);
    } catch (e) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `updateSmsCollect code:${e.code} msg:${e.message}`);
      LogUtils.e(TAG, 'updateSmsCollect: ' + JSON.stringify(e));
      return -1;
    }
  }

  public async updateRcsCollect(context: common.Context, rcsId: number): Promise<number> {
    let rdbStore = await this.getRdbStore(context);
    if (rdbStore == null) {
      return -1;
    }
    try {
      let upInfoValues: ValuesBucket = {
        'is_collect': 1
      };
      let predicates = new rdb.RdbPredicates(DbConstants.TABLE_RCS_INFO);
      predicates.equalTo('rcs_id', rcsId);
      return await RdbStoreUtils.update(rdbStore, upInfoValues, predicates);
    } catch (e) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `updateRcsCollect code:${e.code} msg:${e.message}`);
      LogUtils.e(TAG, 'updateRcsCollect: ' + JSON.stringify(e));
      return -1;
    }
  }

  private async getRdbStore(baseContext: common.Context): Promise<rdb.RdbStore | null> {
    if (!baseContext) {
      LogUtils.e(TAG, 'getRdbStore baseContext is null');
      return null;
    }
    if (this.rdbStore) {
      return this.rdbStore;
    }
    try {
      LogUtils.i(TAG, 'getRdbStore Start');
      if (baseContext.area !== contextConstant.AreaMode.EL2) {
        baseContext.area = contextConstant.AreaMode.EL2;
      }
      this.rdbStore = await rdb.getRdbStore(baseContext,
        {
          name: DbConstants.NEW_DATABASE_NAME,
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

  public async queryFavorateCount(context: common.Context): Promise<number> {
    if (context == null) {
      LogUtils.w(TAG, `queryFavoriteCount failed by null context`)
      return Promise.resolve(0);
    }
    let count = 0;
    try {
      let predicatesInfo = new rdb.RdbPredicates(DbConstants.TABLE_SMS_MMS_INFO);
      let rdbStore = await this.getRdbStore(context);
      if (rdbStore === null) {
        LogUtils.e(TAG, `[queryPduCount] failed : rdbStore === undefined`)
        return Promise.resolve(0);
      }
      predicatesInfo.equalTo('is_collect', 1)
      let queryColumn = ['count(msg_id)'];
      let resultSet = await rdbStore.query(predicatesInfo, queryColumn);
      if (resultSet === undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'query no favorate found.');
        return Promise.resolve(0);
      }
      count = resultSet.getLong(0);
      LogUtils.i(TAG, '[queryFavoriteCount] count : ' + count);
      resultSet.close();
    } catch (error) {
      LogUtils.e(TAG, '[queryFavoriteCount] error : ' + JSON.stringify(error))
    }
    return Promise.resolve(count);
  }

  public async queryFavorateInfo(context: common.Context, index: number,
    progress: BackupRestoreProgress): Promise<void> {
    let rdbStore = await this.getRdbStore(context);
    if (rdbStore === null) {
      LogUtils.e(TAG, `[queryFavorateInfo] failed : rdbStore === undefined`)
      return;
    }
    let predicatesInfo = new rdb.RdbPredicates(DbConstants.TABLE_SMS_MMS_INFO);
    let queryColumn = [OldDBData.favorateInfo.msgId, OldDBData.favorateInfo.receiverNumber,
      OldDBData.favorateInfo.senderNumber, OldDBData.favorateInfo.startTime,
      OldDBData.favorateInfo.msgType, OldDBData.favorateInfo.smsType, OldDBData.favorateInfo.msgTitle,
      OldDBData.favorateInfo.msgContent, OldDBData.favorateInfo.sessionType,
      OldDBData.favorateInfo.groupId, OldDBData.favorateInfo.isSender, OldDBData.favorateInfo.rcsId,
      OldDBData.favorateInfo.detectResContent, OldDBData.favorateInfo.sessionId];
    try {
      predicatesInfo = predicatesInfo.orderByAsc(OldDBData.favorateInfo.msgId)
        .limitAs(DbConstants.QUERY_PAGE_NUM)
        .offsetAs(index).equalTo('is_collect', 1);
      let resultSet = await rdbStore.query(predicatesInfo, queryColumn);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'no favorate info found.');
        return;
      }
      let count = resultSet.rowCount;
      let failCount = 0;
      let successCount = 0;
      let batchFavorateValues: ValuesBucket[] = [];
      do {
        let msgId = resultSet.getLong(resultSet.getColumnIndex(OldDBData.favorateInfo.msgId));
        let receiverNumber = resultSet.getString(resultSet.getColumnIndex(OldDBData.favorateInfo.receiverNumber));
        let senderNumber = resultSet.getString(resultSet.getColumnIndex(OldDBData.favorateInfo.senderNumber));
        let startTime = resultSet.getString(resultSet.getColumnIndex(OldDBData.favorateInfo.startTime));
        let msgType = resultSet.getLong(resultSet.getColumnIndex(OldDBData.favorateInfo.msgType));
        let smsType = resultSet.getLong(resultSet.getColumnIndex(OldDBData.favorateInfo.smsType));
        let msgTitle = resultSet.getString(resultSet.getColumnIndex(OldDBData.favorateInfo.msgTitle));
        let msgContent = resultSet.getString(resultSet.getColumnIndex(OldDBData.favorateInfo.msgContent));
        let sessionType = resultSet.getLong(resultSet.getColumnIndex(OldDBData.favorateInfo.sessionType));
        let groupId = resultSet.getLong(resultSet.getColumnIndex(OldDBData.favorateInfo.groupId));
        let isSender = resultSet.getLong(resultSet.getColumnIndex(OldDBData.favorateInfo.isSender));
        let rcsId = resultSet.getLong(resultSet.getColumnIndex(OldDBData.favorateInfo.rcsId));
        let detectResContent = resultSet.getString(resultSet.getColumnIndex(OldDBData.favorateInfo.detectResContent));
        let sessionId = resultSet.getLong(resultSet.getColumnIndex(OldDBData.favorateInfo.sessionId));
        let partType = 0;
        let partLocationPath = '';
        let recordingTime = '';
        let partSize = '';

        let invalidItem = false;
        // 如果是RCS，查询RCS表获取相关字段。
        if (smsType == 99) {
          let rcsPredicatesInfo = new rdb.RdbPredicates(DbConstants.TABLE_RCS_INFO);
          rcsPredicatesInfo.equalTo('rcs_id', rcsId);
          let rcsQueryColumn = [OldDBData.favorateInfo.msgTitle, OldDBData.favorateInfo.msgContent,
            OldDBData.favorateInfo.detectResContent];
          let rcsResultSet = await rdbStore.query(rcsPredicatesInfo, rcsQueryColumn);
          if (rcsResultSet == undefined || !rcsResultSet.goToFirstRow()) {
            LogUtils.w(TAG, 'no rcs part row found.');
            invalidItem = true;
          } else {
            msgTitle = rcsResultSet.getString(rcsResultSet.getColumnIndex(OldDBData.favorateInfo.msgTitle));
            msgContent = rcsResultSet.getString(rcsResultSet.getColumnIndex(OldDBData.favorateInfo.msgContent));
            detectResContent = rcsResultSet.getString(rcsResultSet.getColumnIndex(
              OldDBData.favorateInfo.detectResContent));
          }
          if (rcsResultSet != undefined) {
            rcsResultSet.close();
          }
        }

        // 如果是彩信，查询part表。
        if (msgType == 1) {
          let mmsPartPredicatesInfo = new rdb.RdbPredicates(DbConstants.TABLE_MMS_PART);
          if (smsType == 99) {
            mmsPartPredicatesInfo.equalTo('rcs_id', rcsId);
          } else {
            mmsPartPredicatesInfo.equalTo('msg_id', msgId);
          }
          // 彩信可能有多条记录，查询type非0结果里最小的那条。
          mmsPartPredicatesInfo.notEqualTo('type', 0);
          mmsPartPredicatesInfo.orderByAsc('type');
          let mmsPartQueryColumn = [NextOldDBData.mmsPart.type, NextOldDBData.mmsPart.locationPath,
            NextOldDBData.mmsPart.recordingTime, NextOldDBData.mmsPart.partSize];
          let mmsPartResultSet = await rdbStore.query(mmsPartPredicatesInfo, mmsPartQueryColumn);
          if (mmsPartResultSet == undefined || !mmsPartResultSet.goToFirstRow()) {
            LogUtils.w(TAG, 'no favorate sms part row found.');
            invalidItem = true;
          } else {
            partType = mmsPartResultSet.getLong(mmsPartResultSet.getColumnIndex(NextOldDBData.mmsPart.type));
            partLocationPath = mmsPartResultSet.getString(mmsPartResultSet.getColumnIndex(
              NextOldDBData.mmsPart.locationPath));
            recordingTime = mmsPartResultSet.getString(mmsPartResultSet.getColumnIndex(
              NextOldDBData.mmsPart.recordingTime));
            partSize = mmsPartResultSet.getString(mmsPartResultSet.getColumnIndex(NextOldDBData.mmsPart.partSize));
          }
          if (mmsPartResultSet != null) {
            mmsPartResultSet.close();
          }
        }
        if (invalidItem) {
          LogUtils.w(TAG, 'invalid item, skip insert to fav info.');
          continue;
        }
        let values : ValuesBucket = new FavorateInfoItem()
          .setMsgId(msgId)
          .setReceiverNumber(receiverNumber)
          .setSenderNumber(senderNumber)
          .setKeepTime(new Date().getTime() + '')
          .setStartTime(startTime)
          .setMsgType(msgType)
          .setSmsType(smsType)
          .setMsgTitle(msgTitle)
          .setMsgContent(msgContent)
          .setSessionType(sessionType)
          .setGroupId(groupId)
          .setPartType(partType)
          .setPartLocationPath(partLocationPath)
          .setIsSender(isSender)
          .setRcsId(rcsId)
          .setDetectResContent(detectResContent)
          .setSessionId(sessionId)
          .setRecordingTime(recordingTime)
          .setPartSize(partSize)
          .createValuesBucket();
        batchFavorateValues.push(values);
      } while (resultSet.goToNextRow());
      resultSet.close();
      if (batchFavorateValues.length > 0) {
        let insertNum = await this.batchInsertFavorate(context, batchFavorateValues);
        if (insertNum == -1) {
          failCount += batchFavorateValues.length;
        } else {
          successCount += insertNum;
        }
      }
      progress.updateProcessCount(progress.getProcessCount() + count);
      LogUtils.i(TAG, 'favorate sms Total Migrations count: ' + count + ',Successful Migrations successCount: ' +
        successCount + 'Failed Migrations failCount: ' + failCount);
    } catch (error) {
      LogUtils.w(TAG, 'query favorateInfo error : ' + JSON.stringify(error));
    }
  }

  public async updateSessionWithUnSupportItem(context: common.Context, sessions: number[],
    progress: BackupRestoreProgress): Promise<number> {
    let rdbStore = await this.getRdbStore(context);
    let processCount: number = 0;
    if (rdbStore == null) {
      LogUtils.e(TAG, 'get rdb store failed when update session with unSupport');
      return processCount;
    }
    try {
      for (let index = 0; index < sessions.length; index++) {
        let sessionId = sessions[index];
        let conditions: string = 'SELECT * FROM sms_mms_info WHERE session_id = ' + sessionId +
          ' ORDER BY start_time DESC LIMIT 1';
        let resultSet: rdb.ResultSet = (rdbStore as rdb.RdbStore).querySqlSync(conditions);
        if (resultSet != undefined && resultSet.goToFirstRow()) {
          let content = resultSet.getString(resultSet.getColumnIndex('msg_content'));
          let time = resultSet.getLong(resultSet.getColumnIndex('start_time'));
          let upSessionValues: ValuesBucket = {
            'content': content,
            'time': time,
          };
          await this.updateSession(context, sessionId, upSessionValues);
          processCount ++;
        } else {
          LogUtils.e(TAG, 'Failed to query the latest item ');
        }
        progress.updateProcessCount(progress.getProcessCount() + 1);
      }
    } catch (err) {
      LogUtils.e(TAG, `update session with unSupport item failed, code is ${err.code},message is ${err.message}`);
    }
    return processCount;
  }
}

export const newDbHelper: NewDbHelper = new NewDbHelper();
export const newDbHelperClone: NewDbHelper = new NewDbHelper();