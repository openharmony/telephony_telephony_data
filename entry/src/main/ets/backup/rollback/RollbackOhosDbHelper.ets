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
import fs from '@ohos.file.fs';
import rdb from '@ohos.data.relationalStore';
import { BackupExtensionContext } from '@kit.CoreFileKit';
import { common, contextConstant } from '@kit.AbilityKit';
import LogUtils from '../../common/utils/LogUtils';
import RollbackDbConstants from './RollbackDbConstants'
import { rollbackNextDbHelper } from './RollbackNextDbHelper'
import { BusinessError } from '@kit.BasicServicesKit';
import { ValuesBucket, ValueType } from '@kit.ArkData';
import DbConstants from '../copyData/DbConstants';
import NextOldDBData from '../copyData/NextOldDBData';
import { StringUtil } from '../../common/utils/StringUtil';
import ErrorUtils from '../../common/utils/ErrorUtils';
import RollbackResult from './RollbackResult';
import FileUtils from '../../common/utils/FileUtils';

const TAG = '[Rollback-OhosDbHelper] :';

const CURRENT_RDB_VERSION: number = 1;

class RollbackOhosDbHelper {
  private rdbStore: rdb.RdbStore | null = null;
  private telephoneMap: Map<string, number> = new Map();

  private rebuildBackupDir(path: string): boolean {
    // Step1 获取临时目录是否存在，清理并重新创建临时目录
    try {
      // Step1.1 判断文件是否存在
      let isReadBackupPathExists: boolean = fs.accessSync(path);
      // Step1.2 文件目录存在，则删除原始目录，为了清理临时目录里面的db wal shm文件
      if (isReadBackupPathExists) {
        try {
          fs.rmdirSync(path);
        } catch (error) {
          LogUtils.i(TAG, `rmdirSync failed: ${error.code}`);
          return false;
        }
      }
      // Step1.3 重新创建目录
      try {
        fs.mkdirSync(path);
      } catch (error) {
        LogUtils.i(TAG, `mkdirSync failed: ${error.code}`);
        return false;
      }
      LogUtils.i(TAG, 'rebuildTempDir success');
      return true;
    } catch (error) {
      LogUtils.i(TAG, `rebuildTempDir error: ${JSON.stringify(error)}`);
      return false;
    }
  }

  public prepareBackupSpace(): boolean {
    let next2OhosTempPath = RollbackDbConstants.NEXT_DATABASE_PATH +
      '/' + RollbackDbConstants.NEXT2OHOS_TEMP_FOLDER;
    let result = this.rebuildBackupDir(next2OhosTempPath);
    return result;
  }

  public doDirCopy(callback: Function) {
    if (FileUtils.rmDirSync(RollbackDbConstants.BACKUP_DATABASE_PATH + '/' +
      RollbackDbConstants.NEXT2OHOS_TEMP_FOLDER) == false) {
      if (callback) {
        callback(new ErrorUtils(400, 'next2ohos dir rm failed'));
      }
      return;
    }
    let result =
      FileUtils.copyDirSync(RollbackDbConstants.NEXT_DATABASE_PATH +
        '/' + RollbackDbConstants.NEXT2OHOS_TEMP_FOLDER,
        RollbackDbConstants.BACKUP_DATABASE_PATH);
    if (result == false) {
      LogUtils.e(TAG, 'next2ohos dir copy failed');
      if (callback) {
        callback(new ErrorUtils(400, 'next2ohos dir copy failed'));
      }
    }
  }

  public async doBackupData(context: common.Context, callback: Function) {
    let rdbStore = await this.getRdbStore(context);
    if (rdbStore == null) {
      LogUtils.e(TAG, 'get rdb store failed');
      if (callback) {
        callback(new ErrorUtils(400, 'get rdb store failed'));
      }
      return;
    }
    await this.doAddressesAndThreadsDataCopy(context, callback);
    await this.doSmsDataCopy(context, callback);
    await this.deleteInvalidSession(context);
    await rdbStore.close();
    rdbStore = null;
    this.doDirCopy(callback);
  }

  private async doAddressesAndThreadsDataCopy(context: common.Context, callBack?: Function) {
    let idSuccess = 0;
    let sessionCount = await rollbackNextDbHelper.queryDataCount(context, DbConstants.NEXT_OLD_SESSION_NAME,
      NextOldDBData.session.id, callBack);
    let sessionPage = Math.ceil(sessionCount / DbConstants.QUERY_PAGE_NUM);

    LogUtils.i(TAG, 'sessionPage:' + sessionPage + ', allCount:' + sessionCount);
    for (let curPage = 1; curPage <= sessionPage; curPage++) {
      idSuccess += await rollbackNextDbHelper.querySessionTable(context,
        DbConstants.QUERY_PAGE_NUM * (curPage - 1), callBack);
    }
    this.doAddressCopy();
  }

  private doAddressCopy() {
    this.telephoneMap = rollbackNextDbHelper.getTelephoneMap();
    let index = 0;
    let batchAddressValues: ValuesBucket[] = [];
    this.telephoneMap.forEach((v, k) => {
      if (v && k) {
        let vb: ValuesBucket = {
          'address': k,
          '_id': v
        };
        batchAddressValues.push(vb);
        index++;
      }
      if (index >= 300) {
        if (this.rdbStore === null) {
          LogUtils.e(TAG, `rdbStore cant null`);
          return
        }
        this.rdbStore.batchInsertSync(RollbackDbConstants.ADDRESSES_TB_NAME, batchAddressValues);
        index = 0;
        batchAddressValues = [];
      }
    });
    if (batchAddressValues.length > 0) {
      if (this.rdbStore === null) {
        LogUtils.e(TAG, `rdbStore cant null`);
        return
      }
      this.rdbStore.batchInsertSync(RollbackDbConstants.ADDRESSES_TB_NAME, batchAddressValues);
      index = 0;
      batchAddressValues = [];
    }
  }

  private async doSmsDataCopy(context: common.Context, callBack?: Function) {
    let idSuccess = 0;
    let smsInfoCount = await rollbackNextDbHelper.queryDataCount(context, DbConstants.NEXT_OLD_SMS_NAME,
      NextOldDBData.smsMmsInfo.msgId, callBack);
    let smsInfoPage = Math.ceil(smsInfoCount / DbConstants.QUERY_PAGE_NUM);

    LogUtils.i(TAG, 'smsInfoPage:' + smsInfoPage + ', allCount:' + smsInfoCount);
    for (let curPage = 1; curPage <= smsInfoPage; curPage++) {
      idSuccess += await rollbackNextDbHelper.queryMmsTable(context,
        DbConstants.QUERY_PAGE_NUM * (curPage - 1), callBack);
    }
  }

  public async deleteInvalidSession(context: common.Context): Promise<void> {
    LogUtils.i(TAG, `delete invalid session start`)
    let rdbStore = await this.getRdbStore(context);
    if (rdbStore === undefined) {
      LogUtils.i(TAG, `delete invalid session, getRdbStore failed`)
      return;
    }
    try {
      let conditions: string = 'DELETE FROM threads_tb WHERE thread_id NOT IN (SELECT thread_id FROM sms_tb)';
      let data = (rdbStore as rdb.RdbStore).executeSync(conditions);
      LogUtils.i(TAG, `delete invalid session result: ${data}`);
    } catch (err) {
      LogUtils.e(TAG, `delete invalid session failed, code is ${err.code},message is ${err.message}`);
    }
  }

  public async queryNextToOhosCount(context: common.Context): Promise<string> {
    return '';
  }

  public async getRdbStore(baseContext: common.Context): Promise<rdb.RdbStore | null> {
    if (!baseContext) {
      LogUtils.e(TAG, 'getRdbStore baseContext is null');
      return null;
    }
    if (this.rdbStore) {
      return this.rdbStore;
    }
    try {
      LogUtils.i(TAG, 'getRdbStore Start');
      if (baseContext.area !== contextConstant.AreaMode.EL1) {
        baseContext.area = contextConstant.AreaMode.EL1;
      }
      let dir = '../../' + RollbackDbConstants.NEXT2OHOS_TEMP_FOLDER;
      this.rdbStore = await rdb.getRdbStore(baseContext,
        {
          name: RollbackDbConstants.SMS_DATABASE_NAME,
          securityLevel: rdb.SecurityLevel.S2,
          isSearchable: true,
          customDir: dir
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
      await this.executeSql(RollbackDbConstants.CREATE_ADDRESSES_TB,
        'initRdbStore createAddressesTable succeed');
      await this.executeSql(RollbackDbConstants.CREATE_THREADS_TB,
        'initRdbStore createThreadsTable succeed');
      await this.executeSql(RollbackDbConstants.CREATE_SMS_TB,
        'initRdbStore createSmsTable succeed');
      await this.executeSql(RollbackDbConstants.CREATE_PDU_TB,
        'initRdbStore createPduTable succeed');
      await this.executeSql(RollbackDbConstants.CREATE_PART_TB,
        'initRdbStore createPartTable succeed');
      await this.executeSql(RollbackDbConstants.CREATE_ADDR_TB,
        'initRdbStore createAddrTable succeed');
      await this.executeSql(RollbackDbConstants.CREATE_FILES_MMS_TB,
        'initRdbStore createFilesMmsTable succeed');
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

  public shouldPerformBackup(backupInfo: string): boolean {
    if (StringUtil.isEmpty(backupInfo)) {
      return false;
    }
    let support: boolean = false;
    let jsonObject: Record<string, string>[] = JSON.parse(backupInfo);
    jsonObject.forEach(obj => {
      let type = obj['type'] as string;
      let detail = obj['detail'] as string;
      LogUtils.e(TAG, type + ':' + detail);
      if (type == RollbackDbConstants.BACKUP_SUBTYPE &&
        detail == RollbackDbConstants.BACKUP_DETAIL) {
        support = true;
      }
    });
    return support;
  }

  public async onBackupProcess(context: BackupExtensionContext): Promise<string> {
    LogUtils.i(TAG, `onRestoreEx doBackup`);
    let rollbackResult: RollbackResult = new RollbackResult();
    if (StringUtil.isEmpty(context.databaseDir)) {
      LogUtils.e(TAG, 'databaseDir is empty, return');
      rollbackResult.setErrCode(RollbackResult.RESTORE_COMMON_ERROR);
      rollbackResult.setErrInfo('databaseDir is empty');
      return rollbackResult.formatRollbackResult();
    }
    // 阶段1：获取临时目录
    // 阶段2：清理临时目录里面的db wal shm文件,sms.db/chatSms.db
    // 阶段3：新建表
    let prepareSucc: boolean = this.prepareBackupSpace();
    if (prepareSucc == false) {
      rollbackResult.setErrCode(RollbackResult.RESTORE_COMMON_ERROR);
      rollbackResult.setErrInfo('create temporary directory failed');
      return rollbackResult.formatRollbackResult();
    }
    // 阶段4：插入数据
    // 4.1 address_tb
    // 4.2 threads_db
    // 4.3 sms_db
    // 阶段5：关库
    let err: ErrorUtils = new ErrorUtils(0, '');
    await this.doBackupData(context, async (error?: ErrorUtils) => {
      if (error) {
        err = error;
      }
    });
    await this.queryNextToOhosCount(context); // no use now, keep for backup result detail display
    rollbackResult.setErrCode((err && err.code > 0) ? RollbackResult.RESTORE_COMMON_ERROR
      : RollbackResult.RESTORE_SUCCESS);
    rollbackResult.setErrInfo(err.message);
    LogUtils.i(TAG, `onRestoreEx end`);
    return rollbackResult.formatRollbackResult();
  }
}

export const rollbackOhosDbHelper: RollbackOhosDbHelper = new RollbackOhosDbHelper();