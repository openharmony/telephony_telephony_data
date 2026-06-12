/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2025. All rights reserved.
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

import fs from '@ohos.file.fs';
import rdb from '@ohos.data.relationalStore';
import common from '@ohos.app.ability.common';
import dataPreferences from '@ohos.data.preferences';
import { ValuesBucket } from '@ohos.data.ValuesBucket';
import { BusinessError } from '@ohos.base';
import BackupExtensionContext from '@ohos.file.BackupExtensionContext';
import contextConstant from '@ohos.app.ability.contextConstant';
import LogUtils from '../../common/utils/LogUtils';
import SessionItem from './bean/SessionItem';
import SmsMmsInfoItem from './bean/SmsMmsInfoItem';
import MmsPduItem from './bean/MmsPduItem';
import MmsPartItem from './bean/MmsPartItem';
import MmsProtocolItem from './bean/MmsProtocolItem';
import RcsInfoItem from './bean/RcsInfoItem';
import SmsSubsectionItem from './bean/SmsSubsectionItem';
import DbConstants from './DbConstants';
import NextOldDBData from './NextOldDBData';
import { newDbHelper } from './NewDbHelper';
import BackupRestoreResult from './BackupRestoreResult';
import ErrorUtils from '../../common/utils/ErrorUtils';
import { HashMap, JSON } from '@kit.ArkTS';
import BackupRestoreProgress from './BackupRestoreProcess';
import { StringUtil } from '../../common/utils/StringUtil';
import FavorateInfoItem from '../model/bean/FavorateInfoItem';
import { newDbHelperClone } from '../database/NewDbHelper';
import LooseObject from './LooseObject';

const TAG = '[Next-Copy-DbHelper] :';
const PREF_NAME: string = 'CLONE_DB_PREFERENCES';
const RAW_SESSION_ID_MAP = new Map<number, number>();
const RAW_MSG_ID_MAP = new Map<number, number>();
const RAW_RCS_ID_MAP = new Map<number, number>();
const ROW_RCS_NEW_MSG_ID_MAP = new Map<number, number>();
const NEW_MSG_GROUP_ID_MAP = new Map<number, number>();
const NEW_RCS_GROUP_ID_MAP = new Map<number, number>();
const NEW_FAV_MSG_ID_MAP = new Map<number, number[]>();
const THREAD_NEED_UPDATE = new Set<number>();
const RCS_GROUPID_MAP = new Map<number, number>();
// SESSION_DROP_DRAFT：当新机旧机相同会话都存在草稿，保留旧机草稿
class DropDraftInfo {
  public isOld: boolean; // 表示是否使用旧机草稿
  public time: number; // 最终保留草稿的时间
  constructor(isOld: boolean = false, time: number = -1) {
    this.isOld = isOld;
    this.time = time;
  }
}
const SESSION_DROP_DRAFT = new Map<number, DropDraftInfo>();
let maxNewGroupId: number = 0;
const TIME_GROUP_ID_MAP = new HashMap<string, number>();
let myPreferences: dataPreferences.Preferences | null = null;

interface needUpdateIds {
  partMsgIdNeedUpdate: number | undefined;
  partRcsIdNeedUpdate: number | undefined;
}

class NextCopyDbHelper {
  private rdbStoreHelper?: rdb.RdbStore;
    private sessionSuccess: number = 0;
    private mmsSuccess: number = 0;
    private pudSuccess: number = 0;
    private partSuccess: number = 0;
    private protocolSuccess: number = 0
    private rcsSuccess: number = 0
    private subsectionSuccess: number = 0
    private favoriteSuccess: number = 0;
    private sessionFailed: number = 0;
    private mmsFailed: number = 0;
    private pudFailed: number = 0;
    private partFailed: number = 0;
    private protocolFailed: number = 0
    private rcsFailed: number = 0
    private subsectionFailed: number = 0
    private favorateFailed: number = 0;
    private versionName: string = '';
    private querySessionColumn: string[] = [];
    private queryMmsColumn: string[] = [];
    private queryPartColumn: string[] = [];
    private queryProtocolColumn: string[] = [];
    private queryRcsColumn: string[] = [];
    private querySubsectionColumn: string[] = [];
    private queryFavoriteColumn: string[] = [];
    private queryChatbotsColumn: string[] = [];

  public setVersion(version: string) {
    this.versionName = version;
    const backupRestoreResult = BackupRestoreResult.getInstance();
    backupRestoreResult.insertOrUpdateBackupInfo('sms', `versionName :${version}`);
  }

  private getClonePathListFile(clonePath: string): string[] {
    try {
      let isClonePathExists = fs.accessSync(clonePath);
      LogUtils.i(TAG, `getClonePathListFile isClonePathExists: ${isClonePathExists}`);
      if (!isClonePathExists) {
        return [];
      }
      return fs.listFileSync(clonePath);
    } catch (error) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `getClonePathListFile code:${error.code} msg:${error.message}`);
      LogUtils.i(TAG, 'getClonePathListFile catch error');
      return [];
    }
  }

  private getClonePath(backupDir: string): string | undefined {
    let elPaths: string[] = ['el1', 'el5', 'el2', 'el3', 'el4'];
    for (let elPath of elPaths) {
      let cloneDBPath = backupDir + '/data/storage/' + elPath + '/database';
      try {
          let isClonePathExists = fs.accessSync(cloneDBPath);
          if (isClonePathExists) {
            return cloneDBPath;
          }
        } catch (error) {
          const backupRestoreResult = BackupRestoreResult.getInstance();
          backupRestoreResult.insertOrUpdateBackupInfo('sms',
            `getClonePath code:${error.code} msg:${error.message}`);
          LogUtils.i(TAG, 'getClonePath catch error');
          return undefined;
        }
    }
    return undefined;
  }

  public async copyDbListFileForClone(applicationContextPath: string,
    context: BackupExtensionContext, callback: Function, progress: BackupRestoreProgress):
    Promise<boolean> {
    LogUtils.i(TAG, `copyDbListFileForClone start`);
    if (context.area !== contextConstant.AreaMode.EL2) {
      context.area = contextConstant.AreaMode.EL2;
    }

    let readClonePath = applicationContextPath + '/rdb';
    const backupDir = context.backupDir + 'restore';
    let cloneDBPath = this.getClonePath(backupDir);
    if (cloneDBPath == undefined) {
        LogUtils.e(TAG, `ClonePath is not exists`);
        if (callback) {
          callback(new ErrorUtils(400, 'ClonePathExists is not exists'));
        }
        return false;
    }
    try {
      let readClonePathFileList = this.getClonePathListFile(readClonePath);
      if (readClonePathFileList.length > 0) {
        LogUtils.e(TAG, 'Some files remain and need to be cleared.');
        try {
          fs.rmdirSync(readClonePath);
        } catch (error) {
          const backupRestoreResult = BackupRestoreResult.getInstance();
          backupRestoreResult.insertOrUpdateBackupInfo('sms',
            `copyDbListFileForClone code:${error.code} msg:${error.message}`);
          LogUtils.e(TAG, `rmdirSync: ${error.code}`);
        }
      }
      let isReadClonePathExists = fs.accessSync(readClonePath);
      if (!isReadClonePathExists) {
        try {
          fs.mkdirSync(readClonePath);
        } catch (error) {
          const backupRestoreResult = BackupRestoreResult.getInstance();
          backupRestoreResult.insertOrUpdateBackupInfo('sms',
            `copyDbListFileForClone code:${error.code} msg:${error.message}`);
          LogUtils.i(TAG, `readClonePath: ${error.code}`);
          if (callback) {
            callback(new ErrorUtils(400, 'mkdir failed'));
          }
        }
      }
      let listFileName = this.getClonePathListFile(cloneDBPath);
      if (listFileName === undefined || listFileName === null || listFileName.length === 0) {
        LogUtils.i(TAG, 'copyDbListFileForClone clonePath is not file');
        if (callback) {
          callback(new ErrorUtils(400, 'copDbListFileForClone clonePath is not file'));
        }
        return false;
      }
      LogUtils.i(TAG, 'All listFileName length:' + listFileName.length);
      listFileName.forEach((value: string) => {
        let srcPath = `${cloneDBPath}/`;
        let destPath = `${readClonePath}/`;
        if (value == 'rdb') {
          fs.copyFileSync(srcPath + 'rdb/' + DbConstants.nextCloneDatabaseName, destPath + DbConstants.nextCloneDatabaseName);
          fs.copyFileSync(srcPath + 'rdb/' + DbConstants.nextCloneDatabaseNameShm, destPath + DbConstants.nextCloneDatabaseNameShm);
          fs.copyFileSync(srcPath + 'rdb/' + DbConstants.nextCloneDatabaseNameWal, destPath + DbConstants.nextCloneDatabaseNameWal);
        }
      });
      LogUtils.i(TAG, 'copyDbFile success');
      await this.doMigrateData(context, callback, progress);
      return true;
    } catch (error) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `copyDbListFileForClone code:${error.code} msg:${error.message}`);
      LogUtils.i(TAG, `copyDbListFileForClone error: ${JSON.stringify(error)}`);
      if (callback) {
        callback(new ErrorUtils(400, JSON.stringify(error)));
      }
    }
    if (callback) {
      callback(new ErrorUtils(400, 'not copy'));
    }
    return false;
  }

  public async queryNextCount(context: common.Context, err?: ErrorUtils): Promise<string> {
    if (err) {
      LogUtils.i(TAG, `err.code: ${err.code}, err.message: ${err.message}`);
    }
    let smsCount = await this.queryDataCount(context, DbConstants.NEXT_OLD_SMS_NAME, NextOldDBData.smsMmsInfo.msgId);
    const backupRestoreResult = BackupRestoreResult.getInstance();
    backupRestoreResult.addErrorInfo(this.mmsFailed <= 0 ? BackupRestoreResult.RESTORE_SUCCESS
      : BackupRestoreResult.RESTORE_COMMON_ERROR, '');
    backupRestoreResult.insertOrUpdateBackupInfo('sms', '', smsCount - this.mmsFailed, 0, this.mmsFailed);
    LogUtils.i(TAG,
      `sessionSuccess: ${this.sessionSuccess}, mmsSuccess: ${this.mmsSuccess}, pudSuccess:
      ${this.pudSuccess},protocolSuccess: ${this.protocolSuccess},partSuccess: ${this.partSuccess},rcsSuccess:
      ${this.rcsSuccess},subsectionSuccess: ${this.subsectionSuccess}, favorateSuccess: ${this.favoriteSuccess}`);
    LogUtils.i(TAG,
      `sessionFailed: ${this.sessionFailed}, mmsFailed: ${this.mmsFailed}, pudFailed:
      ${this.pudFailed},protocolFailed: ${this.protocolFailed},partFailed: ${this.partFailed},rcsFailed:
      ${this.rcsFailed},subsectionFailed: ${this.subsectionFailed}, favorateFailed: ${this.favorateFailed}`);
    return backupRestoreResult.formatRestoreResult();
  }

 private async doSessionTableCopy(context: common.Context, callBack: Function, progress: BackupRestoreProgress) {
    let idFailsSuccess = 0;
    let idSuccess = 0;
    let sessionCount = await this.queryDataCount(context, DbConstants.NEXT_OLD_SESSION_NAME,
      NextOldDBData.session.id, callBack);
    let sessionPage = Math.ceil(sessionCount / DbConstants.QUERY_PAGE_NUM);
    this.querySessionColumn = await this.queryColumns(context, DbConstants.NEXT_OLD_SESSION_NAME);
    let sessionIdFails = this.getFromPreferences(context, DbConstants.NEXT_SESSION_ID_FAIL, []) as number[];
    let sessionMap = await newDbHelper.querySession(context);
    if (sessionIdFails.length > 0) {
      idFailsSuccess += await this.querySessionTable(context, -1, sessionIdFails, sessionMap, progress, callBack);
      sessionIdFails = this.getFromPreferences(context, DbConstants.NEXT_SESSION_ID_FAIL, []) as number[];
    }
    let sessionIdSuccess = this.getFromPreferences(context, DbConstants.NEXT_SESSION_ID_SUCCESS, 0) as number;
    LogUtils.i(TAG, 'sessionPage ：' + sessionPage);
    for (let curPage = 1; curPage <= sessionPage; curPage++) {
      idSuccess += await this.querySessionTable(context,
        DbConstants.QUERY_PAGE_NUM * (curPage - 1) + sessionIdSuccess,
        sessionIdFails, sessionMap, progress, callBack);
    }
    this.sessionSuccess = idFailsSuccess + idSuccess;
    progress.updateProcessCount(progress.getThreadNextCloneProgress());
  }

  private async updateNewSessionDropDraft(context: common.Context): Promise<void> {
    let resultSet = await newDbHelper.queryDraftInfo(context);
    if (resultSet === undefined) {
      return;
    }
    try {
      while (resultSet.goToNextRow()) {
        const startTime = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.startTime));
        const sessionId = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.sessionId));
        if (sessionId === -1) { // session无效，不更新
          LogUtils.e(TAG, 'updateNewSessionDropDraft error : Invalid msg session!');
          continue;
        }
        let draft = SESSION_DROP_DRAFT.get(sessionId);
        if (draft && !draft.isOld) { // 只有isOld为false才用新机的草稿数据
          let time = startTime === '' ? -1 : Number(startTime);
          if (draft.time < time) { // 保留最新
            draft.time = time;
          }
        }
      }
    } catch (error) {
      LogUtils.w(TAG, 'updateNewSessionDropDraft error : ' + JSON.stringify(error));
    }
    resultSet.close();
    return;
  }

  private async doMmsTableCopy(context: common.Context, callback: Function, progress: BackupRestoreProgress) {
    let idFailsSuccess = 0;
    let idSuccess = 0;
    let smsCount = await this.queryDataCount(context,
      DbConstants.NEXT_OLD_SMS_NAME,
      NextOldDBData.smsMmsInfo.msgId, callback);
    let smsPage = Math.ceil(smsCount / DbConstants.QUERY_PAGE_NUM);
    this.queryMmsColumn = await this.queryColumns(context, DbConstants.NEXT_OLD_SMS_NAME);
    let smsIdFails = this.getFromPreferences(context, DbConstants.NEXT_SMS_ID_FAIL, []) as number[];
    await this.updateNewSessionDropDraft(context);
    let smsMmsMap = await newDbHelper.querySmsInfo(context);
    maxNewGroupId = await newDbHelper.queryMaxGroupId(context);
    if (smsIdFails.length > 0) {
      idFailsSuccess += await this.queryMmsTable(context, -1, smsIdFails, smsMmsMap, progress, callback);
      smsIdFails = this.getFromPreferences(context,
        DbConstants.NEXT_SMS_ID_FAIL,
        []) as number[];
    }
    let smsIdSuccess = this.getFromPreferences(context, DbConstants.NEXT_SMS_ID_SUCCESS, 0) as number;
    LogUtils.i(TAG, 'smsPage ：' + smsPage);
    for (let curPage = 1; curPage <= smsPage; curPage++) {
      idSuccess += await this.queryMmsTable(context,
        DbConstants.QUERY_PAGE_NUM * (curPage - 1) + smsIdSuccess,
        smsIdFails, smsMmsMap, progress, callback);
    }
    this.mmsSuccess = idSuccess + idFailsSuccess;
    progress.updateProcessCount(progress.getSmsNextCloneProgress());
  }


  private async doPduTableCopy(context: common.Context, callback: Function, progress: BackupRestoreProgress) {
      let idFailsSuccess = 0;
      let idSuccess = 0;
      let pduCount = await this.queryDataCount(context, DbConstants.NEXT_OLD_PDU_NAME,
        NextOldDBData.mmsPdu.id, callback);
      let pduPage = Math.ceil(pduCount / DbConstants.QUERY_PAGE_NUM);
      let pduIdFails = this.getFromPreferences(context, DbConstants.NEXT_PDU_ID_FAIL, []) as number[];
      if (pduIdFails.length > 0) {
        idFailsSuccess += await this.queryPduTable(context, -1, pduIdFails, progress, callback);
        pduIdFails = this.getFromPreferences(context, DbConstants.NEXT_PDU_ID_FAIL, []) as number[];
      }
      let pduIdSuccess = this.getFromPreferences(context, DbConstants.NEXT_PDU_ID_SUCCESS, 0) as number;
      LogUtils.i(TAG, 'pduPage ：' + pduPage);
      for (let curPage = 1; curPage <= pduPage; curPage++) {
        idSuccess += await this.queryPduTable(context, DbConstants.QUERY_PAGE_NUM * (curPage - 1) +
          pduIdSuccess, pduIdFails, progress, callback);
      }
      this.pudSuccess = idSuccess + idFailsSuccess;
      progress.updateProcessCount(progress.getPduNextCloneProgress());
    }

  private async doPartTableCopy(context: common.Context, progress: BackupRestoreProgress) {
    let idFailsSuccess = 0;
    let idSuccess = 0;
    let partCount = await this.queryDataCount(context, DbConstants.NEXT_OLD_PART_NAME, NextOldDBData.mmsPart.id);
    let partPage = Math.ceil(partCount / DbConstants.QUERY_PAGE_NUM);
    this.queryPartColumn = await this.queryColumns(context, DbConstants.NEXT_OLD_PART_NAME);
    let partIdFails = this.getFromPreferences(context, DbConstants.NEXT_PART_ID_FAIL, []) as number[];
    let sessionMap = await newDbHelper.queryMmsPart(context);

    if (partIdFails.length > 0) {
      idFailsSuccess += await this.queryPartTable(context, -1, partIdFails, sessionMap, progress);
      partIdFails = this.getFromPreferences(context, DbConstants.NEXT_PART_ID_FAIL, []) as number[];
    }
    let partIdSuccess = this.getFromPreferences(context,
      DbConstants.NEXT_PART_ID_SUCCESS, 0) as number;
    LogUtils.i(TAG, 'partPage ：' + partPage);
    for (let curPage = 1; curPage <= partPage; curPage++) {
      idSuccess += await this.queryPartTable(context,
        DbConstants.QUERY_PAGE_NUM * (curPage - 1) + partIdSuccess,
        partIdFails, sessionMap, progress);
    }
    this.partSuccess = idFailsSuccess + idSuccess;
    progress.updateProcessCount(progress.getPartNextCloneProgress());
  }

  private async doProtocolTableCopy(context: common.Context, progress: BackupRestoreProgress) {
    let idFailsSuccess = 0;
    let idSuccess = 0;
    let protocolCount = await this.queryDataCount(context, DbConstants.NEXT_OLD_PROTOCOL_NAME,
      NextOldDBData.mmsProtocol.id);
    let protocolPage = Math.ceil(protocolCount / DbConstants.QUERY_PAGE_NUM);
    this.queryProtocolColumn = await this.queryColumns(context, DbConstants.NEXT_OLD_PROTOCOL_NAME);
    let protocolIdFails = this.getFromPreferences(context,
      DbConstants.NEXT_PROTOCOL_ID_FAIL,
      []) as number[];
    if (protocolIdFails.length > 0) {
      idFailsSuccess += await this.queryProtocolTable(context, -1, protocolIdFails, progress);
      protocolIdFails = this.getFromPreferences(context,
        DbConstants.NEXT_PROTOCOL_ID_FAIL,
        []) as number[];
    }
    let protocolIdSuccess = this.getFromPreferences(context, DbConstants.NEXT_PROTOCOL_ID_SUCCESS, 0) as number;
    LogUtils.i(TAG, 'protocolPage ：' + protocolPage);
    for (let curPage = 1; curPage <= protocolPage; curPage++) {
      idSuccess += await this.queryProtocolTable(context,
        DbConstants.QUERY_PAGE_NUM * (curPage - 1) + protocolIdSuccess, protocolIdFails, progress);
    }
    this.protocolSuccess = idFailsSuccess + idSuccess;
    progress.updateProcessCount(progress.getProtocolNextCloneProgress());
  }

  private buildSmsMmsRowMsgContent(resultSet: rdb.ResultSet): string | undefined {
    let content = newDbHelper.getColumnStringValue(resultSet, NextOldDBData.rcsInfo.msgContent);
    let rcsType = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.rcsType);
    if (rcsType === 0) {
      return content;
    }
    if (rcsType !== 11) {
      return '';
    }
    let contentStr: string = '';
    let msgContent: string | undefined = content;
    if (msgContent === undefined) {
      return contentStr;
    }
    try {
      let content: LooseObject = JSON.parse(msgContent) as LooseObject;
      if (content == undefined || content.message == undefined) {
        return contentStr;
      }
      let message: LooseObject = content.message;
      if (JSON.has(message, 'generalPurposeCard')) {
        let title = message.generalPurposeCard.content.title ?
          message.generalPurposeCard.content.title + String.fromCharCode(10) : '';
        contentStr = contentStr.concat(title, message.generalPurposeCard.content.description ?? '');
      }
      if (JSON.has(message, 'generalPurposeCardCarousel')) {
        let contentArray: LooseObject[] = message.generalPurposeCardCarousel.content;
        let contentStrArray: string[] = [];
        contentArray.forEach(content => {
          let title = content.title ? content.title + String.fromCharCode(10) : '';
          let description = content.description ? content.description + String.fromCharCode(10) : '';
          contentStrArray.push(title, description);
        })
        contentStr = contentStrArray.join('');
      }
    } catch (err) {
      LogUtils.e(TAG, 'buildSmsMmsRowMsgContent err, msg = '+ JSON.stringify(err));
    }
    return contentStr;
  }

  private async doRcsTableCopy(context: common.Context, callback: Function, progress: BackupRestoreProgress) {
    let idFailsSuccess = 0;
    let idSuccess = 0;
    let rcsCount = await this.queryDataCount(context, DbConstants.NEXT_OLD_RCS_NAME,
      NextOldDBData.rcsInfo.rcsId, callback);
    let rcsPage = Math.ceil(rcsCount / DbConstants.QUERY_PAGE_NUM);
    this.queryRcsColumn = await this.queryColumns(context, DbConstants.NEXT_OLD_RCS_NAME);
    let rcsIdFails = this.getFromPreferences(context,
      DbConstants.NEXT_RCS_ID_FAIL,
      []) as number[];
    let resInfoMap = await newDbHelper.queryRcs(context);
      if (rcsIdFails.length > 0) {
        idFailsSuccess += await this.queryRcsTable(context, -1, rcsIdFails, resInfoMap, progress, callback);
        rcsIdFails = this.getFromPreferences(context,
          DbConstants.NEXT_RCS_ID_FAIL,
          []) as number[];
      }
      let rcsIdSuccess = this.getFromPreferences(context, DbConstants.NEXT_RCS_ID_SUCCESS, 0) as number;
      LogUtils.i(TAG, 'rcsPage : ' + rcsPage);
      for (let curPage = 1; curPage <= rcsPage; curPage++) {
        idSuccess += await this.queryRcsTable(context,
          DbConstants.QUERY_PAGE_NUM * (curPage - 1) + rcsIdSuccess,
          rcsIdFails, resInfoMap, progress, callback);
      }
      this.rcsSuccess = idFailsSuccess + idSuccess;
      progress.updateProcessCount(progress.getRcsChatNextCloneProgress());
    }

  private async doSubsectionTableCopy(context: common.Context, progress: BackupRestoreProgress) {
    let idFailsSuccess = 0;
    let idSuccess = 0;
    let subsectionCount = await this.queryDataCount(context, DbConstants.NEXT_OLD_SUBSECTION_NAME,
      NextOldDBData.smsSubsection.id);
    let subsectionPage = Math.ceil(subsectionCount / DbConstants.QUERY_PAGE_NUM);
    this.querySubsectionColumn = await this.queryColumns(context, DbConstants.NEXT_OLD_SUBSECTION_NAME);
    let subsectionIdFails = this.getFromPreferences(context,
      DbConstants.NEXT_SUBSECTION_ID_FAIL,
      []) as number[];
    if (subsectionIdFails.length > 0) {
      idFailsSuccess += await this.querySubsectionTable(context, -1, subsectionIdFails, progress);
      subsectionIdFails = this.getFromPreferences(context,
        DbConstants.NEXT_SUBSECTION_ID_FAIL,
        []) as number[];
    }
    let subsectionIdSuccess = this.getFromPreferences(context,
      DbConstants.NEXT_SUBSECTION_ID_SUCCESS, 0) as number;
    LogUtils.i(TAG, 'subsectionPage ：' + subsectionPage);
    for (let curPage = 1; curPage <= subsectionPage; curPage++) {
      idSuccess += await this.querySubsectionTable(context,
        DbConstants.QUERY_PAGE_NUM * (curPage - 1) + subsectionIdSuccess,
        subsectionIdFails, progress);
    }
    this.subsectionSuccess = idFailsSuccess + idSuccess;
    progress.updateProcessCount(progress.getSubNextCloneProgress());
  }

  private extractDropDraftInfo(): Map<number, number> {
    const result = new Map<number, number>();
    SESSION_DROP_DRAFT.forEach((dropDraftInfo: DropDraftInfo, session: number) => {
      result.set(session, dropDraftInfo.time);
    });
    return result;
  }

  private async doMigrateData(context: common.Context, callBack: Function, progress: BackupRestoreProgress) {
    LogUtils.i(TAG, 'doMigrateData start');
    await this.queryUnreadCount(context, progress);
    await this.queryBlockedMsgCount(context, progress);
    await this.queryTotalMsgCount(context, callBack, progress);
    await this.checkOldDbIsValid(context);
    await this.doSessionTableCopy(context, callBack, progress);
    await this.doMmsTableCopy(context, callBack, progress);
    await this.doRcsTableCopy(context, callBack, progress);
    await this.doPduTableCopy(context, callBack, progress);
    await this.doPartTableCopy(context, progress);
    await this.doProtocolTableCopy(context, progress);
    await this.doSubsectionTableCopy(context, progress);
    await this.doFavoriteInfoCopy(context, callBack, progress);
    await this.doChatBotTableCopy(context, callBack, progress);
    await this.doSpChatBotTableCopy(context, callBack, progress);
    await newDbHelper.deleteInvalidDraft(context, progress, this.extractDropDraftInfo());
    await newDbHelper.processUpdateSession(context, progress, THREAD_NEED_UPDATE);
    await newDbHelper.recountAndUpdateUnread(context, progress);
    await newDbHelper.updateSessionMsgCount(context, progress);
    await newDbHelper.checkSessionIsValid(context, progress);
    await newDbHelper.checkSmsInfoIsValid(context, progress);
    await newDbHelper.queryBlockedMsgCount(context);
    LogUtils.i(TAG, 'doMigrateData end');
  }

  private async doSpChatBotTableCopy(context: common.Context, callback: Function, progress: BackupRestoreProgress) {
    let chatBotSpCount = await this.queryDataCount(context, 'specific_chatbots', '_id', callback);
    progress.setMaapSpecificCount(chatBotSpCount);
    LogUtils.i(TAG, 'chatBotSpCount: ' + chatBotSpCount);
    let chatbotsSpPage = Math.ceil(chatBotSpCount / DbConstants.QUERY_PAGE_NUM);
    this.queryChatbotsColumn = await this.queryColumns(context, 'chatbots');
    LogUtils.i(TAG, 'chatbotsSpPage: ' + chatbotsSpPage);
    for (let curPage = 1; curPage <= chatbotsSpPage; curPage++) {
      await this.queryChatbotsSpTable(context, DbConstants.QUERY_PAGE_NUM * (curPage - 1), progress);
    }
    progress.updateProcessCount(progress.getSpecificpNextCloneProgress());
  }

  public async queryChatbotsSpTable(context: common.Context, index: number,
    progress: BackupRestoreProgress): Promise<void> {
    let predicatesInfo = new rdb.RdbPredicates('specific_chatbots');
    let rdbStore = await this.initDB(context);
    if (rdbStore === undefined) {
      LogUtils.e(TAG, `[queryMaapInfo] failed : rdbStore === undefined`);
      return;
    }
    try {
      let queryColumn = await this.queryColumns(context, 'specific_chatbots');
      predicatesInfo = this.buildPredicatesInfo(predicatesInfo, '_id', index);
      let resultSet = await rdbStore.query(predicatesInfo, queryColumn);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'no raw Rcs found.');
        return;
      }
      let count = resultSet.rowCount;
      let failCount = 0;
      let successCount = 0;
      do {
        let maapID = resultSet.getLong(resultSet.getColumnIndex('_id'));
        let serviceId = resultSet.getString(resultSet.getColumnIndex('service_id'));
        let type = resultSet.getString(resultSet.getColumnIndex('type'));
        let expires = resultSet.getString(resultSet.getColumnIndex('expires'));
        let cacheControl = resultSet.getString(resultSet.getColumnIndex('cache_control'));
        let eTag = resultSet.getString(resultSet.getColumnIndex('e_tag'));
        let valuesMaapInfo: ValuesBucket = {
          'service_id': serviceId,
          'type': type,
          'expires': expires,
          'cache_control': cacheControl,
          'e_tag': eTag,
        };
        let newRawMaapId = await newDbHelperClone.insertSpChatbots(context, valuesMaapInfo);
        if (newRawMaapId === -1) {
          failCount++;
        } else {
          successCount++;
        }
      } while (resultSet.goToNextRow());
      resultSet.close();
      progress.updateProcessCount(progress.getProcessCount() + count);
      LogUtils.i(TAG, 'queryMaapInfo Total Migrations count: ' + count + ',Successful Migrations successCount: ' + successCount + 'Failed Migrations failCount: ' + failCount);
    } catch (error) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `queryChatbotsSpTable code:${error.code} msg:${error.message}`);
      LogUtils.w(TAG, 'MaapInfo error : ' + JSON.stringify(error));
    }
  }

  private async doChatBotTableCopy(context: common.Context, callback: Function, progress: BackupRestoreProgress) {
    let chatBotCount = await this.queryDataCount(context, 'chatbots', '_id', callback);
    progress.setMaapCount(chatBotCount);
    LogUtils.i(TAG, 'ChatBotCount: ' + chatBotCount);
    let chatbotsPage = Math.ceil(chatBotCount / DbConstants.QUERY_PAGE_NUM);
    this.queryChatbotsColumn = await this.queryColumns(context, 'chatbots');
    LogUtils.i(TAG, 'chatbotsPage: ' + chatbotsPage);
    for (let curPage = 1; curPage <= chatbotsPage; curPage++) {
      await this.queryChatbotsTable(context, DbConstants.QUERY_PAGE_NUM * (curPage - 1), progress);
    }
    progress.updateProcessCount(progress.getMaapNextCloneProgress());
  }

  public async queryChatbotsTable(context: common.Context, index: number,
    progress: BackupRestoreProgress): Promise<void> {
    let predicatesInfo = new rdb.RdbPredicates('chatbots');
    let rdbStore = await this.initDB(context);
    if (rdbStore === undefined) {
      LogUtils.e(TAG, `[queryMaapInfo] failed : rdbStore === undefined`);
      return;
    }
    try {
      let queryColumn =
        ['_id', 'service_id', 'service_name', 'service_description', 'callback_phone_number', 'sms', 'service_icon',
          'category_list', 'brief', 'favorite', 'email', 'website', 'address', 'address_lable', 'last_suggested_list',
          'pinyin', 'pinyin_short', 'colour', 'background_image', 'verified', 'verified_by', 'verified_expires',
          'expires', 'cache_control', 'e_tag', 'tc_page', 'recent_use_time', 'disturb', 'persistent_menu', 'share_info',
          'service_provider'];
      predicatesInfo = this.buildPredicatesInfo(predicatesInfo, '_id', index);
      let resultSet = await rdbStore.query(predicatesInfo, queryColumn);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'no raw Rcs found.');
        return;
      }
      let count = resultSet.rowCount;
      let failCount = 0;
      let successCount = 0;
      do {
        let maapID = resultSet.getLong(resultSet.getColumnIndex('_id'));
        let serviceId = resultSet.getString(resultSet.getColumnIndex('service_id'));
        let serviceName = resultSet.getString(resultSet.getColumnIndex('service_name'));
        let serviceDescription = resultSet.getString(resultSet.getColumnIndex('service_description'));
        let callbackPhoneNumber = resultSet.getString(resultSet.getColumnIndex('callback_phone_number'));
        let sms = resultSet.getString(resultSet.getColumnIndex('sms'));
        let serviceIcon = resultSet.getString(resultSet.getColumnIndex('service_icon'));
        let categoryList = resultSet.getString(resultSet.getColumnIndex('category_list'));
        let brief = resultSet.getLong(resultSet.getColumnIndex('brief'));
        let favorite = resultSet.getLong(resultSet.getColumnIndex('favorite'));
        let email = resultSet.getString(resultSet.getColumnIndex('email'));
        let website = resultSet.getString(resultSet.getColumnIndex('website'));
        let address = resultSet.getString(resultSet.getColumnIndex('address'));
        let addressLable = resultSet.getString(resultSet.getColumnIndex('address_lable'));
        let lastSuggestedList = resultSet.getString(resultSet.getColumnIndex('last_suggested_list'));
        let pinyin = resultSet.getString(resultSet.getColumnIndex('pinyin'));
        let pinyin_short = resultSet.getString(resultSet.getColumnIndex('pinyin_short'));
        let colour = resultSet.getString(resultSet.getColumnIndex('colour'));
        let backgroundImage = resultSet.getString(resultSet.getColumnIndex('background_image'));
        let verified = resultSet.getLong(resultSet.getColumnIndex('verified'));
        let verifiedBy = resultSet.getString(resultSet.getColumnIndex('verified_by'));
        let verifiedExpires = resultSet.getString(resultSet.getColumnIndex('verified_expires'));
        let expires = resultSet.getLong(resultSet.getColumnIndex('expires'));
        let cacheControl = resultSet.getString(resultSet.getColumnIndex('cache_control'));
        let eTag = resultSet.getString(resultSet.getColumnIndex('e_tag'));
        let tcPage = resultSet.getString(resultSet.getColumnIndex('tc_page'));
        let recentUseTime = resultSet.getString(resultSet.getColumnIndex('recent_use_time'));
        let disturb = resultSet.getLong(resultSet.getColumnIndex('disturb'));
        let persistentMenu = resultSet.getString(resultSet.getColumnIndex('persistent_menu'));
        let shareInfo = resultSet.getLong(resultSet.getColumnIndex('share_info'));
        let serviceProvider = resultSet.getString(resultSet.getColumnIndex('service_provider'));
        let valuesMaapInfo: ValuesBucket = {
          'service_id': serviceId,
          'service_name': serviceName,
          'service_description': serviceDescription,
          'callback_phone_number': callbackPhoneNumber,
          'sms': sms,
          'service_icon': serviceIcon,
          'category_list': categoryList,
          'brief': brief,
          'favorite': favorite,
          'email': email,
          'website': website,
          'address': address,
          'address_lable': addressLable,
          'last_suggested_list': lastSuggestedList,
          'pinyin': pinyin,
          'pinyin_short': pinyin_short,
          'colour': colour,
          'background_image': backgroundImage,
          'verified': verified,
          'verified_by': verifiedBy,
          'verified_expires': verifiedExpires,
          'expires': expires,
          'cache_control': cacheControl,
          'e_tag': eTag,
          'tc_page': tcPage,
          'recent_use_time': recentUseTime,
          'disturb': disturb,
          'persistent_menu': persistentMenu,
          'share_info': shareInfo,
          'service_provider': serviceProvider
        };
        let newRawMaapId = await newDbHelperClone.insertChatbots(context, valuesMaapInfo);
        if (newRawMaapId === -1) {
          failCount++;
        } else {
          successCount++;
        }
      } while (resultSet.goToNextRow());
      resultSet.close();
      progress.updateProcessCount(progress.getProcessCount() + count);
      LogUtils.i(TAG, 'queryMaapInfo Total Migrations count: ' + count + ',Successful Migrations successCount: ' + successCount + 'Failed Migrations failCount: ' + failCount);
    } catch (error) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `queryChatbotsTable code:${error.code} msg:${error.message}`);
      LogUtils.w(TAG, 'MaapInfo error : ' + JSON.stringify(error));
    }
  }

  private async isFavoriteTableExist(context: common.Context): Promise<boolean> {
    if (context == null) {
      LogUtils.w(TAG, `isFavorateTableExist failed by null context`)
      return false;
    }
    let isExist = false;
    let resultSet: rdb.ResultSet | undefined = undefined;
    try {
      let rdbStore = await this.initDB(context);
      if (rdbStore === undefined) {
        LogUtils.e(TAG, `[isFavorateTableExist] failed : rdbStore === undefined`)
        return false;
      }
      let sql = 'SELECT name FROM sqlite_master WHERE type = \'table\';';
      resultSet = await rdbStore.querySql(sql);
      if (resultSet === undefined) {
        LogUtils.w(TAG, 'resultSet === undefined');
        return false;
      }
      while (resultSet.goToNextRow()) {
        const tableName = resultSet.getString(resultSet.getColumnIndex('name'));
        if (tableName === DbConstants.TABLE_FAVORITE_INFO) {
          isExist = true;
        }
      }
      resultSet.close();
      return isExist;
    } catch (err) {
      if (resultSet) {
        resultSet.close();
      }
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `isFavorateTableExist code:${err.code} msg:${err.message}`);
      LogUtils.e(TAG, `isFavorateTableExist failed, code is ${err.code},message is ${err.message}`);
      return false;
    }
  }

  private async buildBatchInsertFav(resultSet: rdb.ResultSet, context: common.Context,
    oldFavMap: HashMap<string, number>, ids: needUpdateIds): Promise<ValuesBucket | undefined> {
      const receiverNumber = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.favorateInfo.receiverNumber));
      const senderNumber = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.favorateInfo.senderNumber));
      const isSender = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.favorateInfo.isSender));
      const startTime = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.favorateInfo.startTime));
      let number = isSender ? senderNumber : receiverNumber;
      if (oldFavMap.get(number + ':' + isSender + ':' + startTime) !== undefined) {
        LogUtils.e(TAG, 'buildBatchInsertFav, Duplicate fav info, skip');
        return undefined;
      }
      let favItem = new FavorateInfoItem();
      let msgId = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.favorateInfo.msgId));
      let rcsId = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.favorateInfo.rcsId));
      let smsType = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.favorateInfo.smsType));
      let msgType = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.favorateInfo.msgType));
      // 如果可以关联到新机已迁移的rcs_info，需要赋值为新机正确的 rcs_id；
      // 如果无关联，需要重新生成rcs_id，避免新接收到的信息与收藏的 rcs_id 冲突
      // rcs富媒体有无关联都要保证rcs_id唯一，rcs文本可以为0
      let newRcsId: number | undefined;
      // 增强信息富媒体，要保证一定获取到 rcs_id
      if (smsType == 99) {
        newRcsId = RAW_RCS_ID_MAP.get(rcsId);
        if (newRcsId === undefined) {
          // 重新生成新的 rcs_id
          let maxRcsId = await newDbHelper.queryMaxInfoId(context, true);
          if (maxRcsId == -1) {
            LogUtils.e(TAG, 'buildBatchInsertFav, get max msg_id error. skip this item');
            return undefined;;
          }
          // 记录新生成的rcs_id，后续更新mms_part.rcs_id
          newRcsId = maxRcsId;
          ids.partRcsIdNeedUpdate = newRcsId;
        }
      } else {
        newRcsId = 0;
      }
      let groupId = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.favorateInfo.groupId));
      // msg_id 要保证一定存在且唯一，确保应用侧删除逻辑正常
      let newMsgId: number | undefined = RAW_MSG_ID_MAP.get(msgId);
      if (newMsgId === undefined) {
        let maxMsgId = await newDbHelper.queryMaxInfoId(context, false);
        if (maxMsgId == -1) {
          LogUtils.e(TAG, 'buildBatchInsertFav, get max msg_id error. skip this item');
          return undefined;;
        }
        newMsgId = maxMsgId;
        // 记录新生成的msg_id，后续更新mms_part.msg_id
        ids.partMsgIdNeedUpdate = newMsgId;
      } else {
        // 关联已有信息场景，groupId 要取新机的 groupId
        let tmpGroupId = smsType == 99 ? NEW_RCS_GROUP_ID_MAP.get(newRcsId) : NEW_MSG_GROUP_ID_MAP.get(newMsgId);
        groupId = tmpGroupId === undefined ? 0 : tmpGroupId;
      }
      favItem.setMsgId(newMsgId);
      favItem.setRcsId(newRcsId);
      let keepTime = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.favorateInfo.keepTime));
      let msgTitle = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.favorateInfo.msgTitle));
      let msgContent = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.favorateInfo.msgContent));
      let sessionType = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.favorateInfo.sessionType));
      let partType = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.favorateInfo.partType));
      let partLocationPath = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.favorateInfo.partLocationPath));
      let detectResContent = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.favorateInfo.detectResContent));
      let sessionId = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.favorateInfo.sessionId));
      let recordingTime = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.favorateInfo.recordingTime));
      let partSize = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.favorateInfo.partSize));
      favItem.setMsgType(msgType).setKeepTime(keepTime).setSmsType(smsType).setMsgTitle(msgTitle)
        .setMsgContent(msgContent).setSessionType(sessionType).setGroupId(groupId).setPartType(partType)
        .setSessionId(sessionId).setRecordingTime(recordingTime).setPartSize(partSize).setStartTime(startTime)
        .setPartLocationPath(partLocationPath).setDetectResContent(detectResContent).setSenderNumber(senderNumber)
        .setReceiverNumber(receiverNumber).setIsSender(isSender);
    return favItem.createValuesBucket();
 }

  private async migrateFromFavTable(context: common.Context, index: number, favorateMap: HashMap<string, number>,
      progress: BackupRestoreProgress): Promise<number> {
    LogUtils.e(TAG, `migrateFromFavTable start`)
    let failCount = 0;
    let successCount = 0;
    let resultSet: rdb.ResultSet | undefined = undefined;
    let predicates = new rdb.RdbPredicates(DbConstants.TABLE_FAVORITE_INFO);
    let rdbStore = await this.initDB(context);
    if (rdbStore === undefined) {
      LogUtils.e(TAG, `[migrateFromFavTable] failed : rdbStore === undefined`)
      return successCount;
    }
    try {
      predicates = this.buildPredicatesInfoWithoutOrder(predicates, index);
      resultSet = await rdbStore.query(predicates, this.queryFavoriteColumn);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.e(TAG, 'no raw fav found.');
        return successCount;
      }
      let count = resultSet.rowCount;
      do {
        const ids: needUpdateIds = {
          partMsgIdNeedUpdate: undefined,
          partRcsIdNeedUpdate: undefined
        };
        let favValue: ValuesBucket | undefined = await this.buildBatchInsertFav(resultSet, context, favorateMap, ids);
        if (favValue !== undefined) {
          let insertNum = await newDbHelper.insertFavorate(context, favValue);
          if (insertNum == -1) {
            failCount ++;
          } else {
            // 迁移成功后更新mms_part.msg_id ,mms_part.rcs_id
            this.updatePartMsgIdAndRcsId(context, resultSet, ids);
            successCount ++;
          }
        }
      } while (resultSet.goToNextRow());
      resultSet.close();
      this.favorateFailed = this.favorateFailed + failCount;
      progress.updateProcessCount(progress.getProcessCount() + count);
      LogUtils.i(TAG, 'favorate Total Migrations count: ' + count + ',Successful Migrations successCount: ' +
        successCount + 'Failed Migrations failCount: ' + failCount);
    } catch (error) {
      if (resultSet) {
        resultSet.close();
      }
      LogUtils.w(TAG, `migrateFromFavTable code:${error.code} msg:${error.message}`);
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `migrateFromFavTable code:${error.code} msg:${error.message}`);
    }
    return successCount;
  }

  private async updatePartMsgIdAndRcsId(context: common.Context, resultSet: rdb.ResultSet, ids: needUpdateIds) {
    let upInfoValues: ValuesBucket = {};
    let msgIds: number[] | undefined = undefined;
    if (ids.partMsgIdNeedUpdate != undefined && ids.partMsgIdNeedUpdate != 0) {
      upInfoValues['msg_id'] = ids.partMsgIdNeedUpdate;
    }
    if (ids.partRcsIdNeedUpdate != undefined && ids.partRcsIdNeedUpdate != 0) {
      upInfoValues['rcs_id'] = ids.partRcsIdNeedUpdate;
    }
    if (Object.keys(upInfoValues).length === 0) {
      return;
    }
    let oldMsgId = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.favorateInfo.msgId));
    msgIds = NEW_FAV_MSG_ID_MAP.get(oldMsgId);
    if (msgIds === undefined) {
      return;
    }
    await newDbHelper.batchUpdatePart(context, msgIds, upInfoValues);
  }

  private async doFavoriteCopyFromFavTable(context: common.Context, callback: Function,
      progress: BackupRestoreProgress) {
    let idFailsSuccess = 0;
    let idSuccess = 0;
    this.queryPartColumn = await this.queryColumns(context, DbConstants.TABLE_FAVORITE_INFO);
    let favorateCount = await this.queryFavDataCount(context, callback);
    LogUtils.i(TAG, 'favorateCount: ' + favorateCount);
    let favoratePage = Math.ceil(favorateCount / DbConstants.QUERY_PAGE_NUM);
    this.queryFavoriteColumn = await this.queryColumns(context, DbConstants.TABLE_FAVORITE_INFO);
    let favorateMap = await newDbHelper.queryFavoriteInfo(context);
    LogUtils.i(TAG, 'favorateMap: ' + favorateMap.length);
    LogUtils.i(TAG, 'favoratePage: ' + favoratePage);
    for (let curPage = 1; curPage <= favoratePage; curPage++) {
      idSuccess += await this.migrateFromFavTable(context, DbConstants.QUERY_PAGE_NUM * (curPage - 1),
         favorateMap, progress);
    }
    this.favoriteSuccess = idSuccess + idFailsSuccess;
    progress.updateProcessCount(progress.getFavNextCloneProgress());
  }

  private async doFavoriteInfoCopy(context: common.Context, callback: Function, progress: BackupRestoreProgress) {
    // 如果 fav 表存在，优先从 fav 迁移
    if (await this.isFavoriteTableExist(context)) {
      LogUtils.w(TAG, 'Favorite Table Exist ');
      await this.doFavoriteCopyFromFavTable(context, callback, progress);
    } else {
      await this.doFavoriteTableCopyFromCollectField(context, callback, progress);
    }
  }

  private async doFavoriteTableCopyFromCollectField(context: common.Context, callback: Function,
      progress: BackupRestoreProgress) {
    let idFailsSuccess = 0;
    let idSuccess = 0;
    let favoriteCount = await this.queryDataCount(context,
      DbConstants.TABLE_FAVORITE_INFO,
      NextOldDBData.smsMmsInfo.msgId, callback);
    LogUtils.i(TAG, 'favoriteCount: ' + favoriteCount);
    let favoritePage = Math.ceil(favoriteCount / DbConstants.QUERY_PAGE_NUM);
    this.queryFavoriteColumn = await this.queryColumns(context, DbConstants.TABLE_FAVORITE_INFO);
    let favoriteIdFails = this.getFromPreferences(context,
      DbConstants.NEXT_FAVORITE_ID_FAIL,
      []) as number[];
    let favoriteMap = await newDbHelper.queryFavoriteInfo(context);
    LogUtils.i(TAG, 'favoriteMap: ' + favoriteMap.length);
    if (favoriteIdFails.length > 0) {
      idFailsSuccess += await this.queryFavoriteTable(context, -1, favoriteIdFails, favoriteMap, progress, callback);
      favoriteIdFails = this.getFromPreferences(context,
        DbConstants.NEXT_FAVORITE_ID_FAIL,
        []) as number[];
    }
    let favoriteIdSuccess = this.getFromPreferences(context, DbConstants.NEXT_FAVORITE_ID_SUCCESS, 0) as number;
    LogUtils.i(TAG, 'favoritePage: ' + favoritePage);
    for (let curPage = 1; curPage <= favoritePage; curPage++) {
      idSuccess += await this.queryFavoriteTable(context,
        DbConstants.QUERY_PAGE_NUM * (curPage - 1) + favoriteIdSuccess,
        favoriteIdFails, favoriteMap, progress, callback);
    }
    this.favoriteSuccess = idSuccess + idFailsSuccess;
    progress.updateProcessCount(progress.getFavNextCloneProgress());
  }

  private async queryTotalMsgCount(context: Context, callBack: Function, progress: BackupRestoreProgress) {
    let threadsCount = await this.queryDataCount(context, DbConstants.NEXT_OLD_SESSION_NAME,
      NextOldDBData.session.id, callBack);
    let smsCount = await this.queryDataCount(context,
      DbConstants.NEXT_OLD_SMS_NAME,
      NextOldDBData.smsMmsInfo.msgId, callBack);
    let pduCount = await this.queryDataCount(context, DbConstants.NEXT_OLD_PDU_NAME,
      NextOldDBData.mmsPdu.id, callBack);
    let partCount = await this.queryDataCount(context, DbConstants.NEXT_OLD_PART_NAME, NextOldDBData.mmsPart.id);
    let protocolCount = await this.queryDataCount(context, DbConstants.NEXT_OLD_PROTOCOL_NAME,
      NextOldDBData.mmsProtocol.id);
    let rcsCount = await this.queryDataCount(context, DbConstants.NEXT_OLD_RCS_NAME,
      NextOldDBData.rcsInfo.rcsId, callBack);
    let subsectionCount = await this.queryDataCount(context, DbConstants.NEXT_OLD_SUBSECTION_NAME,
      NextOldDBData.smsSubsection.id);
    let favoriteCount = await this.queryFavCount(context, callBack);
    let maapCount = await this.queryDataCount(context, 'chatbots', '_id', callBack);
    let maapSpCount = await this.queryDataCount(context, 'specific_chatbots', '_id', callBack);
    progress.setDelInvalidDraftCount(Math.floor(smsCount * BackupRestoreProgress.delInvalidDraftRatio) + 1);
    let totalCount = threadsCount + smsCount + pduCount + partCount + protocolCount + rcsCount + subsectionCount +
      favoriteCount + maapCount + maapSpCount + threadsCount + threadsCount;
    totalCount = totalCount + threadsCount + await newDbHelper.queryDataCount(context, DbConstants.TABLE_SESSION,
      NextOldDBData.session.id)
    totalCount = totalCount + progress.getCheckSessionCount() + progress.getCheckSmsCount() +
      progress.getDelInvalidDraftCount();
    progress.setThreadCount(threadsCount);
    progress.setSmsCount(smsCount);
    progress.setPduCount(pduCount);
    progress.setPartCount(partCount);
    progress.setRcsChatCount(rcsCount);
    progress.setProtocolCount(protocolCount);
    progress.setSubsectionCount(subsectionCount);
    progress.setMaapCount(maapCount);
    progress.setMaapSpecificCount(maapSpCount);
    progress.setFavoriteCount(favoriteCount);
    let countList: string = 'thread[' + threadsCount + '],sms[' + smsCount + '],pdu[' + pduCount +
      '],part[' + partCount + '],rcsChat[' + rcsCount + '],protocol[' + protocolCount + '],subsection[' +
      subsectionCount + '],favorite[' + favoriteCount + '],maapCount[' + maapCount + '],maapSpCount[' +
      maapSpCount + ']';
    LogUtils.i(TAG, `update progress total count ${totalCount}, listed: ${countList}`);
    const backupRestoreResult = BackupRestoreResult.getInstance();
    backupRestoreResult.insertOrUpdateBackupInfo('sms', `msg_count_list : ${countList}`);
    progress.setTotalCount(totalCount);
    return totalCount;
  }

  private getFromPreferences(context: common.Context, key: string,
    defValue: dataPreferences.ValueType):dataPreferences.ValueType {
    LogUtils.i(TAG, 'getFromPreferences start');
    let preferences: dataPreferences.Preferences | null = this.getPreferences(context);
    let value:dataPreferences.ValueType = defValue;
    if (preferences == null) {
      LogUtils.e(TAG, 'getFromPreferences is null');
      return defValue;
    }
    try {
      value = preferences.getSync(key, defValue);
      LogUtils.i(TAG, 'getFromPreferences Succeeded');
    } catch (err) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `getFromPreferences code:${err.code} msg:${err.message}`);
      let e: BusinessError = err as BusinessError;
      LogUtils.e(TAG, 'Failed to get value code : ' + e.code + ' , message : ' + e.message);
    }
    return value;
  }

  private getPreferences(context: common.Context) {
    if (myPreferences) {
      LogUtils.i(TAG, 'has Preferences');
      return myPreferences;
    }
    try {
      let preferences: dataPreferences.Preferences = dataPreferences.getPreferencesSync(context, {name: PREF_NAME});
      myPreferences = preferences;
      LogUtils.i(TAG, 'Preferences init');
      return preferences;
    } catch (error) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `getPreferences code:${error.code} msg:${error.message}`);
      let err: BusinessError = error as BusinessError;
      LogUtils.e(TAG, 'getPreferences with error message: ' + err.message + ', error code: ' + err.code);
    }
    return null;
  }

  private async initDB(context: common.Context): Promise<rdb.RdbStore | undefined> {
    if (!context) {
      LogUtils.w(TAG, '[createDB] context is empty');
      return undefined;
    }
    if (this.rdbStoreHelper) {
      return Promise.resolve(this.rdbStoreHelper);
    }
    try {
      let OLD_CONFIG: rdb.StoreConfig = {
        name: DbConstants.nextCloneDatabaseName,
        securityLevel: rdb.SecurityLevel.S1
      };
      LogUtils.i(TAG, '[createDB] OLD_CONFIG');
      this.rdbStoreHelper = await rdb.getRdbStore(context, OLD_CONFIG);
    } catch (error) {
      LogUtils.w(TAG, '[createDB] failed : ' + JSON.stringify(error));
      throw new ErrorUtils(400, JSON.stringify(error));
    }
    if (this.rdbStoreHelper) {
      LogUtils.i(TAG, '[createDB] return getRdbStore, version: ' + this.rdbStoreHelper.version);
      return Promise.resolve(this.rdbStoreHelper);
    }
    return undefined;
  }

  private async queryColumns(context: common.Context, table: String): Promise<string[]> {
    let rdbStore = await this.initDB(context);
    if (rdbStore === undefined) {
      LogUtils.i(TAG, `queryColumns, getRdbStore failed`)
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

  private async checkOldDbIsValid(context: common.Context) : Promise<void> {
    try {
      let rdbStore = await this.initDB(context);
      if (rdbStore === undefined) {
        LogUtils.e(TAG, `check old db valid failed : rdbStore === undefined`)
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
          `checkOldDb invalid sessionId :${resultSet.rowCount}`);
      }
      while (resultSet.goToNextRow()) {
        let id = resultSet.getLong(resultSet.getColumnIndex('id'));
        LogUtils.e(TAG, `invalid old session id ${id}`);
      }
      resultSet =
        (rdbStore as rdb.RdbStore).querySqlSync('SELECT msg_id\n' +
          'FROM sms_mms_info WHERE ((session_id NOT IN ( SELECT id FROM session )) AND (is_blocked = 0));\n');
      if (resultSet !== undefined && resultSet.rowCount > 0) {
        const backupRestoreResult = BackupRestoreResult.getInstance();
        backupRestoreResult.insertOrUpdateBackupInfo('sms',
          `checkOldDb invalid msgid :${resultSet.rowCount}`);
      }
      while (resultSet.goToNextRow()) {
        let msgId = resultSet.getLong(resultSet.getColumnIndex('msg_id'));
        LogUtils.e(TAG, `invalid old msg_id ${msgId}`);
      }
      resultSet.close();
    } catch (err) {
      LogUtils.e(TAG, `check old db valid failed, code is ${err.code},message is ${err.message}`);
    }
  }

  private async queryUnreadCount(context: Context, progress: BackupRestoreProgress): Promise<number> {
    if (context == null) {
      LogUtils.w(TAG, `queryUnreadCount failed by null context`)
      return Promise.resolve(0);
    }
    let totalUnreadCount: number = 0;
    try {
      let rdbStore = await this.initDB(context);
      if (rdbStore === undefined) {
        LogUtils.e(TAG, `[queryUnreadCount] failed : rdbStore === undefined`)
        return Promise.resolve(0);
      }
      let sql: string = 'SELECT \n' +
        '(SELECT COUNT(*) FROM sms_mms_info WHERE is_read = 0) + \n' +
        '(SELECT COUNT(*) FROM rcs_info WHERE is_read = 0) AS total_unread;';
      let resultSet: rdb.ResultSet = rdbStore.querySqlSync(sql);
      if (resultSet === undefined) {
        LogUtils.w(TAG, 'resultSet === undefined');
        return Promise.resolve(0);
      }
      while (resultSet.goToNextRow()) {
        totalUnreadCount = resultSet.getLong(resultSet.getColumnIndex('total_unread'));
      }
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `ori_unread_count :${totalUnreadCount}`);
      return totalUnreadCount;
    } catch (err) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `queryUnreadCount code:${err.code} msg:${err.message}`);
      LogUtils.e(TAG, `queryUnreadCount failed, code is ${err.code},message is ${err.message}`);
      return Promise.resolve(0);
    }
  }

  private async queryBlockedMsgCount(context: Context, progress: BackupRestoreProgress): Promise<number> {
    if (context == null) {
      LogUtils.w(TAG, `queryBlockedMsgCount failed by null context`)
      return Promise.resolve(0);
    }
    let totalBlockedCount: number = 0;
    try {
      let rdbStore = await this.initDB(context);
      if (rdbStore === undefined) {
        LogUtils.e(TAG, `[queryBlockedMsgCount] failed : rdbStore === undefined`)
        return Promise.resolve(0);
      }
      let sql: string = 'SELECT \n' +
        '(SELECT COUNT(*) FROM sms_mms_info WHERE is_blocked = 1) AS total_blocked;'
      let resultSet: rdb.ResultSet = rdbStore.querySqlSync(sql);
      if (resultSet === undefined) {
        LogUtils.w(TAG, 'resultSet === undefined');
        return Promise.resolve(0);
      }
      while (resultSet.goToNextRow()) {
        totalBlockedCount = resultSet.getLong(resultSet.getColumnIndex('total_blocked'));
      }
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `ori_blocked_count :${totalBlockedCount}`);
      return totalBlockedCount;
    } catch (err) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `queryBlockedMsgCount code:${err.code} msg:${err.message}`);
      LogUtils.e(TAG, `queryBlockedMsgCount failed, code is ${err.code},message is ${err.message}`);
      return Promise.resolve(0);
    }
  }

  public async queryFavCount(context: common.Context, callBack: Function): Promise<number> {
    if (await this.isFavoriteTableExist(context)) {
      return await this.queryFavDataCount(context, callBack);
    } else {
      return await this.queryDataCount(context, DbConstants.TABLE_FAVORITE_INFO,
        NextOldDBData.smsMmsInfo.msgId, callBack);
    }
  }

  private async queryFavDataCount(context: common.Context, callBack?: Function): Promise<number> {
    if (context == null) {
      LogUtils.w(TAG, `queryFavDataCount failed by null context`)
      return Promise.resolve(0);
    }
    let count = 0;
    try {
      let predicatesInfo = new rdb.RdbPredicates(DbConstants.TABLE_FAVORITE_INFO);
      let rdbStore = await this.initDB(context);
      if (rdbStore === undefined) {
        LogUtils.e(TAG, `[queryFavDataCount] failed : rdbStore === undefined`)
        if (callBack) {
          callBack(new ErrorUtils(400, 'rdbStore === undefined'));
        }
        return Promise.resolve(0);
      }
      let queryColumn = [`count(msg_id)`];
      let resultSet = await rdbStore.query(predicatesInfo, queryColumn);
      if (resultSet === undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'query no fav found.');
        return Promise.resolve(0);
      }
      count = resultSet.getLong(0);
      LogUtils.i(TAG, '[queryFavDataCount] count : ' + count);
      resultSet.close();
    } catch (error) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `queryFavDataCount code:${error.code} msg:${error.message}`);
      LogUtils.e(TAG, '[queryFavDataCount] error : ' + JSON.stringify(error))
      if (callBack) {
        callBack(new ErrorUtils(400, JSON.stringify(error)));
      }
    }
    return Promise.resolve(count);
  }

  public async queryDataCount(context: common.Context, tableName:string,
    queryKey:string, callBack?: Function): Promise<number> {
    if (context == null) {
      LogUtils.w(TAG, `queryDataCount failed by null context`)
      return Promise.resolve(0);
    }
    let count = 0;
    try {
      let tempTableName = '';
      if (DbConstants.TABLE_FAVORITE_INFO == tableName) {
        tempTableName = DbConstants.TABLE_SMS_MMS_INFO
      } else {
        tempTableName = tableName;
      }
      let predicatesInfo = new rdb.RdbPredicates(tempTableName);
      let rdbStore = await this.initDB(context);
      if (rdbStore === undefined) {
        LogUtils.e(TAG, `[queryDataCount] failed : rdbStore === undefined`)
        if (callBack) {
          callBack(new ErrorUtils(400, 'rdbStore === undefined'));
        }
        return Promise.resolve(0);
      }
      let queryColumn = [`count(${queryKey})`];
      if (tableName === DbConstants.TABLE_FAVORITE_INFO) {
        predicatesInfo.equalTo('is_collect', 1);
      }
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

  private async queryDataAllCount(context: common.Context, tableName:string, queryKey:string): Promise<number> {
    if (context == null) {
      LogUtils.w(TAG, `queryDataAllCount failed by null context`)
      return Promise.resolve(0);
    }
    let count = 0;
    try {
      let predicatesInfo = new rdb.RdbPredicates(tableName);
      let rdbStore = await this.initDB(context);
      if (rdbStore === undefined) {
        LogUtils.e(TAG, `[queryDataAllCount] failed : rdbStore === undefined`)
        return Promise.resolve(0);
      }
      let queryColumn = [`count(${queryKey})`];
      let resultSet = await rdbStore.query(predicatesInfo, queryColumn);
      if (resultSet === undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'query no Count found.');
        return Promise.resolve(0);
      }
      count = resultSet.getLong(0);
      LogUtils.i(TAG, '[queryDataAllCount] count : ' + count);
      resultSet.close();
    } catch (error) {
      LogUtils.e(TAG, '[queryDataAllCount] error : ' + JSON.stringify(error))
    }
    return Promise.resolve(count);
  }

  private async querySessionTable(context: common.Context, index: number, sessionIdFails: number[],
      sessionMap: HashMap<string, ValuesBucket>, progress: BackupRestoreProgress,
      callBack?: Function): Promise<number> {
    LogUtils.e(TAG, `[querySessionTable] start`)
    let failCount = 0;
    let successCount = 0;
    try {
      let sessionTable = new rdb.RdbPredicates(DbConstants.NEXT_OLD_SESSION_NAME);
      let rdbStore = await this.initDB(context);
      if (rdbStore === undefined) {
        LogUtils.e(TAG, `[querySessionTable] failed : rdbStore === undefined`)
        if (callBack) {
          callBack(new ErrorUtils(400, 'rdbStore === undefined'));
        }
        return successCount;
      }
      if (index != -1) {
        sessionTable = this.buildPredicatesInfo(sessionTable, NextOldDBData.session.id, index);
      } else {
        sessionTable = this.buildPredicatesInfoFail(sessionTable, NextOldDBData.session.id, sessionIdFails);
      }
      let resultSet = await rdbStore.query(sessionTable, this.querySessionColumn);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'no raw thread found.');
        return successCount;
      }
      let count = resultSet.rowCount;
      do {
        let successFlag = await this.processSessionItem(sessionMap, resultSet, context);
        if (successFlag) {
          successCount++;
        } else {
          failCount++;
        }
      } while (resultSet.goToNextRow());
      resultSet.close();
      this.sessionFailed = this.sessionFailed + failCount;
      progress.updateProcessCount(progress.getProcessCount() + count);
      LogUtils.i(TAG, 'session Total Migrations count: ' + count + ',Successful Migrations successCount: ' + successCount + 'Failed Migrations failCount: ' + failCount);
    } catch (error) {
      LogUtils.w(TAG, 'querySessionTable error : ' + JSON.stringify(error));
      if (callBack) {
        callBack(new ErrorUtils(400, JSON.stringify(error)));
      }
    }
    return successCount;
  }

  private async processSessionItem(sessionMap: HashMap<string, ValuesBucket>, resultSet: rdb.ResultSet,
    context: Context): Promise<boolean> {
    let telephony = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.session.telephone));
    let contactsNum = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.session.contactsNum));
    let hasDraft = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.session.hasDraft));
    // check the same session(have the same phone Number)
    if (sessionMap.hasKey(telephony)) {
      let session: ValuesBucket = sessionMap.get(telephony);
      let time = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.session.time));
      let pinningTime = this.getPinningTime(resultSet, session.pinningTime as number);
      // if no need update or insert, should update new sessionId
      let sessionId = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.session.id));
      RAW_SESSION_ID_MAP.set(sessionId, session.id as number);
      if (session.has_draft == 1 || hasDraft == 1) { // 旧机或新机都有草稿，对应session需要处理草稿
        SESSION_DROP_DRAFT.set(session.id as number, new DropDraftInfo(hasDraft == 1 ? true : false, -1));
      }
      if (session.time != null && time > session.time) {
        return await this.updateSessionItem(resultSet, session, context, pinningTime);
      } else {
        if (hasDraft == 1) { // 当旧机中session时间早，但存在草稿时，更新为旧机草稿
          let ret: boolean = await this.updateSessionDraft(resultSet, session.id as number, context);
        }
        if (pinningTime > 0) {
          return await this.updatePinningTime(resultSet, session.id as number, context, pinningTime);
        }
      }
      return true;
    } else {
      if (contactsNum === 1 && telephony.length > 7) {
        for (let k of sessionMap.keys()) {
          if (k && k.indexOf(',') === -1 && k.length > 7) {
            const phone1 = k.slice(-7);
            const phone2 = telephony.slice(-7);
            if (phone1 === phone2) {
              let session: ValuesBucket = sessionMap.get(k);
              let time = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.session.time));
              let pinningTime = this.getPinningTime(resultSet, session.pinningTime as number);
              // if no need update or insert, should update new sessionId
              let sessionId = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.session.id));
              RAW_SESSION_ID_MAP.set(sessionId, session.id as number);
              if (session.has_draft == 1 || hasDraft == 1) { // 旧机或新机都有草稿，对应session需要处理草稿
                SESSION_DROP_DRAFT.set(session.id as number, new DropDraftInfo(hasDraft == 1 ? true : false, -1));
              }
              if (session.time != null && time > session.time) {
                return await this.updateSessionItem(resultSet, session, context, pinningTime);
              } else {
                if (hasDraft == 1) { // 当旧机中session时间早，但存在草稿时，更新为旧机草稿
                  let ret: boolean = await this.updateSessionDraft(resultSet, session.id as number, context);
                }
                if (pinningTime > 0) {
                  return await this.updatePinningTime(resultSet, session.id as number, context, pinningTime);
                }
              }
              return true;
            }
          }
        }
      }
      let ret = await this.insertSessionItem(resultSet, context);
      if (ret && hasDraft == 1) { // 旧机有草稿，对应session记录到SESSION_DROP_DRAFT，待后续处理
        let sessionId = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.session.id));
        let newSessionId = RAW_SESSION_ID_MAP.get(sessionId);
        if (newSessionId != undefined) {
          SESSION_DROP_DRAFT.set(newSessionId as number,  new DropDraftInfo(true, -1));
        }
      }
      return ret;
    }
  }

  private getPinningTime(resultSet: rdb.ResultSet, newPinningTime: number): number {
    if (resultSet.columnNames.indexOf(NextOldDBData.session.pinningTime) == -1) {
      return newPinningTime;
    }
    let oldPinningTime = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.session.pinningTime));
    if (oldPinningTime !== undefined && oldPinningTime > newPinningTime) {
      return oldPinningTime;
    }
    return newPinningTime;
  }

  private async updatePinningTime(resultSet: rdb.ResultSet, newSessionId: number, context: common.Context,
    pinningTime: number): Promise<boolean> {
    let updateSuccessFlag: boolean = false;
    let sessionId = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.session.id));
    let values: ValuesBucket = {
      'pinning_time': pinningTime
    }
    let rowsId = await newDbHelper.updateSession(context, newSessionId, values);
    if (rowsId == -1) {
      updateSuccessFlag = false;
    } else {
      updateSuccessFlag = true;
      RAW_SESSION_ID_MAP.set(sessionId, newSessionId);
    }
    return updateSuccessFlag;
  }

  private async updateSessionItem(resultSet: rdb.ResultSet, session: ValuesBucket, context: common.Context,
    pinningTime: number): Promise<boolean> {
    let updateSuccessFlag:boolean = false;
    let sessionId = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.session.id));
    let values: ValuesBucket = this.buildUpdateSessionItem(resultSet, session, pinningTime);
    let rowsId = await newDbHelper.updateSession(context, session.id as number, values);
    if (rowsId == -1) {
      updateSuccessFlag = false;
    } else {
      updateSuccessFlag = true;
      RAW_SESSION_ID_MAP.set(sessionId, session.id as number);
    }
    return updateSuccessFlag
  }

  private async insertSessionItem(resultSet: rdb.ResultSet, context: common.Context): Promise<boolean> {
    let insertSuccessFlag:boolean = false;
    let sessionId = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.session.id));
    let oldPinningTime = 0;
    if (resultSet.columnNames.indexOf(NextOldDBData.session.pinningTime) != -1) {
      oldPinningTime = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.session.pinningTime));
    }
    let values: ValuesBucket = this.buildSessionItem(resultSet, oldPinningTime);
    let newRawSessionId = await newDbHelper.insertSession(context, values);
    if (newRawSessionId == -1) {
      insertSuccessFlag = false
    } else {
      insertSuccessFlag = true
      RAW_SESSION_ID_MAP.set(sessionId, newRawSessionId);
    }
    return insertSuccessFlag
  }

  private async updateSessionDraft(resultSet: rdb.ResultSet, newSessionId: number, context: common.Context
    ): Promise<boolean> {
    let updateSuccessFlag:boolean = false;
    let sessionId = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.session.id));
    let content = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.session.content));
    let values: ValuesBucket = {
      'has_draft': 1,
      'content' : content
    }
    let rowsId = await newDbHelper.updateSession(context, newSessionId, values);
    if (rowsId == -1) {
      updateSuccessFlag = false;
    } else {
      updateSuccessFlag = true;
      RAW_SESSION_ID_MAP.set(sessionId, newSessionId);
    }
    return updateSuccessFlag
  }

  private buildUpdateSessionItem(resultSet: rdb.ResultSet, session: ValuesBucket, pinningTime: number) {
    let time = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.session.time);
    let telephone = newDbHelper.getColumnStringValue(resultSet, NextOldDBData.session.telephone);
    let content = newDbHelper.getColumnStringValue(resultSet, NextOldDBData.session.content);
    let contactsNum = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.session.contactsNum);
    let smsType = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.session.smsType);
    let unreadCount = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.session.unreadCount);
    let sendingStatus = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.session.sendingStatus);
    let hasLock = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.session.hasLock);
    let hasDraft = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.session.hasDraft);
    let messageCount = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.session.messageCount);
    let hasMms = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.session.hasMms);
    let hasAttachment = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.session.hasAttachment);
    let yellowPageId = newDbHelper.getColumnStringValue(resultSet, NextOldDBData.session.yellowPageId);
    let blockedType = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.session.blockedType);
    let values = new SessionItem()
      .setTime(time)
      .setTelephone(telephone)
      .setContactsNum(contactsNum)
      .setUnreadCount(unreadCount)
      .setSmsType(smsType)
      .setSendingStatus(sendingStatus)
      .setHasLock(hasLock)
      .setMessageCount(messageCount)
      .setHasMms(hasMms)
      .setHasAttachment(hasAttachment)
      .setYellowPageId(yellowPageId)
      .setBlockedType(blockedType)
      .setPinningTime(pinningTime);

    // 当前函数在旧机的session时间较新时被调用，此时：
    // 新机session无草稿，旧机session无草稿：保留旧机content和hasDraft √
    // 新机session无草稿，旧机session有草稿：保留旧机content和hasDraft √
    // 新机session有草稿，旧机session无草稿：保留新机content和hasDraft ×
    // 新机session有草稿，旧机session有草稿：保留旧机content和hasDraft √
    if (session.has_draft == 0 || hasDraft == 1) { // 只有新机session有草稿，旧机session无草稿，才保留新机草稿状态
      values.setContent(content)
        .setHasDraft(hasDraft);
    } else {
      values.setContent(session.content as string)
        .setHasDraft(1);
    }
    return values.createValuesBucket();
  }

  private buildSessionItem(resultSet: rdb.ResultSet, pinningTime: number) {
    let time = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.session.time);
    let telephone = newDbHelper.getColumnStringValue(resultSet, NextOldDBData.session.telephone);
    let content = newDbHelper.getColumnStringValue(resultSet, NextOldDBData.session.content);
    let contactsNum = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.session.contactsNum);
    let smsType = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.session.smsType);
    let unreadCount = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.session.unreadCount);
    let sendingStatus = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.session.sendingStatus);
    let hasLock = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.session.hasLock);
    let hasDraft = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.session.hasDraft);
    let messageCount = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.session.messageCount);
    let hasMms = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.session.hasMms);
    let hasAttachment = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.session.hasAttachment);
    let yellowPageId = newDbHelper.getColumnStringValue(resultSet, NextOldDBData.session.yellowPageId);
    let blockedType = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.session.blockedType);
    let values = new SessionItem()
      .setTime(time)
      .setTelephone(telephone)
      .setContent(content)
      .setContactsNum(contactsNum)
      .setUnreadCount(unreadCount)
      .setSmsType(smsType)
      .setSendingStatus(sendingStatus)
      .setHasLock(hasLock)
      .setHasDraft(hasDraft)
      .setMessageCount(messageCount)
      .setHasMms(hasMms)
      .setHasAttachment(hasAttachment)
      .setYellowPageId(yellowPageId)
      .setBlockedType(blockedType)
      .setPinningTime(pinningTime);
    return values.createValuesBucket();
  }

  private updateOldSessionDropDraft(newInfoItem: ValuesBucket, newSmsId: number) {
    if (newInfoItem.session_id as number === -1) { // session无效，不更新
      LogUtils.e(TAG, 'updateOldSessionDropDraft error : Invalid msg session: ' + newSmsId);
      return;
    }
    if (newInfoItem.msg_state as number !== 3) {
      return;
    }
    let draft = SESSION_DROP_DRAFT.get(newInfoItem.session_id as number);
    if (draft && draft.isOld) { // 这里是插入旧机的草稿，正常情况isOld始终满足
      let time = newInfoItem.start_time == '' ? -1 : Number(newInfoItem.start_time);
      if (draft.time < time) { // 保留最新
        draft.time = time;
      }
    } else {
      LogUtils.e(TAG, 'updateOldSessionDropDraft error : Invalid draft state: ' + draft?.isOld);
    }
  }

  private async queryMmsTable(context: common.Context, index: number, smsIdFails: number[],
      smsMmsMap: HashMap<string, string>, progress: BackupRestoreProgress,
      callback?: Function): Promise<number> {
    let failCount = 0;
    let successCount = 0;
    try {
      let mmsInfo = new rdb.RdbPredicates(DbConstants.NEXT_OLD_SMS_NAME);
      let rdbStore = await this.initDB(context);
      if (rdbStore === undefined) {
        if (callback) {
          callback(new ErrorUtils(400, 'rdbStore === undefined'));
        }
        return successCount;
      }
      mmsInfo = (index != -1) ? this.buildPredicatesInfo(mmsInfo, NextOldDBData.smsMmsInfo.msgId, index) :
        this.buildPredicatesInfoFail(mmsInfo, NextOldDBData.smsMmsInfo.msgId, smsIdFails);
      let resultSet = await rdbStore.query(mmsInfo, this.queryMmsColumn);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        return successCount;
      }
      let countMms = resultSet.rowCount;
      do {
        let msgId = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.msgId));
        let receiverNumber = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.receiverNumber));
        let senderNumber = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.senderNumber));
        let isSender = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.isSender));
        let startTime = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.startTime));
        let rcsId = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.rcsId));
        let smsType = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.smsType));
        let oriGroupId = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.groupId));
        let newGroupId: number | undefined = 0;
        if (smsType == 99) {
          newGroupId = RCS_GROUPID_MAP.get(oriGroupId);
          if (newGroupId === undefined) {
            newGroupId = maxNewGroupId + 1;
          }
        } else {
          newGroupId = TIME_GROUP_ID_MAP.get(startTime);
          if (newGroupId === undefined) {
            newGroupId = maxNewGroupId + 1;
          }
        }
        let values: ValuesBucket = this.getSmsMmsInfoItem(resultSet, newGroupId);
        let number = '';
        if (!StringUtil.isEmpty(receiverNumber)) {
          number = receiverNumber;
        } else if (!StringUtil.isEmpty(senderNumber)) {
          number = senderNumber;
        }
        let key: string = number + ':' + isSender + ':' + startTime;
        let newRawSmsId: number = smsMmsMap.hasKey(key) ? Number.parseInt(smsMmsMap.get(key)) :
          await newDbHelper.insertSms(context, values);
        if (newRawSmsId == -1) {
          failCount++;
        } else {
          successCount++;
          RAW_MSG_ID_MAP.set(msgId, newRawSmsId);
          ROW_RCS_NEW_MSG_ID_MAP.set(rcsId, newRawSmsId);
          if (smsType == 99 && RCS_GROUPID_MAP.get(oriGroupId) === undefined) {
            maxNewGroupId = newGroupId;
            RCS_GROUPID_MAP.set(oriGroupId, newGroupId);
          }
          if (smsType != 99 && TIME_GROUP_ID_MAP.get(startTime) === undefined) {
            maxNewGroupId = newGroupId;
            TIME_GROUP_ID_MAP.set(startTime, newGroupId);
          }
          NEW_MSG_GROUP_ID_MAP.set(newRawSmsId, newGroupId);
          this.updateOldSessionDropDraft(values, newRawSmsId);
        }
      } while (resultSet.goToNextRow())
      resultSet.close();
      this.mmsFailed = this.mmsFailed + failCount;
      progress.updateProcessCount(progress.getProcessCount() + countMms);
      LogUtils.i(TAG, 'mms Total Migrations count: ' + countMms + ',success: ' + successCount + ' fail: ' + failCount);
    } catch (error) {
      LogUtils.w(TAG, 'query MmsInfo error : ' + JSON.stringify(error));
      if (callback) {
        callback(new ErrorUtils(400, JSON.stringify(error)));
      }
    }
    return successCount;
  }

  private async queryFavoriteTable(context: common.Context, index: number, favorateIdFails: number[],
    favorateMap: HashMap<string, number>, progress: BackupRestoreProgress, callback?: Function): Promise<number> {
    let successCount = 0;
    let failCount = 0;
    try {
      let resultSet = await newDbHelper.queryCollectSmsInfo(context, index);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        progress.updateProcessCount(progress.getProcessCount() + progress.getFavorateCount());
        return successCount;
      }
      let countFavorate = resultSet.rowCount;
      LogUtils.e(TAG, 'old favorite msg count ' + countFavorate);
      do {
        let msgId = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.msgId));
        let receiverNumber = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.receiverNumber));
        let senderNumber = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.senderNumber));
        let isSender = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.isSender));
        let startTime = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.startTime));
        let smsType = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.smsType));
        let msgType = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.msgType));
        let rcsId = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.rcsId));
        let msgTitle = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.msgTitle));
        let msgContent = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.msgContent));
        let detectResContent = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.detectResContent));
        let partType = 0;
        let partLocationPath = '';
        let recordingTime = '';
        let partSize = '';

        // 如果是RCS，查询RCS表获取相关字段
        if (smsType == 99) {
          let rcsPredicatesInfo = new rdb.RdbPredicates(DbConstants.TABLE_RCS_INFO);
          rcsPredicatesInfo.equalTo('rcs_id', rcsId);
          let rcsQueryColumn = [NextOldDBData.rcsInfo.msgTitle, NextOldDBData.rcsInfo.msgContent,
            NextOldDBData.rcsInfo.detectResContent];
          let rcsResultSet = await newDbHelper.queryData(context, rcsPredicatesInfo, rcsQueryColumn);
          if (rcsResultSet == undefined || !rcsResultSet.goToFirstRow()) {
            LogUtils.w(TAG, 'no favorate rcs info row found.');
          } else {
            msgTitle = rcsResultSet.getString(rcsResultSet.getColumnIndex(NextOldDBData.smsMmsInfo.msgTitle));
            msgContent = rcsResultSet.getString(rcsResultSet.getColumnIndex(NextOldDBData.smsMmsInfo.msgContent));
            detectResContent = rcsResultSet.getString(rcsResultSet.getColumnIndex(
              NextOldDBData.smsMmsInfo.detectResContent));
          }
          rcsResultSet.close();
        }

        // 如果是彩信，查询part表。
        if (msgType == 1 || smsType == 99) {
          let mmsPartPredicatesInfo = new rdb.RdbPredicates(DbConstants.TABLE_MMS_PART);
          if (smsType == 99) {
            mmsPartPredicatesInfo.equalTo('rcs_id', rcsId);
          } else {
            mmsPartPredicatesInfo.equalTo('msg_id', msgId);
          }
          // 彩信有2条记录，查询实际的那条。
          mmsPartPredicatesInfo.notEqualTo('type', 0);
          mmsPartPredicatesInfo.orderByAsc('type');
          let queryColumn = [NextOldDBData.mmsPart.type, NextOldDBData.mmsPart.locationPath,
            NextOldDBData.mmsPart.recordingTime, NextOldDBData.mmsPart.partSize];
          let mmsPartResultSet = await newDbHelper.queryData(context, mmsPartPredicatesInfo, queryColumn);
          if (mmsPartResultSet == undefined || !mmsPartResultSet.goToFirstRow()) {
            LogUtils.w(TAG, 'no favorite sms part row found.');
          } else {
            partType = mmsPartResultSet.getLong(mmsPartResultSet.getColumnIndex(NextOldDBData.mmsPart.type));
            partLocationPath = mmsPartResultSet.getString(mmsPartResultSet.getColumnIndex(
              NextOldDBData.mmsPart.locationPath));
            recordingTime = mmsPartResultSet.getString(mmsPartResultSet.getColumnIndex(
              NextOldDBData.mmsPart.recordingTime));
            partSize = mmsPartResultSet.getString(mmsPartResultSet.getColumnIndex(NextOldDBData.mmsPart.partSize));
          }
          mmsPartResultSet.close();
        }

        let values = new FavorateInfoItem()
          .setMsgId(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.msgId))
          .setReceiverNumber(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.smsMmsInfo.receiverNumber))
          .setSenderNumber(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.smsMmsInfo.senderNumber))
          .setKeepTime(new Date().getTime() + '')
          .setStartTime(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.smsMmsInfo.startTime))
          .setMsgType(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.msgType))
          .setSmsType(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.smsType))
          .setMsgTitle(msgTitle)
          .setMsgContent(msgContent)
          .setSessionType(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.sessionType))
          .setGroupId(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.groupId))
          .setPartType(partType)
          .setPartLocationPath(partLocationPath)
          .setIsSender(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.isSender))
          .setRcsId(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.rcsId))
          .setDetectResContent(detectResContent)
          .setSessionId(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.sessionId))
          .setRecordingTime(recordingTime)
          .setPartSize(partSize)
          .createValuesBucket();

        let number = '';
        if (!StringUtil.isEmpty(receiverNumber)) {
          number = receiverNumber;
        } else if (!StringUtil.isEmpty(senderNumber)) {
          number = senderNumber;
        }
        let key: string = number + ':' + isSender + ':' + startTime;
        let newRawSmsId: number = favorateMap.hasKey(key) ? favorateMap.get(key) :
          await newDbHelper.insertFavorate(context, values);
        if (newRawSmsId == -1) {
          failCount++;
        } else {
          successCount++;
        }
      } while (resultSet.goToNextRow())
      resultSet.close();
      this.favorateFailed = this.favorateFailed + failCount;
      progress.updateProcessCount(progress.getProcessCount() + countFavorate);
      LogUtils.i(TAG, 'favorate Total Migrations count: ' + countFavorate + ',success: ' + successCount + ' fail: ' + failCount);
    } catch (error) {
      LogUtils.w(TAG, 'query FavorateInfo error : ' + JSON.stringify(error));
      if (callback) {
        callback(new ErrorUtils(400, JSON.stringify(error)));
      }
    }
    return successCount;
  }

  private getSmsMmsInfoItemFromRcsInfo(resultSet: rdb.ResultSet, rcsId: number, groupId: number): ValuesBucket {
    let oldSessionId = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.sessionId);
    let newSessionId = oldSessionId === undefined ? undefined : RAW_SESSION_ID_MAP.get(oldSessionId);
    let msgContent = this.buildSmsMmsRowMsgContent(resultSet);
    let values = new SmsMmsInfoItem()
      .setSessionId(newSessionId === undefined ? -1 : newSessionId)
      .setSlotId(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.slotId))
      .setStartTime(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.startTime))
      .setReceiverNumber(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.rcsInfo.receiverNumber))
      .setSenderNumber(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.rcsInfo.senderNumber))
      .setRiskUrlBody(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.rcsInfo.riskUrlBody))
      .setMsgContent(msgContent === undefined ? '' : msgContent)
      .setSmsType(99)
      .setRcsId(rcsId)
      .setBlockedPrefNumber(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.rcsInfo.blockedPrefNumber))
      .setBlockedReason(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.blockedReason))
      .setBlockedSources(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.rcsInfo.blockedSources))
      .setBlockedType(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.blockedType))
      .setBlockedTypeText(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.rcsInfo.blockedTypeText))
      .setDetectResContent(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.rcsInfo.detectResContent))
      .setEndTime(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.endTime))
      .setFormatPhoneNumber(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.rcsInfo.formatPhoneNumber))
      .setGroupId(groupId)
      .setHasBeReportedAsSpam(newDbHelper.getColumnIntegerValue(resultSet,
        NextOldDBData.rcsInfo.hasBeReportedAsSpam))
      .setIsAdvancedSecurity(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.isAdvancesSecurity))
      .setIsBlocked(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.isBlocked))
      .setIsCollect(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.isCollect))
      .setIsLock(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.isLock))
      .setIsRead(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.isRead))
      .setIsReport(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.isReport))
      .setIsSender(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.isSender))
      .setMsgState(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.msgState))
      .setMsgTitle(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.rcsInfo.msgTitle))
      .setPhoneNumber(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.rcsInfo.phoneNumber));
    return values.createValuesBucket();
  }

  private getSmsMmsInfoItem(resultSet: rdb.ResultSet, groupId: number): ValuesBucket {
    let oldSessionId = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.sessionId);
    let threadId = oldSessionId === undefined ? undefined : RAW_SESSION_ID_MAP.get(oldSessionId);
    let isSender = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.isSender);
    let msgState = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.msgState);
    let msgType = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.msgType);
    // 接收到的短信并且发送状态为发送中，才修改发送状态为成功，彩信不涉及
    if ((msgState == 1 || msgState == 2) && isSender == 1 && msgType == 0) {
      if (threadId != undefined) {
        THREAD_NEED_UPDATE.add(threadId);
      }
      msgState = 0;
    }
    if (newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.isRead) == 0) {
      LogUtils.e(TAG, 'thread ' + threadId + ' have no read mms ');
    }
    let values = new SmsMmsInfoItem()
    .setSessionId(threadId === undefined ? -1 : threadId)
    .setSlotId(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.slotId))
    .setReceiverNumber(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.smsMmsInfo.receiverNumber))
    .setSenderNumber(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.smsMmsInfo.senderNumber))
    .setIsSender(isSender)
    .setMsgType(msgType)
    .setSmsType(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.smsType))
    .setStartTime(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.startTime))
    .setEndTime(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.endTime))
    .setMsgState(msgState)
    .setMsgTitle(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.smsMmsInfo.msgTitle))
    .setMsgContent(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.smsMmsInfo.msgContent))
    .setOperatorServiceNumber(newDbHelper.getColumnStringValue(resultSet,
      NextOldDBData.smsMmsInfo.operatorServiceNumber))
    .setIsLock(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.isLock))
    .setIsCollect(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.isCollect))
    .setIsRead(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.isRead))
    .setSessionType(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.sessionType))
    .setRetryNumber(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.retryNumber))
    .setGroupId(groupId)
    .setDeviceId(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.deviceId))
    .setIsSubsection(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.isSubsection))
    .setIsSendReport(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.isSendReport))
    .setMsgCode(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.msgCode))
    .setIsAdvancedSecurity(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.isAdvancedSecurity))
    .setRcsId(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.rcsId))
    .setExpiresTime(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.expiresTime))
    .setMmsPdu(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.smsMmsInfo.mmsPdu))
    .setPhoneNumber(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.smsMmsInfo.phoneNumber))
    .setFormatPhoneNumber(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.smsMmsInfo.formatPhoneNumber))
    .setIsBlocked(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.isBlocked))
    .setBlockedReason(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.blockedReason))
    .setBlockedType(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.blockedType))
    .setHasBeReportedAsSpam(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.hasBeReportedAsSpam))
    .setIsReport(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.isReport))
    .setBlockedSources(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.smsMmsInfo.blockedSources))
    .setBlockedTypeText(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.smsMmsInfo.blockedTypeText))
    .setRiskUrlBody(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.smsMmsInfo.riskUrlBody))
    .setDetectResContent(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.smsMmsInfo.detectResContent))
    .setMsgCodeStr(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.smsMmsInfo.msgCodeStr))
    .setBlockedPrefNumber(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.smsMmsInfo.blockedPrefNumber));
    return values.createValuesBucket();
  }

  private async queryPduTable(context: common.Context, index: number,
    pduIdFails: number[], progress: BackupRestoreProgress, callback?: Function): Promise<number> {
    LogUtils.e(TAG, `[queryPduTable] start`)
    let failCount = 0;
    let successCount = 0;
    try {
      let pduInfo = new rdb.RdbPredicates(DbConstants.NEXT_OLD_PDU_NAME);
      let rdbStore = await this.initDB(context);
      if (rdbStore === undefined) {
        LogUtils.e(TAG, `[queryPduTable] failed : rdbStore === undefined`)
        if (callback) {
          callback(new ErrorUtils(400, 'rdbStore === undefined'));
        }
        return successCount;
      }
      let queryPduColumn = [NextOldDBData.mmsPdu.pduContent];
      if (index != -1) {
        pduInfo = this.buildPredicatesInfo(pduInfo, NextOldDBData.mmsPdu.id, index);
      } else {
        pduInfo = this.buildPredicatesInfoFail(pduInfo, NextOldDBData.mmsPdu.id, pduIdFails);
      }
      let resultSet = await rdbStore.query(pduInfo, queryPduColumn);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'no raw pdu found.');
        return successCount;
      }
      let count = resultSet.rowCount;
      do {
        let pduContent = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.mmsPdu.pduContent));
        let values: ValuesBucket = new MmsPduItem()
          .setPduContent(pduContent)
          .createValuesBucket();
        let newRawSmsId = await newDbHelper.insertPdu(context, values);
        if (newRawSmsId == -1) {
          failCount++;
        } else {
          successCount++;
        }
      } while (resultSet.goToNextRow());
      resultSet.close();
      this.pudFailed = this.pudFailed + failCount;
      progress.updateProcessCount(progress.getProcessCount() + count);
      LogUtils.i(TAG, 'pdu Total Migrations count: ' + count + ',Successful Migrations successCount: ' + successCount + 'Failed Migrations failCount: ' + failCount);
    } catch (error) {
      LogUtils.w(TAG, 'query PduInfo error : ' + JSON.stringify(error));
      if (callback) {
        callback(new ErrorUtils(400, JSON.stringify(error)));
      }
    }
    return successCount;
  }

  private async queryPartTable(context: common.Context, index: number, partIdFails: number[],
    partMmsMap: HashMap<string, number>, progress: BackupRestoreProgress): Promise<number> {
    LogUtils.e(TAG, `queryPartTable start`)
    let failCount = 0;
    let successCount = 0;
    let partInfo = new rdb.RdbPredicates(DbConstants.NEXT_OLD_PART_NAME);
    let rdbStore = await this.initDB(context);
    if (rdbStore === undefined) {
      LogUtils.e(TAG, `[queryPartTable] failed : rdbStore === undefined`)
      return successCount;
    }
    try {
      if (index != -1) {
        partInfo = this.buildPredicatesInfo(partInfo, NextOldDBData.mmsPart.id, index);
      } else {
        partInfo = this.buildPredicatesInfoFail(partInfo, NextOldDBData.mmsPart.id, partIdFails);
      }
      let resultSet = await rdbStore.query(partInfo, this.queryPartColumn);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'no raw part found.');
        return successCount;
      }
      let count = resultSet.rowCount;
      do {
        let locationPath: string = resultSet.getString(
          resultSet.getColumnIndex(NextOldDBData.mmsPart.locationPath));
        let partSize: string = resultSet.getString(
          resultSet.getColumnIndex(NextOldDBData.mmsPart.partSize));
        let content: string = resultSet.getString(
          resultSet.getColumnIndex(NextOldDBData.mmsPart.content));
        let beforeRcsId = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.mmsPart.rcsId);
        let rcsId = beforeRcsId === undefined ? 0 : RAW_RCS_ID_MAP.get(beforeRcsId);
        let key = partSize + ':' + locationPath + ':' + content + ':' + (rcsId === undefined ? 0 : rcsId);
        let successFlag = partMmsMap.hasKey(key) ?
          true : await this.insertPartTableItem(context, resultSet);
        if (successFlag) {
          successCount++;
        } else {
          failCount++;
        }
      } while (resultSet.goToNextRow());
      resultSet.close();
      this.partFailed = this.partFailed + failCount;
      progress.updateProcessCount(progress.getProcessCount() + count);
      LogUtils.i(TAG, 'pdu Total Migrations count: ' + count + ',Successful Migrations successCount: ' + successCount + 'Failed Migrations failCount: ' + failCount);
    } catch (error) {
      LogUtils.w(TAG, 'query partInfo error: ' + JSON.stringify(error));
    }
    return successCount;
  }

  private async isRelatedFavorate(context: common.Context, resultSet: rdb.ResultSet): Promise<boolean> {
    // 如果存在 fav 表，判断 part 表是否关联 fav 表
    if (await this.isFavoriteTableExist(context)) {
      let isRelateResultSet: rdb.ResultSet | undefined = undefined;
      let rcsId = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.mmsPart.rcsId));
      let msgId = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.mmsPart.msgId));
      let rdbStore = await this.initDB(context);
      if (rdbStore === undefined) {
        LogUtils.e(TAG, `check isRelatedFavorate failed : rdbStore === undefined`);
        return false;
      }
      try {
        let sql = `SELECT CASE WHEN (${rcsId} != 0 AND EXISTS (SELECT 1 FROM favorate_info WHERE rcs_id = ${rcsId}))
         OR EXISTS (SELECT 1 FROM favorate_info WHERE msg_id = ${msgId}) THEN 1 ELSE 0 END AS result;`;
        isRelateResultSet = rdbStore.querySqlSync(sql);
        if (isRelateResultSet == undefined || !isRelateResultSet.goToFirstRow()) {
          LogUtils.e(TAG, 'no raw isRelatedFavorate found.');
          return false;
        }
        let isExist = isRelateResultSet.getLong(0) == 1 ? true : false;
        isRelateResultSet.close();
        return isExist;
      } catch (error) {
        if (isRelateResultSet) {
          isRelateResultSet.close();
        }
        const backupRestoreResult = BackupRestoreResult.getInstance();
        backupRestoreResult.insertOrUpdateBackupInfo('sms',
          `isRelatedFavorate code:${error.code} msg:${error.message}`);
        LogUtils.e(TAG, `isRelatedFavorate failed, code is ${error.code},message is ${error.message}`);
      }
    }
    return false;
  }

  private async insertPartTableItem(context: common.Context, resultSet: rdb.ResultSet): Promise<boolean> {
    let insertSuccessFlag:boolean = false;
    let isRelatedFav = false;
    let beforeMsgId = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.mmsPart.msgId);
    let partIndex = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.mmsPart.partIndex);
    let partSize = newDbHelper.getColumnStringValue(resultSet, NextOldDBData.mmsPart.partSize);
    let recordingTime = newDbHelper.getColumnStringValue(resultSet, NextOldDBData.mmsPart.recordingTime);
    let type = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.mmsPart.type);
    let locationPath = newDbHelper.getColumnStringValue(resultSet, NextOldDBData.mmsPart.locationPath);
    let encode = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.mmsPart.encode);
    let state = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.mmsPart.state);
    let content = newDbHelper.getColumnStringValue(resultSet, NextOldDBData.mmsPart.content);
    let ct = newDbHelper.getColumnStringValue(resultSet, NextOldDBData.mmsPart.ct);
    let beforeRcsId = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.mmsPart.rcsId);
    let isCollect = await this.isRelatedFavorate(context, resultSet);
    let rcsId = beforeRcsId === undefined ? 0 : RAW_RCS_ID_MAP.get(beforeRcsId);
    let msgId = beforeMsgId === undefined ? 0 : RAW_MSG_ID_MAP.get(beforeMsgId);
    let groupId: number | undefined;
    if (rcsId !== undefined && rcsId > 0) {
      groupId = NEW_RCS_GROUP_ID_MAP.get(rcsId);
    } else if (msgId !== undefined && msgId > 0) {
      groupId = NEW_MSG_GROUP_ID_MAP.get(msgId);
    } else {
      if (!isCollect) {
        const backupRestoreResult = BackupRestoreResult.getInstance();
        backupRestoreResult.insertOrUpdateBackupInfo('sms', `rcsId and msgId are invalid and not need clone.`);
        LogUtils.e(TAG, 'rcsId and msgId are invalid and not need clone.');
        return true;
      } else {
        // 仅关联 fav 表的记录，需要记录新 part._id，后续 fav 表迁移时，更新 msg_id、rcs_id
        isRelatedFav = true;
      }
    }
    let isFraud = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.mmsPart.isFraud);
    let referenceCount = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.mmsPart.referenceCount);

    let values = new MmsPartItem()
      .setMsgId(msgId)
      .setGroupId(groupId)
      .setPartIndex(partIndex)
      .setPartSize(partSize)
      .setRecordingTime(recordingTime)
      .setType(type)
      .setLocationPath(locationPath)
      .setEncode(encode)
      .setState(state)
      .setContent(content)
      .setCt(ct)
      .setRcsId(rcsId)
      .setIsFraud(isFraud)
      .setReferenceCount(referenceCount);
    let newRawSmsId = await newDbHelper.insertPart(context, values.createValuesBucket());
    if (newRawSmsId == -1) {
      insertSuccessFlag = false;
    } else {
      // todo：msg_id 对应多个 part._id
      if (isRelatedFav && beforeMsgId !== undefined) {
        let ids: number[] | undefined = NEW_FAV_MSG_ID_MAP.get(beforeMsgId);
        if (ids === undefined) {
          ids = [newRawSmsId];
        } else {
          ids.push(newRawSmsId);
        }
        NEW_FAV_MSG_ID_MAP.set(beforeMsgId, ids);
      }
      insertSuccessFlag = true;
    }
    return insertSuccessFlag
  }

  private async queryProtocolTable(context: common.Context, index: number, protocolIdFails: number[],
      progress: BackupRestoreProgress): Promise<number> {
    LogUtils.e(TAG, `queryProtocolTable start`)
    let failCount = 0;
    let successCount = 0;
    let protocolInfo = new rdb.RdbPredicates(DbConstants.NEXT_OLD_PROTOCOL_NAME);
    let rdbStore = await this.initDB(context);
    if (rdbStore === undefined) {
      LogUtils.e(TAG, `[queryProtocolTable] failed : rdbStore === undefined`);
      return successCount;
    }
    try {
      if (index != -1) {
        protocolInfo = this.buildPredicatesInfo(protocolInfo, NextOldDBData.mmsProtocol.id, index);
      } else {
        protocolInfo = this.buildPredicatesInfoFail(protocolInfo, NextOldDBData.mmsProtocol.id, protocolIdFails);
      }
      let resultSet = await rdbStore.query(protocolInfo, this.queryProtocolColumn);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.e(TAG, 'no raw protocol found.');
        return successCount;
      }
      let count = resultSet.rowCount;
      do {
        let sucessFlag = await this.insertProtocolItem(resultSet, context);
        if (sucessFlag) {
          successCount++;
        } else {
          failCount++;
        }
      } while (resultSet.goToNextRow());
      resultSet.close();
      this.protocolFailed = this.protocolFailed + failCount;
      progress.updateProcessCount(progress.getProcessCount() + count);
      LogUtils.i(TAG, 'protocol Total Migrations count: ' + count + ',Successful Migrations successCount: ' + successCount + 'Failed Migrations failCount: ' + failCount);
    } catch (error) {
      LogUtils.e(TAG, 'query protocolInfo error : ' + JSON.stringify(error));
    }
    return successCount;
  }

  private async insertProtocolItem(resultSet: rdb.ResultSet, context: common.Context): Promise<boolean> {
    let insertSuccessFlag:boolean = false;
    let beforeMsgId = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.mmsProtocol.msgId));
    let bcc = newDbHelper.getColumnStringValue(resultSet, NextOldDBData.mmsProtocol.bcc);
    let cc = newDbHelper.getColumnStringValue(resultSet, NextOldDBData.mmsProtocol.cc);
    let contentLocation = newDbHelper.getColumnStringValue(resultSet, NextOldDBData.mmsProtocol.contentLocation);
    let date = newDbHelper.getColumnStringValue(resultSet, NextOldDBData.mmsProtocol.date);
    let deliveryReport = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.mmsProtocol.deliveryReport);
    let deliveryTime = newDbHelper.getColumnStringValue(resultSet, NextOldDBData.mmsProtocol.deliveryTime);
    let expiry = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.mmsProtocol.expiry);
    let type = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.mmsProtocol.type);
    let serialNumber = newDbHelper.getColumnStringValue(resultSet, NextOldDBData.mmsProtocol.serialNumber);
    let category = newDbHelper.getColumnStringValue(resultSet, NextOldDBData.mmsProtocol.category);
    let version = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.mmsProtocol.version);
    let size = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.mmsProtocol.size);
    let priority = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.mmsProtocol.priority);
    let readReply = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.mmsProtocol.readReply);
    let reportAllowed = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.mmsProtocol.reportAllowed);
    let responseStatus = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.mmsProtocol.responseStatus);
    let responseText = newDbHelper.getColumnStringValue(resultSet, NextOldDBData.mmsProtocol.responseText);
    let senderVisibility = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.mmsProtocol.senderVisibility);
    let msgId = (RAW_MSG_ID_MAP.get(beforeMsgId) ? RAW_MSG_ID_MAP.get(beforeMsgId) : beforeMsgId) || 0;

    let values: ValuesBucket = new MmsProtocolItem()
      .setMsgId(msgId)
      .setBcc(bcc)
      .setCc(cc)
      .setContentLocation(contentLocation)
      .setDate(date)
      .setDeliveryReport(deliveryReport)
      .setDeliveryTime(deliveryTime)
      .setExpiry(expiry)
      .setType(type)
      .setSerialNumber(serialNumber)
      .setCategory(category)
      .setVersion(version)
      .setSize(size)
      .setPriority(priority)
      .setReadReply(readReply)
      .setReportAllowed(reportAllowed)
      .setResponseStatus(responseStatus)
      .setResponseText(responseText)
      .setSenderVisibility(senderVisibility)
      .createValuesBucket();
    let newRawSmsId = await newDbHelper.insertProtocol(context, values);
    if (newRawSmsId == -1) {
      insertSuccessFlag = false;
    } else {
      insertSuccessFlag = true;
    }
    return insertSuccessFlag
  }

  private async queryRcsTable(context: common.Context, index: number, rcsIdFails: number[],
      rcsInfoMap: HashMap<string, number>, progress: BackupRestoreProgress,
      callback?: Function): Promise<number> {
    let failCount = 0;
    let successCount = 0;
    try {
      let rcsInfo = new rdb.RdbPredicates(DbConstants.NEXT_OLD_RCS_NAME);
      let rdbStore = await this.initDB(context);
      if (rdbStore === undefined) {
        if (callback) {
          callback(new ErrorUtils(400, 'rdbStore === undefined'));
        }
        return successCount;
      }
      if (index != -1) {
        rcsInfo = this.buildPredicatesInfo(rcsInfo, NextOldDBData.rcsInfo.rcsId, index);
      } else {
        rcsInfo = this.buildPredicatesInfoFail(rcsInfo, NextOldDBData.rcsInfo.rcsId, rcsIdFails);
      }
      let resultSet = await rdbStore.query(rcsInfo, this.queryRcsColumn);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'no raw rcs found.');
        return successCount;
      }
      let count = resultSet.rowCount;
      do {
        let rcsId = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.rcsInfo.rcsId));
        let receiverNumber = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.rcsInfo.receiverNumber));
        let senderNumber = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.rcsInfo.senderNumber));
        let startTime = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.rcsInfo.startTime));
        let oriGroupId = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.rcsInfo.groupId));
        let newGroupId = RCS_GROUPID_MAP.get(oriGroupId);
        if (newGroupId === undefined) {
          newGroupId = maxNewGroupId + 1;
        }
        let key: string = receiverNumber + ':' + senderNumber + ':' + startTime;
        let newRcsId = rcsInfoMap.hasKey(key) ? rcsInfoMap.get(key) :
          await this.insertRcsItem(resultSet, newGroupId, context);
        if (newRcsId > 0) {
          successCount++;
          RAW_RCS_ID_MAP.set(rcsId, newRcsId);
          if (RCS_GROUPID_MAP.get(oriGroupId) === undefined) {
            maxNewGroupId = newGroupId;
            RCS_GROUPID_MAP.set(oriGroupId, newGroupId);
          }
          NEW_RCS_GROUP_ID_MAP.set(newRcsId, newGroupId);
          let newMsgId = ROW_RCS_NEW_MSG_ID_MAP.get(rcsId);
          if (newMsgId !== undefined) {
            let content = this.buildSmsMmsRowMsgContent(resultSet);
            let upValues: ValuesBucket = {
              'rcs_id': newRcsId,
              'msg_content': content === undefined ? '' : content
            }
            LogUtils.e(TAG, 'update rcs id exist');
            newDbHelper.updateSms(context, newMsgId, upValues);
          } else {
            LogUtils.e(TAG, 'integrate rcs to info');
            // 如果 rcs_id 不在 sms_mms_info 表，再迁移一份数据去 info 表
            let upValues: ValuesBucket = this.getSmsMmsInfoItemFromRcsInfo(resultSet, newRcsId, newGroupId);
            await newDbHelper.insertSms(context, upValues);
          }
        } else {
          failCount++;
        }
      } while (resultSet.goToNextRow());
      resultSet.close();
      this.rcsFailed = this.rcsFailed + failCount;
      progress.updateProcessCount(progress.getProcessCount() + count);
      LogUtils.i(TAG, 'rcs Total Migrations count: ' + count + ',Successful Migrations successCount: ' + successCount + 'Failed Migrations failCount: ' + failCount);
    } catch (error) {
      LogUtils.w(TAG, 'query rcs error : ' + JSON.stringify(error));
      if (callback) {
        callback(new ErrorUtils(400, JSON.stringify(error)));
      }
    }
    return successCount;
  }

  private async insertRcsItem(resultSet: rdb.ResultSet, groupId: number, context: common.Context): Promise<number> {
    let beforeSessionId = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.rcsInfo.sessionId));
    let threadId = RAW_SESSION_ID_MAP.get(beforeSessionId) || 0;
    if (newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsMmsInfo.isRead) == 0) {
      LogUtils.e(TAG, 'rcs thread ' + threadId + ' have no read mms ');
    }
    let values = new RcsInfoItem()
    .setSlotId(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.slotId))
    .setReceiverNumber(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.rcsInfo.receiverNumber))
    .setSenderNumber(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.rcsInfo.senderNumber))
    .setStartTime(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.rcsInfo.startTime))
    .setEndTime(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.rcsInfo.endTime))
    .setRcsType(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.rcsType))
    .setMsgState(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.msgState))
    .setMsgTitle(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.rcsInfo.msgTitle))
    .setMsgContent(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.rcsInfo.msgContent))
    .setIsLock(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.isLock))
    .setIsRead(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.isRead))
    .setIsSender(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.isSender))
    .setSessionId(threadId)
    .setGroupId(groupId)
    .setMsgId(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.rcsInfo.msgId))
    .setEnrichedCallingType(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.enrichedCallingType))
    .setErrorCode(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.errorCode))
    .setNetworkType(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.rcsInfo.networkType))
    .setOwnerAddr(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.rcsInfo.ownerAddr))
    .setPrivacyMode(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.privacyMode))
    .setProtocol(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.protocol))
    .setReplyPathPresent(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.replyPathPresent))
    .setSeen(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.seen))
    .setServiceCenter(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.rcsInfo.serviceCenter))
    .setServiceKind(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.rcsInfo.serviceKind))
    .setPhoneNumber(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.rcsInfo.phoneNumber))
    .setFormatPhoneNumber(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.rcsInfo.formatPhoneNumber))
    .setIsBlocked(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.isBlocked))
    .setBlockedReason(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.blockedReason))
    .setBlockedSources(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.rcsInfo.blockedSources))
    .setBlockedType(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.blockedType))
    .setBlockedTypeText(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.rcsInfo.blockedTypeText))
    .setClurSize(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.clurSize))
    .setTotalSize(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.totalSize))
    .setRiskUrlBody(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.rcsInfo.riskUrlBody))
    .setHasBeReportedAsSpam(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.hasBeReportedAsSpam))
    .setIsReport(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.isReport))
    .setDetectResContent(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.rcsInfo.detectResContent))
    .setIsCollect(newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.rcsInfo.isCollect))
    .setBlockedPrefNumber(newDbHelper.getColumnStringValue(resultSet, NextOldDBData.rcsInfo.blockedPrefNumber));
    let newRawSmsId = await newDbHelper.insertRcs(context, values.createValuesBucket());
    return newRawSmsId
  }

  private async querySubsectionTable(context: common.Context, index: number,
    subsectionIdFails: number[], progress: BackupRestoreProgress): Promise<number> {
    LogUtils.e(TAG, `[querySubsectionTable] start`)
    let failCount = 0;
    let successCount = 0;
    let subsectionTable = new rdb.RdbPredicates(DbConstants.NEXT_OLD_SUBSECTION_NAME);
    let rdbStore = await this.initDB(context);
    if (rdbStore === undefined) {
      LogUtils.e(TAG, `[querySubsectionTable] failed : rdbStore === undefined`)
      return successCount;
    }
    try {
      if (index != -1) {
        subsectionTable = this.buildPredicatesInfo(subsectionTable, NextOldDBData.smsSubsection.id, index);
      } else {
        subsectionTable = this.buildPredicatesInfoFail(subsectionTable,
          NextOldDBData.smsSubsection.id, subsectionIdFails);
      }
      let resultSet = await rdbStore.query(subsectionTable, this.querySubsectionColumn);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'no raw thread found.');
        return successCount;
      }
      let count = resultSet.rowCount;
      do {
        let sucessFlag = await this.insertSubsectionItem(resultSet, context);
        if (sucessFlag) {
          successCount++;
        } else {
          failCount++;
        }
      } while (resultSet.goToNextRow());
      resultSet.close();
      this.subsectionFailed = this.subsectionFailed + failCount;
      progress.updateProcessCount(progress.getProcessCount() + count);
      LogUtils.i(TAG, 'subsection Total Migrations count: ' + count + ',Successful Migrations successCount: ' + successCount + 'Failed Migrations failCount: ' + failCount);
    } catch (error) {
      LogUtils.w(TAG, 'querySubsectionTable error : ' + JSON.stringify(error));
    }
    return successCount;
  }

  private async insertSubsectionItem(resultSet: rdb.ResultSet, context: common.Context): Promise<boolean> {
    let insertSuccessFlag:boolean = false;
    let slotId = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsSubsection.slotId);
    let smsSubsectionId = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsSubsection.smsSubsectionId);
    let receiverNumber = newDbHelper.getColumnStringValue(resultSet, NextOldDBData.smsSubsection.receiverNumber);
    let senderNumber = newDbHelper.getColumnStringValue(resultSet, NextOldDBData.smsSubsection.senderNumber);
    let isSender = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsSubsection.isSender);
    let startTime = newDbHelper.getColumnStringValue(resultSet, NextOldDBData.smsSubsection.startTime);
    let endTime = newDbHelper.getColumnStringValue(resultSet, NextOldDBData.smsSubsection.endTime);
    let rawPdu = newDbHelper.getColumnStringValue(resultSet, NextOldDBData.smsSubsection.rawPdu);
    let format = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsSubsection.format);
    let destPort = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsSubsection.destPort);
    let subsectionIndex = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsSubsection.subsectionIndex);
    let size = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.smsSubsection.size);

    let values: ValuesBucket = new SmsSubsectionItem()
      .setSlotId(slotId)
      .setSmsSubsectionId(smsSubsectionId)
      .setReceiverNumber(receiverNumber)
      .setSenderNumber(senderNumber)
      .setIsSender(isSender)
      .setStartTime(startTime)
      .setEndTime(endTime)
      .setRawPdu(rawPdu)
      .setFormat(format)
      .setDestPort(destPort)
      .setSubsectionIndex(subsectionIndex)
      .setSize(size)
      .createValuesBucket();
    let newRawSessionId = await newDbHelper.insertSubsection(context, values);
    if (newRawSessionId == -1) {
      insertSuccessFlag = false;
    } else {
      insertSuccessFlag = true;
    }
    return insertSuccessFlag
  }

  private buildPredicatesInfo(predicatesInfo: rdb.RdbPredicates, id: string, index: number): rdb.RdbPredicates {
    predicatesInfo.orderByAsc(id)
      .limitAs(DbConstants.QUERY_PAGE_NUM)
      .offsetAs(index);
    return predicatesInfo;
  }

  private buildPredicatesInfoWithoutOrder(predicatesInfo: rdb.RdbPredicates, index: number): rdb.RdbPredicates {
    predicatesInfo.limitAs(DbConstants.QUERY_PAGE_NUM)
      .offsetAs(index);
    return predicatesInfo;
  }

  private buildPredicatesInfoFail(predicatesInfo: rdb.RdbPredicates, id: string,
    chatIdFails: number[]): rdb.RdbPredicates {
    predicatesInfo.orderByAsc(id)
      .in(id, chatIdFails);
    return predicatesInfo;
  }
}

export const nextCopyDbHelper: NextCopyDbHelper = new NextCopyDbHelper();
