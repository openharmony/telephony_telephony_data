/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

/**
 * log utils class
 */
export class LogUtils {
  d(tag: string, msg: string): void {
    console.debug('[TelephonyData:]' + tag + ':' + msg);
  }

  i(tag: string, msg: string): void {
    console.info('[TelephonyData:]' + tag + ':' + msg);
  }

  w(tag: string, msg: string): void {
    console.warn('[TelephonyData:]' + tag + ':' + msg);
  }

  e(tag: string, msg: string): void {
    console.error('[TelephonyData:]' + tag + ':' + msg);
  }
}

let mLogUtil = new LogUtils();

export default mLogUtil as LogUtils;

