/**
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

import WorkSchedulerExtensionAbility from '@ohos.WorkSchedulerExtensionAbility';
import LogUtils from '../common/utils/LogUtils';
import workScheduler from '@ohos.resourceschedule.workScheduler';
import rdb from '@ohos.data.relationalStore';
import contextConstant from '@ohos.app.ability.contextConstant';
import BackupRestoreWorkScheduleCont from './BackupRestoreWorkScheduleCont';
import FileUtils from '../common/utils/FileUtils';
import fs from '@ohos.file.fs';
import { GskvSharedPreferencesUtils } from '../common/utils/GskvPreferencesUtils';

const TAG = 'BackupRestoreWorkScheduler';
const DB_NAME = 'sms_mms.db';
const BAK_DB_NAME = 'sms_mms.db.bak';
const EL1_PATH = '/data/storage/el1/database/rdb/';
const EL2_PATH = '/data/storage/el2/database/rdb/';
const DELETE_FILES = [
  'sms_mms.db',
  'sms_mms.db-dwr',
  'sms_mms.db-shm',
  'sms_mms.db-wal',
  'sms_mms.db-compare',
  'sms_mms.db.bak',
];
export default class BackupRestoreWorkScheduler extends WorkSchedulerExtensionAbility {
  
  onWorkStart(workInfo: workScheduler.WorkInfo) {
    LogUtils.d(TAG, `onWorkStart, workInfo = ${JSON.stringify(workInfo)}`);
    this.processAutoBackupRestore().then((ret?: number) => {
      LogUtils.i(TAG, `process auto backup-restore finished, ret:${ret}`);
    }).catch((e: Error) => {
      LogUtils.e(TAG, `process auto backup-restore error occurs :${e.message}`);
    });
  }

  onWorkStop(workInfo: workScheduler.WorkInfo) {
    LogUtils.d(TAG, `onWorkStop, workInfo = ${JSON.stringify(workInfo)}`);
  }

  private async processBakeUpFileInEl1() {
    try {
      if (!fs.accessSync(EL1_PATH + DB_NAME)) {
        LogUtils.w(TAG, `no exist sms_mms.db in el1`);
        return;
      }
      if (this.context.area !== contextConstant.AreaMode.EL1) {
        this.context.area = contextConstant.AreaMode.EL1;
      }
      let rdbStoreHelper: rdb.RdbStore | undefined = undefined;
      rdbStoreHelper = await rdb.getRdbStore(this.context!, {
        name: DB_NAME,
        securityLevel: rdb.SecurityLevel.S1,
        isSearchable: false,
        autoCleanDirtyData: false,
        customDir: '../../rdb',
        allowRebuild: true
      });
      if (rdbStoreHelper == null) {
        LogUtils.w(TAG, `rdbStoreHelper is null return`);
        return;
      }
      let conditions: string = 'SELECT name FROM sqlite_master WHERE type = \'table\' ORDER BY name;';
      let resultSet: rdb.ResultSet = rdbStoreHelper.querySqlSync(conditions);
      LogUtils.w(TAG, 'el1 database table number: ' + resultSet.rowCount);
      if (resultSet.rowCount > 5) {
        LogUtils.w(TAG, 'el1 database maybe is using database, do not delete now');
      } else {
        DELETE_FILES.forEach((item) => {
          if (fs.accessSync(EL1_PATH + item)) {
            LogUtils.w(TAG, 'el1 exist, need to remove: ' + item);
            fs.unlinkSync(EL1_PATH + item);
          }
        })
      }
    } catch (error) {
      LogUtils.e(TAG, `processBakeUpFileInEl1 failed, message is ${error.message}`);
    }
  }

  private async processAutoBackupRestore() {
    // EL1之前存在烂库，这里尝试删除一下。
    await this.processBakeUpFileInEl1();
    let rdbStoreHelper: rdb.RdbStore | undefined = undefined;
    let lastArea: contextConstant.AreaMode = this.context.area;

    try {
      // 未迁移成功，不执行备份。防止不完整的库备份。
      if (!GskvSharedPreferencesUtils.isDbMigrationSuccess(this.context)) {
        LogUtils.e(TAG, `isDbMigrationSuccess is false, no need to backup`);
        return;
      }
      // EL2 库不存在，就不用开库备份了。
      if (!fs.accessSync(EL2_PATH + DB_NAME)) {
        LogUtils.w(TAG, `no exist sms_mms.db in el2, no need to backup`);
        return;
      }
      if (this.context.area !== contextConstant.AreaMode.EL2) {
        this.context.area = contextConstant.AreaMode.EL2;
      }
      rdbStoreHelper = await rdb.getRdbStore(this.context!, {
        name: DB_NAME,
        securityLevel: rdb.SecurityLevel.S2,
        isSearchable: true,
        autoCleanDirtyData: false,
        customDir: '../../rdb',
        allowRebuild: true
      });
      if (rdbStoreHelper != null) {
        await this.doDatabaseCheck(rdbStoreHelper);
      } else {
        LogUtils.e(TAG, `GetRdbStore err, rdbStoreHelper is null, skip rebuilt. `);
      }
    } catch (error) {
      LogUtils.w(TAG, `process auto backup restore failed, message is ${error.message}`);
    } finally {
      if (this.context.area !== lastArea) {
        this.context.area = lastArea;
      }
    }
    await rdbStoreHelper?.close();
    return undefined;
  }

  private async doDatabaseCheck(rdbStoreHelper: rdb.RdbStore) {
    let rebuilt: number = rdbStoreHelper.rebuilt;
    LogUtils.i(TAG, `rebuilt type:${JSON.stringify(rebuilt)}`);
    if (rebuilt == rdb.RebuildType.REBUILT) {
      LogUtils.i(TAG, 'rebuild, do restore.');
      await this.doRestore(rdbStoreHelper);
    } else {
      // If data is not rebuilt, the data integrity check is performed.
      const ret = await rdbStoreHelper.execute('pragma integrity_check;');
      LogUtils.i(TAG, `not rebuild, do integrity check ret: ${ret}`);
      if (ret == 'ok') {
        // If the integrity check is passed, perform the backup.
        LogUtils.i(TAG, 'integrity check pass, do backup.');
        await this.doBackup(rdbStoreHelper);
      } else {
        // If the integrity check fails, perform the restoration.
        LogUtils.e(TAG, 'integrity check failed, do restore.');
        await this.doRestore(rdbStoreHelper);
      }
    }
  }

  private async doBackup(rdbStoreHelper: rdb.RdbStore) {
    await rdbStoreHelper.backup(BAK_DB_NAME, (err) => {
      if (err) {
        LogUtils.e(TAG, `Backup failed, code is ${err.code},message is ${err.message}`);
      } else {
        LogUtils.i(TAG, 'Backup success.');
      }
      this.stopWork();
    });
  }

  private async doRestore(rdbStoreHelper: rdb.RdbStore) {
    await rdbStoreHelper.restore(BAK_DB_NAME, (err) => {
      if (err) {
        LogUtils.e(TAG, `Restore failed, code is ${err.code},message is ${err.message}`);
      } else {
        LogUtils.i(TAG, `Restore success.`);
      }
      this.stopWork();
    });
  }

  private stopWork() {
    LogUtils.i(TAG, `stop work schedule `);
    BackupRestoreWorkScheduleCont.getInstance().stopWorkScheduler();
  }
}