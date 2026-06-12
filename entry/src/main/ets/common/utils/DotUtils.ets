/**
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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
import { hiSysEvent } from '@kit.PerformanceAnalysisKit';
import { BusinessError } from '@kit.BasicServicesKit';
import LogUtils from './LogUtils';

const TAG = 'DotUtils';

/**
 * Handle file data
 */
export default class DotUtils {
  public static reportCloneResult(formatReport: string, result: string) {
    try {
      let customizedParams: Record<string, string | number> = {
        'PNAMEID': 'com.ohos.telephonydataability',
        'PVERSIONID': '1.0.32',
        'RECOVERY_REPORT': formatReport,
        'RECOVERY_RESULT': result,
      };
      let eventInfo: hiSysEvent.SysEventInfo = {
        domain: 'MMS_UE',
        name: 'SMS_DATA_CLONE',
        eventType: hiSysEvent.EventType.BEHAVIOR,
        params: customizedParams
      };
      hiSysEvent.write(eventInfo, (err: BusinessError) => {
        LogUtils.i(TAG, `hiSysEvent success`);
      });
    } catch (err) {
      LogUtils.e(TAG, `error code: ${(err as BusinessError).code}, error msg: ${(err as BusinessError).message}`);
    }
  }
}