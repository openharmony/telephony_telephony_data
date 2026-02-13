/**
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

import LogUtils from '../common/utils/LogUtils';
import CommonEventManager from '@ohos.commonEventManager';
import Base from '@ohos.base';

const TAG = 'BackupCommonEvent';

export default class BackupCommonEvent {

  // Sending the common event of refreshing the conversion list after cloning is complete
  static sendRefreshEvent() {
    try {
      CommonEventManager.publish('refreshConversionList', (err) => {
        if (err) {
          LogUtils.e(TAG, `publish refresh event failed, code is ${err.code}, message is ${err.message}`);
        }
      });
    } catch (error) {
      let err:Base.BusinessError = error as Base.BusinessError;
      LogUtils.e(TAG, `publish refresh refresh event failed, code is ${err.code}, message is ${err.message}`);
    }
  }
}
