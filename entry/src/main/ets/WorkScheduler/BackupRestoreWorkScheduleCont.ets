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

import { BusinessError } from '@ohos.base';
import LogUtils from '../common/utils/LogUtils';
import workScheduler from '@ohos.resourceschedule.workScheduler';

const TAG = 'BackupRestoreWorkScheduleCont';
const BACKUP_WORK_ID = 1;

export default class BackupRestoreWorkScheduleCont {
  private isSchedulerStarted: boolean = false;
  private static instance: BackupRestoreWorkScheduleCont;

  private workInfo: workScheduler.WorkInfo = {
    workId: BACKUP_WORK_ID,
    bundleName: 'com.ohos.telephonydataability',
    abilityName: 'BackupRestoreWorkScheduler',
    isRepeat: true,
    repeatCycleTime: 1000 * 60 * 60 * 24 * 2,
    isPersisted: true,
    isDeepIdle: true
  }

  public static getInstance(): BackupRestoreWorkScheduleCont {
    if (!BackupRestoreWorkScheduleCont.instance) {
      BackupRestoreWorkScheduleCont.instance = new BackupRestoreWorkScheduleCont();
    }
    return BackupRestoreWorkScheduleCont.instance;
  }

  constructor() {
  }

  public async setupWorkScheduler(): Promise<void> {
    if (this.isSchedulerStarted) {
      LogUtils.i(TAG, 'backup-restore schedule already started');
      return;
    }
    try {
      const work: workScheduler.WorkInfo = await workScheduler.getWorkStatus(BACKUP_WORK_ID);
      if (work) {
        LogUtils.i(TAG, 'backup-restore schedule exist, break. workInfo is: ' + JSON.stringify(work));
        this.isSchedulerStarted = true;
        return;
      }
      LogUtils.e(TAG, `no work find for ${BACKUP_WORK_ID}`);
    } catch (e) {
      LogUtils.e(TAG, `getWorkStatus for ${BACKUP_WORK_ID} failed:${e?.message || e?.code}`);
    }
    try {
      workScheduler.startWork(this.workInfo);
      LogUtils.i(TAG, `start work schedule success.`);
      this.isSchedulerStarted = true;
    } catch (error) {
      LogUtils.e(TAG, `start work schedule failed. code is ${
      (error as BusinessError).code} message is ${(error as BusinessError).message}`);
    }
  }

  public stopWorkScheduler() {
    try {
      workScheduler.stopWork(this.workInfo);
      LogUtils.i(TAG, `stop work schedule success. `);
      this.isSchedulerStarted = false;
    } catch (error) {
      LogUtils.e(TAG, `stop work schedule failed. code is ${
      (error as BusinessError).code} message is ${(error as BusinessError).message}`);
    }
  }
}