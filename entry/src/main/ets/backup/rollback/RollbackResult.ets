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

import LogUtils from '../../common/utils/LogUtils';

const TAG = 'RollbackResult: ';
const REQUIREMENT_VERSION: number = 1;
export default class RollbackResult {
  public static RESTORE_COMMON_ERROR: number = 13500099;
  public static RESTORE_SUCCESS: number = 0;

  private _errCode: number = RollbackResult.RESTORE_SUCCESS;

  public setErrCode(value: number) {
    this._errCode = value;
  }

  private _errInfo: string = '';

  public setErrInfo(value: string) {
    this._errInfo = value;
  }

  public formatRollbackResult(): string {
    let compatibilityInfo = `{"type": "CompatibilityInfo","next2ohos": ${REQUIREMENT_VERSION}}`;
    let errorInfo = `{"type": "ErrorInfo","errorCode": "${this._errCode}","errorInfo": "${this._errInfo}"}`;
    let result = `{"resultInfo": [${errorInfo},${compatibilityInfo}]}`;
    LogUtils.i(TAG, 'rollback result ' + result);
    return result;
  }
}