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
import rdb from '@ohos.data.relationalStore';
import { ValuesBucket } from '@ohos.data.ValuesBucket';
import LogUtils from '../../common/utils/LogUtils';
import BackupRestoreResult from '../../backup/copyData/BackupRestoreResult';
import dataRdb from '@ohos.data.relationalStore';
import ErrorUtils from './ErrorUtils';

const TAG = 'RdbStoreUtils: ';
export default class RdbStoreUtils {
  // 需要重试的错误码集合
  private static RETRY_ERROR_CODES = new Set<number>([
    14800024, 14800025, 14800026, 14800028, 14800029, 14800047
  ]);

  // 最大重试次数
  private static MAX_RETRY_COUNT = 6;
  // 每次重试间隔（ms）
  private static RETRY_INTERVAL_MS = 500;

  /**
   * 通用的重试函数，适用于插入、更新、删除等数据库操作
   * @param operation - 操作函数，接受重试参数并返回 Promise<number>
   * @param operationName - 操作方法，异常时记录在迁移报告
   * @returns Promise<number> - 返回操作结果
   * @throws { BusinessError } - 数据库操作异常
   */
  private static async retryOperation(operation: () => Promise<number>, operationName: string): Promise<number> {
    let retryCount = 0;
    let lastError: ErrorUtils = new ErrorUtils(0, '');
    while (retryCount < RdbStoreUtils.MAX_RETRY_COUNT) {
      try {
        const result = await operation();
        return result;
      } catch (e) {
        if (e.code && RdbStoreUtils.RETRY_ERROR_CODES.has(e.code)) {
          retryCount++;
          LogUtils.w(TAG, `[retryOperation] Retry #${retryCount}, error code: ${e.code}, message: ${e.message}`);
          lastError = e;
          await new Promise<void>((resolve) => setTimeout(resolve, RdbStoreUtils.RETRY_INTERVAL_MS));
        } else {
          // 非重试错误，直接抛出
          LogUtils.e(TAG, `[retryOperation] Non-retryable error: code:${e.code} ${e.message}`);
          throw new ErrorUtils(e.code, e.message);
        }
      }
    }
    // 重试次数耗尽，记录错误到迁移报告并返回 -1
    BackupRestoreResult.getInstance().insertOrUpdateBackupInfo(operationName,
      `code:${lastError.code} msg:${lastError.message}`);
    LogUtils.e(TAG, `retryOperation: code:${lastError.code} msg:${lastError.message}`);
    return -1;
  }

  /**
   * 插入操作
   * @param rdbStore - RdbStore 实例
   * @param table - 数据库表名
   * @param values - 插入的数据
   */
  public static async insert(rdbStore: rdb.RdbStore, table: string, values: ValuesBucket): Promise<number> {
    return RdbStoreUtils.retryOperation(async () => {
      return rdbStore.insert(table, values);
    }, 'insert');
  }

  /**
   * 批量插入
   * @param rdbStore - RdbStore 实例
   * @param table - 数据库表名
   * @param values - 插入的数据
   * @param conflict - 数据插入表中的冲突解决方法
   */
  public static async batchInsertWithConflictResolution(rdbStore: rdb.RdbStore, table: string,
    values: Array<ValuesBucket>, conflict: dataRdb.ConflictResolution): Promise<number> {
    return RdbStoreUtils.retryOperation(async () => {
      return rdbStore.batchInsertWithConflictResolution(table, values, conflict);
    }, 'batchInsertWithConflictResolution');
  }

  /**
   * 更新操作
   * @param rdbStore - RdbStore 实例
   * @param values - 更新的数据
   * @param predicates - 条件
   */
  public static async update(rdbStore: rdb.RdbStore, values: ValuesBucket,
    predicates: rdb.RdbPredicates): Promise<number> {
    return RdbStoreUtils.retryOperation(async () => {
      return rdbStore.update(values, predicates);
    }, 'update');
  }

  /**
   * 删除操作
   * @param rdbStore - RdbStore 实例
   * @param predicates - 条件
   */
  public static async delete(rdbStore: rdb.RdbStore, predicates: rdb.RdbPredicates): Promise<number> {
    return RdbStoreUtils.retryOperation(async () => {
      return rdbStore.delete(predicates);
    }, 'delete');
  }
}
