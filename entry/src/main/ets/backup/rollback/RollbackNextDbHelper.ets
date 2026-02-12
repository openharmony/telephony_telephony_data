/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
import rdb from '@ohos.data.relationalStore';
import { BackupExtensionContext } from '@kit.CoreFileKit';
import { common, contextConstant } from '@kit.AbilityKit';
import LogUtils from '../../common/utils/LogUtils';
import RollbackDbConstants from './RollbackDbConstants'
import { BusinessError } from '@kit.BasicServicesKit';
import { ValueType } from '@kit.ArkData';
import DbConstants from '../copyData/DbConstants';
import ErrorUtils from '../../common/utils/ErrorUtils';
import NewDbHelper from '../copyData/NewDbHelper';
import NextOldDBData from '../copyData/NextOldDBData';
import { rollbackOhosDbHelper } from './RollbackOhosDbHelper';
import { StringUtil } from '../../common/utils/StringUtil';

const TAG = '[Rollback-NextDbHelper] :';

class RollbackNextDbHelper extends NewDbHelper {
  private versionCode: number = 0;
  private telephoneMap: Map<string, number> = new Map();
  private addressesId: number = 1;
  private sessionIdMap: Map<number, number> = new Map();
  private sessionAddressesMap: Map<number, string> = new Map();
  private groupIdSet: Set<number> = new Set();

  public getTelephoneMap(): Map<string, number> {
    return this.telephoneMap;
  }

  public async querySessionTable(context: common.Context, index: number, callBack?: Function): Promise<number> {
    LogUtils.e(TAG, `[querySessionTable] start`)
    let failCount = 0;
    let successCount = 0;
    try {
      let sessionTable = new rdb.RdbPredicates(DbConstants.NEXT_OLD_SESSION_NAME);
      let rdbStore = await this.getRdbStore(context);
      if (rdbStore === undefined || rdbStore === null) {
        LogUtils.e(TAG, `[querySessionTable] failed : rdbStore === undefined`)
        if (callBack) {
          callBack(new ErrorUtils(400, 'rdbStore === undefined'));
        }
        return successCount;
      }
      let querySessionColumn = [NextOldDBData.session.id, NextOldDBData.session.time,
        NextOldDBData.session.telephone, NextOldDBData.session.content,
        NextOldDBData.session.contactsNum, NextOldDBData.session.smsType,
        NextOldDBData.session.unreadCount, NextOldDBData.session.sendingStatus,
        NextOldDBData.session.hasDraft, NextOldDBData.session.hasLock,
        NextOldDBData.session.messageCount, NextOldDBData.session.hasMms, NextOldDBData.session.hasAttachment];
      if (this.versionCode >= 1000021) {
        querySessionColumn.push(NextOldDBData.session.yellowPageId)
      }
      sessionTable = this.buildPredicatesInfo(sessionTable, NextOldDBData.session.id, index);
      let resultSet = await rdbStore.query(sessionTable, querySessionColumn);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'no raw thread found.');
        return successCount;
      }
      let count = resultSet.rowCount;
      do {
        let successFlag = await this.processSessionItem(resultSet, context);
        if (successFlag) {
          successCount++;
        } else {
          failCount++;
        }
      } while (resultSet.goToNextRow());
      resultSet.close();
      LogUtils.i(TAG, 'session Total Migrations count: ' + count + ',Successful Migrations successCount: ' + successCount + 'Failed Migrations failCount: ' + failCount);
    } catch (error) {
      LogUtils.w(TAG, 'querySessionTable error : ' + JSON.stringify(error));
      if (callBack) {
        callBack(new ErrorUtils(400, JSON.stringify(error)));
      }
    }
    return successCount;
  }

  public async queryMmsTable(context: common.Context, index: number, callback?: Function): Promise<number> {
    let failCount = 0;
    let successCount = 0;
    try {
      let mmsInfo = new rdb.RdbPredicates(DbConstants.NEXT_OLD_SMS_NAME);
      let rdbStore = await this.getRdbStore(context);
      if (rdbStore === undefined || rdbStore === null) {
        if (callback) {
          callback(new ErrorUtils(400, 'rdbStore === undefined'));
        }
        return successCount;
      }
      let queryMmsColumn = this.buildQueryInfo();
      mmsInfo = this.buildPredicatesInfo(mmsInfo, NextOldDBData.smsMmsInfo.msgId, index);
      let resultSet = await rdbStore.query(mmsInfo, queryMmsColumn);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        return successCount;
      }
      let countMms = resultSet.rowCount;
      do {
        let values: rdb.ValuesBucket | undefined = this.getSmsMmsInfoItem(resultSet);
        let newRdbStore = await rollbackOhosDbHelper.getRdbStore(context);
        if (newRdbStore === null) {
          LogUtils.e(TAG, 'new rdbStore can not null');
          return -1;
        }
        if (values) {
          await newRdbStore.insert(RollbackDbConstants.SMS_TB_NAME, values);
        }
      } while (resultSet.goToNextRow())
      resultSet.close();
      LogUtils.i(TAG, 'mms Total Migrations count: ' + countMms + ',success: ' + successCount + ' fail: ' + failCount);
    } catch (error) {
      LogUtils.w(TAG, 'query MmsInfo error : ' + JSON.stringify(error));
      if (callback) {
        callback(new ErrorUtils(400, JSON.stringify(error)));
      }
    }
    return successCount;
  }

  private getSmsMmsInfoItem(resultSet: rdb.ResultSet): rdb.ValuesBucket | undefined {
    let msgType = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.msgType));
    let smsType = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.smsType));
    let groupId = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.groupId));
    if (msgType === 1 || smsType === 99) {
      return undefined;
    }
    let oldSessionId = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.sessionId));
    let newSessionId = this.sessionIdMap.get(oldSessionId);
    if (newSessionId === undefined) {
      LogUtils.e(TAG, `sessionId can not null`);
      return undefined;
    }
    let address: string = '';
    if (this.sessionAddressesMap.get(newSessionId) != undefined) {
      let receiverNumber = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.receiverNumber));
      let senderNumber = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.senderNumber));
      address = StringUtil.isEmpty(receiverNumber) ? senderNumber : receiverNumber;
    } else {
      address = '';
    }

    let type: number = 1;
    let msgState: number = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.msgState));
    if (msgState === 2) {
      type = 5;
    } else if (msgState === 0) {
      type = 2;
    } else if (msgState === 3) {
      type = 3;
    } else if (msgState === 1) {
      type = 4;
    }
    let isSender: number = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.isSender));
    if (isSender === 1) {
      type = 1;
    }
    let value: rdb.ValuesBucket = {
      'thread_id': newSessionId,
      'address': address,
      'sub_id': resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.slotId)),
      'type': type,
      'date': resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.startTime)),
      'date_sent': resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.endTime)),
      'body': resultSet.getString(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.msgContent)),
      'read': resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.isRead)),
      'group_id': groupId
    }
    return value;
  }

  private buildQueryInfo() {
    let queryMmsColumn = [NextOldDBData.smsMmsInfo.msgId, NextOldDBData.smsMmsInfo.slotId,
      NextOldDBData.smsMmsInfo.receiverNumber, NextOldDBData.smsMmsInfo.senderNumber,
      NextOldDBData.smsMmsInfo.isSender, NextOldDBData.smsMmsInfo.msgType, NextOldDBData.smsMmsInfo.smsType,
      NextOldDBData.smsMmsInfo.startTime, NextOldDBData.smsMmsInfo.endTime, NextOldDBData.smsMmsInfo.msgState,
      NextOldDBData.smsMmsInfo.msgTitle, NextOldDBData.smsMmsInfo.msgContent,
      NextOldDBData.smsMmsInfo.operatorServiceNumber, NextOldDBData.smsMmsInfo.isLock,
      NextOldDBData.smsMmsInfo.isCollect, NextOldDBData.smsMmsInfo.isRead, NextOldDBData.smsMmsInfo.sessionType,
      NextOldDBData.smsMmsInfo.retryNumber, NextOldDBData.smsMmsInfo.sessionId, NextOldDBData.smsMmsInfo.groupId,
      NextOldDBData.smsMmsInfo.deviceId, NextOldDBData.smsMmsInfo.isSubsection, NextOldDBData.smsMmsInfo.isSendReport,
      NextOldDBData.smsMmsInfo.msgCode, NextOldDBData.smsMmsInfo.isAdvancedSecurity,
      NextOldDBData.smsMmsInfo.rcsId, NextOldDBData.smsMmsInfo.expiresTime,
      NextOldDBData.smsMmsInfo.mmsPdu];
    if (this.versionCode >= 1000019) {
      // Compatibility processing is added for phone_number and format_phone_number in version 1000019.
      queryMmsColumn.push(NextOldDBData.smsMmsInfo.phoneNumber, NextOldDBData.smsMmsInfo.formatPhoneNumber);
    }
    if (this.versionCode >= 1000021) {
      queryMmsColumn.push(NextOldDBData.smsMmsInfo.isBlocked, NextOldDBData.smsMmsInfo.blockedReason,
        NextOldDBData.smsMmsInfo.blockedSources, NextOldDBData.smsMmsInfo.blockedType,
        NextOldDBData.smsMmsInfo.blockedTypeText, NextOldDBData.smsMmsInfo.riskUrlBody,
        NextOldDBData.smsMmsInfo.hasBeReportedAsSpam, NextOldDBData.smsMmsInfo.isReport,
        NextOldDBData.smsMmsInfo.detectResContent, NextOldDBData.smsMmsInfo.msgCodeStr);
    }
    return queryMmsColumn;
  }

  private buildPredicatesInfo(predicatesInfo: rdb.RdbPredicates, id: string, index: number): rdb.RdbPredicates {
    predicatesInfo.orderByAsc(id)
      .limitAs(DbConstants.QUERY_PAGE_NUM)
      .offsetAs(index);
    return predicatesInfo;
  }

  private async processSessionItem(resultSet: rdb.ResultSet, context: Context): Promise<boolean> {
    let telephony = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.session.telephone));
    let address: string = '';
    let recipientIds: string = '';
    telephony.split(',').forEach(telephone => {
      if (!this.telephoneMap.get(telephone)) {
        this.telephoneMap.set(telephone, this.addressesId);
        this.addressesId++;
      }
      address += (telephone + ' ');
      recipientIds += (this.telephoneMap.get(telephone) + ' ');
    });
    let sessionId = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.session.id));
    let newRdbStore = await rollbackOhosDbHelper.getRdbStore(context);
    if (newRdbStore === null) {
      LogUtils.e(TAG, 'new rdbStore can not null');
      return false;
    }
    address = address.trim();
    recipientIds = recipientIds.trim();
    let valuesBucket: rdb.ValuesBucket = {
      'recipient_ids': recipientIds,
      'address': address
    }
    let threadId = await newRdbStore.insert(RollbackDbConstants.THREADS_TB_NAME, valuesBucket);
    if (threadId !== -1) {
      this.sessionIdMap.set(sessionId, threadId);
      this.sessionAddressesMap.set(threadId, address);
      return true;
    }
    return false;
  }
}

export const rollbackNextDbHelper: RollbackNextDbHelper = new RollbackNextDbHelper();