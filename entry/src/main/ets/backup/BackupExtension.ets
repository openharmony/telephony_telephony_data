/**
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

import BackupExtensionAbility, { BundleVersion } from '@ohos.application.BackupExtensionAbility';
import LogUtils from '../common/utils/LogUtils';
import SharedPreferencesUtils from '../common/utils/SharedPreferencesUtils';
import { copyDbHelper } from './copyData/DbHelper';
import { nextCopyDbHelper } from './copyData/NextDbHelper';
import MmsPresenter from './presenter/mms/MmsPresenter';
import BackupConstants from './BackupConstants';
import BackupCommonEvent from './BackupCommonEvent'
import BackupRestoreProgress from './copyData/BackupRestoreProcess';
import { RestoreTaskPoolUtil} from './copyData/RestoreTaskPoolUtil'
import { rollbackOhosDbHelper } from './rollback/RollbackOhosDbHelper';
import DbConstants from './copyData/DbConstants';
import fs from '@ohos.file.fs';
import BackupCountInfo from './copyData/BackupCountInfo';
import BackupRestoreResult from '../backup/copyData/BackupRestoreResult';
import { preferences } from '@kit.ArkData';
import { GskvSharedPreferencesUtils } from '../common/utils/GskvPreferencesUtils';
import { relationalStore } from '@kit.ArkData';
import { contextConstant } from '@kit.AbilityKit';

const TAG = 'BackupExtension';
const SCENARIO_BACKUP: number = 1;

export class GlobalContext {
  private constructor() { }
  private static instance: GlobalContext;
  private myObjects = new Map<string, Object>();

  public static getContext(): GlobalContext {
    if (!GlobalContext.instance) {
      GlobalContext.instance = new GlobalContext();
    }
    return GlobalContext.instance;
  }

  getObject(value: string): Object | undefined {
    return this.myObjects.get(value);
  }

  setObject(key: string, objectClass: Object | null): void {
    this.myObjects.set(key, objectClass as Object);
  }
}

export default class BackupExtension extends BackupExtensionAbility {
  private backupRestoreProcess: BackupRestoreProgress | undefined = undefined;
  private backupCountInfo: BackupCountInfo | undefined = undefined;
  public static readonly NEED_TEMPORARILY_COPIED_FILE: string[] = [DbConstants.nextCloneDatabaseName,
    DbConstants.nextCloneDatabaseNameShm, DbConstants.nextCloneDatabaseNameWal];

  onBackup() {
    LogUtils.i(TAG, `onBackup init`);
  }

async onRelease(scenario: number): Promise<void> {
    // 备份场景
    if (scenario == SCENARIO_BACKUP) {
      // 没有迁移成功，el1不能删除
      if (!GskvSharedPreferencesUtils.isDbMigrationSuccess(this.context)) {
        LogUtils.w(TAG, `db migration is false, not process onRelease`);
        return;
      }
      let el1Config : relationalStore.StoreConfig = {
        name: DbConstants.nextCloneDatabaseName,
        securityLevel: relationalStore.SecurityLevel.S1,
        customDir: '../../rdb'
      }
      try {
        let el1Context = this.context.createAreaModeContext(contextConstant.AreaMode.EL1)
        await relationalStore.deleteRdbStore(el1Context, el1Config);
        LogUtils.w(TAG, `onRelease delete el1 ok`);
        return;
      } catch (error) {
        LogUtils.e(TAG, `onRelease error code ${error.code} error msg: ${error.message}`);
        return;
      }
    }
  }

  async onBackupEx(backupInfo: string): Promise<string> {
    LogUtils.i(TAG, `onBackupEx init, backupInfo:` + backupInfo);
    if (!this.copyRdbFileFromEl2ToEl1ForClone()) {
        LogUtils.w(TAG, 'copy EL2_RDB to el1 failed.');
    }
    if (rollbackOhosDbHelper.shouldPerformBackup(backupInfo)) {
      let result = await rollbackOhosDbHelper.onBackupProcess(this.context);
      LogUtils.i(TAG, `onBackupEx finish, resultInfo:` + result);
      return result;
    }
    return '';
  }

  async onRestore(bundleVersion: BundleVersion): Promise<void> {
    LogUtils.i(TAG, `onRestore start`)
    const versionName: string = bundleVersion.name;
    LogUtils.i(TAG, `onRestore versionName:` + versionName);
    if (versionName.startsWith(BackupConstants.OHOS2NEXT_UPGRADE_VERSION)) {
      LogUtils.i(TAG, `onRestore doUpgrade`);
    } else if (versionName.startsWith(BackupConstants.OHOS2NEXT_CLONE_VERSION) ||
      versionName.startsWith(BackupConstants.OHOS2NEXT_CLOUD_CLONE_VERSION)) {
      if (this.context.databaseDir) {
        LogUtils.i(TAG, `onRestore doClone`)
        copyDbHelper.setVersion(versionName);
        await copyDbHelper.copyDbListFileForClone(this.context.databaseDir, this.context);
      }
    } else if (versionName === BackupConstants.ARD2NEXT_CLONE_VERSION) {
    } else if (versionName === BackupConstants.IOS2NEXT_CLOUD_CLONE_VERSION) {
    } else if (versionName === BackupConstants.WINPC2NEXT_CLOUD_CLONE_VERSION) {
    } else {
      if (this.context.databaseDir) {
        LogUtils.i(TAG, `onRestore doNextClone`)
        nextCopyDbHelper.setVersion(versionName);
        await nextCopyDbHelper.copyDbListFileForClone(this.context.databaseDir, this.context, ()=>{},
          new BackupRestoreProgress());
      }
    }
    BackupCommonEvent.sendRefreshEvent();
    LogUtils.i(TAG, `onRestore end`);
  }

  async onRestoreEx(bundleVersion: BundleVersion): Promise<string> {
    LogUtils.i(TAG, `onRestoreEx start`)
    let result: string = '';
    const versionName: string = bundleVersion.name;
    LogUtils.i(TAG, `onRestore versionName:` + versionName);
    if (this.backupRestoreProcess == undefined) {
      this.backupRestoreProcess = new BackupRestoreProgress();
    }
    if (versionName.startsWith(BackupConstants.OHOS2NEXT_UPGRADE_VERSION)) {
      LogUtils.i(TAG, `onRestoreEx doUpgrade`)
      let restoreTaskPoolUtil: RestoreTaskPoolUtil = RestoreTaskPoolUtil.getInstance();
      return await restoreTaskPoolUtil.executeUpgradeLongTask(this.context, bundleVersion, this.backupRestoreProcess);
    } else if (versionName.startsWith(BackupConstants.OHOS2NEXT_CLONE_VERSION) ||
      versionName.startsWith(BackupConstants.OHOS2NEXT_CLOUD_CLONE_VERSION) ||
      versionName.startsWith(BackupConstants.ARD2NEXT_CLONE_VERSION) ||
      versionName.startsWith(BackupConstants.IOS2NEXT_CLOUD_CLONE_VERSION)) {
      LogUtils.i(TAG, `onRestoreEx doClone`)
      let restoreTaskPoolUtil: RestoreTaskPoolUtil = RestoreTaskPoolUtil.getInstance();
      return await restoreTaskPoolUtil.executeBackupLongTask(this.context, bundleVersion, this.backupRestoreProcess);
    } else if (versionName === BackupConstants.IOS2NEXT_CLOUD_CLONE_VERSION) {
      LogUtils.i(TAG, `onRestoreEx end`);
      return Promise.resolve(result);
    } else if (versionName === BackupConstants.WINPC2NEXT_CLOUD_CLONE_VERSION) {
      LogUtils.i(TAG, `onRestoreEx end`);
      return Promise.resolve(result);
    } else {
      let restoreTaskPoolUtil: RestoreTaskPoolUtil = RestoreTaskPoolUtil.getInstance();
      return await restoreTaskPoolUtil.executeBackupLongTask(this.context, bundleVersion, this.backupRestoreProcess);
    }
  }

  getBackupInfo(): string {
    LogUtils.e(TAG, 'enter getBackupInfo')
    if (this.backupCountInfo == undefined) {
      this.backupCountInfo = new BackupCountInfo();
    }
    let restoreTaskPoolUtil: RestoreTaskPoolUtil = RestoreTaskPoolUtil.getInstance();
    return restoreTaskPoolUtil.backupStatisticTask(this.context, this.backupCountInfo);
  }

  onProcess(): string {
    try {
      LogUtils.i(TAG, `onProcess start`);
      if (this.backupRestoreProcess == undefined) {
        this.backupRestoreProcess = new BackupRestoreProgress();
      }
      let result: string = this.backupRestoreProcess.getProgress();
      LogUtils.i(TAG, `onProcess result: ${result}`);
      return result;
    } catch (error) {
      LogUtils.i(TAG, `onProcess failed error code: ${error.code} error name: ${error.message}`);
      return '';
    }
  }

  // 备份的时候（云备份、克隆备份),需要将el2复制到el1。
  private copyRdbFileFromEl2ToEl1ForClone(): boolean {
    let el2RdbDir = DbConstants.EL2_RDB;
    let el1RdbDir = DbConstants.EL1_RDB;
    // 没有迁移成功，说明el2不存在数据库。直接返回。el1有库就用，没有就不提供数据。
    if (!GskvSharedPreferencesUtils.isDbMigrationSuccess(this.context)) {
      LogUtils.w(TAG, `db migration is false, return`);
      return false;
    }
    try {
      // accessSync
      const isel2RdbDirExists = fs.accessSync(el2RdbDir);
      if (!isel2RdbDirExists) {
        LogUtils.w(TAG, `el2RdbDir is not exists`);
        return false;
      }
      const isEl1RdbDirExists = fs.accessSync(el1RdbDir);
      if (!isEl1RdbDirExists) {
        try {
          fs.mkdirSync(el1RdbDir);
          LogUtils.w(TAG, `el1RdbDir is not exists, create it`);
        } catch (error) {
          LogUtils.e(TAG, `el1RdbDir create failed : ${error}`);
          return false;
        }
      }
      BackupExtension.NEED_TEMPORARILY_COPIED_FILE.forEach((value: string) => {
        let srcPath = `${el2RdbDir}/${value}`;
        let destPath = `${el1RdbDir}/${value}`;
        let isSrcFileExists = fs.accessSync(srcPath);
        if (isSrcFileExists) {
          fs.copyFileSync(srcPath, destPath);
        } else {
          LogUtils.w(TAG, `srcFile is not exists`);
        }
      });
      LogUtils.w(TAG, 'copyRdbFileFromEl2ToEl1ForClone success');
      return true;
    } catch (error) {
      LogUtils.w(TAG, `copyRdbFileFromEl2ToEl1ForClone error: ${JSON.stringify(error)}`);
    }
    return false;
  }
}
