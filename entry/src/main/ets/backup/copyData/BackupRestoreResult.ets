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

import DotUtils from '../../common/utils/DotUtils';
import LogUtils from '../../common/utils/LogUtils';
import { StringUtil } from '../../common/utils/StringUtil';

const TAG = 'BackupRestoreResult: ';
const RESTORE_BACKUP_INFO: string = 'sms';

interface BackupInfo {
  backupInfo: string;
  successCount: number;
  duplicateCount: number;
  failedCount: number;
  details: string[];
}

interface ErrorInfo {
  type: 'ErrorInfo';
  errorCode: string;
  errorInfo: string;
}

interface CountInfo {
  type: 'CountInfo';
  infos: BackupInfo[];
}

interface ResultInfo {
  errorInfo?: ErrorInfo;
  countInfo?: CountInfo;
}

/**
 * 示例调用
 * const backupRestoreResult = BackupRestoreResult.getInstance();
 * 添加错误信息
 * backupRestoreResult.addErrorInfo('0', "");
 * 插入或更新备份信息
 * backupRestoreResult.insertOrUpdateBackupInfo("sms", successCounts, failCount, ["/1.jpg", "/xxx.jpg"]);
 * 返回格式化的恢复结果
 * backupRestoreResult.formatRestoreResult();
 */
export default class BackupRestoreResult {
  public static RESTORE_COMMON_ERROR: string = '13500099';
  public static RESTORE_SUCCESS: string = '0';
  private static instance: BackupRestoreResult;
  private resultInfo: ResultInfo = {};
  private constructor() {}

  /**
   * 获取单例实例
  */
  public static getInstance(): BackupRestoreResult {
    if (!BackupRestoreResult.instance) {
      BackupRestoreResult.instance = new BackupRestoreResult();
    }
    return BackupRestoreResult.instance;
  }

  /**
   * 添加错误信息，确保只存在一个 ErrorInfo
  */
  public addErrorInfo(errorCode: string, errorInfo: string): void {
    if (!this.resultInfo.errorInfo) {
      this.resultInfo.errorInfo = {
        type: 'ErrorInfo',
        errorCode: errorCode,
        errorInfo: errorInfo
      };
    }
  }

  /**
   * 添加或更新备份信息，确保只存在一个 CountInfo，并将 BackupInfo 添加到其中
  */
  public insertOrUpdateBackupInfo(backupInfo: string, details: string = '', successCount: number = 0,
    duplicateCount: number = 0, failedCount: number = 0): void {
    if (!this.resultInfo.countInfo) {
      this.resultInfo.countInfo = {
        type: 'CountInfo',
        infos: []
      };
    }
    // 检查是否已有相同 backupInfo 的记录
    const existing = this.resultInfo.countInfo.infos.find(item => item.backupInfo === backupInfo);
    if (existing) {
      // 更新现有记录的计数
      existing.successCount = successCount;
      existing.duplicateCount = duplicateCount;
      existing.failedCount = failedCount;
      // 如果 details 不为空，则添加到 existing 的 details 中（去重）
      if (details.length > 0 && !existing.details.includes(details) && existing.details.length < 50) {
        existing.details.push(details);
      }
    } else {
      const backupInfoObj: BackupInfo = {
        backupInfo: backupInfo,
        successCount: successCount,
        duplicateCount: duplicateCount,
        failedCount: failedCount,
        details: []
      };
      // 如果 details 为空数组，则不添加 details 属性
      if (details.length > 0) {
        backupInfoObj.details.push(details);
      }
      this.resultInfo.countInfo.infos.push(backupInfoObj);
    }
  }

  /**
   * 格式化返回结果
  */
  public formatRestoreResult(): string {
    let result = JSON.stringify({ resultInfo: [this.resultInfo.errorInfo, this.resultInfo.countInfo].filter(Boolean) },
      null, 4);
    DotUtils.reportCloneResult(result,
      this.resultInfo.errorInfo === undefined ? '' : this.resultInfo.errorInfo.errorCode);
    return result;
  }

  /**
   * 格式化备份返回结果
   */
  public formatBackupResult(totalCount: number): string {
    let countMap: Map<string, ESObject> = new Map();
    countMap.set('backupInfo', RESTORE_BACKUP_INFO);
    countMap.set('number', totalCount);
    let result = `[${JSON.stringify(StringUtil.Map2Rec(countMap))}]`;
    LogUtils.i(TAG, 'Backup result ' + result);
    return result;
  }

}