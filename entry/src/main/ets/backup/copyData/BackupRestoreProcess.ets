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

@Sendable
export default class BackupRestoreProgress {
  private progress: string = '';
  private totalCount: number = 0;
  private processCount: number = 0;
  private addressCount: number = 0;
  private threadsCount: number = 0;
  private rcsThreadsCount: number = 0;
  private smsCount: number = 0;
  private pduCount: number = 0;
  private partCount: number = 0;
  private rcsChatCount: number = 0;
  private rcsGroupCount: number = 0;
  private maapCount: number = 0;
  private maapSpecificCount: number = 0;
  private protocolCount: number = 0;
  private subsectionCount: number = 0;
  private favoriteCount: number = 0;
  private favSmsCount: number = 0;
  private favMmsCount: number = 0;
  private checkSessionProcess: number = 1;
  private checkSmsProcess: number = 1;
  private delInvalidDraftCount: number = 0;
  private invalidItemCount: number = 0;
  private unSupportItemCount: number = 0;
  public static readonly delInvalidDraftRatio: number = 0.001;

  public getCheckSessionCount(): number {
    return this.checkSessionProcess;
  }

  public getCheckSmsCount(): number {
    return this.checkSmsProcess;
  }

  public setTotalCount(value: number) {
    this.totalCount = value;
  }

  public getTotalCount(): number {
    return this.totalCount;
  }

  public updateProcessCount(value: number) {
    this.processCount = value;
  }

  public getProcessCount(): number {
    return this.processCount;
  }

  public getAddressCount(): number {
    return this.addressCount;
  }

  public getThreadCount(): number {
    return this.threadsCount;
  }

  public getRcsThreadCount(): number {
    return this.rcsThreadsCount;
  }

  public getSmsCount(): number {
    return this.smsCount;
  }

  public getPduCount(): number {
    return this.pduCount;
  }

  public getPartCount(): number {
    return this.partCount;
  }

  public getRcsChatCount(): number {
    return this.rcsChatCount;
  }

  public getRcsGroupCount(): number {
    return this.rcsGroupCount;
  }

  public getMaapCount(): number {
    return this.maapCount;
  }

  public getMaapSpecificCount(): number {
    return this.maapSpecificCount;
  }

  public getProtocolCount(): number {
    return this.protocolCount;
  }

  public getSubsectionCount(): number {
    return this.subsectionCount;
  }

  public getFavorateCount(): number {
    return this.favoriteCount;
  }

  public getFavSmsCount(): number {
    return this.favSmsCount;
  }

  public getFavMmsCount(): number {
    return this.favMmsCount;
  }

  public getDelInvalidDraftCount(): number {
    return this.delInvalidDraftCount;
  }

  public setAddressCount(value: number) {
    this.addressCount = value;
  }

  public setThreadCount(value: number) {
    this.threadsCount = value;
  }

  public setRcsThreadCount(value: number) {
    this.rcsThreadsCount = value;
  }

  public setSmsCount(value: number) {
    this.smsCount = value;
  }

  public setPduCount(value: number) {
    this.pduCount = value;
  }

  public setPartCount(value: number) {
    this.partCount = value;
  }

  public setRcsChatCount(value: number) {
    this.rcsChatCount = value;
  }

  public setRcsGroupCount(value: number) {
    this.rcsGroupCount = value;
  }

  public setMaapCount(value: number) {
    this.maapCount = value;
  }

  public setMaapSpecificCount(value: number) {
    this.maapSpecificCount = value;
  }

  public setProtocolCount(value: number) {
    this.protocolCount = value;
  }

  public setSubsectionCount(value: number) {
    this.subsectionCount = value;
  }

  public setFavoriteCount(value: number) {
    this.favoriteCount = value;
  }

  public setFavSmsCount(value: number) {
    this.favSmsCount = value;
  }

  public setFavMmsCount(value: number) {
    this.favMmsCount = value;
  }

  public setDelInvalidDraftCount(value: number) {
    this.delInvalidDraftCount = value;
  }

  public setInvalidItemCount(value: number) {
    this.invalidItemCount = value;
  }

  public getInvalidItemCount(): number {
    return this.invalidItemCount;
  }

  public setUnSupportItemCount(value: number) {
    this.unSupportItemCount = value;
  }

  public getUnSupportItemCount(): number {
    return this.unSupportItemCount;
  }

  public getAddressMigrateProgress(): number {
    return this.addressCount;
  }

  public getThreadMigrateProgress(): number {
    return this.addressCount + this.threadsCount;
  }

  public getRcsThreadMigrateProgress(): number {
    return this.addressCount + this.threadsCount + this.rcsThreadsCount;
  }

  public getSmsMigrateProgress(): number {
    return this.addressCount + this.threadsCount + this.rcsThreadsCount + this.smsCount;
  }

  public getPduMigrateProgress(): number {
    return this.addressCount + this.threadsCount + this.rcsThreadsCount + this.smsCount + this.pduCount;
  }

  public getPartMigrateProgress(): number {
    return this.addressCount + this.threadsCount + this.rcsThreadsCount + this.smsCount + this.pduCount +
      this.partCount;
  }

  public getRcsChatMigrateProgress(): number {
    return this.addressCount + this.threadsCount + this.rcsThreadsCount + this.smsCount + this.pduCount +
      this.partCount + this.rcsChatCount;
  }

  public getRcsGroupMigrateProgress(): number {
    return this.addressCount + this.threadsCount + this.rcsThreadsCount + this.smsCount + this.pduCount +
      this.partCount + this.rcsChatCount + this.rcsGroupCount;
  }

  public getMaapMigrateProgress(): number {
    return this.addressCount + this.threadsCount + this.rcsThreadsCount + this.smsCount + this.pduCount +
      this.partCount + this.rcsChatCount + this.rcsGroupCount + this.maapCount;
  }

  public getFavSmsMigrateProgress(): number {
    return this.addressCount + this.threadsCount + this.rcsThreadsCount + this.smsCount + this.pduCount +
      this.partCount + this.rcsChatCount + this.rcsGroupCount + this.maapCount + this.favSmsCount;
  }

  public getFavMmsMigrateProgress(): number {
    return this.addressCount + this.threadsCount + this.rcsThreadsCount + this.smsCount + this.pduCount +
      this.partCount + this.rcsChatCount + this.rcsGroupCount + this.maapCount + this.favSmsCount +
      this.favMmsCount;
  }

  public getInvalidMigrateProgress(): number {
    return this.addressCount + this.threadsCount + this.rcsThreadsCount + this.smsCount + this.pduCount +
      this.partCount + this.rcsChatCount + this.rcsGroupCount + this.maapCount + this.favSmsCount +
      this.favMmsCount + this.invalidItemCount;
  }

  public getUnSupportItemMigrateProgress(): number {
    return this.addressCount + this.threadsCount + this.rcsThreadsCount + this.smsCount + this.pduCount +
      this.partCount + this.rcsChatCount + this.rcsGroupCount + this.maapCount + this.favSmsCount +
      this.favMmsCount + this.invalidItemCount + this.unSupportItemCount;
  }

  public getAddressCloneProgress(): number {
    return this.addressCount;
  }

  public getThreadCloneProgress(): number {
    return this.addressCount + this.threadsCount;
  }

  public getSmsCloneProgress(): number {
    return this.addressCount + this.threadsCount + this.smsCount;
  }

  public getPduCloneProgress(): number {
    return this.addressCount + this.threadsCount + this.smsCount + this.pduCount;
  }

  public getPartCloneProgress(): number {
    return this.addressCount + this.threadsCount + this.smsCount + this.pduCount + this.partCount;
  }

  public getRcsChatCountCloneProgress(): number {
    return this.addressCount + this.threadsCount + this.smsCount + this.pduCount + this.partCount +
      this.rcsChatCount;
  }

  public getRcsGroupCountCloneProgress(): number {
    return this.addressCount + this.threadsCount + this.smsCount + this.pduCount + this.partCount +
      this.rcsChatCount + this.rcsGroupCount;
  }

  public getThreadNextCloneProgress(): number {
    return this.threadsCount;
  }

  public getSmsNextCloneProgress(): number {
    return this.threadsCount + this.smsCount;
  }

  public getRcsChatNextCloneProgress(): number {
    return this.threadsCount + this.smsCount + this.rcsChatCount;
  }

  public getPduNextCloneProgress(): number {
    return this.threadsCount + this.smsCount + this.rcsChatCount + this.pduCount;
  }

  public getPartNextCloneProgress(): number {
    return this.threadsCount + this.smsCount + this.rcsChatCount + this.pduCount + this.partCount;
  }

  public getProtocolNextCloneProgress(): number {
    return this.threadsCount + this.smsCount + this.rcsChatCount + this.pduCount + this.partCount +
      this.protocolCount;
  }

  public getSubNextCloneProgress(): number {
    return this.threadsCount + this.smsCount + this.rcsChatCount + this.pduCount + this.partCount +
      this.protocolCount + this.subsectionCount;
  }

  public getFavNextCloneProgress(): number {
    return this.threadsCount + this.smsCount + this.rcsChatCount + this.pduCount + this.partCount +
      this.protocolCount + this.subsectionCount + this.favoriteCount;
  }

  public getMaapNextCloneProgress(): number {
    return this.threadsCount + this.smsCount + this.rcsChatCount + this.pduCount + this.partCount +
      this.protocolCount + this.subsectionCount + this.favoriteCount + this.maapCount;
  }

  public getSpecificpNextCloneProgress(): number {
    return this.threadsCount + this.smsCount + this.rcsChatCount + this.pduCount + this.partCount +
      this.protocolCount + this.subsectionCount + this.favoriteCount + this.maapCount + this.maapSpecificCount;
  }

  public getProgress(): string {
    if (this.processCount > this.totalCount) {
      LogUtils.e('BackupRestoreProgress', 'Progress is greater than total');
      this.processCount = this.totalCount;
    }
    this.progress = `{"progressInfo":[{"name":"telephonydataability","processed": ${this.processCount},"total": ${this.totalCount}}]}`;
    return this.progress;
  }

}