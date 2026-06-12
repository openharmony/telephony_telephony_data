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

import { StringUtil } from '../../common/utils/StringUtil';
import { rollbackOhosDbHelper } from './RollbackOhosDbHelper'
import BackupExtensionContext from '@ohos.file.BackupExtensionContext';
import taskPool from '@ohos.taskpool';
import LogUtils from '../../common/utils/LogUtils';
import ErrorUtils from '../../common/utils/ErrorUtils';
import BackupRestoreResult from '../copyData/BackupRestoreResult';

const TAG = 'BackupTaskPoolUtil';

export async function next2OhosBackupFunction(context: BackupExtensionContext, backupInfo: string): Promise<string> {
  const TAG = 'BackupTaskPoolUtil';
  if (StringUtil.isEmpty(context.databaseDir)) {
    LogUtils.e(TAG, 'databaseDir is empty, return');
    return Promise.resolve('');
  }

  let result: string = '';
  let err: ErrorUtils = new ErrorUtils(0, 'copy success');
  LogUtils.i(TAG, `onRestoreEx doBackup`);
  LogUtils.i(TAG, `TESTLOG: onRestoreEx doBackup, databaseDir: $context.databaseDir`);
  // 阶段1：获取临时目录
  // 阶段2：清理临时目录里面的db wal shm文件,sms.db/chatSms.db
  // 阶段3：新建表
  let prepareSucc: boolean = rollbackOhosDbHelper.prepareBackupSpace(context);
  if (prepareSucc == false) {
    const backupRestoreResult = BackupRestoreResult.getInstance();
    backupRestoreResult.addErrorInfo(BackupRestoreResult.RESTORE_COMMON_ERROR, '');
    backupRestoreResult.insertOrUpdateBackupInfo('sms', 'create next2ohos dir failed');
    return backupRestoreResult.formatRestoreResult();
  }
  // 阶段4：插入数据
  // 4.1 address_tb
  // 4.2 threads_db
  // 4.3 sms_db
  // 阶段5：关库
  await rollbackOhosDbHelper.doBackupData(context);
  result = await rollbackOhosDbHelper.queryNextToOhosCount(context);
  LogUtils.i(TAG, `onRestoreEx end`);
  return result;
}

async function ohos2NextCloneLongTask(context: BackupExtensionContext, backupInfo: string): Promise<string> {
  let ohos2nextTask = new taskPool.LongTask(next2OhosBackupFunction, context, backupInfo);
  let result = '';
  try {
    let res = await taskPool.execute(ohos2nextTask, taskPool.Priority.HIGH);
    if (res) {
      result = res + '';
    }
  } catch (error) {
    LogUtils.e(TAG, `ohos2nextTask error. Code is ${error.code}, message is ${error.message}`);
  }
  taskPool.terminateTask(ohos2nextTask);
  return result;
}

export class RollbackTaskPoolUtil {
  private static sInstance: RollbackTaskPoolUtil;

  static getInstance() {
    if (RollbackTaskPoolUtil.sInstance == null) {
      RollbackTaskPoolUtil.sInstance = new RollbackTaskPoolUtil();
    }
    return RollbackTaskPoolUtil.sInstance;
  }

  public async executeBackupLongTask(context: BackupExtensionContext, backupInfo: string): Promise<string> {
    return await ohos2NextCloneLongTask(context, backupInfo);
  }
}