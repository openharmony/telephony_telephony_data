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
import fs from '@ohos.file.fs';
import LogUtils from './LogUtils';
import { StringUtil } from './StringUtil';

const TAG = 'FileUtils';

/**
 * Handle file data
 */
export default class FileUtils {

  /**
   * Copy file util
   *
   * @param srcPath copy from this path
   * @param dstPath copy to this path
   * @returns if copy success
   */
  static copyFile(srcPath: string, dstPath: string): boolean {
    try {
      fs.copyFileSync(srcPath, dstPath);
      LogUtils.i(TAG, 'copyFile success');
      return true;
    } catch (error) {
      LogUtils.i(TAG, 'copyFile error: ' + JSON.stringify(error));
      return false;
    }
  }

  static copyDir(srcPath: string, dstPath: string): boolean {
    try {
      fs.copyDir(srcPath, dstPath, 1);
      LogUtils.i(TAG, 'copyDir success');
      return true;
    } catch (error) {
      LogUtils.i(TAG, 'copyDir error: ' + JSON.stringify(error));
      return false;
    }
  }

  static copyDirSync(srcPath: string, dstPath: string): boolean {
    try {
      fs.copyDirSync(srcPath, dstPath);
      LogUtils.i(TAG, 'copyDirSync success');
      return true;
    } catch (error) {
      LogUtils.i(TAG, 'copyDirSync error: ' + JSON.stringify(error));
      return false;
    }
  }

  static rmDirSync(path: string): boolean {
    let isExists: boolean = fs.accessSync(path);
    if (isExists) {
      try {
        fs.rmdirSync(path);
      } catch (error) {
        LogUtils.i(TAG, `next2ohos dir rm failed: ${error.code}`);
        return false;
      }
    }
    return true;
  }

  static isFileExists(path: string): boolean {
    if (StringUtil.isEmpty(path)) {
      return false;
    }
    let exist: boolean = false;
    try {
      exist = fs.accessSync(path);
    } catch (error) {
      LogUtils.e(TAG, `isFileExists error: ${error.message}`);
    }
    return exist;
  }
}