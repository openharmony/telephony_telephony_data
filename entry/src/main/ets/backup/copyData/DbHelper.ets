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
import HashMap from '@ohos.util.HashMap';
import LogUtils from '../../common/utils/LogUtils';
import TelephoneUtil from '../../common/utils/TelephoneUtil';
import { StringUtil } from '../../common/utils/StringUtil';
import SmsMmsInfoItem from './bean/SmsMmsInfoItem';
import MmsPartItem from './bean/MmsPartItem';
import SessionItem from './bean/SessionItem';
import UpdateSessionItem from './bean/UpdateSessionItem';
import DbConstants from './DbConstants';
import OldDBData from './OldDBData';
import NextOldDBData from './NextOldDBData';
import { newDbHelper } from './NewDbHelper';
import { newDbHelperClone } from '../database/NewDbHelper';
import LooseObject from './LooseObject';
import HashSet from '@ohos.util.HashSet';
import BackupRestoreResult from './BackupRestoreResult';
import RcsInfoItem from '../model/bean/RcsInfoItem';
import BackupConstants from '../BackupConstants';
import BackupRestoreProgress from './BackupRestoreProcess';
import DataBaseUtil from '../../common/utils/DataBaseUtil';

const TAG = '[Copy-DbHelper] :';
const PREF_NAME: string = 'CLONE_DB_PREFERENCES';
const RAW_THREAD_ID_MAP = new Map<number, number>();
const RAW_PDU_ID_MAP = new Map<number, Array<number>>();
const THREAD_PHONE_MAP = new Map<number, Array<string>>();
const SESSION_TIME_MAP = new Map<number, number>();
const SESSION_STATUS_MAP = new Map<number, number>();
const SESSION_CONTENT_MAP = new Map<number, string>();
const SESSION_MESSAGE_COUNT_MAP = new Map<number, number>();
const MAX_GROUP_ID: number[] = [];
const SMS_GROUP_ID_MAP = new HashMap<number, number>();
let SMS_INFO_MAP = new HashMap<string, string>();
let myPreferences: dataPreferences.Preferences | null = null;
let SMS_ID_SESSION_ID_MAP = new Map<number, number>();
let SESSION_ID_SMS_ID_MAP = new HashMap<number, number>();
let SMS_ID_GROUP_ID_MAP = new HashMap<number, number>();
let MMS_ID_RECORDING_TIME_MAP = new HashMap<number, string>();
let MMS_ID_TIME_MAP = new HashMap<number, number>();
let SESSION_ID_HAS_MMS_MAP = new HashMap<number, number>();
let ADDRESS_ID_MAP = new HashMap<number, string>();
let GROUP_ID: number = 0;
let sessionIdMap = new HashMap<string, number>();
const GROUP_ID_MAP = new Map<number, number>();
const SESSION_HAS_DRAFT_SET = new HashSet<number>();
const tableName1 = 'coman'
const tableName2 = 'droid'

class CopyDbHelper {
    private rdbStoreHelper?: rdb.RdbStore;
    private rdbRcsStoreHelper?: rdb.RdbStore;
    private doSessionSuccess = 0;
    private doMmsSuccess = 0;
    private doPduSuccess = 0;
    private doPartSuccess = 0;
    private doRcsChatSuccess = 0;
    private doSessionFailed = 0;
    private doMmsFailed = 0;
    private doPduFailed = 0;
    private doPartFailed = 0;
    private doRcsChatFailed = 0;
    private doRcsGroupFailed = 0;
    private version: string = '';
    private queryMmsColumn: string[] = [];
    private queryPduColumn: string[] = [];
    private queryPartColumn: string[] = [];
    private queryRcsColumn: string[] = [];
    private queryRcsGroupColumn: string[] = [];
    private hasPinning: boolean = false;

  public setVersion(version: string) {
    this.version = version;
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

  public async queryOhosToNextCount(context: common.Context): Promise<string> {
    let smsCount = await this.queryDataCount(context, DbConstants.OLD_SMS_NAME, OldDBData.sms.threadId);
    let pduCount = await this.queryDataCount(context, DbConstants.OLD_PDU_NAME, OldDBData.pdu.id);
    let rcsCount = await this.queryRcsDataCount(context, DbConstants.OLD_RCS_CHAT_TABLE_NAME, OldDBData.chat.id);
    let rcsGroupCount = await this.queryRcsDataCount(context, DbConstants.OLD_RCS_GROUP_TABLE_NAME,
      OldDBData.rcsGroup.id);
    let failCount = this.doMmsFailed + this.doPduFailed + this.doRcsChatFailed + this.doRcsGroupFailed;
    let successCounts = smsCount + pduCount + rcsCount + rcsGroupCount - failCount;
    const backupRestoreResult = BackupRestoreResult.getInstance();
    backupRestoreResult.addErrorInfo(failCount <= 0 ? BackupRestoreResult.RESTORE_SUCCESS
      : BackupRestoreResult.RESTORE_COMMON_ERROR, '');
    backupRestoreResult.insertOrUpdateBackupInfo('sms', '', successCounts, 0, failCount);
    LogUtils.i(TAG,
      `doSessionSuccess: ${this.doSessionSuccess},doMmsSuccess: ${this.doMmsSuccess},doPduSuccess:
      ${this.doPduSuccess},doPartSuccess: ${this.doPartSuccess}`);
    LogUtils.i(TAG,
      `doSessionFailed: ${this.doSessionFailed},doMmsFailed: ${this.doMmsFailed},doPduFailed:
      ${this.doPduFailed},doPartFailed: ${this.doPartFailed}`);
    return backupRestoreResult.formatRestoreResult();
  }

  public async copyDbListFileForClone(applicationContextPath: string, context: BackupExtensionContext):
    Promise<boolean> {
    LogUtils.i(TAG, `copyDbListFileForClone start`);
    let listFileName: string[] = [];
    if (context.area !== contextConstant.AreaMode.EL2) {
      context.area = contextConstant.AreaMode.EL2;
    }
    let readClonePath = applicationContextPath + '/rdb';
    const backupDir = context.backupDir + 'restore';
    try {
      let isClonePathExists = fs.accessSync(backupDir);
      if (!isClonePathExists) {
        LogUtils.e(TAG, `ClonePathExists is not exists`);
        return false;
      }
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
          LogUtils.e(TAG, `readClonePath: ${error.code}`);
        }
      }
      listFileName = this.getClonePathListFile(backupDir);
      if (listFileName === undefined || listFileName === null || listFileName.length === 0) {
        LogUtils.e(TAG, 'copyDbListFileForClone clonePath is not file');
        return false;
      }
      listFileName.forEach((value: string) => {
        this.doFilesCopy(backupDir, value, readClonePath);
      });
      LogUtils.i(TAG, 'copyDbFile success');
      return true;
    } catch (error) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `copyDbListFileForClone code:${error.code} msg:${error.message}`);
      LogUtils.e(TAG, `copyDbListFileForClone error: ${JSON.stringify(error)}`);
      this.printCopyFileInfo(listFileName, backupDir, readClonePath);
    }
    return false;
  }

  private doFilesCopy(backupDir: string, value: string, readClonePath: string) {
    let srcPath = `${backupDir}/${value}`;
    let destPath = `${readClonePath}/${value}`;
    if (fs.statSync(srcPath).isFile()) {
      fs.copyFileSync(srcPath, destPath);
    } else if (fs.statSync(srcPath).isDirectory()) {
      try {
        fs.copyDirSync(srcPath, readClonePath);
      } catch (error) {
        const backupRestoreResult = BackupRestoreResult.getInstance();
        backupRestoreResult.insertOrUpdateBackupInfo('sms',
          `doFilesCopy code:${error.code} msg:${error.message}`);
        LogUtils.i(TAG, `copyDbListFileForClone error: ${JSON.stringify(error)}`);
      }
    } else {
      LogUtils.e(TAG, 'invalid src file ');
    }
  }

  private printCopyFileInfo(listFileName: string[], backupDir: string, readClonePath: string) {
    try {
      listFileName.forEach((value: string) => {
        let srcPath = `${backupDir}/${value}`;
        let destPath = `${readClonePath}/${value}`;
        let statSrc = fs.statSync(srcPath);
        if (fs.accessSync(srcPath)) {
          LogUtils.e(TAG, 'printCopyFileInfo srcName:' + value + ' ,isFile:' + statSrc.isFile() +
            ' ,ino:' + statSrc.ino + ' ,mode:' + statSrc.mode + ' ,uid:' + statSrc.uid + ' ,gid:' + statSrc.gid +
            ' ,size:' + statSrc.size + ' ,atime:' + statSrc.atime + ' ,mtime:' + statSrc.mtime + ' ,ctime:' +
          statSrc.ctime);
        } else {
          LogUtils.e(TAG, `printCopyFileInfo file: srcPath not exist`);
        }
        let statDst = fs.statSync(destPath);
        if (fs.accessSync(destPath)) {
          LogUtils.e(TAG, 'printCopyFileInfo dstName:' + value + ' ,isFile:' + statDst.isFile() +
            ' ,ino:' + statDst.ino + ' ,mode:' + statDst.mode + ' ,uid:' + statDst.uid + ' ,gid:' + statDst.gid +
            ' ,size:' + statDst.size + ' ,atime:' + statDst.atime + ' ,mtime:' + statDst.mtime + ' ,ctime:' +
          statDst.ctime);
        } else {
          LogUtils.e(TAG, `printCopyFileInfo file: destPath not exist in`);
        }
      });
    } catch (error) {
      let err: BusinessError = error as BusinessError;
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `printCopyFileInfo code:${error.code} msg:${error.message}`);
      LogUtils.e(TAG, 'printCopyFileInfo with error message: ' + err.message + ', error code: ' + err.code);
    }
  }

  private async doSessionTableCopy(context: common.Context, progress: BackupRestoreProgress) {
    let idFailCount = 0;
    let idSuccessCount = 0;
    let threadsCount = await this.queryDataCount(context, DbConstants.OLD_THREADS_NAME, OldDBData.threads.threadId);
    let threadsPage = Math.ceil(threadsCount / DbConstants.QUERY_PAGE_NUM);
    let threadIdFails = this.getFromPreferences(context, DbConstants.THREAD_ID_FAIL, []) as number[];
    let sessionMap = await newDbHelper.querySession(context);
    if (this.version.startsWith(BackupConstants.IOS2NEXT_CLOUD_CLONE_VERSION)) {
      this.hasPinning =
        await this.isFieldExistInTable(context, DbConstants.OLD_THREADS_NAME, OldDBData.threads.pinningTime);
    }
    // 获取新机已有的 session、id、time、content、status
    await newDbHelper.querySessionTime(context, SESSION_TIME_MAP, SESSION_STATUS_MAP, SESSION_CONTENT_MAP);
    if (threadIdFails.length > 0) {
      idFailCount += await this.queryThreadInfo(context, -1, threadIdFails, sessionMap, progress);
      threadIdFails = this.getFromPreferences(context, DbConstants.THREAD_ID_FAIL, []) as number[];
    }
    let threadIdSuccess = this.getFromPreferences(context, DbConstants.THREAD_ID_SUCCESS, 0) as number;
    LogUtils.i(TAG, 'threadsPage ：' + threadsPage);
    for (let curPage = 1; curPage <= threadsPage; curPage++) {
      idSuccessCount += await this.queryThreadInfo(context, DbConstants.QUERY_PAGE_NUM * (curPage - 1) +
        threadIdSuccess, threadIdFails, sessionMap, progress);
    }
    this.doSessionSuccess = idFailCount + idSuccessCount;
    progress.updateProcessCount(progress.getThreadCloneProgress());
  }

  private async doMmsTableCopy(context: common.Context, progress: BackupRestoreProgress) {
    let idFailCount = 0;
    let idSuccessCount = 0;
    let smsCount = await this.queryDataCount(context, DbConstants.OLD_SMS_NAME, OldDBData.sms.threadId);
    let smsPage = Math.ceil(smsCount / DbConstants.QUERY_PAGE_NUM);
    this.queryMmsColumn = await this.queryColumns(context, DbConstants.OLD_SMS_NAME);
    let smsIdFails = this.getFromPreferences(context, DbConstants.SMS_ID_FAIL, []) as number[];
    let smsInfoSet = await newDbHelper.querySmsInfo(context);
    let maxGroupId = await newDbHelper.queryMaxGroupId(context);
    MAX_GROUP_ID[0] = maxGroupId;
    SMS_INFO_MAP = smsInfoSet;
    if (smsIdFails.length > 0) {
      idFailCount += await this.queryMmsInfo(context, -1, smsIdFails, smsInfoSet, progress);
      smsIdFails = this.getFromPreferences(context, DbConstants.SMS_ID_FAIL, []) as number[];
    }
    let smsIdSuccess = this.getFromPreferences(context, DbConstants.SMS_ID_SUCCESS, 0) as number;
    LogUtils.i(TAG, 'smsPage ：' + smsPage);
    for (let curPage = 1; curPage <= smsPage; curPage++) {
      idSuccessCount += await this.queryMmsInfo(context, DbConstants.QUERY_PAGE_NUM * (curPage - 1) + smsIdSuccess,
        smsIdFails, smsInfoSet, progress);
    }
    this.doMmsSuccess = idFailCount + idSuccessCount;
    progress.updateProcessCount(progress.getSmsCloneProgress());
  }

  private async doPduTableCopy(context: common.Context, progress: BackupRestoreProgress) {
    let idFailCount = 0;
    let idSuccessCount = 0;
    let pduCount = await this.queryDataCount(context, DbConstants.OLD_PDU_NAME, OldDBData.pdu.id);
    let pduPage = Math.ceil(pduCount / DbConstants.QUERY_PAGE_NUM);
    this.queryPduColumn = await this.queryColumns(context, DbConstants.OLD_PDU_NAME);
    let pduIdFails = this.getFromPreferences(context, DbConstants.PDU_ID_FAIL, []) as number[];
    if (pduIdFails.length > 0) {
      idFailCount += await this.queryPduInfo(context, -1, pduIdFails, progress);
      pduIdFails = this.getFromPreferences(context, DbConstants.PDU_ID_FAIL, []) as number[];
    }
    let pduIdSuccess = this.getFromPreferences(context, DbConstants.PDU_ID_SUCCESS, 0) as number;
    LogUtils.i(TAG, 'pduPage ：' + pduPage);
    for (let curPage = 1; curPage <= pduPage; curPage++) {
      idSuccessCount += await this.queryPduInfo(context, DbConstants.QUERY_PAGE_NUM * (curPage - 1) +
        pduIdSuccess, pduIdFails, progress);
    }
    this.doPduSuccess = idFailCount + idSuccessCount;
    progress.updateProcessCount(progress.getPduCloneProgress());
  }

  private async doPartTableCopy(context: common.Context, progress: BackupRestoreProgress) {
    let idFailCount = 0;
    let idSuccessCount = 0;
    let partCount = await this.queryDataCount(context, DbConstants.OLD_PART_NAME, OldDBData.part.id);
    let partPage = Math.ceil(partCount / DbConstants.QUERY_PAGE_NUM);
    this.queryPartColumn = await this.queryColumns(context, DbConstants.OLD_PART_NAME);
    let partIdFails = this.getFromPreferences(context, DbConstants.PART_ID_FAIL, []) as number[];
    if (partIdFails.length > 0) {
      idFailCount += await this.queryPartInfo(context, -1, partIdFails, progress);
      partIdFails = this.getFromPreferences(context, DbConstants.PART_ID_FAIL, []) as number[];
    }
    let partIdSuccess = this.getFromPreferences(context, DbConstants.PART_ID_SUCCESS, 0) as number;
    LogUtils.i(TAG, 'partPage ：' + partPage);
    for (let curPage = 1; curPage <= partPage; curPage++) {
      idSuccessCount += await this.queryPartInfo(context, DbConstants.QUERY_PAGE_NUM * (curPage - 1) +
        partIdSuccess, partIdFails, progress);
    }
    this.doPartSuccess = idFailCount + idSuccessCount;
    progress.updateProcessCount(progress.getPartCloneProgress());
    // 更新part表语音文件的recording_time
    try {
      let resultSet = await newDbHelper.queryPart(context)
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'No data exists in the part table, do nothing.');
        return;
      }
      do {
        let partId = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.mmsPart.id);
        let msgId = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.mmsPart.msgId);
        let type = newDbHelper.getColumnIntegerValue(resultSet, NextOldDBData.mmsPart.type);
        if (partId === undefined || msgId === undefined || type === undefined) {
          LogUtils.e(TAG, 'invalid key field');
          continue;
        }
        if (MMS_ID_RECORDING_TIME_MAP.hasKey(msgId) && type == 3) {
          let upValues: ValuesBucket = {
            'recording_time': MMS_ID_RECORDING_TIME_MAP.get(msgId)
          }
          await newDbHelper.updatePart(context, partId, upValues)
        }
      } while (resultSet.goToNextRow());
      LogUtils.i(TAG, 'update part table end.');
    } catch (error) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `doPartTableCopy code:${error.code} msg:${error.message}`);
      LogUtils.w(TAG, 'update part table failed');
    }
  }

  private async prepareAddressTableData(context: common.Context, progress: BackupRestoreProgress) {
    let addressCount = await this.queryDataCount(context, DbConstants.OLD_ADDRESSES_NAME, OldDBData.addresses.ID);
    let addressPage = Math.ceil(addressCount / DbConstants.QUERY_PAGE_NUM);
    LogUtils.i(TAG, 'addressPage：' + addressPage);
    for (let curPage = 1; curPage <= addressPage; curPage++) {
      await this.queryAddressInfo(context, DbConstants.QUERY_PAGE_NUM * (curPage - 1), progress);
    }
    progress.updateProcessCount(progress.getAddressCloneProgress());
  }

  public async doMigrateData(context: common.Context, progress: BackupRestoreProgress) {
    LogUtils.i(TAG, 'doMigrateData start');
    await this.queryUnreadCount(context, progress);
    await this.queryTotalMsgCount(context, progress);
    await this.prepareAddressTableData(context, progress);
    await this.doSessionTableCopy(context, progress);
    await this.doMmsTableCopy(context, progress);
    await this.doPduTableCopy(context, progress);
    await this.doPartTableCopy(context, progress);
    await this.doRcsChatTableCopy(context, progress);
    await this.doRcsGroupThreadsCopy(context, progress);
    await newDbHelper.deleteSms(context);
    await newDbHelper.deleteInvalidSession(context, progress);
    await newDbHelper.deleteInvalidSmsMmsInfo(context, progress);
    await newDbHelper.deleteInvalidRcsInfo(context, progress);
    await newDbHelper.recountAndUpdateUnread(context, progress);
    await newDbHelper.updateSessionMsgCount(context, progress);
    progress.updateProcessCount(progress.getTotalCount());
    this.checkSessionTime();
    LogUtils.i(TAG, 'doMigrateData end');
  }

  private async queryTotalMsgCount(context: Context, progress: BackupRestoreProgress) {
    let addressCount = await this.queryDataCount(context, DbConstants.OLD_ADDRESSES_NAME, OldDBData.addresses.ID);
    let threadsCount = await this.queryDataCount(context, DbConstants.OLD_THREADS_NAME, OldDBData.threads.threadId);
    let threadNewCount = await newDbHelper.queryDataCount(context, DbConstants.TABLE_SESSION,
      NextOldDBData.session.id);
    // In scenarios where clones are repeated for multiple times, the number of unread sessions
    // between the old and new servers needs to be counted.
    let totalThreadCount = threadsCount + threadNewCount;
    let smsCount = await this.queryDataCount(context, DbConstants.OLD_SMS_NAME, OldDBData.sms.threadId);
    let pduCount = await this.queryDataCount(context, DbConstants.OLD_PDU_NAME, OldDBData.pdu.id);
    let partCount = await this.queryDataCount(context, DbConstants.OLD_PART_NAME, OldDBData.part.id);
    let rcsChatCount = await this.queryRcsDataCount(context, DbConstants.OLD_RCS_CHAT_TABLE_NAME, OldDBData.chat.id);
    let rcsGroupCount = await this.queryRcsDataCount(context, DbConstants.OLD_RCS_GROUP_TABLE_NAME,
      OldDBData.rcsGroup.id);
    let totalCount = addressCount + threadsCount + smsCount + pduCount + partCount + rcsChatCount + rcsGroupCount +
      totalThreadCount;
    // 为迁移完成后计算会话信息数量提供总量计数
    totalCount = totalCount + threadsCount + threadNewCount;
    // 迁移完成后清理无效会话、info提供count总量计数
    let invalidItemCount = threadsCount + smsCount + pduCount + rcsChatCount + rcsGroupCount;
    totalCount = totalCount + invalidItemCount;
    progress.setAddressCount(addressCount);
    progress.setThreadCount(threadsCount);
    progress.setSmsCount(smsCount);
    progress.setPduCount(pduCount);
    progress.setPartCount(partCount);
    progress.setRcsChatCount(rcsChatCount);
    progress.setRcsGroupCount(rcsGroupCount);
    let countList: string = 'address[' + addressCount + '],thread[' + threadsCount + '],sms[' + smsCount +
      '],pdu[' + pduCount + '],part[' + partCount + '],rcsChat[' + rcsChatCount + '],rcsGroup[' +
      rcsGroupCount + ']';
    LogUtils.i(TAG, `update progress total count ${totalCount}, listed: ${countList}`);
    const backupRestoreResult = BackupRestoreResult.getInstance();
    backupRestoreResult.insertOrUpdateBackupInfo('sms', `msg_count_list : ${countList}`);
    progress.setTotalCount(totalCount);
    return totalCount;
  }

  private checkSessionTime() {
    // if latestTime is zero， output log
    SESSION_TIME_MAP.forEach((latestTime, sessionId) => {
      if (latestTime == 0) {
        LogUtils.i(TAG, 'the latest time in session is zero.');
      }
    });
  }

  private getFromPreferences(context: common.Context, key: string,
    defValue: dataPreferences.ValueType):dataPreferences.ValueType {
    LogUtils.i(TAG, 'getFromPreferences start');
    let preferences: dataPreferences.Preferences | null = this.getPreferences(context);
    if (preferences == null) {
      LogUtils.e(TAG, 'getFromPreferences is null');
      return defValue;
    }
    LogUtils.i(TAG, 'getFromPreferences end');
    let value:dataPreferences.ValueType = defValue;
    try {
      value = preferences.getSync(key, defValue);
      LogUtils.i(TAG, 'getFromPreferences Succeeded');
    } catch (err) {
      let e: BusinessError = err as BusinessError;
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `getFromPreferences code:${err.code} msg:${err.message}`);
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
      let err: BusinessError = error as BusinessError;
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `getPreferences code:${err.code} msg:${err.message}`);
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
        name: DbConstants.cloneDatabaseName,
        securityLevel: rdb.SecurityLevel.S1
      };
      LogUtils.i(TAG, '[createDB] OLD_CONFIG');
      this.rdbStoreHelper = await rdb.getRdbStore(context, OLD_CONFIG);
    } catch (error) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `initDB code:${error.code} msg:${error.message}`);
      LogUtils.w(TAG, '[createDB] failed : ' + JSON.stringify(error));
    }
    if (!this.rdbRcsStoreHelper) {
      try {
        let OLD_CONFIG1: rdb.StoreConfig = {
          name: 'chatSms.db',
          securityLevel: rdb.SecurityLevel.S1
        };
        LogUtils.i(TAG, '[createDB] OLD_CONFIG1');
        this.rdbRcsStoreHelper = await rdb.getRdbStore(context, OLD_CONFIG1);
      } catch (error) {
        const backupRestoreResult = BackupRestoreResult.getInstance();
        backupRestoreResult.insertOrUpdateBackupInfo('sms',
          `initDB code:${error.code} msg:${error.message}`);
        LogUtils.w(TAG, '[createDB] failed : ' + JSON.stringify(error));
      }
    }
    if (this.rdbStoreHelper) {
      return Promise.resolve(this.rdbStoreHelper);
    }
    return undefined;
  }

  private async queryRcsUnreadCountWithTable(rdbStore: rdb.RdbStore, table: string): Promise<number> {
    let totalUnreadCount: number = 0;
    try {
      let sql = 'SELECT (SELECT COUNT(*) FROM ' + table + ' WHERE read = 0) AS total_count;';
      let resultSet = await rdbStore.querySql(sql);
      if (resultSet === undefined) {
        LogUtils.w(TAG, 'resultSet === undefined');
        return Promise.resolve(0);
      }
      while (resultSet.goToNextRow()) {
        totalUnreadCount = resultSet.getLong(resultSet.getColumnIndex('total_count'));
      }
      return totalUnreadCount;
    } catch (err) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `queryRcsUnreadCountWithTable code:${err.code} msg:${err.message}`);
      LogUtils.e(TAG, `queryRcsUnreadCountWithTable failed, code is ${err.code},message is ${err.message}`);
      return Promise.resolve(0);
    }
  }

  private async queryRcsUnreadCount(context: common.Context, progress: BackupRestoreProgress): Promise<number> {
    LogUtils.w(TAG, `queryRcsUnreadCount start! `)
    if (context == null) {
      LogUtils.w(TAG, `queryRcsUnreadCount failed by null context`)
      return Promise.resolve(0);
    }
    let count = 0;
    try {
      let rdbStore = await this.initRcsDB(context);
      if (rdbStore === undefined) {
        LogUtils.e(TAG, `[queryRcsUnreadCount] failed : rdbStore === undefined`)
        return Promise.resolve(0);
      }
      let sql = 'SELECT name FROM sqlite_master WHERE type = \'table\';';
      let resultSet = await rdbStore.querySql(sql);
      if (resultSet === undefined) {
        LogUtils.w(TAG, 'resultSet === undefined');
        return Promise.resolve(0);
      }
      while (resultSet.goToNextRow()) {
        const tableName = resultSet.getString(resultSet.getColumnIndex('name'));
        if (tableName === DbConstants.OLD_RCS_CHAT_TABLE_NAME ||
            tableName === DbConstants.OLD_RCS_GROUP_TABLE_NAME) {
          count = count + await this.queryRcsUnreadCountWithTable(rdbStore, tableName);
        }
      }
      LogUtils.i(TAG, '[queryRcsUnreadCount] count : ' + count);
      resultSet.close();
      return count;
    } catch (err) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `queryRcsUnreadCount code:${err.code} msg:${err.message}`);
      LogUtils.e(TAG, `queryRcsUnreadCount failed, code is ${err.code},message is ${err.message}`);
      return Promise.resolve(0);
    }
  }

  private async querySMSUnreadCount(context: Context, progress: BackupRestoreProgress): Promise<number> {
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
      let sql: string = 'SELECT\n' +
        '  ( SELECT COUNT( * ) FROM sms_tb WHERE read = 0 ) + \n' +
        '  ( SELECT COUNT( * ) FROM pdu_tb WHERE read = 0 ) AS total_count;';
      let resultSet: rdb.ResultSet = rdbStore.querySqlSync(sql);
      if (resultSet === undefined) {
        LogUtils.w(TAG, 'resultSet === undefined');
        return Promise.resolve(0);
      }
      while (resultSet.goToNextRow()) {
        totalUnreadCount = resultSet.getLong(resultSet.getColumnIndex('total_count'));
      }
      return totalUnreadCount;
    } catch (err) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `queryUnreadCount code:${err.code} msg:${err.message}`);
      LogUtils.e(TAG, `queryUnreadCount failed, code is ${err.code},message is ${err.message}`);
      return Promise.resolve(0);
    }
  }

  private async queryUnreadCount(context: Context, progress: BackupRestoreProgress): Promise<number> {
    let count: number = 0;
    count = await this.querySMSUnreadCount(context, progress) + await this.queryRcsUnreadCount(context, progress);
    const backupRestoreResult = BackupRestoreResult.getInstance();
    backupRestoreResult.insertOrUpdateBackupInfo('sms',
      `ori_unread_count :${count}`);
    return count;
  }

  private async queryDataCount(context: common.Context, tableName:string, queryKey:string): Promise<number> {
    if (context == null) {
      LogUtils.w(TAG, `queryDataCount failed by null context`)
      return Promise.resolve(0);
    }
    let count = 0;
    try {
      let predicatesInfo = new rdb.RdbPredicates(tableName);
      let rdbStore = await this.initDB(context);
      if (rdbStore === undefined) {
        LogUtils.e(TAG, `[queryDataCount] failed : rdbStore === undefined`)
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
      LogUtils.e(TAG, `[queryDataCount] error : ${error.code}`)
    }
    return Promise.resolve(count);
  }

  private async queryAddressInfo(context: common.Context, index: number,
      progress: BackupRestoreProgress): Promise<void> {
    let addressInfo = new rdb.RdbPredicates(DbConstants.OLD_ADDRESSES_NAME);
    let rdbStore = await this.initDB(context);
    if (rdbStore === undefined) {
      LogUtils.e(TAG, `[queryAddressInfo] failed : rdbStore === undefined`)
      return;
    }
    try {
      let queryAddressColumn = [OldDBData.addresses.ID, OldDBData.addresses.address]
      this.buildPredicatesInfo(addressInfo, OldDBData.addresses.ID, index);
      let resultSet = await rdbStore.query(addressInfo, queryAddressColumn);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'no raw address found.');
        return;
      }
      do {
        let ID = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.addresses.ID);
        let address = newDbHelper.getColumnStringValue(resultSet, OldDBData.addresses.address);
        if (ID === undefined || address === undefined) {
          LogUtils.e(TAG, 'Invalid key field');
          continue;
        }
        ADDRESS_ID_MAP.set(ID, address);
      } while (resultSet.goToNextRow())
      progress.updateProcessCount(progress.getProcessCount() + resultSet.rowCount);
      resultSet.close();
    } catch (error) {
      LogUtils.w(TAG, 'query AddressInfo error : ' + JSON.stringify(error));
    }
  }


  private getMmsInfoPredicates(mmsInfoPredicatesInfo: rdb.RdbPredicates, id: string,
                               index: number, smsIdFails: number[]): rdb.RdbPredicates {
    if (index != -1) {
      mmsInfoPredicatesInfo = this.buildPredicatesInfo(mmsInfoPredicatesInfo, OldDBData.sms.threadId, index);
    } else {
      mmsInfoPredicatesInfo = this.buildPredicatesInfoFail(mmsInfoPredicatesInfo, OldDBData.sms.threadId, smsIdFails);
    }
    return mmsInfoPredicatesInfo
  }

  private buildInsertMmsInfo(sessionIDUpdateContentMap:HashMap<number, ValuesBucket>, resultSet:rdb.ResultSet,
    smsInfoSet: HashMap<string, string>, batchSmsValues: ValuesBucket[]): void {
    let smsInfoActionData: LooseObject = {};
    let arrPhoneNumber = this.buildSmsMmsInfoAction(smsInfoActionData, resultSet);
    let phonyNumber: string = StringUtil.isEmpty(smsInfoActionData.receiverNumber) ? smsInfoActionData.senderNumber :
      smsInfoActionData.receiverNumber;
    let isSender = smsInfoActionData.type == 1 ? 1 : 0;
    let smsType = (arrPhoneNumber != undefined && arrPhoneNumber.length > 1) ? 0 :
      (TelephoneUtil.judgeIsInfoMsg(phonyNumber) ? 1 : (this.isChatbotNumber(arrPhoneNumber?.toString()) ? 1 : 0));
    let sessionId = RAW_THREAD_ID_MAP.get(smsInfoActionData.threadId);
    let maxGroupId = MAX_GROUP_ID[0];
    let newGroupId: number = -1;
    if (this.version.startsWith(BackupConstants.ARD2NEXT_CLONE_VERSION) ||
      this.version.startsWith(BackupConstants.IOS2NEXT_CLOUD_CLONE_VERSION)) {
      newGroupId = SMS_GROUP_ID_MAP.get(smsInfoActionData.startTime);
      if (!newGroupId || newGroupId == -1) {
        newGroupId = ++maxGroupId;
        SMS_GROUP_ID_MAP.set(smsInfoActionData.startTime, newGroupId);
      }
    } else {
      newGroupId = SMS_GROUP_ID_MAP.get(smsInfoActionData.groupId);
      if (!newGroupId || newGroupId == -1) {
        newGroupId = ++maxGroupId;
        SMS_GROUP_ID_MAP.set(smsInfoActionData.groupId, newGroupId);
      }
    }
    let isRepeat: boolean = false;
    if (arrPhoneNumber != undefined && arrPhoneNumber.length > 1) {
      for (let index = 0; index < arrPhoneNumber.length; index++) {
        const number = arrPhoneNumber[index];
        let key = number + ':' + isSender + ':' + smsInfoActionData.startTime;
        isRepeat = SMS_INFO_MAP.hasKey(key);
      }
    }
    let key = phonyNumber + ':' + isSender + ':' + smsInfoActionData.startTime;
    isRepeat = SMS_INFO_MAP.hasKey(key) || isRepeat;
    this.judgeCloneSmsInfo(isRepeat, sessionId, smsInfoActionData, smsType,
      sessionIDUpdateContentMap, arrPhoneNumber, newGroupId, batchSmsValues);
  }

  private judgeCloneSmsInfo(isRepeat: boolean, sessionId: number | undefined,
    smsInfoActionData: LooseObject, smsType: number,
    sessionIDUpdateContentMap: HashMap<number, ValuesBucket>,
    arrPhoneNumber: string[] | undefined, newGroupId: number,
    batchSmsValues: ValuesBucket[]) {
    if (!isRepeat) {
      if (sessionId === undefined) {
        LogUtils.e(TAG, 'invalid sms info msg, no matched session id');
        return;
      }
      this.generateUpdateSessionData(sessionId, smsInfoActionData.isRead,
        smsInfoActionData.startTime, smsInfoActionData.msgContent,
        smsInfoActionData.msgState, smsType, smsInfoActionData.hasDraft,
        smsInfoActionData.hasLocked, sessionIDUpdateContentMap);
      let values: ValuesBucket = new SmsMmsInfoItem().setReceiverNumber(smsInfoActionData.receiverNumber)
        .setSenderNumber(smsInfoActionData.senderNumber).setMsgTitle(smsInfoActionData.msgTitle)
        .setMsgContent(smsInfoActionData.msgContent).setMsgState(smsInfoActionData.msgState)
        .setIsLock(smsInfoActionData.isLock).setIsRead(smsInfoActionData.isRead)
        .setSessionId(sessionId).setGroupId(newGroupId)
        .setStartTime(smsInfoActionData.startTime).setEndTime(smsInfoActionData.endTime)
        .setSlotId(smsInfoActionData.slotId).setMsgType(0)
        .setIsSender(smsInfoActionData.type == 1 ? 1 : 0).setSmsType(smsType)
        .createValuesBucket();
      batchSmsValues.push(values);

      MAX_GROUP_ID[0] = newGroupId;
      GROUP_ID_MAP.set(newGroupId, newGroupId)
    }
  }

  private buildSmsMmsInfoAction(smsInfoActionData: LooseObject, resultSet: rdb.ResultSet) {
    smsInfoActionData.slotId = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.sms.subId);
    let address = newDbHelper.getColumnStringValue(resultSet, OldDBData.sms.address);
    smsInfoActionData.type = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.sms.type);
    smsInfoActionData.receiverNumber = smsInfoActionData.type == 1 ? '' : address;
    smsInfoActionData.senderNumber = smsInfoActionData.type == 1 ? address : '';
    smsInfoActionData.startTime = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.sms.date);
    smsInfoActionData.endTime = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.sms.dateSent);
    smsInfoActionData.status = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.sms.status);
    smsInfoActionData.msgTitle = newDbHelper.getColumnStringValue(resultSet, OldDBData.sms.subject);
    smsInfoActionData.msgContent = newDbHelper.getColumnStringValue(resultSet, OldDBData.sms.body);
    smsInfoActionData.isLock = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.sms.locked);
    smsInfoActionData.isRead = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.sms.read);
    smsInfoActionData.threadId = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.sms.threadId);
    if (!this.version.startsWith(BackupConstants.ARD2NEXT_CLONE_VERSION)) {
      smsInfoActionData.groupId = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.sms.groupId);
    }
    let arrPhoneNumber = THREAD_PHONE_MAP.get(smsInfoActionData.threadId);
    smsInfoActionData.hasDraft = 0;
    smsInfoActionData.hasLocked = smsInfoActionData.isLock = 1 ? 1 : 0;
    smsInfoActionData.msgState = 0;
    if (smsInfoActionData.type == 5) {
      smsInfoActionData.msgState = 2;
    } else if (smsInfoActionData.type == 3) {
      smsInfoActionData.msgState = 3;
      smsInfoActionData.hasDraft = 1;
    } else if (smsInfoActionData.type == 2) {
      smsInfoActionData.msgState = 0;
    } else if (smsInfoActionData.type == 4) {
      smsInfoActionData.msgState = 1;
    } else if (smsInfoActionData.type == 1) {
      smsInfoActionData.msgState = 0;
    } else {
      LogUtils.e(TAG, 'invalid msg type ' + smsInfoActionData.type);
    }
    if (smsInfoActionData.isRead == 0) {
      LogUtils.e(TAG, 'thread ' + smsInfoActionData.threadId + ' have no read sms ');
    }
    return arrPhoneNumber;
  }

  private async queryMmsInfo(context: common.Context, index: number,
      smsIdFails: number[], smsInfoSet: HashMap<string, string>, progress: BackupRestoreProgress): Promise<number> {
    LogUtils.e(TAG, `[queryMmsInfo] start`)
    let failCount = 0;
    let successCount = 0;
    let mmsInfo = new rdb.RdbPredicates(DbConstants.OLD_SMS_NAME);
    let rdbStore = await this.initDB(context);
    if (rdbStore === undefined) {
      LogUtils.e(TAG, `[queryMmsInfo] failed : rdbStore === undefined`)
      return successCount;
    }
    try {
      mmsInfo = this.getMmsInfoPredicates(mmsInfo, OldDBData.sms.threadId, index, smsIdFails);
      let resultSet = await rdbStore.query(mmsInfo, this.queryMmsColumn);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.e(TAG, 'no raw Mms found.');
        return successCount;
      }
      let countMms = resultSet.rowCount;
      let batchSmsValues: ValuesBucket[] = [];
      let sessionIDUpdateContentMap = new HashMap<number, ValuesBucket>();
      do {
         this.buildInsertMmsInfo(sessionIDUpdateContentMap, resultSet, smsInfoSet, batchSmsValues);
      } while (resultSet.goToNextRow())
      resultSet.close();
      LogUtils.i(TAG, `batchInsertSms len = ${JSON.stringify(batchSmsValues.length)}`)
      if (batchSmsValues.length > 0) {
        let insertNum = await newDbHelper.batchInsertSms(context, batchSmsValues);
        if (insertNum == -1) {
          failCount += batchSmsValues.length;
        } else {
          successCount += insertNum;
        }
      }
      let arrayFromMap = Array.from(sessionIDUpdateContentMap);
      for (let index = 0; index < arrayFromMap.length; index++) {
        const element = arrayFromMap[index];
        await newDbHelper.updateSession(context, element[0], element[1]);
      }
      progress.updateProcessCount(progress.getProcessCount() + countMms);
      this.doMmsFailed = this.doMmsFailed + failCount;
      LogUtils.i(TAG, 'mms Total Migrations count: ' + countMms + ',Successful Migrations successCount: ' + successCount + 'Failed Migrations failCount: ' + failCount);
    } catch (error) {
      LogUtils.w(TAG, 'query MmsInfo error : ' + JSON.stringify(error));
    }
    return successCount;
  }

  private generateUpdateSessionData(sessionId: number, isRead: number, startTime: number, msgContent: string, msgState:
    number, smsType: number, hasDraft: number, hasLocked: number, sessionIDUpdateContentMap:
    HashMap<number, ValuesBucket>) {
    let messageCount = SESSION_MESSAGE_COUNT_MAP.get(sessionId) || 0;
    messageCount ++;
    SESSION_MESSAGE_COUNT_MAP.set(sessionId, messageCount);
    let unreadCount = 0;
    // 能否在 session 迁移的时候更新 SESSION_TIME_MAP  RAW_THREAD_ID_MAP(新机 session， 旧机 session)
    let beforeTime = SESSION_TIME_MAP.get(sessionId);
    let beforeContent = SESSION_CONTENT_MAP.get(sessionId);
    let beforeStatus = SESSION_STATUS_MAP.get(sessionId);
    let resultTime = startTime;
    let resultContent = msgContent;
    let resultStatus = msgState;
    if (hasDraft == 1) {
      SESSION_HAS_DRAFT_SET.add(sessionId);
    }
    if (SESSION_HAS_DRAFT_SET.has(sessionId)) {
      hasDraft = 1;
    }
    if (beforeTime) {
      if (beforeTime > resultTime) {
        resultTime = beforeTime;
        resultContent = beforeContent === undefined ? '' : beforeContent;
        resultStatus = beforeStatus === undefined ? -1 : beforeStatus;
      } else {
        SESSION_TIME_MAP.set(sessionId, resultTime);
        SESSION_CONTENT_MAP.set(sessionId, resultContent);
        SESSION_STATUS_MAP.set(sessionId, resultStatus);
      }
    } else {
      SESSION_TIME_MAP.set(sessionId, resultTime);
      SESSION_CONTENT_MAP.set(sessionId, resultContent);
      SESSION_STATUS_MAP.set(sessionId, resultStatus);
    }
    let upValues: ValuesBucket = this.generateUpdateSessionDataBuilder(resultTime,
      resultContent, messageCount, smsType, resultStatus, unreadCount, hasDraft, hasLocked);
    sessionIDUpdateContentMap.set(sessionId, upValues)
  }

  private generateUpdateSessionDataBuilder(resultTime: number, resultContent: string, messageCount: number,
                                           smsType: number, resultStatus: number, unreadCount: number,
                                           hasDraft: number, hasLocked: number) {
    let upValues: ValuesBucket = {
      'time': resultTime,
      'content': resultContent,
      'message_count': messageCount,
      'sms_type': smsType,
      'sending_status': resultStatus,
      'unread_count': unreadCount
    };
    if (hasDraft == 1 || hasLocked == 1) {
      upValues = {
        'has_draft': hasDraft,
        'has_lock': hasLocked,
        'time': resultTime,
        'content': resultContent,
        'message_count': messageCount,
        'sms_type': smsType,
        'sending_status': resultStatus,
        'unread_count': unreadCount
      };
    }
    return upValues;
  }

  private async queryPduInfo(context: common.Context, index: number, pduIdFails: number[],
      progress: BackupRestoreProgress): Promise<number> {
    LogUtils.i(TAG, `[queryPduInfo] start`)
    let pduActionData: LooseObject = {}
    pduActionData.failCount = 0;
    pduActionData.successCount = 0;
    let maxGroupId = MAX_GROUP_ID[0];
    let pduInfo = new rdb.RdbPredicates(DbConstants.OLD_PDU_NAME);
    let rdbStore = await this.initDB(context);
    if (rdbStore === undefined) {
      LogUtils.e(TAG, `[queryPduInfo] failed : rdbStore === undefined`)
      return pduActionData.successCount;
    }
    try {
      if (index != -1) {
        pduInfo = this.buildPredicatesInfo(pduInfo, OldDBData.pdu.id, index);
      } else {
        pduInfo = this.buildPredicatesInfoFail(pduInfo, OldDBData.pdu.id, pduIdFails);
      }
      let resultSet = await rdbStore.query(pduInfo, this.queryPduColumn);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'no raw pdu found.');
        return pduActionData.successCount;
      }
      let count = resultSet.rowCount;
      maxGroupId = await this.buildPduInfo(resultSet, maxGroupId, context, rdbStore, pduActionData);
      MAX_GROUP_ID[0] = maxGroupId;
      resultSet.close();
      this.doPduFailed = this.doPduFailed + pduActionData.failCount;
      progress.updateProcessCount(progress.getProcessCount() + count);
      LogUtils.i(TAG, 'pdu Total Migrations count: ' + count + ',Successful Migrations successCount: ' + pduActionData.successCount +
        'Failed Migrations failCount: ' + pduActionData.failCount);
    } catch (error) {
      LogUtils.w(TAG, 'query PduInfo error : ' + JSON.stringify(error));
    }
    return pduActionData.successCount;
  }

  private async buildPduInfo(resultSet: rdb.ResultSet, maxGroupId: number, context: Context, rdbStore: rdb.RdbStore,
    pduActionData: LooseObject) {
    do {
      let msgId = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.pdu.subId);
      let threadId = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.pdu.threadId);
      let read = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.pdu.threadId);
      let msgBox = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.pdu.msgBox);
      let pduId = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.pdu.id);
      let startTime = newDbHelper.getColumnIntegerValueWithDefault(resultSet, OldDBData.pdu.date, 0);
      let ctT = newDbHelper.getColumnStringValue(resultSet, OldDBData.pdu.ctT);
      let ctL = newDbHelper.getColumnStringValue(resultSet, OldDBData.pdu.ctL);
      let st = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.pdu.st);
      if (read == 0) {
        LogUtils.e(TAG, 'thread ' + threadId + ' have no read mms ');
      }
      if (pduId === undefined || threadId === undefined) {
        LogUtils.e(TAG, 'Invalid key field');
        continue;
      }
      // 双 psu.st ：未启动-128，下载中-129，传输失败-130，保存失败-135
      if (st == 128 || st == 129 || st == 130 || st == 135) {
        LogUtils.e(TAG, 'not support pdu st ' + st + ' ,break ');
        continue;
      }
      let groupId = ++maxGroupId;
      await this.copyPdu(context, resultSet, rdbStore, groupId, pduActionData, pduId, threadId, startTime);
    } while (resultSet.goToNextRow());
    return maxGroupId;
  }

  private async copyPdu(context: Context, resultSet: rdb.ResultSet, rdbStore: rdb.RdbStore, groupId: number,
    pduActionData: LooseObject, pduId: number, threadId: number, startTime: number) {
    let curSessionId = RAW_THREAD_ID_MAP.get(threadId);
    if (curSessionId === undefined) {
      LogUtils.e(TAG, 'invalid pdu msg, no matched session id');
      return;
    }
    let arrValues = await this.getSmsMmsInfoValue(context, resultSet, rdbStore, groupId);
    LogUtils.i(TAG, `pdu arrValues len: ${arrValues.length}`);
    for (let index = 0; index < arrValues.length; index++) {
      const values = arrValues[index];
      let newRawSmsId = await newDbHelper.insertSms(context, values);
      if (newRawSmsId == -1) {
        pduActionData.failCount++;
      } else {
        let msgIds = RAW_PDU_ID_MAP.get(pduId);
        LogUtils.e(TAG, 'pdu get new msgIds ' + msgIds + ' from ' + pduId);
        if (msgIds === undefined) {
          let partId: number[] = [];
          partId.push(newRawSmsId);
          LogUtils.e(TAG, `pdu set [old pdu id -> new msg id]: ${pduId} -> ${partId}`);
          RAW_PDU_ID_MAP.set(pduId, partId);
        } else {
          msgIds.push(newRawSmsId);
          LogUtils.e(TAG, `pdu set group [old pdu id -> new msg id]: ${pduId} -> ${msgIds}`);
          RAW_PDU_ID_MAP.set(pduId, msgIds);
        }
        SMS_ID_SESSION_ID_MAP.set(newRawSmsId, curSessionId);
        SMS_ID_GROUP_ID_MAP.set(newRawSmsId, groupId);
        MMS_ID_TIME_MAP.set(newRawSmsId, startTime * 1000);
        pduActionData.successCount++;
      }
    }
  }

  private async getSmsMmsInfoValue(context: common.Context, resultSet: rdb.ResultSet, rdbStore: rdb.RdbStore,
    groupId: number): Promise<Array<ValuesBucket>> {
    let pduSmsInfo: LooseObject = {};
    let arrPhoneNumber = this.buildPduMmsInfoAction(pduSmsInfo, resultSet);
    let msgContent = await this.getMsgContent(rdbStore, pduSmsInfo.pduId);
    let smsType = (arrPhoneNumber != undefined && arrPhoneNumber.length > 1) ? 0 :
      (TelephoneUtil.judgeIsInfoMsg(pduSmsInfo.senderNumber ? pduSmsInfo.senderNumber : pduSmsInfo.receiverNumber) ?
        1 : (this.isChatbotNumber(arrPhoneNumber?.toString()) ? 1 : 0));
    let sessionId = RAW_THREAD_ID_MAP.get(pduSmsInfo.threadId);
    let isSender = pduSmsInfo.msgBox == 1 ? 1 : 0;
    let isRepeat: boolean = false;
    arrPhoneNumber?.forEach((number) => {
      let key = number + ':' + isSender + ':' + pduSmsInfo.startTime + '000';
      isRepeat = SMS_INFO_MAP.hasKey(key);
    })
    let arrValues: ValuesBucket[] =
      await this.buildMmsArrValues(isRepeat, sessionId, pduSmsInfo, smsType,
        context, arrPhoneNumber, msgContent, resultSet, groupId);
    return arrValues;
  }

  private isChatbotNumber(telephone: string | undefined): boolean {
    if (telephone?.startsWith('sip:') && telephone?.includes('botplatform')) {
      return true
    }
    return false
  }

  private buildPhoneNumber(actionData: LooseObject): string {
    if (!actionData) {
      return '';
    }
    return (!StringUtil.isEmpty(actionData.senderNumber)) ? actionData.senderNumber : actionData.receiverNumber;
  }

  private async buildFormatPhoneNumber(actionData: LooseObject): Promise<string> {
    if (!actionData) {
      return '';
    }
    let phoneNumber: string = this.buildPhoneNumber(actionData);
    return TelephoneUtil.formatPhoneNumberToE164(phoneNumber, actionData.subId);
  }

  private async buildMmsArrValues(isRepeat: boolean, sessionId: number | undefined,
    actionData: LooseObject, smsType: number, context: Context, arrPhoneNumber: string[] | undefined,
    msgContent: string, resultSet: rdb.ResultSet, groupId: number) {
    let arrValues: ValuesBucket[] = [];
    if (!isRepeat) {
      if (sessionId === undefined) {
        LogUtils.e(TAG, 'invalid sms info msg, no matched session id');
        return arrValues;
      }
      let updateSessionItem =
        new UpdateSessionItem(sessionId, actionData.isRead, actionData.startTime, actionData.sub, actionData.msgState,
          smsType, actionData.hasDraft);
      let upValues: ValuesBucket = this.getUpdateSessionValue(updateSessionItem);
      await newDbHelper.updateSession(context, sessionId, upValues);
      if (arrPhoneNumber != undefined && arrPhoneNumber.length > 1) {
        for (let index = 0; index < arrPhoneNumber.length; index++) {
          actionData.senderNumber = '';
          actionData.receiverNumber = arrPhoneNumber[index];
          let values: ValuesBucket = new SmsMmsInfoItem().setSlotId(actionData.subId).setMsgType(1)
            .setStartTime(actionData.startTime * 1000).setEndTime(actionData.endTime * 1000)
            .setMsgState(actionData.msgState).setSenderNumber(actionData.senderNumber)
            .setReceiverNumber(actionData.receiverNumber).setMsgTitle(actionData.sub).setMsgContent(msgContent)
            .setIsLock(newDbHelper.getColumnIntegerValue(resultSet, OldDBData.pdu.locked))
            .setIsRead(actionData.isRead).setSessionId(sessionId).setGroupId(groupId)
            .setIsSender(0).setSmsType(smsType)
            .createValuesBucket()
          arrValues.push(values);
        }
      } else {
        let values: ValuesBucket = new SmsMmsInfoItem().setSlotId(actionData.subId).setMsgType(1)
          .setStartTime(actionData.startTime * 1000).setEndTime(actionData.endTime * 1000)
          .setMsgState(actionData.msgState).setSenderNumber(actionData.senderNumber)
          .setReceiverNumber(actionData.receiverNumber).setMsgTitle(actionData.sub).setMsgContent(msgContent)
          .setIsLock(newDbHelper.getColumnIntegerValue(resultSet, OldDBData.pdu.locked))
          .setIsRead(actionData.isRead).setSessionId(sessionId).setGroupId(groupId)
          .setIsSender(actionData.msgBox == 1 ? 1 : 0).setSmsType(smsType)
          .createValuesBucket();
        arrValues.push(values);
      }
    }
    return arrValues;
  }

  private buildPduMmsInfoAction(actionData: LooseObject, resultSet: rdb.ResultSet) {
    actionData.pduId = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.pdu.id);
    actionData.subId = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.pdu.subId);
    actionData.threadId = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.pdu.threadId);
    actionData.msgBox = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.pdu.msgBox);
    actionData.startTime = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.pdu.date);
    actionData.endTime = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.pdu.dateSent);
    actionData.isRead = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.pdu.read);
    actionData.subCs = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.pdu.subCs);
    actionData.sub = newDbHelper.getColumnStringValue(resultSet, OldDBData.pdu.sub);
    let arrPhoneNumber = THREAD_PHONE_MAP.get(actionData.threadId);
    actionData.msgState = 0;
    actionData.hasDraft = 0;
    if (actionData.msgBox === 5) {
      actionData.msgState = 2;
    } else if (actionData.msgBox === 3) {
      actionData.msgState = 3;
      actionData.hasDraft = 1;
    } else if (actionData.msgBox === 4) {
      actionData.msgState = 1;
    }
    actionData.senderNumber = '';
    actionData.receiverNumber = '';
    let isSender: boolean = actionData.msgBox == 1 ? true : false;
    actionData.receiverNumber = isSender ? '' : (arrPhoneNumber === undefined ? '' : arrPhoneNumber[0]);
    actionData.senderNumber = isSender ? (arrPhoneNumber === undefined ? '' : arrPhoneNumber.join()) : '';
    if (actionData.subCs == 106) { // 双106需要转码，0为utf8不需要转码
      actionData.sub = StringUtil.codeConversion(actionData.sub);
    }
    return arrPhoneNumber;
  }

  private async getMsgContent(rdbStore: rdb.RdbStore, pduId: number): Promise<string> {
    let msgContent = '';
    try {
      let partInfo = new rdb.RdbPredicates(DbConstants.OLD_PART_NAME);
      partInfo.equalTo('mid', pduId);
      partInfo.equalTo('ct', 'text/plain');
      let canResultSet = await rdbStore.query(partInfo, ['text']);
      if (canResultSet != undefined && canResultSet.goToFirstRow()) {
        msgContent = canResultSet.getString(canResultSet.getColumnIndex('text'));
      }
      canResultSet.close();
    } catch (e) {
      LogUtils.w(TAG, 'query file error : ' + JSON.stringify(e));
    }

    return msgContent;
  }

  private getUpdateSessionValue(updateSessionItem: UpdateSessionItem): ValuesBucket {
    let sessionId = updateSessionItem.getSessionId();
    let curMsgCount = SESSION_MESSAGE_COUNT_MAP.get(sessionId);
    let messageCount = curMsgCount === undefined ? 0 : curMsgCount;
    messageCount ++;
    SESSION_MESSAGE_COUNT_MAP.set(sessionId, messageCount);
    let unreadCount = 0;
    let beforeTime = SESSION_TIME_MAP.get(sessionId);
    let beforeContent = SESSION_CONTENT_MAP.get(sessionId);
    let beforeStatus = SESSION_STATUS_MAP.get(sessionId);
    let beforeHasMms = SESSION_ID_HAS_MMS_MAP.get(sessionId);
    let resultTime = updateSessionItem.getStartTime() * 1000;
    let resultContent = updateSessionItem.getMsgTitle();
    let resultStatus = updateSessionItem.getMsgState();
    let resultHasMms = 1;
    if (beforeTime) {
      if (beforeTime > resultTime) {
        resultTime = beforeTime;
        resultContent = beforeContent === undefined ? '' : beforeContent;
        resultStatus = beforeStatus === undefined ? -1 : beforeStatus;
        resultHasMms = beforeHasMms === undefined ? resultHasMms : beforeHasMms;
      } else {
        this.setMapValue(sessionId, resultTime, resultContent, resultStatus);
        SESSION_ID_HAS_MMS_MAP.set(sessionId, resultHasMms);
      }
    } else {
      this.setMapValue(sessionId, resultTime, resultContent, resultStatus);
      SESSION_ID_HAS_MMS_MAP.set(sessionId, resultHasMms);
    }

    let upValues: ValuesBucket = {
      'has_mms': resultHasMms, 'time': resultTime,
      'content': resultContent === undefined ? '' : resultContent, // 修改content字段无数据问题
      'message_count': messageCount, 'sms_type': updateSessionItem.getSmsType(),
      'sending_status': resultStatus, 'unread_count': unreadCount
    }

    // 修改彩信草稿丢失，列表显示(无主题)，应该显示“[草稿][附件]（无主题）问题
    if (updateSessionItem.getHasDraft() == 1) {
      upValues = {
        'has_mms': resultHasMms, 'time': resultTime,
        'content': resultContent === undefined ? '' : resultContent,
        'message_count': messageCount,
        'sms_type': updateSessionItem.getSmsType(),
        'sending_status': resultStatus, 'unread_count': unreadCount,
        'has_draft': updateSessionItem.getHasDraft()
      }
    }
    return upValues;
  }

  private setMapValue(sessionId: number, resultTime: number, resultContent: string, resultStatus: number) {
    SESSION_TIME_MAP.set(sessionId, resultTime);
    SESSION_CONTENT_MAP.set(sessionId, resultContent);
    SESSION_STATUS_MAP.set(sessionId, resultStatus);
  }

  private async queryPartInfo(context: common.Context, index: number, partIdFails: number[],
      progress: BackupRestoreProgress): Promise<number> {
    LogUtils.e(TAG, `queryPartInfo start`)
    let failCount = 0;
    let successCount = 0;
    let partInfo = new rdb.RdbPredicates(DbConstants.OLD_PART_NAME);
    let rdbStore = await this.initDB(context);
    if (rdbStore === undefined) {
      LogUtils.e(TAG, `[queryPartInfo] failed : rdbStore === undefined`)
      return successCount;
    }
    try {
      if (index != -1) {
        partInfo = this.buildPredicatesInfo(partInfo, OldDBData.part.id, index);
      } else {
        partInfo = this.buildPredicatesInfoFail(partInfo, OldDBData.part.id, partIdFails);
      }
      let resultSet = await rdbStore.query(partInfo, this.queryPartColumn);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'no raw part found.');
        return successCount;
      }
      let count = resultSet.rowCount;
      let batchPartValues: ValuesBucket[] = [];
      do {
        await this.buildBatchInsertPart(resultSet, context, batchPartValues);
      } while (resultSet.goToNextRow());
      resultSet.close();
      if (batchPartValues.length > 0) {
        let insertNum = await newDbHelper.batchInsertPart(context, batchPartValues);
        if (insertNum == -1) {
          failCount += batchPartValues.length;
        } else {
          successCount += insertNum;
        }
      }
      this.doPartFailed = this.doPartFailed + failCount;
      progress.updateProcessCount(progress.getProcessCount() + count);
      LogUtils.i(TAG, 'part Total Migrations count: ' + count + ',Successful Migrations successCount: ' + successCount +
        'Failed Migrations failCount: ' + failCount);
    } catch (error) {
      LogUtils.w(TAG, 'query partInfo error : ' + JSON.stringify(error));
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `OHOS2NEXT part code:${error.code} msg:${error.message}`);
    }
    return successCount;
  }

  private async buildBatchInsertPart(resultSet: rdb.ResultSet, context: Context, batchPartValues: ValuesBucket[]) {
    let mid = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.part.mid);
    if (mid === undefined) {
      LogUtils.e(TAG, 'key field part mid is invalid');
      return;
    }
    let msgId = RAW_PDU_ID_MAP.get(mid);
    LogUtils.e(TAG, `part get ${mid} -> new msg_id is ${msgId}`);
    if (msgId !== undefined && msgId?.length > 0) {
      for (let index = 0; index < msgId.length; index++) {
        const item = msgId[index];
        let values = await this.getInsertValue(context, item, resultSet);
        batchPartValues.push(values);
      }
    }
  }

  private async getInsertValue(context: common.Context, msgId: number, resultSet: rdb.ResultSet):
    Promise<ValuesBucket> {
    let ct = newDbHelper.getColumnStringValue(resultSet, OldDBData.part.ct);
    let partContent = newDbHelper.getColumnStringValueWithDefault(resultSet, OldDBData.part.text, '');
    let text = newDbHelper.getColumnStringValueWithDefault(resultSet, OldDBData.part.text, '');
    let data = newDbHelper.getColumnStringValue(resultSet, OldDBData.part.data);
    let mid = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.part.mid);
    let partId = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.part.id);
    let prefix = this.version.startsWith(BackupConstants.ARD2NEXT_CLONE_VERSION) ?
      mid?.toString() + '_' + partId?.toString() + '_' : '';
    let fileName = prefix + DataBaseUtil.getPartFileName(resultSet);
    let location = '/data/storage/el2/base/haps/entry/files/';
    let hasAttachment = 0;
    ct = ct === undefined ? '' : ct;
    if (ct.toLowerCase() === 'text/x-vCard'.toLowerCase()) {
      location = location + StringUtil.getPathTail(data);
    } else if (ct != 'application/smil') {
      location = location + fileName;
    } else {
      let mid = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.part.mid);
      let id = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.part.id);
      location = location + 'smil_' + mid?.toString() + '_' + id?.toString() + '.xml';
    }
    // 获取该msgId对应的recording_time
    this.getAudioTime(text, msgId);
    let type: number = -1;
    if (this.version.startsWith(BackupConstants.IOS2NEXT_CLOUD_CLONE_VERSION)) {
      let data = newDbHelper.getColumnStringValueWithDefault(resultSet, OldDBData.part.data, '');
      location = '/data/storage/el2/base/haps/entry/files/' + data;
    }
    if (ct === 'application/smil') {
      type = 0;
    } else if (StringUtil.getType(ct) === 'image') {
      hasAttachment = 1;
      type = 1;
    } else if (StringUtil.getType(ct) === 'video') {
      hasAttachment = 1;
      type = 2;
    } else if (StringUtil.getType(ct) === 'audio') {
      hasAttachment = 1;
      type = 3;
    } else if (ct.toLowerCase() === 'text/x-vCard'.toLowerCase()) {
      hasAttachment = 1;
      type = 4;
    } else if (ct === 'text/plain') {
      type = 7;
      if (msgId) {
        await newDbHelper.updateSms(context, msgId, {'msg_content': partContent});
      } else {
        LogUtils.i(TAG, '[getInsertValue] msg id is not define , can not update sms info!');
      }
    }

    // Update the value of the has_attachment field in the session table.
    this.updateSessionHasAttachment(context, msgId, hasAttachment);
    let groupId = SMS_ID_GROUP_ID_MAP.get(msgId);
    let values: ValuesBucket = new MmsPartItem()
      .setMsgId(msgId)
      .setType(type)
      .setCt(ct)
      .setGroupId(groupId)
      .setContent(partContent)
      .setLocationPath(location)
      .createValuesBucket();
    return values;
  }

  private async updateSessionHasAttachment(context: common.Context, msgId: number, hasAttachment: number) {
    let sessionId = SMS_ID_SESSION_ID_MAP.get(msgId);
    if (sessionId === undefined) {
      LogUtils.e(TAG, 'invalid sms info msg, no matched session id');
      return;
    }
    let sessionTime = SESSION_TIME_MAP.get(sessionId);
    if (sessionTime === undefined) {
      LogUtils.e(TAG, 'invalid sms info msg, no matched session time');
      return;
    }
    if (MMS_ID_TIME_MAP.get(msgId) < sessionTime) {
      return;
    }

    if (!SESSION_ID_SMS_ID_MAP.hasKey(sessionId)) {
      // 该session表的has_attachment未被更新过，直接更新
      let upValues: ValuesBucket = {
        'has_attachment': hasAttachment
      }
      await newDbHelper.updateSession(context, sessionId, upValues);
      SESSION_ID_SMS_ID_MAP.set(sessionId, msgId);
      return;
    }

    if (SESSION_ID_SMS_ID_MAP.get(sessionId) == msgId) {
      // 该session表的has_attachment已被更新，此时的msgId为同一个，只更新有附件的情况
      if (hasAttachment == 1) {
        let upValues: ValuesBucket = {
          'has_attachment': hasAttachment
        }
        await newDbHelper.updateSession(context, sessionId, upValues);
      }
    } else if (SESSION_ID_SMS_ID_MAP.get(sessionId) < msgId) {
      // 该session表的has_attachment已被更新，此时的msgId是比该session之前的msgId大，以最新的为主
      let upValues: ValuesBucket = { 'has_attachment': hasAttachment }
      await newDbHelper.updateSession(context, sessionId, upValues);
      SESSION_ID_SMS_ID_MAP.set(sessionId, msgId);
    } else {
      LogUtils.i(TAG, 'Last msgId less than current msgId, do nothing.');
    }
  }

  private async queryThreadInfo(context: common.Context, index: number, threadsIdFails: number[],
    sessionMap: HashMap<string, LooseObject>, progress: BackupRestoreProgress): Promise<number> {
    LogUtils.e(TAG, `[queryThreadInfo] start`)
    let threadActionData: LooseObject = {};
    threadActionData.failCount = 0;
    threadActionData.successCount = 0;
    let threadInfo = new rdb.RdbPredicates(DbConstants.OLD_THREADS_NAME);
    let rdbStore = await this.initDB(context);
    if (rdbStore === undefined) {
      LogUtils.e(TAG, `[queryThreadInfo] failed : rdbStore === undefined`);
      return threadActionData.successCount;
    }
    try {
      let queryThreadColumn = [OldDBData.threads.threadId, OldDBData.threads.address, OldDBData.threads.recipientIds];
      if (this.hasPinning) {
        queryThreadColumn.push(OldDBData.threads.pinningTime);
      }
      if (index != -1) {
        threadInfo = this.buildPredicatesInfo(threadInfo, OldDBData.threads.threadId, index);
      } else {
        threadInfo = this.buildPredicatesInfoFail(threadInfo, OldDBData.threads.threadId, threadsIdFails);
      }
      let resultSet = await rdbStore.query(threadInfo, queryThreadColumn);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'no raw thread found.');
        return threadActionData.successCount;
      }
      let count = resultSet.rowCount;
      LogUtils.i(TAG, `thread count = ${count}`);
      do {
        if (this.hasPinning) {
          threadActionData.pinningTime = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.threads.pinningTime);
        }
        await this.copyThreadInfo(threadActionData, resultSet, rdbStore, sessionMap, context);
      } while (resultSet.goToNextRow());
      resultSet.close();
      this.doSessionFailed = this.doSessionFailed + threadActionData.failCount;
      progress.updateProcessCount(progress.getProcessCount() + count);
      LogUtils.i(TAG,
        'thread Total Migrations count: ' + count + ',Successful Migrations successCount: ' + threadActionData.successCount +
          'Failed Migrations failCount: ' + threadActionData.failCount);
    } catch (error) {
      LogUtils.w(TAG, 'query threadInfo error : ' + JSON.stringify(error));
    }
    return threadActionData.successCount;
  }

  private async copyThreadInfo(threadActionData: LooseObject, resultSet: rdb.ResultSet, rdbStore: rdb.RdbStore,
    sessionMap: HashMap<string, LooseObject>, context: Context) {
    threadActionData.sessionId = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.threads.threadId);
    threadActionData.address = newDbHelper.getColumnStringValue(resultSet, OldDBData.threads.address);
    if (threadActionData.sessionId === undefined || threadActionData.address === undefined) {
      LogUtils.e(TAG, 'query thread info, Invalid key field');
      return;
    }
    let arrPhoneNumber = this.getArrPhoneNumber(resultSet, rdbStore);
    THREAD_PHONE_MAP.set(threadActionData.sessionId, arrPhoneNumber);
    threadActionData.telephone = arrPhoneNumber.join();
    threadActionData.contactsNum = arrPhoneNumber.length;
    let session = sessionMap.get(threadActionData.telephone);
    if (session) {
      LogUtils.e(TAG, 'Duplicate session with the same phone');
      RAW_THREAD_ID_MAP.set(threadActionData.sessionId, session.id);
      SESSION_ID_HAS_MMS_MAP.set(session.id, session.hasMms);
    } else {
      let flag: boolean = true;
      if (threadActionData.contactsNum === 1 && threadActionData.telephone.length > 7) {
        for (let k of sessionMap.keys()) {
          if (k && k.indexOf(',') === -1 && k.length > 7) {
            const phone1 = k.slice(-7);
            const phone2 = (threadActionData.telephone as string).slice(-7);
            if (phone1 === phone2) {
              session = sessionMap.get(k);
              if (session) {
                RAW_THREAD_ID_MAP.set(threadActionData.sessionId, session.id);
                SESSION_ID_HAS_MMS_MAP.set(session.id, session.hasMms);
                LogUtils.e(TAG, 'Duplicate session, The last seven digits are the same');
                flag = false;
              }
              break;
            }
          }
        }
      }
      if (flag) {
        let values: ValuesBucket = new SessionItem().setTelephone(threadActionData.telephone)
          .setContactsNum(threadActionData.contactsNum)
          .setPinningTime(threadActionData.pinningTime)
          .createValuesBucket();
        if (StringUtil.isSip(threadActionData.telephone)) {
          LogUtils.w(TAG, 'copyThreadInfo skip insert session item, telephone is sip');
          values.sms_type = 1;
        }
        let newRawSessionId = await newDbHelper.insertSession(context, values);
        if (newRawSessionId == -1) {
          threadActionData.failCount++;
        } else {
          threadActionData.successCount++;
          RAW_THREAD_ID_MAP.set(threadActionData.sessionId, newRawSessionId);
          SESSION_ID_HAS_MMS_MAP.set(newRawSessionId, 0);
        }
      }
    }
  }

  private getArrPhoneNumber(resultSet: rdb.ResultSet, rdbStore: rdb.RdbStore) {
    let recipientIds = resultSet.getString(resultSet.getColumnIndex(OldDBData.threads.recipientIds));
    let arrRecipient = recipientIds.split(' ');
    let arrPhoneNumber: string[] = [];
    for (let i = 0; i < arrRecipient.length; i++) {
      let element = Number(arrRecipient[i]);
      if (!Number.isNaN(element)) {
        let phoneNumber = ADDRESS_ID_MAP.get(element);
        arrPhoneNumber.push(phoneNumber);
      }
    }
    return arrPhoneNumber;
  }

  private buildPredicatesInfo(predicatesInfo: rdb.RdbPredicates, id: string, index: number): rdb.RdbPredicates {
    predicatesInfo.orderByAsc(id)
      .limitAs(DbConstants.QUERY_PAGE_NUM)
      .offsetAs(index);
    return predicatesInfo;
  }

  private buildPredicatesInfoFail(predicatesInfo: rdb.RdbPredicates,
    id: string, chatIdFails: number[]): rdb.RdbPredicates {
    predicatesInfo.orderByAsc(id)
      .in(id, chatIdFails);
    return predicatesInfo;
  }

  private getAudioTime(text: string, msgId: number): string {
    let str = text.toLowerCase();
    if (str.indexOf('audio') == -1) {
      return '';
    }
    let recordTime = str.substring(str.indexOf('dur="') + 5, str.indexOf('"', str.indexOf('dur="') + 5))
      .replace('ms', '');
    LogUtils.i(TAG, 'audioSmileDur is:' + str.substring(str.indexOf('dur="') + 5, str.indexOf('"', str.indexOf('dur="') + 5)))
    if (recordTime.indexOf('s') >= 0) {
      recordTime = String(Number(recordTime.replace('s', '')) * 1000)
    }

    let recordingTime = this.formatMilliseconds(recordTime);
    MMS_ID_RECORDING_TIME_MAP.set(msgId, recordingTime);
    return recordingTime;
  }

  private formatMilliseconds(value: string): string {
    // 对比XXX，跟XXX数据保持一致，向下取整
    let second = Math.floor((Number(value) / 1000)); // second
    if (Number.isNaN(second)) {
      second = 0;
      LogUtils.w(TAG, 'second is set to 0, raw value: ' + value);
    }
    let minute = 0; // minute
    if (second >= 60) {
      minute = Number((second / 60).toFixed(0));
      second = second % 60;
    }
    let timeStr: string = ((minute >= 10) ? minute : ('0' + minute)) + ':' +
      ((second >= 10) ? second : ('0' + second));
    return timeStr;
  }

  private async queryRcsDataCount(context: common.Context, tableName: string, queryKey: string): Promise<number> {
    LogUtils.w(TAG, `queryRcsDataCount start! `)
    if (context == null) {
      LogUtils.w(TAG, `queryRcsDataCount failed by null context`)
      return Promise.resolve(0);
    }
    let count = 0;
    try {
      let predicatesInfo = new rdb.RdbPredicates(tableName);
      let rdbStore = await this.initRcsDB(context);
      if (rdbStore === undefined) {
        LogUtils.e(TAG, `[queryRcsDataCount] failed : rdbStore === undefined`)
        return Promise.resolve(0);
      }
      let queryColumn = [`count(${queryKey})`];
      let resultSet = await rdbStore.query(predicatesInfo, queryColumn);
      if (resultSet === undefined) {
        LogUtils.w(TAG, 'resultSet === undefined');
        return Promise.resolve(0);
      }
      if (!resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'query no threads found.');
        return Promise.resolve(0);
      }
      count = resultSet.getLong(0);
      LogUtils.i(TAG, '[queryRcsDataCount] count : ' + count);
      resultSet.close();
    } catch (error) {
      LogUtils.e(TAG, `[queryRcsDataCount] error : ${error.code}`)
    }
    return Promise.resolve(count);
  }

  private async initRcsDB(context: common.Context): Promise<rdb.RdbStore | undefined> {
    LogUtils.w(TAG, `initRcsDB start!context.databaseDir + `)
    if (!context) {
      LogUtils.w(TAG, '[createRcsDB] context is empty');
      return undefined;
    }
    if (this.rdbRcsStoreHelper) {
      LogUtils.i(TAG, '[createRcsDB] return');
      return Promise.resolve(this.rdbRcsStoreHelper);
    }
    try {
      let OLD_CONFIG: rdb.StoreConfig = {
        name: 'chatSms.db',
        securityLevel: rdb.SecurityLevel.S1
      };
      LogUtils.i(TAG, '[createRcsDB] OLD_CONFIG');
      this.rdbRcsStoreHelper = await rdb.getRdbStore(context, OLD_CONFIG);
    } catch (error) {
      LogUtils.w(TAG, '[createRcsDB] failed : ' + JSON.stringify(error));
    }
    if (this.rdbRcsStoreHelper) {
      return Promise.resolve(this.rdbRcsStoreHelper);
    }
    return undefined;
  }

  private async doRcsChatTableCopy(context: common.Context, progress: BackupRestoreProgress) {
    LogUtils.i(TAG, 'doRcsChatTableCopy start!');
    let idFailCount = 0;
    let idSuccessCount = 0;
    let threadsCount = await this.queryRcsDataCount(context, DbConstants.OLD_RCS_CHAT_TABLE_NAME, '_id');
    let threadsPage = Math.ceil(threadsCount / DbConstants.QUERY_PAGE_NUM);
    this.queryRcsColumn = await this.queryColumns(context, DbConstants.OLD_RCS_CHAT_TABLE_NAME);
    let threadIdFails = this.getFromPreferences(context, 'chat_id_fail', []) as number[];
    let rcsInfoMap = await newDbHelper.queryRcs(context);
    sessionIdMap = await newDbHelper.querySessionIdMap(context);
    if (threadIdFails.length > 0) {
      idFailCount += await this.queryRcsChatInfo(context, -1, threadIdFails, rcsInfoMap, progress);
      threadIdFails = this.getFromPreferences(context, 'chat_id_fail', []) as number[];
    }
    let threadIdSuccess = this.getFromPreferences(context, 'chat_id_success', 0) as number;
    for (let curPage = 1; curPage <= threadsPage; curPage++) {
      idSuccessCount += await this.queryRcsChatInfo(context, DbConstants.QUERY_PAGE_NUM * (curPage - 1) +
        threadIdSuccess, threadIdFails, rcsInfoMap, progress);
    }
    this.doRcsChatSuccess = idFailCount + idSuccessCount;
    progress.updateProcessCount(progress.getRcsChatCountCloneProgress());
  }

  private async queryRcsChatInfo(context: common.Context, index: number, threadsIdFails: number[],
      rcsInfoMap: HashMap<string, number>, progress: BackupRestoreProgress): Promise<number> {
    LogUtils.e(TAG, `[queryRcsChatInfo] start!`)
    let failCount = 0;
    let successCount = 0;
    let predicatesInfo = new rdb.RdbPredicates(DbConstants.OLD_RCS_CHAT_TABLE_NAME);
    let rdbStore = await this.initRcsDB(context);
    if (rdbStore === undefined) {
      LogUtils.e(TAG, `[queryChatInfo] failed : rdbStore === undefined`)
      return 0;
    }
    try {
      predicatesInfo = this.buildPredicatesInfo(predicatesInfo, OldDBData.chat.id, index);
      let resultSet = await rdbStore.query(predicatesInfo, this.queryRcsColumn);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'no raw Rcs found.');
        return 0;
      }
      let count = resultSet.rowCount;
      do {
        do {
          GROUP_ID = GROUP_ID + 1;
        } while (GROUP_ID_MAP.has(GROUP_ID));
        GROUP_ID_MAP.set(GROUP_ID, GROUP_ID);
        let chatId = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.chat.id);
        let threadId = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.chat.threadId);
        let address = newDbHelper.getColumnStringValue(resultSet, OldDBData.chat.address);
        let person = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.chat.person);
        let date = newDbHelper.getColumnIntegerValueWithDefault(resultSet, OldDBData.chat.date, 0);
        let dateSent = newDbHelper.getColumnIntegerValueWithDefault(resultSet, OldDBData.chat.dateSent, 0);
        let protocol = newDbHelper.getColumnIntegerValueWithDefault(resultSet, OldDBData.chat.protocol, -1);
        let read = newDbHelper.getColumnIntegerValueWithDefault(resultSet, OldDBData.chat.read, 1);
        let status = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.chat.status);
        let type = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.chat.type);
        let replyPathPresent = newDbHelper.getColumnIntegerValueWithDefault(resultSet,
          OldDBData.chat.replyPathPresent, -1);
        let subject = newDbHelper.getColumnStringValueWithDefault(resultSet, OldDBData.chat.subject, '');
        let body = newDbHelper.getColumnStringValueWithDefault(resultSet, OldDBData.chat.body, '');
        let serviceCenter = newDbHelper.getColumnStringValueWithDefault(resultSet, OldDBData.chat.serviceCenter, '');
        let locked = newDbHelper.getColumnIntegerValueWithDefault(resultSet, OldDBData.chat.locked, 0);
        let subId = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.chat.subId);
        let networkType = newDbHelper.getColumnIntegerValueWithDefault(resultSet, OldDBData.chat.networkType, 0);
        let errorCode = newDbHelper.getColumnIntegerValueWithDefault(resultSet, OldDBData.chat.errorCode, 0);
        let seen = newDbHelper.getColumnIntegerValueWithDefault(resultSet, OldDBData.chat.seen, -1);
        let privacyMode = newDbHelper.getColumnIntegerValueWithDefault(resultSet, OldDBData.chat.privacyMode, 0);
        let serviceKind = newDbHelper.getColumnStringValueWithDefault(resultSet, OldDBData.chat.serviceKind, '');
        let fileType = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.chat.fileType);
        let sdkSmsId = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.chat.sdkSmsId);
        let enrichedCallingType = newDbHelper.getColumnIntegerValueWithDefault(resultSet,
          OldDBData.chat.enrichedCallingType, 0);
        let ownerAddr = newDbHelper.getColumnStringValueWithDefault(resultSet, OldDBData.chat.ownerAddr, '');
        let globalGroupId = newDbHelper.getColumnStringValue(resultSet, OldDBData.chat.globalGroupId);
        if (read == 0) {
          LogUtils.e(TAG, 'thread ' + threadId + ' have no read rcs chat ');
        }
        if (chatId === undefined || address === undefined || threadId === undefined ||
          subId === undefined || type === undefined || fileType === undefined || sdkSmsId === undefined) {
          LogUtils.e(TAG, 'Invalid key field');
          continue;
        }
        let isCollect = 0;
        let msgState: number = this.rcsMsgStateBuilder(type);
        let senderNumber = '';
        let receiverNumber = '';
        if (type === 1) {
          senderNumber = address;
        } else {
          receiverNumber = address;
        }
        let key: string = receiverNumber + ':' + senderNumber + ':' + date;
        if (rcsInfoMap.hasKey(key)) {
          successCount++;
          LogUtils.i(TAG, 'rcs repeats');
          continue;
        }
        if (StringUtil.isSip(address)) {
          LogUtils.e(TAG, 'skip insert rcs item, telephone is sip');
          if (serviceCenter === 'rcs.im' && type === 1) {
            LogUtils.e(TAG, 'queryChatInfo need insert');
          } else {
            LogUtils.e(TAG, 'queryChatInfo no need insert');
            successCount++;
            continue;
          }
        }
        let sessionId = 0;
        // The RCS information exists in historical sessions.
        // You need to obtain sessionId from existing sessions.
        let curSessionId = RAW_THREAD_ID_MAP.get(threadId)
        if (curSessionId === undefined) {
          let sessionAddress = address.includes('+86') ? address.replace('+86', '') : address
          if (sessionIdMap.hasKey(sessionAddress)) {
            sessionId = sessionIdMap.get(sessionAddress);
          } else if (sessionIdMap.hasKey(address)) {
            sessionId = sessionIdMap.get(address);
          } else {
            LogUtils.i(TAG, 'rcs info not exist in historical sessions, need create session.');
            sessionId = await this.rcsSessionInsert(address, context);
            RAW_THREAD_ID_MAP.set(threadId, sessionId);
          }
        } else {
          LogUtils.i(TAG, 'rcs info exist in historical sessions.');
          sessionId = curSessionId;
        }
        let values: ValuesBucket = this.rcsInfoBuilder(subId, receiverNumber, senderNumber, date, dateSent, fileType,
          subject, body, msgState, locked, read, isCollect, sessionId, type, sdkSmsId, enrichedCallingType, errorCode,
          networkType, ownerAddr, privacyMode, protocol, replyPathPresent, seen, serviceCenter, serviceKind);
        if (values.rcs_type != 0 || values.session_id === -1) {
          LogUtils.i(TAG, 'rcs chart skip, rcs type: ' + values.rcs_type);
          continue;
        }
        let newRawRasInfoId = await newDbHelperClone.insertRcsInfo(context, values);
        let newRawSmsIdStatus = 0;
        newRawSmsIdStatus = await this.rcsSmsInsert(newRawRasInfoId, receiverNumber, senderNumber, subject, body,
          msgState, locked, sessionId, date, dateSent, subId, type, isCollect, context, newRawSmsIdStatus, read,
          fileType);
        if (newRawRasInfoId == -1 || newRawSmsIdStatus == -1) {
          failCount++;
        } else {
          successCount++;
        }
      } while (resultSet.goToNextRow());
      resultSet.close();
      this.doRcsChatFailed = this.doRcsChatFailed + failCount;
      progress.updateProcessCount(progress.getProcessCount() + count);
      LogUtils.i(TAG, 'rcsInfo Total Migrations count: ' + count + ',Successful Migrations successCount: ' + successCount + 'Failed Migrations failCount: ' + failCount);
    } catch (error) {
      LogUtils.w(TAG, 'query rcsInfo error : ' + JSON.stringify(error));
    }
    return successCount;
  }

  private rcsMsgStateBuilder(type: number) {
    let msgState: number = 0;
    if (type == 5) {
      msgState = 2;
    } else if (type == 4) {
      msgState = 1;
    }
    return msgState;
  }

  private rcsInfoBuilder(subId: number, receiverNumber: string, senderNumber: string, date: number, dateSent: number,
                         fileType: number, subject: string, body: string, msgState: number, locked: number,
                         read: number, isCollect: number, sessionId: number, type: number, sdkSmsId: number,
                         enrichedCallingType: number, errorCode: number, networkType: number, ownerAddr: string,
                         privacyMode: number, protocol: number, replyPathPresent: number, seen: number,
                         serviceCenter: string, serviceKind: string): ValuesBucket {
    return new RcsInfoItem()
      .setSlotId(subId)
      .setReceiverNumber(receiverNumber)
      .setSenderNumber(senderNumber)
      .setStartTime(date)
      .setEndTime(dateSent)
      .setRcsType(fileType)
      .setMsgTitle(subject)
      .setMsgContent(body)
      .setMsgState(msgState)
      .setIsLock(locked)
      .setIsRead(read)
      .setIsCollect(isCollect)
      .setSessionId(sessionId)
      .setGroupId(GROUP_ID)
      .setIsSender(type == 1 ? 1 : 0)
      .setMsgId(sdkSmsId + '' + date)
      .setEnrichedCallingType(enrichedCallingType)
      .setErrorCode(errorCode)
      .setNetworkType(networkType)
      .setOwnerAddr(ownerAddr)
      .setPrivacyMode(privacyMode)
      .setProtocol(protocol)
      .setReplyPathPresent(replyPathPresent)
      .setSeen(seen)
      .setServiceCenter(serviceCenter)
      .setServiceKind(serviceKind)
      .createValuesBucket();
  }

  private async rcsCollectCheck(chatId: number, serviceCenter: string, body: string, rdbStore: rdb.RdbStore) {
    let isCollect = 0;
    try {
      let favMmsPredicatesInfo = new rdb.RdbPredicates(tableName1 + tableName2 + 'rcsbackupfav_sms_tb');
      favMmsPredicatesInfo.equalTo('origin_id', chatId + '');
      favMmsPredicatesInfo.equalTo('service_center', serviceCenter);
      favMmsPredicatesInfo.equalTo('body', body);
      let favMmsResultSet = await rdbStore.query(favMmsPredicatesInfo, ['_id']);
      if (favMmsResultSet != undefined && favMmsResultSet.goToFirstRow()) {
        isCollect = 1;
      }
      favMmsResultSet.close();
    } catch (e) {
      LogUtils.w(TAG, 'query favMms error : ' + JSON.stringify(e));
    }
    return isCollect;
  }

  private async rcsSessionInsert(address: string, context: Context) {
    let valueSession = new SessionItem().setTelephone(address)
      .createValuesBucket();
    if (StringUtil.isSip(address)) {
      LogUtils.w(TAG, 'rcsSessionInsert skip insert session item, telephone is sip');
      valueSession.sms_type = 1;
    }
    return await newDbHelper.insertSession(context, valueSession);
  }

  private async rcsSmsInsert(newRawRasInfoId: number, receiverNumber: string, senderNumber: string, subject: string,
                             body: string, msgState: number, locked: number, sessionId: number, date: number,
                             dateSent: number, subId: number, type: number, isCollect: number, context: Context,
                             newRawSmsIdStatus: number, read: number, fileType: number) {
    if (newRawRasInfoId != -1) {
      let valuesMmsInfo: ValuesBucket = new SmsMmsInfoItem()
        .setReceiverNumber(receiverNumber)
        .setSenderNumber(senderNumber)
        .setMsgTitle(subject)
        .setMsgContent(body)
        .setMsgState(msgState)
        .setIsLock(locked)
        .setIsRead(1)
        .setSessionId(sessionId)
        .setGroupId(GROUP_ID)
        .setStartTime(date)
        .setEndTime(dateSent)
        .setSlotId(subId)
        .setMsgType(0)
        .setIsSender(type == 1 ? 1 : 0)
        .setIsCollect(isCollect)
        .setRcsId(newRawRasInfoId)
        .setSmsType(99)
        .createValuesBucket();
      let newRawSmsId = await newDbHelperClone.insertSms(context, valuesMmsInfo);
      newRawSmsIdStatus = newRawSmsId;
      let sessionTime = await newDbHelperClone.getSessionTime(sessionId.toString(), context);
      let messageCount = SESSION_MESSAGE_COUNT_MAP.get(sessionId) || 0;
      messageCount ++;
      SESSION_MESSAGE_COUNT_MAP.set(sessionId, messageCount);
      await this.rcsSessionUpDate(sessionTime, date, 0, body, context, sessionId, msgState, fileType, messageCount);
    }
    return newRawSmsIdStatus;
  }

  private async rcsSessionUpDate(sessionTime: number, date: number, unreadCount: number,
    body: string, context: Context, sessionId: number, msgState: number, fileType: number, msgCount: number) {
    if (sessionTime <= date) {
      let hasAttachment: number = 0;
      // 0 Subject、1 Pictures、2 Video、3 Audio、4 Text、5 business card
      if (fileType == 1 || fileType == 2 || fileType == 3 || fileType == 5) {
        hasAttachment = 1;
      }
      let upSessionValues: ValuesBucket = {
        'content': body,
        'time': date,
        'unread_count': unreadCount,
        'sending_status': msgState,
        'has_attachment': hasAttachment,
        'message_count': msgCount
      };
      await newDbHelperClone.updateSession(context, sessionId, upSessionValues);
    }
  }

  private async doRcsGroupThreadsCopy(context: common.Context, progress: BackupRestoreProgress) {
    LogUtils.i(TAG, 'doRcsGroupThreadsCopy start!');
    let idFailCount = 0;
    let idSuccessCount = 0;
    let threadsCount = await this.queryRcsDataCount(context, DbConstants.OLD_RCS_GROUP_TABLE_NAME, '_id');
    let threadsPage = Math.ceil(threadsCount / DbConstants.QUERY_PAGE_NUM);
    this.queryRcsGroupColumn = await this.queryColumns(context, DbConstants.OLD_RCS_GROUP_TABLE_NAME);
    let threadIdFails = this.getFromPreferences(context, 'group_id_fail', []) as number[];
    let sessionMap = await newDbHelper.querySession(context);
    if (threadIdFails.length > 0) {
      idFailCount += await this.queryRcsGroupInfo(context, -1, threadIdFails, sessionMap, progress);
      threadIdFails = this.getFromPreferences(context, 'group_id_fail', []) as number[];
    }
    let threadIdSuccess = this.getFromPreferences(context, 'group_id_fail', 0) as number;
    for (let curPage = 1; curPage <= threadsPage; curPage++) {
      idSuccessCount += await this.queryRcsGroupInfo(context, DbConstants.QUERY_PAGE_NUM * (curPage - 1) +
        threadIdSuccess, threadIdFails, sessionMap, progress);
    }
    this.doRcsChatSuccess = idFailCount + idSuccessCount;
    progress.updateProcessCount(progress.getRcsGroupCountCloneProgress());
  }

  private async queryRcsGroupInfo(context: common.Context, index: number, threadsIdFails: number[],
      sessionMap: HashMap<string, LooseObject>, progress: BackupRestoreProgress): Promise<number> {
    LogUtils.i(TAG, 'queryRcsGroupInfo start!');
    let failCount = 0;
    let successCount = 0;
    let predicatesInfo = new rdb.RdbPredicates(DbConstants.OLD_RCS_GROUP_TABLE_NAME);
    let rdbStore = await this.initRcsDB(context);
    if (rdbStore === undefined) {
      LogUtils.e(TAG, `[queryRcsGroupInfo] failed : rdbStore === undefined`)
      return 0;
    }
    try {
      predicatesInfo = this.buildPredicatesInfo(predicatesInfo, OldDBData.rcsGroup.id, index);
      let resultSet = await rdbStore.query(predicatesInfo, this.queryRcsGroupColumn);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'no raw Rcs found.');
        return 0;
      }
      let count = resultSet.rowCount;
      do {
        do {
          GROUP_ID = GROUP_ID + 1;
        } while (GROUP_ID_MAP.has(GROUP_ID));
        GROUP_ID_MAP.set(GROUP_ID, GROUP_ID);
        let chatId = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.rcsGroup.id);
        let threadId = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.rcsGroup.threadId);
        let type = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.rcsGroup.type);
        let address = newDbHelper.getColumnStringValue(resultSet, OldDBData.rcsGroup.address);
        let date = newDbHelper.getColumnIntegerValueWithDefault(resultSet, OldDBData.rcsGroup.date, 0);
        let read = newDbHelper.getColumnIntegerValueWithDefault(resultSet, OldDBData.rcsGroup.read, 1);
        let status = newDbHelper.getColumnIntegerValueWithDefault(resultSet, OldDBData.rcsGroup.status, 0);
        let fileMode = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.rcsGroup.fileMode);
        let globalDate = newDbHelper.getColumnStringValue(resultSet, OldDBData.rcsGroup.globalDate);
        let body = newDbHelper.getColumnStringValueWithDefault(resultSet, OldDBData.rcsGroup.body, '');
        let globalId = newDbHelper.getColumnStringValue(resultSet, OldDBData.rcsGroup.global_id);
        let operateCode = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.rcsGroup.operateCode);
        let errorCode = newDbHelper.getColumnIntegerValueWithDefault(resultSet, OldDBData.rcsGroup.errorCode, 0);
        let metaData = newDbHelper.getColumnStringValue(resultSet, OldDBData.rcsGroup.metaData);
        let seen = newDbHelper.getColumnIntegerValueWithDefault(resultSet, OldDBData.rcsGroup.seen, -1);
        let serviceKind = newDbHelper.getColumnStringValueWithDefault(resultSet, OldDBData.rcsGroup.serviceKind, '');
        let fileType = newDbHelper.getColumnIntegerValue(resultSet, OldDBData.rcsGroup.fileType);
        let sdkRcsGroupMessageId = newDbHelper.getColumnIntegerValue(resultSet,
          OldDBData.rcsGroup.sdkRcsGroupMessageId);
        let mentionList = newDbHelper.getColumnStringValue(resultSet, OldDBData.rcsGroup.mentionList);
        if (read == 0) {
          LogUtils.e(TAG, 'thread ' + threadId + ' have no read rcs group msg ');
        }
        if (chatId === undefined || type === undefined || fileType === undefined || globalId === undefined) {
          LogUtils.e(TAG, 'Invalid key field');
          continue;
        }
        let arrPhoneNumber: string[] = [];
        await this.getGroupPhone(threadId + '', rdbStore, arrPhoneNumber);
        arrPhoneNumber.sort();
        let telephone = arrPhoneNumber.join(',');
        let contactsNum = arrPhoneNumber.length;
        let sessions = sessionMap.get(telephone);
        let sessionId = 0;
        let sessionHasDraft = 0;
        if (!sessions) {
          sessionHasDraft = 0;
          let valueSession = new SessionItem().setTelephone(telephone)
            .setContactsNum(contactsNum)
            .setContent(body)
            .createValuesBucket();
          sessionId = await newDbHelper.insertSession(context, valueSession);
          let value: ValuesBucket = this.rcsGroupValueBuilder(date, telephone, contactsNum, sessionId, sessionMap);
          sessions = value;
        } else {
          sessionId = sessions.id;
          sessionHasDraft = sessions.has_draft
        }

        let msgState: number = 0;
        msgState = this.msgStateBuilder(status, msgState);
        let isCollect = 0;
        let hasDraft = 0;
        if (type === 112) {
          // 112 is draft msg type
          msgState = 3;
          hasDraft = 1;
          let jsonObject: Record<string, string> = JSON.parse(body);
          let bodyDraft = jsonObject['edittext'];
          let valuesMmsInfoDraft: ValuesBucket = new SmsMmsInfoItem().setMsgTitle(bodyDraft)
            .setMsgContent(bodyDraft).setIsRead(1).setReceiverNumber(telephone).setSessionId(sessionId)
            .setStartTime(date).setMsgType(0).setIsSender(0).setIsCollect(0).setRcsId(0)
            .setMsgState(msgState).setSmsType(0).setIsCollect(isCollect).setGroupId(GROUP_ID).createValuesBucket();
          let newRawSmsId = await newDbHelperClone.insertSms(context, valuesMmsInfoDraft);
          let upValuesDraft: ValuesBucket = {
            'has_draft': hasDraft,
            'content': bodyDraft,
            'time': date,
          }
          LogUtils.i(TAG, 'rcs group skip, update draft session');
          await newDbHelperClone.updateSession(context, sessionId, upValuesDraft);
          continue;
        }
        if (fileType !== 0 || type === 112) {
          LogUtils.i(TAG, 'rcs group skip, file type: ' + fileType + ' type: ' + type);
          continue;
        }
        let messageCount: number = 0;
        if (arrPhoneNumber.length > 0) {
          messageCount = SESSION_MESSAGE_COUNT_MAP.get(sessionId) || 0;
          messageCount ++;
          SESSION_MESSAGE_COUNT_MAP.set(sessionId, messageCount);
        }
        for (let i = 0; i < arrPhoneNumber.length; i++) {
          const key = arrPhoneNumber[i] + ':' + 0 + ':' + date;
          if (SMS_INFO_MAP.hasKey(key)) {
            continue;
          }
          let values: ValuesBucket = new RcsInfoItem().setStartTime(date).setRcsType(fileType).setMsgContent(body)
            .setIsRead(read).setReceiverNumber(arrPhoneNumber[i]).setIsSender(0).setSessionId(sessionId)
            .setErrorCode(errorCode).setSeen(seen).setGroupId(GROUP_ID).setMsgState(msgState).setMsgId(globalId)
            .setIsCollect(isCollect === 1 && i === 0 ? 1 : 0).setServiceKind(serviceKind).createValuesBucket();
          let newRawRasInfoId = await newDbHelperClone.insertRcsInfo(context, values);
          let newRawSmsIdStatus = 0;
          newRawSmsIdStatus = await this.rcsGroupSmsInsert(newRawRasInfoId, body, arrPhoneNumber, i, sessionId, date,
            type, msgState, isCollect, context, newRawSmsIdStatus, sessionHasDraft, messageCount);
          failCount = newRawRasInfoId === -1 ? failCount++ : failCount;
          successCount = newRawRasInfoId === -1 ? successCount : successCount++;
        }
      } while (resultSet.goToNextRow());
      this.doRcsGroupFailed = this.doRcsGroupFailed + failCount;
      progress.updateProcessCount(progress.getProcessCount() + count);
      resultSet.close();
      LogUtils.i(TAG, 'queryRcsGroupInfo Total Migrations count: ' + count + ',Successful Migrations successCount: ' + successCount + 'Failed Migrations failCount: ' + failCount);
    } catch (error) {
      LogUtils.w(TAG, 'query RcsGroupInfo error : ' + JSON.stringify(error));
    }
    return successCount;
  }

  private async rcsGroupSmsInsert(newRawRasInfoId: number, body: string, arrPhoneNumber: string[], i: number,
                                  sessionId: number, date: number, type: number, msgState: number, isCollect: number,
                                  context: Context, newRawSmsIdStatus: number, sessionHasDraft: number,
                                  msgCount: number) {
    if (newRawRasInfoId != -1) {
      let valuesMmsInfo: ValuesBucket = new SmsMmsInfoItem()
        .setMsgContent(body)
        .setIsRead(1)
        .setReceiverNumber(arrPhoneNumber[i])
        .setSessionId(sessionId)
        .setStartTime(date)
        .setMsgType(0)
        .setIsSender(0)
        .setIsCollect(0)
        .setRcsId(newRawRasInfoId)
        .setMsgState(msgState)
        .setSmsType(99)
        .setIsCollect(isCollect === 1 && i === 0 ? 1 : 0)
        .setGroupId(GROUP_ID)
        .createValuesBucket();
      let newRawSmsId = await newDbHelperClone.insertSms(context, valuesMmsInfo);
      newRawSmsIdStatus = newRawSmsId;
      if (sessionHasDraft === 0) {
        let sessionTime = await newDbHelperClone.getSessionTime(sessionId.toString(), context);
        await this.renovateSessionDate(sessionTime, date, body, context, sessionId, msgCount);
      }
    }
    return newRawSmsIdStatus;
  }

  private rcsGroupValueBuilder(date: number, telephone: string, contactsNum: number, sessionId: number,
                               sessionMap: HashMap<string, LooseObject>) {
    let value: ValuesBucket = {
      'time': date,
      'telephone': telephone,
      'content': 0,
      'contacts_num': contactsNum,
      'unread_count': 0,
      'sms_type': 0,
      'sending_status': 0,
      'has_lock': 0,
      'has_draft': 0,
      'message_count': 0,
      'has_mms': 0,
      'has_attachment': 0,
      'id': sessionId,
    };
    sessionMap.set(telephone, value);
    return value;
  }

  private async renovateSessionDate(sessionTime: number, date: number, body: string, context: common.Context,
    sessionId: number, msgCount: number) {
    if (sessionTime <= date) {
      let upSessionValues: ValuesBucket = {
        'content': body,
        'time': date,
        'message_count': msgCount
      };
      await newDbHelper.updateSession(context, sessionId, upSessionValues);
    } else {
      let upSessionValues: ValuesBucket = {
        'message_count': msgCount
      };
      await newDbHelper.updateSession(context, sessionId, upSessionValues);
    }
  }

  private msgStateBuilder(status: number, msgState: number) {
    if (status == 4) {
      msgState = 2;
    } else if (status == 5) {
      msgState = 1;
    }
    return msgState;
  }

  private async isCollectBuilder(chatId: number, rdbStore: rdb.RdbStore, body: string) {
    let isCollect = 0;
    try {
      let favMmsPredicatesInfo = new rdb.RdbPredicates(tableName1 + tableName2 + 'rcsbackupfav_sms_tb');
      favMmsPredicatesInfo.equalTo('origin_id', chatId + '');
      favMmsPredicatesInfo.equalTo('body', body);
      favMmsPredicatesInfo.equalTo('service_center', 'rcs.groupchat');
      let favMmsResultSet = await rdbStore.query(favMmsPredicatesInfo, ['_id']);
      if (favMmsResultSet != undefined && favMmsResultSet.goToFirstRow()) {
        isCollect = 1;
      }
      favMmsResultSet.close();
    } catch (e) {
      LogUtils.w(TAG, 'query favMms error : ' + JSON.stringify(e));
    }
    return isCollect;
  }


  private async getGroupPhone(arrRecipient: string, rdbStore: rdb.RdbStore, arrPhoneNumber: string[]) {
    LogUtils.i(TAG, 'getGroupPhone start');
      try {
        let canonicalPredicatesInfo = new rdb.RdbPredicates(tableName1 + tableName2 + 'rcsbackuprcs_group_members_tb');
        let element = arrRecipient;
        canonicalPredicatesInfo.equalTo('thread_id', element);
        let canResultSet = await rdbStore.query(canonicalPredicatesInfo, ['rcs_id']);
        if (canResultSet == undefined || !canResultSet.goToFirstRow()) {
          return;
        }
        do {
          let phoneNumber = canResultSet.getString(canResultSet.getColumnIndex('rcs_id'));
          arrPhoneNumber.push(phoneNumber);
        } while (canResultSet.goToNextRow());
        canResultSet.close();
      } catch (e) {
        LogUtils.w(TAG, 'query getGroupPhone error : ' + JSON.stringify(e));
      }
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
      LogUtils.e(TAG, `queryColumns failed, code is ${err.code},message is ${err.message}`);
    }
    return [];
  }

  private async isFieldExistInTable(context: common.Context, table: String, field: string): Promise<boolean> {
    const columns = await this.queryColumns(context, table);
    if (columns.includes(field)) {
      LogUtils.i(TAG, `isFieldExistInTable, ${table} has column ${field}`);
      return true;
    }
    LogUtils.i(TAG, `isFieldExistInTable, ${table} does not have column ${field}`);
    return false;
  }
}

export const copyDbHelper: CopyDbHelper = new CopyDbHelper();
