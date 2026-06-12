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

import type Want from '@ohos.application.Want';
import rpc from '@ohos.rpc';
import ServiceExtensionAbility from '@ohos.app.ability.ServiceExtensionAbility';
import BackupRestoreServiceStub from './stub/BackupRestoreServiceStub';

export default class BackupRestoreServiceAbility extends ServiceExtensionAbility {

  onCreate(want: Want): void {
    super.onCreate(want);
  }

  onDestroy(): void {
    super.onDestroy();
  }

  onRequest(want: Want, startId: number): void {
    super.onRequest(want, startId);
  }

  onConnect(want: Want): rpc.RemoteObject | Promise<rpc.RemoteObject> {
    return new BackupRestoreServiceStub('MmsBackupRestoreStub');
  }

  onDisconnect(want: Want): void | Promise<void> {
    return super.onDisconnect(want);
  }

  onReconnect(want: Want): void {
    super.onReconnect(want);
  }
}