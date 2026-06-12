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

import data_preferences from '@ohos.data.preferences';
import { BusinessError } from '@ohos.base'
import LogUtils from './LogUtils';

const NAME: string = 'DB_PREFERENCES';
const TAG: string = 'SharedPreferencesUtils';

class SharedPreferencesUtils {
  private mPreferences?: data_preferences.Preferences | null;
  private context?: Context;

  constructor() {
  }

  /*
   * init if Call From serviceAbility context is Null
   * @param ctx Context used for dataShare
   */
  init(ctx: Context) {
    if (ctx == null) {
      LogUtils.e(TAG, 'init ctx is null');
      return
    }
    this.context = ctx;
    this.mPreferences = this.getPreferences();
    LogUtils.i(TAG, 'SharedPreferencesUtils init');
  }

  clear() {
    this.mPreferences?.clear();
    this.mPreferences?.flush();
  }

  /**
   * getFromPreferences
   *
   * @param key to be get
   * @param defValue get default to Preferences
   * @return the value get from Preferences
   */
  public getFromPreferences(key: string, defValue: data_preferences.ValueType):data_preferences.ValueType {
    let preferences: data_preferences.Preferences | null = this.getPreferences();
    let value: data_preferences.ValueType = defValue;
    if (null == preferences) {
      LogUtils.i(TAG, 'getFromPreferences is null');
      return defValue;
    }
    try {
      value = preferences.getSync(key, defValue);
      LogUtils.i(TAG, 'getFromPreferences Succeeded');
    } catch (err) {
      let e: BusinessError = err as BusinessError;
      LogUtils.e(TAG, 'Failed to get value code : ' + e.code + ' , message : ' + e.message);
    }
    return value;
  }

  /**
   * saveToPreferences
   *
   * @param key save to Preferences
   * @param value save to Preferences
   */
  public saveToPreferences(key: string, value: data_preferences.ValueType) {
    let preferences: data_preferences.Preferences | null = this.getPreferences();
    if (null != preferences) {
      try {
        preferences.putSync(key, value);
        LogUtils.i(TAG, 'saveToPreferences put success key');
      } catch (err) {
        let e: BusinessError = err as BusinessError;
        LogUtils.e(TAG, 'saveToPreferences failed code : ' + e.code + ' , message : ' + e.message);
      }
      preferences.flush()
    } else {
      LogUtils.e(TAG, 'saveToPreferences preferences is null');
    }
  }

  private getPreferences() {
    if (this.mPreferences) {
      LogUtils.i(TAG, 'has Preferences');
      return this.mPreferences;
    }
    try {
      let preferences: data_preferences.Preferences = data_preferences.getPreferencesSync(this.context, {name: NAME});
      this.mPreferences = preferences;
      LogUtils.i(TAG, 'Preferences init');
      return preferences;
    } catch (error) {
      let err: BusinessError = error as BusinessError;
      LogUtils.e(TAG, 'getPreferences with error message: ' + err.message + ', error code: ' + err.code);
    }
    return null;
  }
}

export default new SharedPreferencesUtils();