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

import rpc from '@ohos.rpc';
import LogUtils from '../../common/utils/LogUtils';
import BackupRestoreWorkScheduleCont from '../../WorkScheduler/BackupRestoreWorkScheduleCont';

const TAG = 'BackupRestoreServiceStub';
const START_BACKUP_RESTORE_REQUEST: number = 100;
const MMS_MGR_INTERFACE_TOKEN: string = 'OHOS.MMS.BackupRestoreServiceStub';

export default class BackupRestoreServiceStub extends rpc.RemoteObject {

  constructor(descriptor: string) {
    super(descriptor);
  }

  onRemoteMessageRequest(code: number, data: rpc.MessageSequence, reply: rpc.MessageSequence,
    options: rpc.MessageOption): boolean | Promise<boolean> {
    try {
      if (data.readInterfaceToken() !== MMS_MGR_INTERFACE_TOKEN) {
        LogUtils.i(TAG, `InterfaceToken not matched.`);
        return false;
      }
    } catch (error) {
      LogUtils.i(TAG, `read data exception, error is ${JSON.stringify(error)}`);
      return false;
    }
    if (code === START_BACKUP_RESTORE_REQUEST) {
      LogUtils.i(TAG, 'process remote request ');
      BackupRestoreWorkScheduleCont.getInstance().setupWorkScheduler();
    }
    return true;
  }
}