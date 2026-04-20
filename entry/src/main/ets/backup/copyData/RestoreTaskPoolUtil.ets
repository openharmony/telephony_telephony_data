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
import { BundleVersion } from '@ohos.application.BackupExtensionAbility';
import { StringUtil } from '../../common/utils/StringUtil';
import BackupRestoreProgress from './BackupRestoreProcess';
import BackupRestoreResult from './BackupRestoreResult';
import { copyDbHelper } from './DbHelper';
import { nextCopyDbHelper } from './NextDbHelper';
import BackupExtensionContext from '@ohos.file.BackupExtensionContext';
import taskPool from '@ohos.taskpool';
import LogUtils from '../../common/utils/LogUtils';
import BackupCommonEvent from './../BackupCommonEvent'
import BackupConstants from './../BackupConstants';
import ErrorUtils from '../../common/utils/ErrorUtils';
import MmsPresenter from './../presenter/mms/MmsPresenter';
import SharedPreferencesUtils from '../../common/utils/SharedPreferencesUtils';
import { GlobalContext } from '../BackupExtension';
import BackupCountInfo from './BackupCountInfo';
import { newDbHelper } from './NewDbHelper';

const TAG = 'RestoreTaskPoolUtil';

@Concurrent
export async function ohos2NextCloneFunction(context: BackupExtensionContext, bundleVersion: BundleVersion,
  progress: BackupRestoreProgress): Promise<string> {
  if (StringUtil.isEmpty(context.databaseDir)) {
    LogUtils.e('RestoreTaskPoolUtil', 'databaseDir is empty, return');
    return Promise.resolve('');
  }
  let result: string = '';
  let err: ErrorUtils = new ErrorUtils(0, 'copy success');
  if ((bundleVersion.name.startsWith(BackupConstants.OHOS2NEXT_CLONE_VERSION) ||
    bundleVersion.name.startsWith(BackupConstants.OHOS2NEXT_CLOUD_CLONE_VERSION) ||
    bundleVersion.name.startsWith(BackupConstants.ARD2NEXT_CLONE_VERSION) ||
    bundleVersion.name.startsWith(BackupConstants.IOS2NEXT_CLOUD_CLONE_VERSION))) {
    LogUtils.i('RestoreTaskPoolUtil', `onRestoreEx doClone`)
    copyDbHelper.setVersion(bundleVersion.name);
    let copySucc = await copyDbHelper.copyDbListFileForClone(context.databaseDir, context);
    if (copySucc == false) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.addErrorInfo(BackupRestoreResult.RESTORE_COMMON_ERROR, '');
      backupRestoreResult.insertOrUpdateBackupInfo('sms', 'copy db from backupDir failed');
      return backupRestoreResult.formatRestoreResult();
    }
    await copyDbHelper.doMigrateData(context, progress);
    result = await copyDbHelper.queryOhosToNextCount(context);
  } else {
    LogUtils.i('RestoreTaskPoolUtil', `onRestoreEx doNextClone`)
    LogUtils.i('RestoreTaskPoolUtil', `onRestoreEx version code: ${bundleVersion.code}`);
    nextCopyDbHelper.setVersion(bundleVersion.name);
    await nextCopyDbHelper.copyDbListFileForClone(context.databaseDir, context,
      async (error?: ErrorUtils) => {
        if (error) {
          err = error;
        }
      }, progress);
    result = await nextCopyDbHelper.queryNextCount(context, err);
  }
  BackupCommonEvent.sendRefreshEvent();
  LogUtils.i('TAG', `onRestoreEx end`);
  return result;
}

@Concurrent
export async function ohos2NextUpgradeFunction(context: BackupExtensionContext, bundleVersion: BundleVersion,
  progress: BackupRestoreProgress): Promise<string> {
  let result: string = '';
  GlobalContext.getContext().setObject('context', context);
  SharedPreferencesUtils.init(context);
  const presenter: MmsPresenter = MmsPresenter.getInstance();
  presenter.init();
  result = await presenter.migrateDbFile(context, progress, bundleVersion);
  return result;
}

async function ohos2NextCloneLongTask(context: BackupExtensionContext, bundleVersion: BundleVersion,
  progress: BackupRestoreProgress): Promise<string> {
  let ohos2nextTask = new taskPool.LongTask(ohos2NextCloneFunction, context, bundleVersion,
    progress);
  let result = '';
  try {
    let res = await taskPool.execute(ohos2nextTask, taskPool.Priority.HIGH);
    if (res) {
      result = res + '';
    }
  } catch (error) {
    LogUtils.e('TAG', `ohos2nextTask error. Code is ${error.code}, message is ${error.message}`);
  }
  taskPool.terminateTask(ohos2nextTask);
  return result;
}

function getBackupStatisticTask(context: BackupExtensionContext, countInfo: BackupCountInfo): string {
  let getBackupInfoTask = new taskPool.LongTask(getBackupInfoFunction, context, countInfo);
  let result = '';
  try {
    taskPool.execute(getBackupInfoTask, taskPool.Priority.LOW);
  } catch (error) {
    LogUtils.e(TAG, `getBackupStatisticTask error. Code is ${error.code}, message is ${error.message}`);
    return result;
  }
  let startTime = Date.now();
  while (Date.now() - startTime < 2000 && !countInfo.getUpdateSuc()) {
    continue;
  }
  result = countInfo.getBackupCountInfo();
  taskPool.terminateTask(getBackupInfoTask);
  LogUtils.i(TAG, 'getBackupInfo result:' + result);
  return result;
}

async function ohos2NextUpgradeLongTask(context: BackupExtensionContext, bundleVersion: BundleVersion,
  progress: BackupRestoreProgress): Promise<string> {
  let ohos2nextTask = new taskPool.LongTask(ohos2NextUpgradeFunction, context, bundleVersion,
    progress);
  let result = '';
  try {
    let res = await taskPool.execute(ohos2nextTask, taskPool.Priority.HIGH);
    if (res) {
      result = res + '';
    }
  } catch (error) {
    LogUtils.e('TAG', `ohos2nextTask error. Code is ${error.code}, message is ${error.message}`);
  }
  taskPool.terminateTask(ohos2nextTask);
  return result;
}

@Concurrent
async function getBackupInfoFunction(context: BackupExtensionContext, countInfo: BackupCountInfo) {
  await newDbHelper.querySmsMmsDataCount(context, countInfo);
}

export class RestoreTaskPoolUtil {
  private static sInstance: RestoreTaskPoolUtil;

  static getInstance() {
    if (RestoreTaskPoolUtil.sInstance == null) {
      RestoreTaskPoolUtil.sInstance = new RestoreTaskPoolUtil();
    }
    return RestoreTaskPoolUtil.sInstance;
  }

  public async executeUpgradeLongTask(context: BackupExtensionContext, bundleVersion: BundleVersion,
    progress: BackupRestoreProgress): Promise<string> {
    return await ohos2NextUpgradeLongTask(context, bundleVersion, progress);
  }

  public async executeBackupLongTask(context: BackupExtensionContext, bundleVersion: BundleVersion,
    progress: BackupRestoreProgress): Promise<string> {
    return await ohos2NextCloneLongTask(context, bundleVersion, progress);
  }

  public backupStatisticTask(context: BackupExtensionContext, countInfo: BackupCountInfo): string {
    return getBackupStatisticTask(context, countInfo);
  }
}