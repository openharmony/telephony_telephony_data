/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
import { StringUtil } from './StringUtil';
import rdb from '@ohos.data.relationalStore';

const TAG = 'DataBaseUtil';

class DataBaseUtil {

  public getPartFileName(resultSet: rdb.ResultSet): string {
    if (resultSet === undefined) {
      LogUtils.w(TAG, 'resultSet invalid.');
      return '';
    }
    let cl = resultSet.getString(resultSet.getColumnIndex('cl'));
    let data = resultSet.getString(resultSet.getColumnIndex('_data'));
    let name = resultSet.getString(resultSet.getColumnIndex('name'));
    if (!StringUtil.isEmpty(cl)) {
      return cl;
    }
    if (!StringUtil.isEmpty(name)) {
      return name;
    }
    let dataFn = StringUtil.getPathTail(data)
    if (!StringUtil.isEmpty(dataFn)) {
      return dataFn;
    }
    return '';
  }

  public getThirdPartFileName(resultSet: rdb.ResultSet): string {
    if (resultSet === undefined) {
      LogUtils.w(TAG, 'resultSet invalid.');
      return '';
    }
    let cl = resultSet.getString(resultSet.getColumnIndex('cl'));
    let data = resultSet.getString(resultSet.getColumnIndex('_data'));
    let name = resultSet.getString(resultSet.getColumnIndex('name'));
    let dataFn = StringUtil.getPathTail(data)
    if (!StringUtil.isEmpty(dataFn)) {
      return dataFn;
    }
    if (!StringUtil.isEmpty(cl)) {
      return cl;
    }
    if (!StringUtil.isEmpty(name)) {
      return name;
    }
    return '';
  }

}

// Singleton
export default new DataBaseUtil();