/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

import LogUtils from './LogUtils';
import preferences from '@ohos.data.preferences';
import { contextConstant } from '@kit.AbilityKit';

const TAG: string = 'GskvSharedPreferencesUtils';

// 一个进程写， 多个进程读。可以使用此方案。路径在 /data/storage/el1/base/preferences/telephony_data_sms_preferences
export class GskvSharedPreferencesUtils {
  public static isDbMigrationSuccess(context: Context): boolean {
    let dataPreferences: preferences.Preferences | null = null;
    const key = 'move_to_el2_result';
    let result = -1;
    let options: preferences.Options = {
      name: 'telephony_data_sms_preferences',
    };
    try {
      let el1Context = context.getApplicationContext().createAreaModeContext(contextConstant.AreaMode.EL1)
      preferences.removePreferencesFromCacheSync(el1Context, options); // 读之前，推荐删除缓存。
      dataPreferences = preferences.getPreferencesSync(el1Context, options);
      result = dataPreferences.getSync(key, -1) as number; // -1 表示文件不存在
      LogUtils.i(TAG, `isDbMigrationSuccess result = ${result}`);
    } catch (error) {
      LogUtils.i(TAG, `isDbMigrationSuccess failed error code: ${error.code} error name: ${error.message}`);
    }
    return result === 0; // 0 表示迁移成功
  }
}