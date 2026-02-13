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

import rdb from '@ohos.data.relationalStore';
import { ValuesBucket } from '@ohos.data.ValuesBucket';
import LogUtils from '../../common/utils/LogUtils';
import DbConstants from './DbConstants';
import { newDbHelper } from './NewDbHelper';
import common from '@ohos.app.ability.common';
import SessionItem from '../model/bean/SessionItem';
import SmsMmsInfoItem from '../model/bean/SmsMmsInfoItem';
import OldDBData from './OldDBData';
import MmsPartItem from '../model/bean/MmsPartItem';
import { StringUtil } from '../../common/utils/StringUtil';
import SharedPreferencesUtils from '../../common/utils/SharedPreferencesUtils';
import RcsInfoItem from '../model/bean/RcsInfoItem';
import NextOldDBData from '../copyData/NextOldDBData';
import HashMap from '@ohos.util.HashMap';
import LooseObject from '../copyData/LooseObject';
import BackupRestoreProgress from '../copyData/BackupRestoreProcess';
import BackupRestoreResult from '../copyData/BackupRestoreResult';
import FavorateInfoItem from '../model/bean/FavorateInfoItem';
import TelephoneUtil from '../../common/utils/TelephoneUtil';

const TAG = 'DbHelper';
const RAW_THREAD_ID_MAP = new Map<number, Array<number>>();
const RAW_RCS_THREAD_ID_MAP = new Map<number, Array<number>>();
const RAW_PDU_ID_MAP = new Map<number, Array<number>>();
const THREAD_PHONE_MAP = new Map<number, Array<string>>();
let MAX_GROUP_ID: number = 0;
const GROUP_ID_MAP = new Map<number, number>();
let GROUP_ID: number = 0;
const INFO_ID_GROUP = new Map<number, number>();
const RCS_ID_GROUP = new Map<number, number>();
const MMS_ID_RECORDING_TIME_MAP = new HashMap<number, string>();
const MMS_ID_TEXT_MAP = new HashMap<number, string>();
let ADDRESS_ID_MAP = new HashMap<number, string>();
const MAIN_USER_ID: number = 100;
const SESSION_NEED_UPDATE: number[] = [];
const RAW_RCS_ID_MAP = new Map<number, number>();
const RAW_MSG_ID_MAP = new Map<number, number>();

interface detectResContentData {
  entity: never[];
  code: number;
  content: string;
  bundleName: string;
}

interface needUpdateIds {
  msgNeedUpdate: number | undefined;
  rcsNeedUpdate: number | undefined;
}

interface partDataNeedInsert {
  partItems: MmsPartItem[];
}

interface favKeyFiled {
  type: number;
  locationPath: string;
  recordTime: string;
}

class DbHelper {
  private doThreadFailed = 0;
  private doRcsThreadFailed = 0;
  private doMmsFailed = 0;
  private doPduFailed = 0;
  private doPartFailed = 0;
  private doRcsChatFailed = 0;
  private doRcsGroupFailed = 0;
  private doRcsChatbotsFailed = 0;
  private doFavSmsFailed = 0;
  private doFavMmsFailed = 0;

  private rdbStoreHelper?: rdb.RdbStore;
  private rdbMaapStoreHelper?: rdb.RdbStore;
  private versionName: string = '';
  private unSupportCountFav = 0;
  private unrelatedFavSms = 0;
  private unrelatedFavMms = 0;

  constructor() {
  }

  public async queryThreadsCount(context: common.Context, localId: number): Promise<number> {
    if (context == null) {
      LogUtils.w(TAG, `queryThreadsCount failed by null context`)
      return Promise.resolve(0);
    }
    let count = 0;
    try {
      let predicatesInfo = new rdb.RdbPredicates(DbConstants.OLD_SESSION_TABLE_NAME);
      let rdbStore = await this.initDB(context);
      if (rdbStore === undefined) {
        LogUtils.e(TAG, `[queryThreadsCount] failed : rdbStore === undefined`)
        return Promise.resolve(0);
      }
      let queryColumn = ['count(_id)'];
      if (localId == MAIN_USER_ID) {
        predicatesInfo.equalTo('privacy_mode', 0)
      }
      let resultSet = await rdbStore.query(predicatesInfo, queryColumn);
      if (resultSet === undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'query no threads found.');
        return Promise.resolve(0);
      }
      count = resultSet.getLong(0);
      LogUtils.i(TAG, '[queryThreadsCount] count : ' + count);
      resultSet.close();
    } catch (error) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `queryThreadsCount code:${error.code} msg:${error.message}`);
      LogUtils.e(TAG, '[queryThreadsCount] error : ' + JSON.stringify(error))
    }
    return Promise.resolve(count);
  }

  private buildThreadsInfoQueryColumn(): string[] {
    return [OldDBData.threads.id,
          OldDBData.threads.date,
          OldDBData.threads.recipientIds,
          OldDBData.threads.snippet,
          OldDBData.threads.type,
          OldDBData.threads.unreadCount,
          OldDBData.threads.messageCount,
          OldDBData.threads.hasAttachment,
          OldDBData.threads.numberType,
          OldDBData.threads.error,
          OldDBData.threads.snippetCs,
          OldDBData.threads.snippetPrivacy,
          OldDBData.threads.datePrivacy,
          OldDBData.threads.privacyMode]
  }

  private buildFavSmsQueryColumn(): string[] {
    return [OldDBData.favSms.id,
      OldDBData.favSms.threadId,
      OldDBData.favSms.originId,
      OldDBData.favSms.dateFavadd,
      OldDBData.favSms.address,
      OldDBData.favSms.person,
      OldDBData.favSms.date,
      OldDBData.favSms.dateSent,
      OldDBData.favSms.protocol,
      OldDBData.favSms.read,
      OldDBData.favSms.status,
      OldDBData.favSms.type,
      OldDBData.favSms.replyPathPresent,
      OldDBData.favSms.subject,
      OldDBData.favSms.body,
      OldDBData.favSms.serviceCenter,
      OldDBData.favSms.locked,
      OldDBData.favSms.subId,
      OldDBData.favSms.networkType,
      OldDBData.favSms.errorCode,
      OldDBData.favSms.seen,
      OldDBData.favSms.privacyMode,
      OldDBData.favSms.addrBody,
      OldDBData.favSms.timeBody,
      OldDBData.favSms.riskUrlBody,
      OldDBData.favSms.isSecret]
  }

  private buildFavPduQueryColumn(): string[] {
    return [OldDBData.favPdu.id,
      OldDBData.favPdu.threadId,
      OldDBData.favPdu.originId,
      OldDBData.favPdu.dateFavadd,
      OldDBData.favPdu.date,
      OldDBData.favPdu.dateSent,
      OldDBData.favPdu.msgBox,
      OldDBData.favPdu.read,
      OldDBData.favPdu.mId,
      OldDBData.favPdu.sub,
      OldDBData.favPdu.subCs,
      OldDBData.favPdu.ctT,
      OldDBData.favPdu.ctL,
      OldDBData.favPdu.exp,
      OldDBData.favPdu.mCls,
      OldDBData.favPdu.mType,
      OldDBData.favPdu.v,
      OldDBData.favPdu.mSize,
      OldDBData.favPdu.pri,
      OldDBData.favPdu.rr,
      OldDBData.favPdu.rptA,
      OldDBData.favPdu.respSt,
      OldDBData.favPdu.st,
      OldDBData.favPdu.trId,
      OldDBData.favPdu.retrSt,
      OldDBData.favPdu.retrTxt,
      OldDBData.favPdu.retrTxtCs,
      OldDBData.favPdu.readStatus,
      OldDBData.favPdu.ctCls,
      OldDBData.favPdu.respTxt,
      OldDBData.favPdu.dTm,
      OldDBData.favPdu.dRpt,
      OldDBData.favPdu.locked,
      OldDBData.favPdu.seen,
      OldDBData.favPdu.subId,
      OldDBData.favPdu.networkType,
      OldDBData.favPdu.textOnly,
      OldDBData.favPdu.privacyMode]
  }

  private buildFavAddrQueryColumn(): string[] {
    return [OldDBData.favAddr.id,
      OldDBData.favAddr.msgId,
      OldDBData.favAddr.contactId,
      OldDBData.favAddr.address,
      OldDBData.favAddr.type,
      OldDBData.favAddr.charset]
  }

  private buildFavPartQueryColumn(): string[] {
    return [OldDBData.favPart.id,
      OldDBData.favPart.mid,
      OldDBData.favPart.seq,
      OldDBData.favPart.ct,
      OldDBData.favPart.name,
      OldDBData.favPart.chset,
      OldDBData.favPart.cd,
      OldDBData.favPart.fn,
      OldDBData.favPart.cid,
      OldDBData.favPart.cl,
      OldDBData.favPart.cttS,
      OldDBData.favPart.cttT,
      OldDBData.favPart.data,
      OldDBData.favPart.text]
  }

  private setFavorateAddrAndType(favItem: FavorateInfoItem, resultSet: rdb.ResultSet, msgBox: number): boolean {
    if (resultSet == undefined || !resultSet.goToFirstRow()) {
      LogUtils.w(TAG, 'setFavorateAddr fail, resultSet invalid');
      return false;
    }
    do {
      let address = resultSet.getString(resultSet.getColumnIndex(OldDBData.favAddr.address));
      let type = resultSet.getLong(resultSet.getColumnIndex(OldDBData.favAddr.type));
      // receiver_number: msg_box=2 取 addr表中type=151的address
      // sender_number:   msg_box=1 取 addr表中type=137的address
      if (msgBox == 2 && type == 151) {
        favItem.setReceiverNumber(address);
        favItem.setIsSender(0);
        favItem.setSmsType(TelephoneUtil.judgeIsInfoMsg(address) ? 1 :
          (this.isChatbotNumber(address?.toString()) ? 1 : 0));
        return true;
      }
      if (msgBox == 1 && type == 137) {
        favItem.setSenderNumber(address);
        favItem.setIsSender(1);
        favItem.setSmsType(TelephoneUtil.judgeIsInfoMsg(address) ? 1 :
          (this.isChatbotNumber(address?.toString()) ? 1 : 0));
        return true;
      }
    } while (resultSet.goToNextRow());
    return false;
  }

  private setFavKeyField(favItem: FavorateInfoItem, partResultSet: rdb.ResultSet): boolean {
    let minType: number = Number.MAX_SAFE_INTEGER;
    let minLocation: string = '';
    let recordingTime: string | undefined = undefined;
    do {
      let ct = partResultSet.getString(partResultSet.getColumnIndex(OldDBData.part.ct));
      let data = partResultSet.getString(partResultSet.getColumnIndex(OldDBData.part.data));
      let text = partResultSet.getString(partResultSet.getColumnIndex(OldDBData.part.text));
      if (ct === 'application/smil') {
        recordingTime = this.extractRecordingTime(text);
      }
      let partType = this.transContentTypeToPartType(ct);
      if (partType === -1) {
        return false;
      }
      if (partType !== 0 && partType < minType) {
        minType = partType;
        minLocation = data ? '/data/storage/el2/base/haps/entry/files/' +
        data.substring(data.lastIndexOf('/') + 1) : '';
      }
    } while (partResultSet.goToNextRow());
    if (minType !== Number.MAX_SAFE_INTEGER) {
      favItem.setPartType(minType);
      favItem.setPartLocationPath(minLocation);
      favItem.setRecordingTime(minType === 3 ? recordingTime : '');
    } else {
      LogUtils.w(TAG, 'no valid part data, use default type and location');
      favItem.setPartType(0);
      favItem.setPartLocationPath('');
      favItem.setRecordingTime('');
    }
    return true;
  }

  private setFavorateField(favItem: FavorateInfoItem, pduResultSet: rdb.ResultSet): boolean {
    let keepTime = pduResultSet.getLong(pduResultSet.getColumnIndex(OldDBData.favPdu.dateFavadd));
    favItem.setKeepTime(keepTime.toString());
    let startTime = pduResultSet.getLong(pduResultSet.getColumnIndex(OldDBData.favPdu.date));
    favItem.setStartTime((startTime * 1000).toString());
    favItem.setMsgType(1);
    favItem.setSmsType(0);
    let subCs = pduResultSet.getLong(pduResultSet.getColumnIndex(OldDBData.favPdu.subCs));
    let sub = pduResultSet.getString(pduResultSet.getColumnIndex(OldDBData.favPdu.sub));
    favItem.setMsgTitle(subCs == 106 ? StringUtil.codeConversion(sub) : sub);
    return false;
  }

  private setFavorateContent(favItem: FavorateInfoItem, partResultSet: rdb.ResultSet) {
    if (partResultSet == undefined || !partResultSet.goToFirstRow()) {
      LogUtils.w(TAG, 'setFavorateContent, no raw part found');
      return;
    }
    do {
      let ct = partResultSet.getString(partResultSet.getColumnIndex('ct'));
      if (ct.toLowerCase() === 'text/plain'.toLowerCase()) {
        favItem.setMsgContent(partResultSet.getString(partResultSet.getColumnIndex('text')));
        break;
      }
    } while (partResultSet.goToNextRow());
  }

  private transContentTypeToPartType(ct: string): number {
    let type: number = -1;
    if (ct === 'application/smil') {
      type = 0;
    } else if (StringUtil.getType(ct) === 'image') {
      type = 1;
    } else if (StringUtil.getType(ct) === 'video') {
      type = 2;
    } else if (StringUtil.getType(ct) === 'audio') {
      type = 3;
    } else if (ct.toLowerCase() === 'text/x-vCard'.toLowerCase()) {
      type = 4;
    } else if (ct.toLowerCase() === 'text/plain'.toLowerCase()) {
      type = 7;
    } else {
      LogUtils.w(TAG, 'unSupport ContentType: ' + ct);
    }
    return type;
  }

  private updatePartGroupId(partData: partDataNeedInsert, groupId: number) {
    partData.partItems.forEach((item: MmsPartItem) => {
      item.setGroupId(groupId);
    });
  }

  private buildFavPartInsertItems(resultSet: rdb.ResultSet, msgId: number,
      partData: partDataNeedInsert) {
    let recordingTime: string | undefined = undefined;
    let index: number = -1;
    let audioIndex = -1;
    do {
      index++;
      let partId = resultSet.getLong(resultSet.getColumnIndex(OldDBData.part.id));
      let mid = resultSet.getLong(resultSet.getColumnIndex(OldDBData.part.mid));
      let ct = resultSet.getString(resultSet.getColumnIndex(OldDBData.part.ct));
      let data = resultSet.getString(resultSet.getColumnIndex(OldDBData.part.data));
      let text = resultSet.getString(resultSet.getColumnIndex(OldDBData.part.text));
      let type: number = -1;
      let location = data ? '/data/storage/el2/base/haps/entry/files/fav/' +
      data.substring(data.lastIndexOf('/') + 1) : '';
      if (ct === 'application/smil') {
        location = '/data/storage/el2/base/haps/entry/files/fav/smil_' +
        mid?.toString() + '_' + partId?.toString() + '.xml'
        recordingTime = this.extractRecordingTime(text);
      }
      if (StringUtil.getType(ct) === 'audio') {
        audioIndex = index;
      }
      type = this.transContentTypeToPartType(ct);
      if (type === -1) {
        partData.partItems = [];
        return;
      }
      let values = new MmsPartItem()
        .setLocationPath(location)
        .setContent(text)
        .setMsgId(msgId)
        .setType(type)
        .setCt(ct)
        .setGroupId(0)
      partData.partItems.push(values);
    } while (resultSet.goToNextRow());
    if (recordingTime !== undefined && audioIndex != -1) {
      partData.partItems[audioIndex].setRecordingTime(recordingTime);
    }
  }

  private findMinTypeAndLocationPath(partData: partDataNeedInsert): favKeyFiled {
    const filteredItems = partData.partItems.filter(item => item.getType() !== 0);
    if (filteredItems.length === 0) {
      return { type: 0, locationPath: '', recordTime: '' };
    }
    let minItem = filteredItems.reduce((min, item) => (item.getType() < min.getType() ? item : min), filteredItems[0]);
    return { type: minItem.getType(), locationPath: minItem.getLocationPath(), recordTime: minItem.getRecordTime() };
  }

  private async buildFavMmsInsertItems(context: common.Context, pduResultSet: rdb.ResultSet,
    partDataToInsert: partDataNeedInsert, ids: needUpdateIds): Promise<FavorateInfoItem | undefined> {
    let id = pduResultSet.getLong(pduResultSet.getColumnIndex(OldDBData.favPdu.id));
    let partResultSet = await this.queryFavPartInfoWithMsgId(context, id);
    // 没有找到对应的 part 信息，视为无效 pdu
    if (partResultSet == undefined || !partResultSet.goToFirstRow()) {
      LogUtils.w(TAG, 'buildFavMmsInsertItems, no raw fav part found with msgid ' + id);
      return undefined;
    }
    let addrResultSet = await this.queryFavAddrInfoWithMsgId(context, id);
    if (addrResultSet == undefined || !addrResultSet.goToFirstRow()) {
      LogUtils.w(TAG, 'buildFavMmsInsertItems, no raw fav addr found with msgid ' + id);
      return undefined;
    }
    let favMmsItem = new FavorateInfoItem();
    let msgBox = pduResultSet.getLong(pduResultSet.getColumnIndex(OldDBData.favPdu.msgBox));
    // 没有找到对应的 addr 信息，视为无效 pdu
    if (!this.setFavorateAddrAndType(favMmsItem, addrResultSet, msgBox)) {
      LogUtils.w(TAG, 'buildFavMmsInsertItems, no invalid addrs with msgid ' + id);
      return undefined;
    }
    let originId = pduResultSet.getLong(pduResultSet.getColumnIndex(OldDBData.favPdu.originId));
    let newMsgIds = RAW_PDU_ID_MAP.get(originId) || [];
    if (newMsgIds.length <= 0) {
      LogUtils.w(TAG, 'buildFavMmsInsertItems, no related msgIds');
      //  如果仅有收藏，需要从已有 info 表中获取到最大的 msg_id，确定新插入的 fav 和 part 的 msg_id 对应关系，
      //  即无论是否有关联，msg_id 都不能为 0
      let maxMsgId = await newDbHelper.queryMaxMsgIdFromInfo(context);
      if (maxMsgId == -1) {
        LogUtils.w(TAG, 'buildFavMmsInsertItems, get max msgId error ');
        return undefined;
      }
      favMmsItem.setMsgId(maxMsgId);
      // 同时生成需要插入的 part 数据
      this.buildFavPartInsertItems(partResultSet, maxMsgId, partDataToInsert);
      if (partDataToInsert.partItems.length === 0) {
        LogUtils.w(TAG, 'buildFavMmsInsertItems, invalid part item ');
        return undefined;
      }
      // 从已构造的 mms_part 数据中取出 part_type 和 location 填入 favItem
      const partData: favKeyFiled = this.findMinTypeAndLocationPath(partDataToInsert);
      favMmsItem.setPartType(partData.type);
      favMmsItem.setPartLocationPath(partData.locationPath);
      favMmsItem.setRecordingTime(partData.recordTime);
      let groupId = this.getAvailableGroupId();
      favMmsItem.setGroupId(groupId);
      this.updatePartGroupId(partDataToInsert, groupId);
    } else {
      favMmsItem.setMsgId(newMsgIds[0]).setRcsId(0);
      favMmsItem.setGroupId(INFO_ID_GROUP.get(newMsgIds[0]));
      ids.msgNeedUpdate = newMsgIds[0];
      // 从已关联的 mms_part 数据中取出 part_type 和 location 填入 favItem
      if (!this.setFavKeyField(favMmsItem, partResultSet)) {
        LogUtils.w(TAG, 'buildFavMmsInsertItems, invalid part item ');
        return undefined;
      }
    }
    // other field
    this.setFavorateField(favMmsItem, pduResultSet);
    this.setFavorateContent(favMmsItem, partResultSet);
    addrResultSet.close();
    partResultSet.close();
    return favMmsItem;
  }

  private isMapInfo(jsonObject: Record<string, string>): boolean {
    return (
      typeof jsonObject['longitude'] === 'string' &&
        typeof jsonObject['latitude'] === 'string' &&
        typeof jsonObject['mapimage'] === 'string'
    );
  }

  private getOriginMsgIsRcsMap(info: string): boolean {
    try {
      let jsonObject: Record<string, string> = JSON.parse(info);
      // 判断是否为地图信息
      if (this.isMapInfo(jsonObject)) {
        return true;
      } else {
        return false;
      }
    } catch (e) {
      LogUtils.w(TAG, 'getOriginMsgIsRcsMap parse fail');
      return false;
    }
  }

  private getAvailableGroupId(): number {
    do {
      GROUP_ID = GROUP_ID + 1;
    } while (GROUP_ID_MAP.has(GROUP_ID));
    GROUP_ID_MAP.set(GROUP_ID, GROUP_ID);
    return GROUP_ID;
  }

  private async buildFavSmsInsertItem(context: common.Context, resultSet: rdb.ResultSet,
    data: needUpdateIds): Promise<FavorateInfoItem | undefined> {
    let serviceCenter = resultSet.getString(resultSet.getColumnIndex(OldDBData.favSms.serviceCenter));
    // rcs 富媒体暂不支持迁移，收藏表也不迁移，迁移报告记录不支持的 rcs 富媒体收藏信息数量
    let body = resultSet.getString(resultSet.getColumnIndex(OldDBData.favSms.body));
    let address = resultSet.getString(resultSet.getColumnIndex(OldDBData.favSms.address));
    let type = resultSet.getLong(resultSet.getColumnIndex(OldDBData.favSms.type));
    if (/rcs.*file/i.test(serviceCenter)) {
      LogUtils.w(TAG, 'queryFavSmsInfo, unSupport rcs mms.');
      return undefined;
    }
    // XXX fav_sms 包含 rcs 富媒体，fav_sms 不含 part_type 字段，且不一定有关联的 chat 表信息，需要从 body 内容区分
    // rcs 应用号信息，发送的信息不支持迁移，收藏保持一致
    if ('rcs.im' === serviceCenter && (this.getOriginMsgIsRcsMap(body) || (StringUtil.isSip(address) && type === 2))) {
      LogUtils.w(TAG, 'queryFavSmsInfo, unSupport rcs mms.');
      return undefined;
    }
    let favSmsItem = new FavorateInfoItem();
    let date = resultSet.getString(resultSet.getColumnIndex(OldDBData.favSms.date));
    let favAdd = resultSet.getString(resultSet.getColumnIndex(OldDBData.favSms.dateFavadd));
    let subject = resultSet.getString(resultSet.getColumnIndex(OldDBData.favSms.subject));
    let originId = resultSet.getLong(resultSet.getColumnIndex(OldDBData.favSms.originId));
    // 从XX rcsId 或 msgId 映射表均可以取到 fav_sms.origin_id 对应的有效 id，这里要区分是否rcs信息
    let isRcs = /rcs/i.test(serviceCenter);
    let detectResContent = `{"entity":[],"code":0,"content":"${body}","bundleName":"com.ohos.mms"}`;
    let newRcsId: number | undefined = RAW_RCS_ID_MAP.get(originId);
    let rcsId = (isRcs && newRcsId !== undefined) ? newRcsId : 0;
    data.rcsNeedUpdate = rcsId;
    let newMsgId: number | undefined = RAW_MSG_ID_MAP.get(originId);
    let msgId = (!isRcs && newMsgId !== undefined) ? newMsgId : 0;
    data.msgNeedUpdate = msgId;
    // 必须保证 msg_id 有值且唯一
    // 注意：支持 rcs 富媒体迁移时，需要考虑收藏的场景，非关联的rcs富媒体收藏在迁移时要保证 rcs_id 不冲突，删除rcs富媒体收藏
    // 的同时会删除对应part表信息，以 rcs_id 为准
    // 如果 msg_id 为0，即找不到关联的 msg_id，msg_id 需要生成最大值，rcs 富媒体暂不支持迁移，也不需要关联mms_part，rcs_id
    // 无关联的场景设置为0，不影响应用侧删除
    if (msgId == 0) {
      LogUtils.w(TAG, 'buildFavSmsInsertItem, no related msgIds');
      let maxMsgId = await newDbHelper.queryMaxMsgIdFromInfo(context);
      if (maxMsgId == -1) {
        LogUtils.w(TAG, 'buildFavSmsInsertItem, get max msgId error ');
        return undefined;
      }
      msgId = maxMsgId;
    }
    let groupId: number | undefined = 0;
    if (isRcs) {
      groupId = RCS_ID_GROUP.get(rcsId);
    } else {
      groupId = INFO_ID_GROUP.get(msgId);
    }
    let smsType = TelephoneUtil.judgeIsInfoMsg(address) ? 1 : (this.isChatbotNumber(address?.toString()) ? 1 : 0);
    favSmsItem.setStartTime(date)
      .setKeepTime(favAdd)
      .setReceiverNumber(type == 2 ? address : '')
      .setSenderNumber(type == 1 ? address : '')
      .setIsSender(type == 1 ? 1 : 0)
      .setMsgType(0)
      .setMsgContent(body)
      .setSmsType(isRcs ? 99 : smsType)
      .setDetectResContent(isRcs ? detectResContent : '')
      .setRcsId(rcsId)
      .setMsgId(msgId)
      .setGroupId(groupId === undefined ? this.getAvailableGroupId() : groupId)
      .setMsgTitle(subject);
    return favSmsItem;
  }

  private isChatbotNumber(telephone: string | undefined): boolean {
    if (telephone?.startsWith('sip:') && telephone?.includes('botplatform')) {
      return true
    }
    return false
  }

  private async queryThreadsInfoBuildSessionItem(resultSet: rdb.ResultSet, threadsId: number,
    numberType: number, rdbStore: rdb.RdbStore): Promise<SessionItem> {
    let sessionItem = new SessionItem();
    let date = resultSet.getLong(resultSet.getColumnIndex(OldDBData.threads.date));
    let datePrivacy = resultSet.getLong(resultSet.getColumnIndex(OldDBData.threads.datePrivacy));
    let messageCount = resultSet.getLong(resultSet.getColumnIndex(OldDBData.threads.messageCount));
    let snippetCs = resultSet.getLong(resultSet.getColumnIndex(OldDBData.threads.snippetCs));
    let snippet = resultSet.getString(resultSet.getColumnIndex(OldDBData.threads.snippet));
    let snippetPrivacy = resultSet.getString(resultSet.getColumnIndex(OldDBData.threads.snippetPrivacy));
    if (snippetCs == 106) {
      snippet = StringUtil.codeConversion(snippet);
    }
    let hasAttachment = resultSet.getLong(resultSet.getColumnIndex(OldDBData.threads.hasAttachment));
    let unreadCount = resultSet.getLong(resultSet.getColumnIndex(OldDBData.threads.unreadCount));
    let error = resultSet.getLong(resultSet.getColumnIndex(OldDBData.threads.error));
    let recipientIds = resultSet.getString(resultSet.getColumnIndex(OldDBData.threads.recipientIds));
    let arrRecipient = recipientIds.split(' ');
    let arrPhoneNumber = this.getArrPhoneNumber(resultSet, rdbStore);
    THREAD_PHONE_MAP.set(threadsId, arrPhoneNumber);
    let telephone = arrPhoneNumber.join();
    let contactsNum = arrPhoneNumber.length;
    let smsType = 0;
    if (numberType == 0) {
      smsType = 0;
    } else if (numberType == 2) {
      smsType = 1;
    } else if (numberType == 1) {
      smsType = 2;
    }
    let sendingStatus = 0;
    if (error != 0) {
      sendingStatus = 2;
    }
    sessionItem.setTime(date == 0 ? datePrivacy : date)
      .setContent(StringUtil.isEmpty(snippet) ? snippetPrivacy : snippet)
      .setTelephone(telephone)
      .setContactsNum(contactsNum)
      .setSmsType(smsType)
      .setUnreadCount(unreadCount)
      .setMessageCount(messageCount)
      .setHasAttachment(hasAttachment)
      .setSendingStatus(sendingStatus)
    return sessionItem;
  }

  public async queryFavSmsCount(context: common.Context): Promise<number> {
    if (context == null) {
      LogUtils.w(TAG, `queryFavSmsCount failed by null context`)
      return Promise.resolve(0);
    }
    let count = 0;
    try {
      let predicatesInfo = new rdb.RdbPredicates('fav_sms');
      let rdbStore = await this.initDB(context);
      if (rdbStore === undefined) {
        LogUtils.e(TAG, `[queryFavSmsCount] failed : rdbStore === undefined`)
        return Promise.resolve(0);
      }
      let queryColumn = ['count(_id)'];
      let resultSet = await rdbStore.query(predicatesInfo, queryColumn);
      if (resultSet === undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'query no fav sms.');
        return Promise.resolve(0);
      }
      count = resultSet.getLong(0);
      resultSet.close();
    } catch (error) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `queryFavSmsCount code:${error.code} msg:${error.message}`);
      LogUtils.e(TAG, '[queryFavSmsCount] error : ' + JSON.stringify(error))
    }
    return Promise.resolve(count);
  }

  public async queryFavMmsCount(context: common.Context): Promise<number> {
    if (context == null) {
      LogUtils.w(TAG, `queryFavMmsCount failed by null context`)
      return Promise.resolve(0);
    }
    let count = 0;
    try {
      let predicatesInfo = new rdb.RdbPredicates('fav_pdu');
      let rdbStore = await this.initDB(context);
      if (rdbStore === undefined) {
        LogUtils.e(TAG, `[queryFavMmsCount] failed : rdbStore === undefined`)
        return Promise.resolve(0);
      }
      let queryColumn = ['count(_id)'];
      let resultSet = await rdbStore.query(predicatesInfo, queryColumn);
      if (resultSet === undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'query no fav mms.');
        return Promise.resolve(0);
      }
      count = resultSet.getLong(0);
      resultSet.close();
    } catch (error) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `queryFavMmsCount code:${error.code} msg:${error.message}`);
      LogUtils.e(TAG, '[queryFavMmsCount] error : ' + JSON.stringify(error))
    }
    return Promise.resolve(count);
  }

  public async queryAddressCount(context: common.Context): Promise<number> {
    if (context == null) {
      LogUtils.w(TAG, `queryAddressCount failed by null context`)
      return Promise.resolve(0);
    }
    let count = 0;
    try {
      let predicatesInfo = new rdb.RdbPredicates('canonical_addresses');
      let rdbStore = await this.initDB(context);
      if (rdbStore === undefined) {
        LogUtils.e(TAG, `[queryAddressCount] failed : rdbStore === undefined`)
        return Promise.resolve(0);
      }
      let queryColumn = ['count(_id)'];
      let resultSet = await rdbStore.query(predicatesInfo, queryColumn);
      if (resultSet === undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'query no threads found.');
        return Promise.resolve(0);
      }
      count = resultSet.getLong(0);
      resultSet.close();
    } catch (error) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `queryAddressCount code:${error.code} msg:${error.message}`);
      LogUtils.e(TAG, '[queryAddressCount] error : ' + JSON.stringify(error))
    }
    return Promise.resolve(count);
  }

  private getArrPhoneNumber(resultSet: rdb.ResultSet, rdbStore: rdb.RdbStore) {
    let recipientIds = resultSet.getString(resultSet.getColumnIndex(OldDBData.threads.recipientIds));
    let arrRecipient = recipientIds.split(' ');
    let arrPhoneNumber: string[] = [];
    for (let i = 0; i < arrRecipient.length; i++) {
      let element = Number(arrRecipient[i]);
      if (!Number.isNaN(element)) {
        let phoneNumber = ADDRESS_ID_MAP.get(element);
        arrPhoneNumber.push(phoneNumber);
      }
    }
    return arrPhoneNumber;
  }

  public async queryAddressInfo(context: common.Context, index: number,
      progress: BackupRestoreProgress): Promise<void> {
    let addressInfo = new rdb.RdbPredicates('canonical_addresses');
    let rdbStore = await this.initDB(context);
    if (rdbStore === undefined) {
      LogUtils.e(TAG, `[queryAddressInfo] failed : rdbStore === undefined`)
      return;
    }
    try {
      let queryAddressColumn = ['_id', 'address']
      addressInfo = this.buildPredicatesInfo(addressInfo, '_id', index);
      let resultSet = await rdbStore.query(addressInfo, queryAddressColumn);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'no raw address found.');
        return;
      }
      do {
        let ID = resultSet.getLong(resultSet.getColumnIndex('_id'));
        let address = resultSet.getString(resultSet.getColumnIndex('address'));
        ADDRESS_ID_MAP.set(ID, address);
      } while (resultSet.goToNextRow())
      progress.updateProcessCount(progress.getProcessCount() + resultSet.rowCount)
      resultSet.close();
    } catch (error) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `queryAddressInfo code:${error.code} msg:${error.message}`);
      LogUtils.w(TAG, '[queryAddressInfo] error : ' + JSON.stringify(error));
    }
  }

  public checkAndReportFavSmsMigrate() {
    if (this.unSupportCountFav > 0) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `queryFavSmsUnSupport :${this.unSupportCountFav}`);
    }
    if (this.unrelatedFavSms > 0) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `queryFavSmsUnrelated sms:${this.unrelatedFavSms} mms:${this.unrelatedFavMms}`);
    }
  }

  private async queryFavPartInfoWithMsgId(context: common.Context, msgId: number): Promise<rdb.ResultSet | undefined> {
    let rdbStore = await this.initDB(context);
    if (rdbStore === undefined) {
      LogUtils.e(TAG, `[queryFavPartInfoWithMsgId] failed : rdbStore === undefined`);
      return undefined;
    }
    try {
      let queryColumn = this.buildFavPartQueryColumn();
      let predicatesInfo = new rdb.RdbPredicates(DbConstants.OLD_FAV_PART_NAME);
      predicatesInfo.equalTo('mid', msgId);
      return await rdbStore.query(predicatesInfo, queryColumn);
    } catch (e) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `queryFavPartInfoWithMsgId code:${e.code} msg:${e.message}`);
      LogUtils.w(TAG, 'queryFavPartInfoWithMsgId error : ' + JSON.stringify(e));
      return undefined;
    }
  }

  private async queryFavAddrInfoWithMsgId(context: common.Context, msgId: number): Promise<rdb.ResultSet | undefined> {
    let rdbStore = await this.initDB(context);
    if (rdbStore === undefined) {
      LogUtils.e(TAG, `[queryFavAddrInfoWithMsgId] failed : rdbStore === undefined`);
      return undefined;
    }
    try {
      let queryColumn = this.buildFavAddrQueryColumn();
      let predicatesInfo = new rdb.RdbPredicates(DbConstants.OLD_FAV_ADDR_NAME);
      predicatesInfo.equalTo('msg_id', msgId);
      return await rdbStore.query(predicatesInfo, queryColumn);
    } catch (e) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `queryFavAddrInfoWithMsgId code:${e.code} msg:${e.message}`);
      LogUtils.w(TAG, 'queryFavAddrInfoWithMsgId error : ' + JSON.stringify(e));
      return undefined;
    }
  }

  public async queryFavMmsInfo(context: common.Context, index: number,
    progress: BackupRestoreProgress): Promise<void> {
    let predicatesInfo = new rdb.RdbPredicates(DbConstants.OLD_FAV_PDU_NAME);
    let rdbStore = await this.initDB(context);
    if (rdbStore === undefined) {
      LogUtils.e(TAG, `[queryFavMmsInfo] failed : rdbStore === undefined`);
      return;
    }
    try {
      let queryColumn = this.buildFavPduQueryColumn();
      predicatesInfo = this.buildPredicatesInfo(predicatesInfo, OldDBData.favPdu.id, index);
      let resultSet = await rdbStore.query(predicatesInfo, queryColumn);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'queryFavMmsInfo, no raw fav pdu found.');
        return;
      }
      let count = resultSet.rowCount;
      let failCount = 0;
      let successCount = 0;
      do {
        //  1、完整的XXX fav表所有数据，在方法内部查询addr和part，对外不呈现细节
        //  2、如果仅有收藏，需要插入 mms_part 的数据（对象数组，需要构造），方法内部完成 reference_count 的赋值，
        //     reference_count 可以赋值为 -1， 应用侧兼容 -1 的场景，即加入 reference_count 前的历史数据
        //  3、如果仅有收藏，需要从已有 info 表中获取到最大的 msg_id，确定新插入的 fav 和 part 的 msg_id 对应关系，
        //     即无论是否有关联，msg_id 都不能为 0，sms 暂时可以为0，仅 fav 可存储，无其他关联表
        const ids: needUpdateIds = {
          msgNeedUpdate: undefined,
          rcsNeedUpdate: undefined
        };
        const partData: partDataNeedInsert = {partItems: []};
        let favInfoItem = await this.buildFavMmsInsertItems(context, resultSet, partData, ids);
        // part 表先于 fav 表插入，如果有需要插入且插入失败，不执行 fav 插入
        if (partData.partItems.length > 0) {
          let batchPartValues: ValuesBucket[] = [];
          partData.partItems.forEach((item: MmsPartItem) => {
            batchPartValues.push(item.createValuesBucket());
          });
          let insertNum = await newDbHelper.batchInsertPart(context, batchPartValues);
          if (insertNum == -1) {
            LogUtils.w(TAG, 'fav batchInsertPart failed');
            continue;
          }
        }
        if (favInfoItem === undefined) {
          successCount++;
          continue;
        }
        let favId = await newDbHelper.insertFavorate(context, favInfoItem.createValuesBucket());
        if (favId == -1) {
          failCount++;
        } else {
          successCount++;
        }
        // 如果 fav 的记录有跟 info 表关联，需要同时更新 info 和 rcsinfo 的 is_collect 为 1
        if (ids.msgNeedUpdate !== undefined && ids.msgNeedUpdate != 0) {
          await newDbHelper.updateSmsCollect(context, false, ids.msgNeedUpdate);
        } else {
          LogUtils.w(TAG, 'queryFavMmsInfo, have unrelated fav.');
          this.unrelatedFavMms++;
        }
      } while (resultSet.goToNextRow());
      resultSet.close();
      progress.updateProcessCount(progress.getProcessCount() + count);
      this.doFavMmsFailed = this.doFavMmsFailed + failCount;
      LogUtils.i(TAG, 'favMms Total Migrations count: ' + count + ',Successful Migrations successCount: ' +
        successCount + 'Failed Migrations failCount: ' + failCount + ',unrelated count ' + this.unrelatedFavMms);
    } catch (error) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `queryFavMmsInfo code:${error.code} msg:${error.message}`);
      LogUtils.w(TAG, 'query FavMmsInfo error : ' + JSON.stringify(error));
    }
  }

  public async queryFavSmsInfo(context: common.Context, index: number,
    progress: BackupRestoreProgress): Promise<void> {
    let predicatesInfo = new rdb.RdbPredicates(DbConstants.OLD_FAV_SMS_NAME);
    let rdbStore = await this.initDB(context);
    if (rdbStore === undefined) {
      LogUtils.e(TAG, `[queryFavSmsInfo] failed : rdbStore === undefined`);
      return;
    }
    try {
      let queryColumn = this.buildFavSmsQueryColumn();
      predicatesInfo = this.buildPredicatesInfo(predicatesInfo, OldDBData.favSms.id, index);
      let resultSet = await rdbStore.query(predicatesInfo, queryColumn);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'queryFavSmsInfo, no raw fav sms found.');
        return;
      }
      let count = resultSet.rowCount;
      let failCount = 0;
      let successCount = 0;
      do {
        const data: needUpdateIds = {
          msgNeedUpdate: undefined,
          rcsNeedUpdate: undefined
        };
        let favSmsItem = await this.buildFavSmsInsertItem(context, resultSet, data);
        if (favSmsItem === undefined) {
          this.unSupportCountFav++;
          successCount++;
          continue;
        }
        let favId = await newDbHelper.insertFavorate(context, favSmsItem.createValuesBucket());
        if (favId == -1) {
          failCount++;
        } else {
          successCount++;
        }
        // 如果 fav 的记录有跟 info 表关联，需要同时更新 info 和 rcsinfo 的 is_collect 为 1
        if (data.msgNeedUpdate !== undefined && data.msgNeedUpdate != 0) {
          await newDbHelper.updateSmsCollect(context, false, data.msgNeedUpdate);
        } else if (data.rcsNeedUpdate !== undefined && data.rcsNeedUpdate != 0) {
          await newDbHelper.updateSmsCollect(context, true, data.rcsNeedUpdate);
          await newDbHelper.updateRcsCollect(context, data.rcsNeedUpdate);
        } else {
          LogUtils.w(TAG, 'queryFavSmsInfo, have unrelated fav.');
          this.unrelatedFavSms++;
        }
      } while (resultSet.goToNextRow());
      resultSet.close();
      progress.updateProcessCount(progress.getProcessCount() + count);
      this.doFavSmsFailed = this.doFavSmsFailed + failCount;
      LogUtils.i(TAG, 'favSms Total Migrations count: ' + count + ',Successful Migrations successCount: ' +
        successCount + 'Failed Migrations failCount: ' + failCount + ',unSupport count ' + this.unSupportCountFav +
        ',unrelated count ' + this.unrelatedFavSms);
    } catch (error) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `queryFavSmsInfo code:${error.code} msg:${error.message}`);
      LogUtils.w(TAG, 'query FavSmsInfo error : ' + JSON.stringify(error));
    }
  }

  public async queryThreadsInfo(context: common.Context, index: number, localId: number,
    progress: BackupRestoreProgress): Promise<void> {
    let predicatesInfo = new rdb.RdbPredicates(DbConstants.OLD_SESSION_TABLE_NAME);
    let rdbStore = await this.initDB(context);
    if (rdbStore === undefined) {
      LogUtils.e(TAG, `[querySessionInfo] failed : rdbStore === undefined`);
      return;
    }
    try {
      let queryColumn = this.buildThreadsInfoQueryColumn();
      predicatesInfo = this.buildPredicatesInfo(predicatesInfo, OldDBData.threads.id, index);
      if (localId == MAIN_USER_ID) {
        predicatesInfo.equalTo('privacy_mode', 0)
      }
      let resultSet = await rdbStore.query(predicatesInfo, queryColumn);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'queryThreadsInfo, no raw Session found.');
        return;
      }
      let count = resultSet.rowCount;
      let failCount = 0;
      let successCount = 0;
      do {
        let threadsId = resultSet.getLong(resultSet.getColumnIndex(OldDBData.threads.id));
        let numberType = resultSet.getLong(resultSet.getColumnIndex(OldDBData.threads.numberType));
        let sessionItem: SessionItem = await this.queryThreadsInfoBuildSessionItem(resultSet,
           threadsId, numberType, rdbStore);
        let unreadCount = resultSet.getLong(resultSet.getColumnIndex(OldDBData.threads.unreadCount));
        let values: ValuesBucket = sessionItem.createValuesBucket();
        if (StringUtil.isSip(values.telephone as string)) {
          LogUtils.w(TAG, `[querySessionInfo] telephony is sip, unread count ${unreadCount}`);
          sessionItem.setSmsType(1);
          values.sms_type = 1;
        }
        let sessionId = await newDbHelper.insertSession(context, values);
        if (sessionId == -1) {
          failCount++;
        } else {
          successCount++;
          let threads: number[] = [];
          threads[0] = sessionId;
          threads[1] = numberType;
          threads[2] = sessionItem.getTime(); // 2: session 最新消息时间
          RAW_THREAD_ID_MAP.set(threadsId, threads);
        }
      } while (resultSet.goToNextRow());
      resultSet.close();
      progress.updateProcessCount(progress.getProcessCount() + count);
      this.doThreadFailed = this.doThreadFailed + failCount;
      LogUtils.i(TAG, 'threads Total Migrations count: ' + count + ',Successful Migrations successCount: ' + successCount + 'Failed Migrations failCount: ' + failCount);
    } catch (error) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `queryThreadsInfo code:${error.code} msg:${error.message}`);
      LogUtils.w(TAG, 'query SessionInfo error : ' + JSON.stringify(error));
    }
  }

  public async queryRcsThreadsCount(context: common.Context, localId: number): Promise<number> {
    if (context == null) {
      LogUtils.w(TAG, `queryRcsThreadsCount failed by null context`)
      return Promise.resolve(0);
    }
    let count = 0;
    try {
      let predicatesInfo = new rdb.RdbPredicates(DbConstants.OLD_RCS_THREADS_NAME);
      let rdbStore = await this.initDB(context);
      if (rdbStore === undefined) {
        LogUtils.e(TAG, `[queryRcsThreadsCount] failed : rdbStore === undefined`)
        return Promise.resolve(0);
      }
      let queryColumn = ['count(_id)'];
      if (localId == MAIN_USER_ID) {
        predicatesInfo.equalTo('privacy_mode', 0)
      }
      let resultSet = await rdbStore.query(predicatesInfo, queryColumn);
      if (resultSet === undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'query no rcsThreads found.');
        return Promise.resolve(0);
      }
      count = resultSet.getLong(0);
      LogUtils.i(TAG, '[queryRcsThreadsCount] count : ' + count);
      resultSet.close();
    } catch (error) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `queryRcsThreadsCount code:${error.code} msg:${error.message}`);
      LogUtils.e(TAG, '[queryRcsThreadsCount] error : ' + JSON.stringify(error))
    }
    return Promise.resolve(count);
  }

  public async queryRcsThreadsInfo(context: common.Context, index: number, localId: number,
    progress: BackupRestoreProgress): Promise<void> {
    let predicatesInfo = new rdb.RdbPredicates(DbConstants.OLD_RCS_THREADS_NAME);
    let rdbStore = await this.initDB(context);
    if (rdbStore === undefined) {
      LogUtils.e(TAG, `[queryRcsThreadsInfo] failed : rdbStore === undefined`);
      return;
    }
    try {
      let queryColumn = [OldDBData.rcsThreads.id, OldDBData.rcsThreads.date, OldDBData.rcsThreads.messageCount,
        OldDBData.rcsThreads.recipientIds, OldDBData.rcsThreads.snippet, OldDBData.rcsThreads.error,
        OldDBData.rcsThreads.hasAttachment, OldDBData.rcsThreads.unreadCount, OldDBData.rcsThreads.numberType,
        OldDBData.rcsThreads.privacyMode, OldDBData.rcsThreads.fileType, OldDBData.rcsThreads.type,
        OldDBData.rcsThreads.snippetCs];
      predicatesInfo = this.buildPredicatesInfo(predicatesInfo, OldDBData.rcsThreads.id, index);
      if (localId == MAIN_USER_ID) {
        predicatesInfo.equalTo('privacy_mode', 0)
      }
      let resultSet = await rdbStore.query(predicatesInfo, queryColumn);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'queryRcsThreadsInfo, no raw Session found.');
        return;
      }
      let count = resultSet.rowCount;
      let failCount = 0;
      let successCount = 0;
      do {
        let threadsId = resultSet.getLong(resultSet.getColumnIndex(OldDBData.rcsThreads.id));
        let date = resultSet.getLong(resultSet.getColumnIndex(OldDBData.rcsThreads.date));
        let messageCount = resultSet.getLong(resultSet.getColumnIndex(OldDBData.rcsThreads.messageCount));
        let hasAttachment = resultSet.getLong(resultSet.getColumnIndex(OldDBData.rcsThreads.hasAttachment));
        let snippet = resultSet.getString(resultSet.getColumnIndex(OldDBData.rcsThreads.snippet));
        let unreadCount = resultSet.getLong(resultSet.getColumnIndex(OldDBData.rcsThreads.unreadCount));
        let numberType = resultSet.getLong(resultSet.getColumnIndex(OldDBData.rcsThreads.numberType));
        let type = resultSet.getLong(resultSet.getColumnIndex(OldDBData.rcsThreads.type));
        let error = resultSet.getLong(resultSet.getColumnIndex(OldDBData.rcsThreads.error));
        let recipientIds = resultSet.getString(resultSet.getColumnIndex(OldDBData.rcsThreads.recipientIds));
        let fileType = resultSet.getLong(resultSet.getColumnIndex(OldDBData.rcsThreads.fileType));
        let snippetCs = resultSet.getLong(resultSet.getColumnIndex(OldDBData.rcsThreads.snippetCs));
        LogUtils.i(TAG, 'recipientIds : ' + recipientIds);
        let arrRecipientIds = recipientIds.split(' ');
        let arrId: string[] = [];
        await this.getThreadId(arrRecipientIds, rdbStore, arrId)
        LogUtils.i(TAG, 'arrId : ' + arrId.join());
        let arrRecipient = arrId;
        let arrPhoneNumber: string[] = [];
        await this.getGroupPhone(arrRecipient, rdbStore, arrPhoneNumber);
        arrPhoneNumber.sort();
        THREAD_PHONE_MAP.set(threadsId, arrPhoneNumber);
        let telephone = arrPhoneNumber.join(',');
        if (recipientIds.length < 5 || recipientIds.slice(0, 5) !== 'Mass_') {
          let arrRecipientIds = recipientIds.split(' ');
          let arrPhoneNumberId: string[] = [];
          await this.getPhone(arrRecipientIds, rdbStore, arrPhoneNumberId);
          THREAD_PHONE_MAP.set(threadsId, arrPhoneNumberId);
          telephone = arrPhoneNumberId.join();
        }
        let contactsNum = arrPhoneNumber.length;

        let sessions = RAW_THREAD_ID_MAP.get(threadsId);
        if (sessions) {
          try {
            let threadsPredicatesInfo = new rdb.RdbPredicates(DbConstants.OLD_SESSION_TABLE_NAME);
            threadsPredicatesInfo.equalTo(OldDBData.threads.id, threadsId);
            if (localId == MAIN_USER_ID) {
              threadsPredicatesInfo.equalTo('privacy_mode', 0)
            }
            let threadsResultSet = await rdbStore.query(threadsPredicatesInfo, [OldDBData.threads.date,
              OldDBData.threads.snippet, OldDBData.threads.snippetCs, OldDBData.threads.type,
              OldDBData.threads.unreadCount, OldDBData.threads.messageCount, OldDBData.threads.hasAttachment,
              OldDBData.threads.numberType, OldDBData.threads.error]);
            if (threadsResultSet != undefined && threadsResultSet.goToFirstRow()) {
              let tDate = threadsResultSet.getLong(threadsResultSet.getColumnIndex(OldDBData.threads.date));
              let tMessageCount = threadsResultSet.getLong(threadsResultSet.getColumnIndex(
                OldDBData.threads.messageCount));
              let tUnreadCount = threadsResultSet.getLong(threadsResultSet.getColumnIndex(
                OldDBData.threads.unreadCount));
              messageCount = messageCount + tMessageCount;
              unreadCount = unreadCount + tUnreadCount;
              if (date < tDate) {
                date = tDate;
                snippet = threadsResultSet.getString(threadsResultSet.getColumnIndex(OldDBData.threads.snippet));
                snippetCs = threadsResultSet.getLong(threadsResultSet.getColumnIndex(OldDBData.threads.snippetCs));
                type = threadsResultSet.getLong(threadsResultSet.getColumnIndex(OldDBData.threads.type));
                numberType = threadsResultSet.getLong(threadsResultSet.getColumnIndex(OldDBData.threads.numberType));
                hasAttachment = threadsResultSet.getLong(threadsResultSet.getColumnIndex(
                  OldDBData.threads.hasAttachment));
                error = threadsResultSet.getLong(threadsResultSet.getColumnIndex(OldDBData.threads.error));
              }
            }
            threadsResultSet.close();
          } catch (e) {
            const backupRestoreResult = BackupRestoreResult.getInstance();
            backupRestoreResult.insertOrUpdateBackupInfo('sms',
              `queryRcsThreadsInfo code:${e.code} msg:${e.message}`);
            LogUtils.w(TAG, 'query threads error : ' + JSON.stringify(e));
          }
        }

        let sendingStatus = 0;
        if (error != 0) {
          sendingStatus = 2;
        }
        // rcs 草稿并且存在 edittext 的 json 格式，要取 edittext 的内容
        if (snippet.indexOf('edittext') > -1 && type == 112) {
          const jsonObject = JSON.parse(snippet) as Array<LooseObject>;
          snippet = jsonObject['edittext'] as string;
        }

        let smsType = 0;
        if (numberType == 0) {
          smsType = 0;
        } else if (numberType == 2) {
          smsType = 1;
        } else if (numberType == 1) {
          smsType = 2;
        }

        if (snippetCs == 106) {
          snippet = StringUtil.codeConversion(snippet);
        }

        let values: ValuesBucket = new SessionItem()
          .setTelephone(telephone)
          .setContactsNum(contactsNum)
          .setSmsType(smsType)
          .setTime(date)
          .setContent(snippet)
          .setUnreadCount(unreadCount)
          .setMessageCount(messageCount)
          .setHasAttachment(hasAttachment)
          .setSendingStatus(sendingStatus)
          .createValuesBucket();

        if (sessions) {
          await newDbHelper.updateSession(context, sessions[0], values);
          sessions[2] = date; // 2: session 最新消息时间；data为thread和rcsThread最新时间比较后结果
          LogUtils.i(TAG, 'update already exist sessions : ' + sessions.join() + 'threadsId: ' + threadsId);
          if (fileType !== 0) {
            LogUtils.e(TAG, 'insert not supported file type ' + fileType);
            SESSION_NEED_UPDATE.push(sessions[0]);
          }
          successCount++;
        } else {
          let sessionId = await newDbHelper.insertSession(context, values);
          if (sessionId == -1) {
            failCount++;
          } else {
            successCount++;
            let threads: number[] = [];
            threads[0] = sessionId;
            threads[1] = numberType;
            threads[2] = date; // 2: session 最新消息时间
            LogUtils.i(TAG, 'insert rcs_threads '  + threadsId + ' to session : ' + threads.join());
            RAW_THREAD_ID_MAP.set(threadsId, threads);
            if (fileType !== 0) {
              LogUtils.e(TAG, 'insert not supported file type ' + fileType);
              SESSION_NEED_UPDATE.push(sessionId);
            }
          }
        }
      } while (resultSet.goToNextRow());
      resultSet.close();
      progress.updateProcessCount(progress.getProcessCount() + count);
      this.doRcsThreadFailed = this.doRcsThreadFailed + failCount;
      LogUtils.i(TAG, 'rcsThreads Total Migrations count: ' + count + ',Successful Migrations successCount: ' + successCount + 'Failed Migrations failCount: ' + failCount);
    } catch (error) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `queryRcsThreadsInfo code:${error.code} msg:${error.message}`);
      LogUtils.w(TAG, 'query rcsThreadsInfo error : ' + JSON.stringify(error));
    }
  }

  private async getPhone(arrRecipient: string[], rdbStore: rdb.RdbStore, arrPhoneNumber: string[]) {
    for (let index = 0; index < arrRecipient.length; index++) {
      try {
        let canonicalPredicatesInfo = new rdb.RdbPredicates(DbConstants.OLD_CANONICAL_ADDRESS_NAME);
        let element = arrRecipient[index];
        canonicalPredicatesInfo.equalTo(OldDBData.canonicalAddresses.id, element);
        let canResultSet = await rdbStore.query(canonicalPredicatesInfo, [OldDBData.canonicalAddresses.address]);
        if (canResultSet != undefined && canResultSet.goToFirstRow()) {
          let phoneNumber = canResultSet.getString(canResultSet.getColumnIndex(OldDBData.canonicalAddresses.address));
          arrPhoneNumber.push(phoneNumber);
        }
        canResultSet.close();
      } catch (e) {
        const backupRestoreResult = BackupRestoreResult.getInstance();
        backupRestoreResult.insertOrUpdateBackupInfo('sms',
          `getPhone code:${e.code} msg:${e.message}`);
        LogUtils.w(TAG, 'query address error : ' + JSON.stringify(e));
      }
    }
  }

  private async getThreadId(arrRecipient: string[], rdbStore: rdb.RdbStore, arrThreadId: string[]) {
    for (let index = 0; index < arrRecipient.length; index++) {
      try {
        let canonicalPredicatesInfo = new rdb.RdbPredicates('rcs_groups');
        let element = arrRecipient[index];
        canonicalPredicatesInfo.equalTo('name', element);
        let canResultSet = await rdbStore.query(canonicalPredicatesInfo, ['thread_id']);
        if (canResultSet != undefined && canResultSet.goToFirstRow()) {
          let threadId = canResultSet.getString(canResultSet.getColumnIndex('thread_id'));
          arrThreadId.push(threadId);
        }
        canResultSet.close();
      } catch (e) {
        const backupRestoreResult = BackupRestoreResult.getInstance();
        backupRestoreResult.insertOrUpdateBackupInfo('sms',
          `getThreadId code:${e.code} msg:${e.message}`);
        LogUtils.w(TAG, 'query address error : ' + JSON.stringify(e));
      }
    }
  }

  private async getGroupPhone(arrRecipient: string[], rdbStore: rdb.RdbStore, arrPhoneNumber: string[]) {
    LogUtils.i(TAG, 'getGroupPhone start');
    for (let index = 0; index < arrRecipient.length; index++) {
      try {
        let canonicalPredicatesInfo = new rdb.RdbPredicates(DbConstants.OLD_RCS_GROUP_MEMBER_NAME);
        let element = arrRecipient[index];
        canonicalPredicatesInfo.equalTo('thread_id', element);
        let canResultSet = await rdbStore.query(canonicalPredicatesInfo, ['rcs_id']);
        if (canResultSet == undefined || !canResultSet.goToFirstRow()) {
          return;
        }
        do {
          let phoneNumber = canResultSet.getString(canResultSet.getColumnIndex('rcs_id'));
          arrPhoneNumber.push(phoneNumber);
        } while (canResultSet.goToNextRow());
        canResultSet.close();
      } catch (e) {
        const backupRestoreResult = BackupRestoreResult.getInstance();
        backupRestoreResult.insertOrUpdateBackupInfo('sms',
          `getGroupPhone code:${e.code} msg:${e.message}`);
        LogUtils.w(TAG, 'query getGroupPhone error : ' + JSON.stringify(e));
      }
    }
  }

  public async querySmsCount(context: common.Context, localId:number): Promise<number> {
    if (context == null) {
      LogUtils.w(TAG, `querySmsCount failed by null context`)
      return Promise.resolve(0);
    }
    let count = 0;
    try {
      let predicatesInfo = new rdb.RdbPredicates(DbConstants.OLD_SMS_NAME);
      let rdbStore = await this.initDB(context);
      if (rdbStore === undefined) {
        LogUtils.e(TAG, `[querySmsCount] failed : rdbStore === undefined`)
        return Promise.resolve(0);
      }
      let queryColumn = ['count(_id)'];
      if (localId == MAIN_USER_ID) {
        predicatesInfo.equalTo('privacy_mode', 0)
      }
      let resultSet = await rdbStore.query(predicatesInfo, queryColumn);
      if (resultSet === undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'query no Sms found.');
        return Promise.resolve(0);
      }
      count = resultSet.getLong(0);
      LogUtils.i(TAG, '[querySmsCount] count : ' + count);
      resultSet.close();
    } catch (error) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `querySmsCount code:${error.code} msg:${error.message}`);
      LogUtils.e(TAG, '[querySmsCount] error : ' + JSON.stringify(error))
    }
    return Promise.resolve(count);
  }

  public async querySmsInfo(context: common.Context, index: number, localId:number,
    progress: BackupRestoreProgress): Promise<void> {
    let maxGroupId = MAX_GROUP_ID;
    let predicatesInfo = new rdb.RdbPredicates(DbConstants.OLD_SMS_NAME);
    let rdbStore = await this.initDB(context);
    if (rdbStore === undefined) {
      LogUtils.e(TAG, `[querySmsInfo] failed : rdbStore === undefined`);
      return;
    }
    try {
      let queryColumn = [OldDBData.sms.id, OldDBData.sms.address, OldDBData.sms.person, OldDBData.sms.subject,
        OldDBData.sms.body, OldDBData.sms.status, OldDBData.sms.locked, OldDBData.sms.read, OldDBData.sms.threadId,
        OldDBData.sms.groupId, OldDBData.sms.date, OldDBData.sms.dateSent, OldDBData.sms.subId, OldDBData.sms.type,
        OldDBData.chat.serviceCenter, OldDBData.sms.isSatellite];
      predicatesInfo = this.buildPredicatesInfo(predicatesInfo, OldDBData.sms.id, index);
      if (localId == MAIN_USER_ID) {
        predicatesInfo.equalTo('privacy_mode', 0)
      }
      let resultSet = await rdbStore.query(predicatesInfo, queryColumn);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'no raw Sms found.');
        return;
      }
      let count = resultSet.rowCount;
      let failCount = 0;
      let successCount = 0;
      let sessionIDUpdateContentMap = new HashMap<number, ValuesBucket>();
      do {
        let smsId = resultSet.getLong(resultSet.getColumnIndex(OldDBData.sms.id));
        let address = resultSet.getString(resultSet.getColumnIndex(OldDBData.sms.address));
        let person = resultSet.getLong(resultSet.getColumnIndex(OldDBData.sms.person));
        let subject = resultSet.getString(resultSet.getColumnIndex(OldDBData.sms.subject));
        let body = resultSet.getString(resultSet.getColumnIndex(OldDBData.sms.body));
        let status = resultSet.getLong(resultSet.getColumnIndex(OldDBData.sms.status));
        let locked = resultSet.getLong(resultSet.getColumnIndex(OldDBData.sms.locked));
        let read = resultSet.getLong(resultSet.getColumnIndex(OldDBData.sms.read));
        let threadId = resultSet.getLong(resultSet.getColumnIndex(OldDBData.sms.threadId));
        let groupId = resultSet.getLong(resultSet.getColumnIndex(OldDBData.sms.groupId));
        let date = resultSet.getLong(resultSet.getColumnIndex(OldDBData.sms.date));
        let dateSent = resultSet.getLong(resultSet.getColumnIndex(OldDBData.sms.dateSent));
        let subId = resultSet.getLong(resultSet.getColumnIndex(OldDBData.sms.subId));
        let type = resultSet.getLong(resultSet.getColumnIndex(OldDBData.sms.type));
        let serviceCenter = resultSet.getString(resultSet.getColumnIndex(OldDBData.chat.serviceCenter));
        let isSatellite = resultSet.getLong(resultSet.getColumnIndex(OldDBData.sms.isSatellite));
        let msgState: number = 0;
        if (type == 5) {
          msgState = 2;
        } else if (type == 4) {
          msgState = 1;
        }
        //会话是否有草稿
        let hasDraft = 0;
        if (type == 3) {
          msgState = 3;
          hasDraft = 1;
        }
        //会话是否有锁
        let hasLocked = 0;
        if (locked == 1) {
          hasLocked = 1;
        }
        let senderNumber = '';
        let receiverNumber = '';
        if (type === 1) {
          senderNumber = address;
        } else {
          receiverNumber = address;
        }
        let sessions = RAW_THREAD_ID_MAP.get(threadId) || [];
        let sessionId = -1;
        if (sessions && sessions.length > 1) {
          sessionId = sessions[0];
        }
        if (hasDraft == 1 || hasLocked == 1 && sessionId != -1) {
          let upValues: ValuesBucket = {
            'has_draft': hasDraft,
            'has_lock': hasLocked
          }
          sessionIDUpdateContentMap.set(sessionId, upValues);
        }
        let numberType = sessions.length > 2 ? sessions[1] : 0;
        let smsType = 0;
        if (numberType == 0) {
          smsType = 0;
        } else if (numberType == 2) {
          smsType = 1;
        } else if (numberType == 1) {
          smsType = 2;
        }
        if (groupId > maxGroupId) {
          maxGroupId = groupId;
        }
        let values: ValuesBucket = new SmsMmsInfoItem()
          .setReceiverNumber(receiverNumber)
          .setSenderNumber(senderNumber)
          .setMsgTitle(subject)
          .setMsgContent(body)
          .setMsgState(msgState)
          .setIsLock(locked)
          .setIsRead(read)
          .setSessionId(sessionId)
          .setGroupId(groupId)
          .setStartTime(date)
          .setEndTime(dateSent)
          .setSlotId(subId)
          .setMsgType(isSatellite == 1 ? 2 : 0)
          .setIsSender(type == 1 ? 1 : 0)
          .setSmsType(smsType)
          .createValuesBucket();
        let newMsgId = await newDbHelper.insertSms(context, values);
        if (newMsgId == -1) {
          failCount++;
        } else {
          successCount++;
          MAX_GROUP_ID = maxGroupId;
          GROUP_ID_MAP.set(groupId, groupId);
          RAW_MSG_ID_MAP.set(smsId, newMsgId);
          INFO_ID_GROUP.set(newMsgId, groupId);
        }
      } while (resultSet.goToNextRow());
      resultSet.close();
      let arrayFromMap = Array.from(sessionIDUpdateContentMap);
      for (let index = 0; index < arrayFromMap.length; index++) {
        const element = arrayFromMap[index];
        await newDbHelper.updateSession(context, element[0], element[1]);
      }
      progress.updateProcessCount(progress.getProcessCount() + count);
      this.doMmsFailed = this.doMmsFailed + failCount;
      LogUtils.i(TAG, 'sms Total Migrations count: ' + count + ',Successful Migrations successCount: ' + successCount + 'Failed Migrations failCount: ' + failCount);
    } catch (error) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `querySmsInfo code:${error.code} msg:${error.message}`);
      LogUtils.w(TAG, 'query SmsInfo error : ' + JSON.stringify(error));
    }
  }

  public async queryPduCount(context: common.Context, localId:number): Promise<number> {
    if (context == null) {
      LogUtils.w(TAG, `queryPduCount failed by null context`)
      return Promise.resolve(0);
    }
    let count = 0;
    try {
      let predicatesInfo = new rdb.RdbPredicates(DbConstants.OLD_PDU_NAME);
      let rdbStore = await this.initDB(context);
      if (rdbStore === undefined) {
        LogUtils.e(TAG, `[queryPduCount] failed : rdbStore === undefined`)
        return Promise.resolve(0);
      }
      let queryColumn = ['count(_id)'];
      if (localId == MAIN_USER_ID) {
        predicatesInfo.equalTo('privacy_mode', 0)
      }
      let resultSet = await rdbStore.query(predicatesInfo, queryColumn);
      if (resultSet === undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'query no pdu found.');
        return Promise.resolve(0);
      }
      count = resultSet.getLong(0);
      LogUtils.i(TAG, '[queryPduCount] count : ' + count);
      resultSet.close();
    } catch (error) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `queryPduCount code:${error.code} msg:${error.message}`);
      LogUtils.e(TAG, '[queryPduCount] error : ' + JSON.stringify(error))
    }
    return Promise.resolve(count);
  }

  public async queryPduInfo(context: common.Context, index: number, localId:number,
    progress: BackupRestoreProgress): Promise<void> {
    LogUtils.i(TAG, 'queryPduInfo start');
    let maxGroupId = MAX_GROUP_ID;
    let predicatesInfo = new rdb.RdbPredicates(DbConstants.OLD_PDU_NAME);
    let rdbStore = await this.initDB(context);
    if (rdbStore === undefined) {
      LogUtils.e(TAG, `[queryPduInfo] failed : rdbStore === undefined`);
      return;
    }
    try {
      let queryColumn = [OldDBData.pdu.id, OldDBData.pdu.date, OldDBData.pdu.dateSent, OldDBData.pdu.sub,
        OldDBData.pdu.locked, OldDBData.pdu.read, OldDBData.pdu.threadId, OldDBData.pdu.msgBox, OldDBData.pdu.subId,
        OldDBData.pdu.mSize, OldDBData.pdu.ctT, OldDBData.pdu.ctL, OldDBData.pdu.st];
      predicatesInfo = this.buildPredicatesInfo(predicatesInfo, OldDBData.pdu.id, index);
      if (localId == MAIN_USER_ID) {
        predicatesInfo.equalTo('privacy_mode', 0)
      }
      let resultSet = await rdbStore.query(predicatesInfo, queryColumn);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'queryPduInfo, no raw Session found.');
        return;
      }
      let count = resultSet.rowCount;
      let failCount = 0;
      let successCount = 0;
      do {
        let pduId = resultSet.getLong(resultSet.getColumnIndex(OldDBData.pdu.id));
        let subId = resultSet.getLong(resultSet.getColumnIndex(OldDBData.pdu.subId));
        let date = resultSet.getLong(resultSet.getColumnIndex(OldDBData.pdu.date));
        let dateSent = resultSet.getLong(resultSet.getColumnIndex(OldDBData.pdu.dateSent));
        let sub = StringUtil.codeConversion(resultSet.getString(resultSet.getColumnIndex(OldDBData.pdu.sub)));
        let locked = resultSet.getLong(resultSet.getColumnIndex(OldDBData.pdu.locked));
        let read = resultSet.getLong(resultSet.getColumnIndex(OldDBData.pdu.read));
        let threadId = resultSet.getLong(resultSet.getColumnIndex(OldDBData.pdu.threadId));
        let msgBox = resultSet.getLong(resultSet.getColumnIndex(OldDBData.pdu.msgBox));
        let ctT = resultSet.getString(resultSet.getColumnIndex(OldDBData.pdu.ctT));
        let ctL = resultSet.getString(resultSet.getColumnIndex(OldDBData.pdu.ctL));
        let st = resultSet.getLong(resultSet.getColumnIndex(OldDBData.pdu.st));
        let arrPhoneNumber = THREAD_PHONE_MAP.get(threadId) || [];
        let senderNumber = '';
        let receiverNumber = '';
        if (msgBox === 1) {
          senderNumber = arrPhoneNumber[0];
        } else {
          receiverNumber = arrPhoneNumber.join();
        }
        let msgState: number = 0;
        if (msgBox == 5) {
          msgState = 2;
        } else if (msgBox == 4) {
          msgState = 1;
        }

        //会话是否有草稿
        let hasDraft = 0;
        if (msgBox == 3) {
          msgState = 3;
          hasDraft = 1;
        }
        // 双 psu.st ：未启动-128，下载中-129，传输失败-130，保存失败-135
        if (st == 128 || st == 129 || st == 130 || st == 135) {
          LogUtils.e(TAG, 'not support pdu st ' + st + ' ,break ');
          let sessionId = RAW_THREAD_ID_MAP.get(threadId);
          if (sessionId != undefined) {
            LogUtils.e(TAG, 'unSupport pdu,need update session');
            SESSION_NEED_UPDATE.push(sessionId[0]);
          }
          continue;
        }
        //会话是否有锁
        let hasLocked = 0;
        if (locked == 1) {
          hasLocked = 1;
        }

        let sessions = RAW_THREAD_ID_MAP.get(threadId) || [];
        let sessionId = -1;
        if (sessions && sessions.length > 1) {
          sessionId = sessions[0];
        }
        if ((hasDraft == 1 || hasLocked == 1) && sessionId != -1) {
          let upValues: ValuesBucket = {
            'has_draft': hasDraft,
            'has_lock': hasLocked,
            'has_mms': 1,
            'has_attachment': hasDraft == 1 ? 1 : 0
          }
          await newDbHelper.updateSession(context, sessionId, upValues);
        } else if (sessions[2] <= date * 1000) { // 2: session时间; 1000：单位转换
          let upValues: ValuesBucket = {
            'has_mms': 1
          }
          await newDbHelper.updateSession(context, sessionId, upValues);
        }

        let numberType = sessions.length > 2 ? sessions[1] : 0;

        let smsType = 0;
        if (numberType == 0) {
          smsType = 0;
        } else if (numberType == 2) {
          smsType = 1;
        } else if (numberType == 1) {
          smsType = 2;
        }
        let groupId = ++maxGroupId;
        let arrPhoneNumberLength = arrPhoneNumber?.length || 0;
        if (arrPhoneNumberLength > 1) {
          let newRawSmsIds: number[] = [];
          for (let index = 0; index < arrPhoneNumberLength; index++) {
            if (arrPhoneNumber == undefined) {
              break;
            };
            receiverNumber = arrPhoneNumber[index];
            let values: ValuesBucket = this.buildSmsBucket(sub, locked, read, sessionId, subId, date, dateSent,
              senderNumber, receiverNumber, msgState, smsType, msgBox, groupId);
            let newRawSmsId = await newDbHelper.insertSms(context, values);
            if (newRawSmsId == -1) {
              failCount++;
            } else {
              MAX_GROUP_ID = groupId;
              newRawSmsIds.push(newRawSmsId)
              successCount++;
              GROUP_ID_MAP.set(groupId, groupId)
              INFO_ID_GROUP.set(newRawSmsId, groupId);
            }
          }
          RAW_PDU_ID_MAP.set(pduId, newRawSmsIds);
        } else {
          let newRawSmsIds: number[] = [];
          let values: ValuesBucket = this.buildSmsBucket(sub, locked, read, sessionId, subId, date, dateSent,
            senderNumber, receiverNumber, msgState, smsType, msgBox, groupId);
          let newRawSmsId = await newDbHelper.insertSms(context, values);
          if (newRawSmsId == -1) {
            failCount++;
          } else {
            MAX_GROUP_ID = groupId;
            newRawSmsIds.push(newRawSmsId);
            RAW_PDU_ID_MAP.set(pduId, newRawSmsIds);
            successCount++;
            GROUP_ID_MAP.set(groupId, groupId)
            INFO_ID_GROUP.set(newRawSmsId, groupId);
          }
        }
      } while (resultSet.goToNextRow());
      resultSet.close();
      progress.updateProcessCount(progress.getProcessCount() + count);
      this.doPduFailed = this.doPduFailed + failCount;
      LogUtils.i(TAG, 'pdu Total Migrations count: ' + count + ',Successful Migrations successCount: ' + successCount + 'Failed Migrations failCount: ' + failCount);
    } catch (error) {
      LogUtils.w(TAG, 'query PduInfo error : ' + JSON.stringify(error));
    }
  }

  private buildSmsBucket(sub: string, locked: number, read: number, sessionId: number, subId: number, date: number,
                         dateSent: number, senderNumber: string, receiverNumber: string, msgState: number,
                         smsType: number, msgBox: number, groupId: number): ValuesBucket {
    return new SmsMmsInfoItem()
      .setMsgTitle(sub)
      .setIsLock(locked)
      .setIsRead(read)
      .setSessionId(sessionId)
      .setSlotId(subId)
      .setStartTime(date * 1000)
      .setEndTime(dateSent * 1000)
      .setSenderNumber(senderNumber)
      .setReceiverNumber(receiverNumber)
      .setMsgType(1)
      .setMsgState(msgState)
      .setSmsType(smsType)
      .setIsSender(msgBox == 1 ? 1 : 0)
      .setGroupId(groupId)
      .createValuesBucket();
  }

  public async queryPartCount(context: common.Context): Promise<number> {
    if (context == null) {
      LogUtils.w(TAG, `queryPartCount failed by null context`)
      return Promise.resolve(0);
    }
    let count = 0;
    try {
      let predicatesInfo = new rdb.RdbPredicates(DbConstants.OLD_PART_NAME);
      let rdbStore = await this.initDB(context);
      if (rdbStore === undefined) {
        LogUtils.e(TAG, `[queryPartCount] failed : rdbStore === undefined`)
        return Promise.resolve(0);
      }
      let queryColumn = ['count(_id)'];
      let resultSet = await rdbStore.query(predicatesInfo, queryColumn);
      if (resultSet === undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'query no pdu found.');
        return Promise.resolve(0);
      }
      count = resultSet.getLong(0);
      LogUtils.i(TAG, '[queryPartCount] count : ' + count);
      resultSet.close();
    } catch (error) {
      LogUtils.e(TAG, '[queryPartCount] error : ' + JSON.stringify(error))
    }
    return Promise.resolve(count);
  }

  public async queryPartInfo(context: common.Context, index: number, progress: BackupRestoreProgress): Promise<void> {
    let predicatesInfo = new rdb.RdbPredicates(DbConstants.OLD_PART_NAME);
    let rdbStore = await this.initDB(context);
    if (rdbStore === undefined) {
      LogUtils.e(TAG, `[queryPartInfo] failed : rdbStore === undefined`);
      return;
    }
    try {
      let queryColumn = [OldDBData.part.id, OldDBData.part.mid, OldDBData.part.ct, OldDBData.part.data,
        OldDBData.part.text];
      predicatesInfo = this.buildPredicatesInfo(predicatesInfo, OldDBData.part.id, index);
      let resultSet = await rdbStore.query(predicatesInfo, queryColumn);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'no raw Sms found.');
        return;
      }
      let count = resultSet.rowCount;
      let failCount = 0;
      let successCount = 0;
      let batchPartValues: ValuesBucket[] = [];
      do {
        let partId = resultSet.getLong(resultSet.getColumnIndex(OldDBData.part.id));
        let mid = resultSet.getLong(resultSet.getColumnIndex(OldDBData.part.mid));
        let ct = resultSet.getString(resultSet.getColumnIndex(OldDBData.part.ct));
        let data = resultSet.getString(resultSet.getColumnIndex(OldDBData.part.data));
        let text = resultSet.getString(resultSet.getColumnIndex(OldDBData.part.text));
        let type: number = -1;
        let location = data ? '/data/storage/el2/base/haps/entry/files/' +
          data.substring(data.lastIndexOf('/') + 1) : '';
        if (ct === 'application/smil') {
          location = '/data/storage/el2/base/haps/entry/files/smil_' +
            mid?.toString() + '_' + partId?.toString() + '.xml'
          type = 0;
        } else if (StringUtil.getType(ct) === 'image') {
          type = 1;
        } else if (StringUtil.getType(ct) === 'video') {
          type = 2;
        } else if (StringUtil.getType(ct) === 'audio') {
          type = 3;
        } else if (ct === 'text/x-vCard') {
          type = 4;
        } else if (ct === 'text/plain') {
          type = 7;
        }

        // 旧机 pdu id 对应新机 sms_mms_info.msg_id
        let rawPduIdValue = RAW_PDU_ID_MAP.get(mid) || [];
        if (rawPduIdValue.length <= 0) {
          LogUtils.e(TAG, 'not found mid from new msgIds');
          break;
        }
        for (let groupIndex = 0; groupIndex < rawPduIdValue.length; groupIndex++) {
          let msgId = rawPduIdValue[groupIndex];
          LogUtils.i(TAG, 'part partId = ' + partId + ', mid = ' + mid + ', type = ' + type + ', msgId = ' + msgId);
          await this.setAudioTime(text, msgId);
          if (type == 7) {
            MMS_ID_TEXT_MAP.set(msgId, text);
          }
          let pduIdGroupValue = INFO_ID_GROUP.get(msgId) || 0;
          let values: ValuesBucket = new MmsPartItem()
            .setLocationPath(location)
            .setContent(text)
            .setMsgId(msgId)
            .setType(type)
            .setCt(ct)
            .setGroupId(pduIdGroupValue)
            .createValuesBucket();
          batchPartValues.push(values);
          if (pduIdGroupValue != undefined) {
            GROUP_ID_MAP.set(pduIdGroupValue, pduIdGroupValue);
          }
        }
      } while (resultSet.goToNextRow());
      resultSet.close();
      if (batchPartValues.length > 0) {
        let insertNum = await newDbHelper.batchInsertPart(context, batchPartValues);
        if (insertNum == -1) {
          failCount += batchPartValues.length;
        } else {
          successCount += insertNum;
        }
      }
      progress.updateProcessCount(progress.getProcessCount() + count);
      this.doPartFailed = this.doPartFailed + failCount;
      LogUtils.i(TAG, 'part Total Migrations count: ' + count + ',Successful Migrations successCount: ' + successCount + 'Failed Migrations failCount: ' + failCount);
    } catch (error) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `OHOS2NEXT part code:${error.code} msg:${error.message}`);
      LogUtils.w(TAG, 'query PartInfo error : ' + JSON.stringify(error));
    }
  }

  public async queryChatCount(context: common.Context, localId:number): Promise<number> {
    if (context == null) {
      LogUtils.w(TAG, `queryChatCount failed by null context`)
      return Promise.resolve(0);
    }
    let count = 0;
    try {
      let predicatesInfo = new rdb.RdbPredicates(DbConstants.OLD_CHAT_NAME);
      let rdbStore = await this.initDB(context);
      if (rdbStore === undefined) {
        LogUtils.e(TAG, `[queryChatCount] failed : rdbStore === undefined`)
        return Promise.resolve(0);
      }
      let queryColumn = ['count(_id)'];
      if (localId == MAIN_USER_ID) {
        predicatesInfo.equalTo('privacy_mode', 0)
      }
      let resultSet = await rdbStore.query(predicatesInfo, queryColumn);
      if (resultSet === undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'query no chat found.');
        return Promise.resolve(0);
      }
      count = resultSet.getLong(0);
      LogUtils.i(TAG, '[queryChatCount] count : ' + count);
      resultSet.close();
    } catch (error) {
      LogUtils.e(TAG, '[queryChatCount] error : ' + JSON.stringify(error))
    }
    return Promise.resolve(count);
  }

  public async queryGroupCount(context: common.Context): Promise<number> {
    LogUtils.i(TAG, `queryGroupCount start!`)
    if (context == null) {
      LogUtils.w(TAG, `queryGroupCount failed by null context`)
      return Promise.resolve(0);
    }
    let count = 0;
    try {
      let predicatesInfo = new rdb.RdbPredicates(DbConstants.OLD_RCS_GROUP_NAME);
      let rdbStore = await this.initDB(context);
      if (rdbStore === undefined) {
        LogUtils.e(TAG, `[queryGroupCount] failed : rdbStore === undefined`)
        return Promise.resolve(0);
      }
      let queryColumn = ['count(_id)'];
      let resultSet = await rdbStore.query(predicatesInfo, queryColumn);
      if (resultSet === undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'query no Group found.');
        return Promise.resolve(0);
      }
      count = resultSet.getLong(0);
      LogUtils.i(TAG, '[queryGroupCount] count : ' + count);
      resultSet.close();
    } catch (error) {
      LogUtils.e(TAG, '[queryGroupCount] error : ' + JSON.stringify(error))
    }
    return Promise.resolve(count);
  }

  public async queryMaapCount(context: common.Context): Promise<number> {
    LogUtils.i(TAG, `queryMaapCount start!`)
    if (context == null) {
      LogUtils.w(TAG, `queryMaapCount failed by null context`)
      return Promise.resolve(0);
    }
    let count = 0;
    try {
      let predicatesInfo = new rdb.RdbPredicates('chatbots');
      let rdbStore = await this.initMaapDB(context);
      if (rdbStore === undefined) {
        LogUtils.e(TAG, `[queryGroupCount] failed : rdbStore === undefined`)
        return Promise.resolve(0);
      }
      let queryColumn = ['count(_id)'];
      let resultSet = await rdbStore.query(predicatesInfo, queryColumn);
      if (resultSet === undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'query no Group found.');
        return Promise.resolve(0);
      }
      count = resultSet.getLong(0);
      LogUtils.i(TAG, '[queryMaapCount] count : ' + count);
      resultSet.close();
    } catch (error) {
      LogUtils.e(TAG, '[queryMaapCount] error : ' + JSON.stringify(error))
    }
    return Promise.resolve(count);
  }

  public async queryChatInfo(context: common.Context, index: number, localId:number,
    progress: BackupRestoreProgress): Promise<void> {
    let predicatesInfo = new rdb.RdbPredicates(DbConstants.OLD_CHAT_NAME);
    let rdbStore = await this.initDB(context);
    if (rdbStore === undefined) {
      LogUtils.e(TAG, `[queryChatInfo] failed : rdbStore === undefined`);
      return;
    }
    try {
      let queryColumn = [OldDBData.chat.id, OldDBData.chat.threadId, OldDBData.chat.address, OldDBData.chat.person,
        OldDBData.chat.date, OldDBData.chat.dateSent, OldDBData.chat.protocol, OldDBData.chat.read,
        OldDBData.chat.status, OldDBData.chat.type, OldDBData.chat.replyPathPresent, OldDBData.chat.subject,
        OldDBData.chat.body, OldDBData.chat.serviceCenter, OldDBData.chat.locked, OldDBData.chat.subId,
        OldDBData.chat.networkType, OldDBData.chat.errorCode, OldDBData.chat.seen, OldDBData.chat.privacyMode,
        OldDBData.chat.serviceKind, OldDBData.chat.fileType, OldDBData.chat.sdkSmsId,
        OldDBData.chat.enrichedCallingType, OldDBData.chat.ownerAddr, OldDBData.chat.globalGroupId];
      predicatesInfo = this.buildPredicatesInfo(predicatesInfo, OldDBData.chat.id, index);
      if (localId == MAIN_USER_ID) {
        predicatesInfo.equalTo('privacy_mode', 0)
      }
      let resultSet = await rdbStore.query(predicatesInfo, queryColumn);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'no raw Rcs found.');
        return;
      }
      let count = resultSet.rowCount;
      let failCount = 0;
      let successCount = 0;

      do {
        do {
          GROUP_ID = GROUP_ID + 1;
        } while (GROUP_ID_MAP.has(GROUP_ID));
        GROUP_ID_MAP.set(GROUP_ID, GROUP_ID);
        let chatId = resultSet.getLong(resultSet.getColumnIndex(OldDBData.chat.id));
        let threadId = resultSet.getLong(resultSet.getColumnIndex(OldDBData.chat.threadId));
        let address = resultSet.getString(resultSet.getColumnIndex(OldDBData.chat.address));
        let person = resultSet.getLong(resultSet.getColumnIndex(OldDBData.chat.person));
        let date = resultSet.getLong(resultSet.getColumnIndex(OldDBData.chat.date));
        let dateSent = resultSet.getLong(resultSet.getColumnIndex(OldDBData.chat.dateSent));
        let protocol = resultSet.getLong(resultSet.getColumnIndex(OldDBData.chat.protocol));
        let read = resultSet.getLong(resultSet.getColumnIndex(OldDBData.chat.read));
        let status = resultSet.getLong(resultSet.getColumnIndex(OldDBData.chat.status));
        let type = resultSet.getLong(resultSet.getColumnIndex(OldDBData.chat.type));
        let replyPathPresent = resultSet.getLong(resultSet.getColumnIndex(OldDBData.chat.replyPathPresent));
        let subject = resultSet.getString(resultSet.getColumnIndex(OldDBData.chat.subject));
        let body = resultSet.getString(resultSet.getColumnIndex(OldDBData.chat.body));
        let serviceCenter = resultSet.getString(resultSet.getColumnIndex(OldDBData.chat.serviceCenter));
        let locked = resultSet.getLong(resultSet.getColumnIndex(OldDBData.chat.locked));
        let subId = resultSet.getLong(resultSet.getColumnIndex(OldDBData.chat.subId));
        let networkType = resultSet.getLong(resultSet.getColumnIndex(OldDBData.chat.networkType));
        let errorCode = resultSet.getLong(resultSet.getColumnIndex(OldDBData.chat.errorCode));
        let seen = resultSet.getLong(resultSet.getColumnIndex(OldDBData.chat.seen));
        let privacyMode = resultSet.getLong(resultSet.getColumnIndex(OldDBData.chat.privacyMode));
        let serviceKind = resultSet.getString(resultSet.getColumnIndex(OldDBData.chat.serviceKind));
        let fileType = resultSet.getLong(resultSet.getColumnIndex(OldDBData.chat.fileType));
        let sdkSmsId = resultSet.getLong(resultSet.getColumnIndex(OldDBData.chat.sdkSmsId));
        let enrichedCallingType = resultSet.getLong(resultSet.getColumnIndex(OldDBData.chat.enrichedCallingType));
        let ownerAddr = resultSet.getString(resultSet.getColumnIndex(OldDBData.chat.ownerAddr));
        let globalGroupId = resultSet.getString(resultSet.getColumnIndex(OldDBData.chat.globalGroupId));
        let msgState: number = 0;
        if (type == 5) {
          msgState = 2;
        } else if (type == 4) {
          msgState = 1;
        }

        let senderNumber = '';
        let receiverNumber = '';
        if (type === 1) {
          senderNumber = address;
        } else {
          receiverNumber = address;
        }
        let sessions = RAW_THREAD_ID_MAP.get(threadId);
        let sessionId = -1;
        if (sessions) {
          sessionId = sessions[0];
        }
        let sessionNumber = await newDbHelper.getSessionNumber(sessionId.toString(), context)

        let values: ValuesBucket = new RcsInfoItem()
          .setSlotId(subId)
          .setReceiverNumber(receiverNumber)
          .setSenderNumber(senderNumber)
          .setStartTime(date)
          .setEndTime(dateSent)
          .setRcsType(fileType)
          .setMsgTitle(subject)
          .setMsgContent(body)
          .setMsgState(msgState)
          .setIsLock(locked)
          .setIsRead(read)
          .setSessionId(sessionId)
          .setGroupId(GROUP_ID)
          .setIsSender(type == 1 ? 1 : 0)
          .setMsgId(sdkSmsId + '' + date)
          .setEnrichedCallingType(enrichedCallingType)
          .setErrorCode(errorCode)
          .setNetworkType(networkType)
          .setOwnerAddr(ownerAddr)
          .setPrivacyMode(privacyMode)
          .setProtocol(protocol)
          .setReplyPathPresent(replyPathPresent)
          .setSeen(seen)
          .setServiceCenter(serviceCenter)
          .setServiceKind(serviceKind)
          .createValuesBucket();
        if (values.rcs_type === 0 && values.session_id != -1) {
          if (StringUtil.isSip(sessionNumber)) {
            LogUtils.e(TAG, 'queryChatInfo telephony is sip');
            if (serviceCenter === 'rcs.im' && type === 1) {
              LogUtils.e(TAG, 'queryChatInfo need insert');
            } else {
              LogUtils.e(TAG, 'queryChatInfo no need insert');
              continue;
            }
          }
          let newRawRasInfoId = await newDbHelper.insertRcsInfo(context, values);
          let newRawSmsIdStatus = 0;
          if (newRawRasInfoId != -1) {
            LogUtils.i(TAG, 'insertRcsInfo newRawRasInfoId1 ' + newRawRasInfoId);
            RCS_ID_GROUP.set(newRawRasInfoId, GROUP_ID);
            let valuesMmsInfo: ValuesBucket = new SmsMmsInfoItem()
              .setReceiverNumber(receiverNumber)
              .setSenderNumber(senderNumber)
              .setMsgTitle(subject)
              .setMsgContent(body)
              .setMsgState(msgState)
              .setIsLock(locked)
              .setIsRead(1)
              .setSessionId(sessionId)
              .setGroupId(GROUP_ID)
              .setStartTime(date)
              .setEndTime(dateSent)
              .setSlotId(subId)
              .setMsgType(0)
              .setIsSender(type == 1 ? 1 : 0)
              .setRcsId(newRawRasInfoId)
              .setSmsType(99)
              .createValuesBucket();
            let newRawSmsId = await newDbHelper.insertSms(context, valuesMmsInfo);
            LogUtils.i(TAG, 'insertRcsInfo newRawRasInfoId2 ' + newRawSmsId);
            newRawSmsIdStatus = newRawSmsId
            let sessionTime = await newDbHelper.getSessionTime(sessionId?.toString(), context)
            if (sessionTime <= date) {
              let upSessionValues: ValuesBucket = {
                'content': body,
                'time': date,
              }
              await newDbHelper.updateSession(context, sessionId, upSessionValues);
            }
          }
          if (newRawRasInfoId == -1 || newRawSmsIdStatus == -1) {
            failCount++;
          } else {
            successCount++;
            RAW_RCS_ID_MAP.set(chatId, newRawRasInfoId);
          }
        }
      } while (resultSet.goToNextRow());
      resultSet.close();
      progress.updateProcessCount(progress.getProcessCount() + count);
      this.doRcsChatFailed = this.doRcsChatFailed + failCount;
      LogUtils.i(TAG, 'rcsInfo Total Migrations count: ' + count + ',Successful Migrations successCount: ' + successCount + 'Failed Migrations failCount: ' + failCount);
    } catch (error) {
      LogUtils.w(TAG, 'query rcsInfo error : ' + JSON.stringify(error));
    }
  }

  public async updateSessionWithUnSupportItem(context: common.Context,
    progress: BackupRestoreProgress): Promise<number> {
    if (!context) {
      LogUtils.w(TAG, 'context is empty');
      return 0;
    }
    return await newDbHelper.updateSessionWithUnSupportItem(context, SESSION_NEED_UPDATE, progress);
  }

  public async queryRcsGroupInfo(context: common.Context, index: number,
    progress: BackupRestoreProgress): Promise<void> {
    let predicatesInfo = new rdb.RdbPredicates(DbConstants.OLD_RCS_GROUP_NAME);
    let rdbStore = await this.initDB(context);
    if (rdbStore === undefined) {
      LogUtils.e(TAG, `[queryRcsGroupInfo] failed : rdbStore === undefined`);
      return;
    }
    try {
      let queryColumn = [OldDBData.rcsGroup.id, OldDBData.rcsGroup.threadId, OldDBData.rcsGroup.type,
        OldDBData.rcsGroup.address, OldDBData.rcsGroup.date, OldDBData.rcsGroup.read, OldDBData.rcsGroup.status,
        OldDBData.rcsGroup.fileMode, OldDBData.rcsGroup.globalDate, OldDBData.rcsGroup.body,
        OldDBData.rcsGroup.global_id, OldDBData.rcsGroup.operateCode, OldDBData.rcsGroup.errorCode,
        OldDBData.rcsGroup.metaData, OldDBData.rcsGroup.seen, OldDBData.rcsGroup.serviceKind,
        OldDBData.rcsGroup.fileType, OldDBData.rcsGroup.sdkRcsGroupMessageId, OldDBData.rcsGroup.mentionList];
      predicatesInfo = this.buildPredicatesInfo(predicatesInfo, OldDBData.rcsGroup.id, index);
      let resultSet = await rdbStore.query(predicatesInfo, queryColumn);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'no raw Rcs found.');
        return;
      }
      let count = resultSet.rowCount;
      let failCount = 0;
      let successCount = 0;
      do {
        do {
          GROUP_ID = GROUP_ID + 1;
        } while (GROUP_ID_MAP.has(GROUP_ID));
        GROUP_ID_MAP.set(GROUP_ID, GROUP_ID);
        let chatId = resultSet.getLong(resultSet.getColumnIndex(OldDBData.rcsGroup.id));
        let threadId = resultSet.getLong(resultSet.getColumnIndex(OldDBData.rcsGroup.threadId));
        let type = resultSet.getLong(resultSet.getColumnIndex(OldDBData.rcsGroup.type));
        let address = resultSet.getString(resultSet.getColumnIndex(OldDBData.rcsGroup.address));
        let date = resultSet.getLong(resultSet.getColumnIndex(OldDBData.rcsGroup.date));
        let read = resultSet.getLong(resultSet.getColumnIndex(OldDBData.rcsGroup.read));
        let status = resultSet.getLong(resultSet.getColumnIndex(OldDBData.rcsGroup.status));
        let fileMode = resultSet.getLong(resultSet.getColumnIndex(OldDBData.rcsGroup.fileMode));
        let globalDate = resultSet.getString(resultSet.getColumnIndex(OldDBData.rcsGroup.globalDate));
        let body = resultSet.getString(resultSet.getColumnIndex(OldDBData.rcsGroup.body));
        let globalId = resultSet.getString(resultSet.getColumnIndex(OldDBData.rcsGroup.global_id));
        let operateCode = resultSet.getLong(resultSet.getColumnIndex(OldDBData.rcsGroup.operateCode));
        let errorCode = resultSet.getLong(resultSet.getColumnIndex(OldDBData.rcsGroup.errorCode));
        let metaData = resultSet.getString(resultSet.getColumnIndex(OldDBData.rcsGroup.metaData));
        let seen = resultSet.getLong(resultSet.getColumnIndex(OldDBData.rcsGroup.seen));
        let serviceKind = resultSet.getString(resultSet.getColumnIndex(OldDBData.rcsGroup.serviceKind));
        let fileType = resultSet.getLong(resultSet.getColumnIndex(OldDBData.rcsGroup.fileType));
        let sdkRcsGroupMessageId = resultSet.getLong(resultSet.getColumnIndex(OldDBData.rcsGroup.sdkRcsGroupMessageId));
        let mentionList = resultSet.getString(resultSet.getColumnIndex(OldDBData.rcsGroup.mentionList));
        let nameId = await this.getNameId(threadId?.toString(), rdbStore);
        let realId = await this.getRealId(nameId, rdbStore);
        let sessions = RAW_THREAD_ID_MAP.get(realId);
        let sessionId = this.sessionIdBuilder(sessions);
        let msgState: number = 0;
        msgState = this.msgStateBuilder(status, msgState);
        let telephone = THREAD_PHONE_MAP.get(Number(realId)) || [];
        let hasDraft = 0;
        if (type === 112) {
          msgState = 3;
          hasDraft = 1;
          const jsonObject = JSON.parse(body) as Array<LooseObject>;
          let bodyDraft = jsonObject['edittext'] as string;
          let valuesMmsInfoDraft: ValuesBucket = new SmsMmsInfoItem().setMsgTitle(bodyDraft)
            .setMsgContent(bodyDraft).setIsRead(1).setReceiverNumber(telephone.join(',')).setSessionId(sessionId)
            .setStartTime(date).setMsgType(0).setIsSender(0).setRcsId(0)
            .setMsgState(msgState).setSmsType(0).setGroupId(GROUP_ID).createValuesBucket();
          let newRawSmsId = await newDbHelper.insertSms(context, valuesMmsInfoDraft);
          let upValuesDraft: ValuesBucket = {
            'has_draft': hasDraft,
            'content': bodyDraft,
            'time': date,
          }
          await newDbHelper.updateSession(context, sessionId, upValuesDraft);
          continue;
        }
        let length = telephone != undefined ? telephone.length : 0;
        for (let i = 0; i < length; i++) {
          if (fileType !== 0 || type === 112 || telephone == undefined || telephone[i] == undefined) {
            LogUtils.e(TAG, `invalid rcs group field, fileType: ${fileType}, type: ${type}`);
            break;
          }
          let values: ValuesBucket = new RcsInfoItem().setStartTime(date).setRcsType(fileType).setMsgContent(body)
            .setIsRead(read).setReceiverNumber(telephone[i]).setIsSender(type == 1 ? 1 : 0).setSessionId(sessionId)
            .setErrorCode(errorCode).setSeen(seen).setGroupId(GROUP_ID).setMsgState(msgState).setMsgId(globalId)
            .setServiceKind(serviceKind).createValuesBucket();
          let newRawRasInfoId = await newDbHelper.insertRcsInfo(context, values);
          let newRawSmsIdStatus = 0;
          if (newRawRasInfoId != -1) {
            RCS_ID_GROUP.set(newRawRasInfoId, GROUP_ID);
            let valuesMmsInfo: ValuesBucket = new SmsMmsInfoItem()
              .setMsgContent(body)
              .setIsRead(1)
              .setReceiverNumber(telephone[i])
              .setSessionId(sessionId)
              .setStartTime(date)
              .setMsgType(0)
              .setIsSender(type == 1 ? 1 : 0)
              .setRcsId(newRawRasInfoId)
              .setMsgState(msgState)
              .setSmsType(99)
              .setGroupId(GROUP_ID)
              .createValuesBucket();
            let newRawSmsId = await newDbHelper.insertSms(context, valuesMmsInfo);
            newRawSmsIdStatus = newRawSmsId
            let sessionTime = await newDbHelper.getSessionTime(sessionId?.toString(), context)
            await this.renovateSessionDate(sessionTime, date, body, context, sessionId);
          }
          if (newRawRasInfoId === -1 || newRawRasInfoId === -1) {
            failCount++;
          } else {
            successCount++;
          }
        }
      } while (resultSet.goToNextRow());
      resultSet.close();
      progress.updateProcessCount(progress.getProcessCount() + count);
      this.doRcsGroupFailed = this.doRcsGroupFailed + failCount;
      LogUtils.i(TAG, 'queryRcsGroupInfo Total Migrations count: ' + count + ',Successful Migrations successCount: ' + successCount + 'Failed Migrations failCount: ' + failCount);
    } catch (error) {
      LogUtils.w(TAG, 'query RcsGroupInfo error : ' + JSON.stringify(error));
    }
  }

  public async queryMaapInfo(context: common.Context, index: number,
    progress: BackupRestoreProgress): Promise<void> {
    let predicatesInfo = new rdb.RdbPredicates('chatbots');
    let rdbStore = await this.initMaapDB(context);
    if (rdbStore === undefined) {
      LogUtils.e(TAG, `[queryMaapInfo] failed : rdbStore === undefined`);
      return;
    }
    try {
      let queryColumn =
        ['_id', 'service_id', 'service_name', 'service_description', 'callback_phone_number', 'sms', 'service_icon',
          'category_list', 'brief', 'favorite', 'email', 'website', 'address', 'address_lable', 'last_suggested_list',
          'pinyin', 'pinyin_short', 'colour', 'background_image', 'verified', 'verified_by', 'verified_expires',
          'expires', 'cache_control', 'e_tag', 'tc_page', 'recent_use_time', 'disturb', 'persistent_menu', 'share_info',
          'service_provider'];
      predicatesInfo = this.buildPredicatesInfo(predicatesInfo, '_id', index);
      let resultSet = await rdbStore.query(predicatesInfo, queryColumn);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'no raw Rcs found.');
        return;
      }
      let count = resultSet.rowCount;
      let failCount = 0;
      let successCount = 0;
      do {
        let maapID = resultSet.getLong(resultSet.getColumnIndex('_id'));
        let serviceId = resultSet.getString(resultSet.getColumnIndex('service_id'));
        let serviceName = resultSet.getString(resultSet.getColumnIndex('service_name'));
        let serviceDescription = resultSet.getString(resultSet.getColumnIndex('service_description'));
        let callbackPhoneNumber = resultSet.getString(resultSet.getColumnIndex('callback_phone_number'));
        let sms = resultSet.getString(resultSet.getColumnIndex('sms'));
        let serviceIcon = resultSet.getString(resultSet.getColumnIndex('service_icon'));
        let categoryList = resultSet.getString(resultSet.getColumnIndex('category_list'));
        let brief = resultSet.getLong(resultSet.getColumnIndex('brief'));
        let favorite = resultSet.getLong(resultSet.getColumnIndex('favorite'));
        let email = resultSet.getString(resultSet.getColumnIndex('email'));
        let website = resultSet.getString(resultSet.getColumnIndex('website'));
        let address = resultSet.getString(resultSet.getColumnIndex('address'));
        let addressLable = resultSet.getString(resultSet.getColumnIndex('address_lable'));
        let lastSuggestedList = resultSet.getString(resultSet.getColumnIndex('last_suggested_list'));
        let pinyin = resultSet.getString(resultSet.getColumnIndex('pinyin'));
        let pinyin_short = resultSet.getString(resultSet.getColumnIndex('pinyin_short'));
        let colour = resultSet.getString(resultSet.getColumnIndex('colour'));
        let backgroundImage = resultSet.getString(resultSet.getColumnIndex('background_image'));
        let verified = resultSet.getLong(resultSet.getColumnIndex('verified'));
        let verifiedBy = resultSet.getString(resultSet.getColumnIndex('verified_by'));
        let verifiedExpires = resultSet.getString(resultSet.getColumnIndex('verified_expires'));
        let expires = resultSet.getLong(resultSet.getColumnIndex('expires'));
        let cacheControl = resultSet.getString(resultSet.getColumnIndex('cache_control'));
        let eTag = resultSet.getString(resultSet.getColumnIndex('e_tag'));
        let tcPage = resultSet.getString(resultSet.getColumnIndex('tc_page'));
        let recentUseTime = resultSet.getString(resultSet.getColumnIndex('recent_use_time'));
        let disturb = resultSet.getLong(resultSet.getColumnIndex('disturb'));
        let persistentMenu = resultSet.getString(resultSet.getColumnIndex('persistent_menu'));
        let shareInfo = resultSet.getLong(resultSet.getColumnIndex('share_info'));
        let serviceProvider = resultSet.getString(resultSet.getColumnIndex('service_provider'));
        let valuesMaapInfo: ValuesBucket = {
          'service_id': serviceId,
          'service_name': serviceName,
          'service_description': serviceDescription,
          'callback_phone_number': callbackPhoneNumber,
          'sms': sms,
          'service_icon': serviceIcon,
          'category_list': categoryList,
          'brief': brief,
          'favorite': favorite,
          'email': email,
          'website': website,
          'address': address,
          'address_lable': addressLable,
          'last_suggested_list': lastSuggestedList,
          'pinyin': pinyin,
          'pinyin_short': pinyin_short,
          'colour': colour,
          'background_image': backgroundImage,
          'verified': verified,
          'verified_by': verifiedBy,
          'verified_expires': verifiedExpires,
          'expires': expires,
          'cache_control': cacheControl,
          'e_tag': eTag,
          'tc_page': tcPage,
          'recent_use_time': recentUseTime,
          'disturb': disturb,
          'persistent_menu': persistentMenu,
          'share_info': shareInfo,
          'service_provider': serviceProvider
        };
        let newRawMaapId = await newDbHelper.insertChatbots(context, valuesMaapInfo);
        if (newRawMaapId === -1) {
          failCount++;
        } else {
          successCount++;
        }
      } while (resultSet.goToNextRow());
      resultSet.close();
      progress.updateProcessCount(progress.getProcessCount() + count);
      this.doRcsChatbotsFailed = this.doRcsChatbotsFailed + failCount;
      LogUtils.i(TAG, 'queryMaapInfo Total Migrations count: ' + count + ',Successful Migrations successCount: ' + successCount + 'Failed Migrations failCount: ' + failCount);
    } catch (error) {
      LogUtils.w(TAG, 'MaapInfo error : ' + JSON.stringify(error));
    }
  }

  private async renovateSessionDate(sessionTime: number, date: number, body: string, context: common.Context,
    sessionId: number) {
    if (sessionTime <= date) {
      let upSessionValues: ValuesBucket = {
        'content': body,
        'time': date,
      };
      await newDbHelper.updateSession(context, sessionId, upSessionValues);
    }
  }

  private sessionIdBuilder(sessions: number[] | undefined) {
    let sessionId = -1;
    if (sessions) {
      sessionId = sessions[0];
    }
    return sessionId;
  }

  private msgStateBuilder(status: number, msgState: number) {
    if (status == 4) {
      msgState = 2;
    } else if (status == 5) {
      msgState = 1;
    }
    return msgState;
  }

  private async getNameId(arrRecipient: string, rdbStore: rdb.RdbStore) {
    let resultNameId = '';
    try {
      let canonicalPredicatesInfo = new rdb.RdbPredicates('rcs_groups');
      let element = arrRecipient;
      canonicalPredicatesInfo.equalTo('thread_id', element);
      let canResultSet = await rdbStore.query(canonicalPredicatesInfo, ['name']);
      if (canResultSet != undefined && canResultSet.goToFirstRow()) {
        resultNameId = canResultSet.getString(canResultSet.getColumnIndex('name'));
      }
      if (canResultSet != undefined) {
        canResultSet.close();
      }
    } catch (e) {
      LogUtils.w(TAG, 'query address error : ' + JSON.stringify(e));
    }
    return resultNameId;
  }

  private async getRealId(arrRecipient: string, rdbStore: rdb.RdbStore,) {
    let resultRealId = 0;
    try {
      let canonicalPredicatesInfo = new rdb.RdbPredicates('rcs_threads');
      let element = arrRecipient;
      canonicalPredicatesInfo.equalTo('recipient_ids', element);
      let canResultSet = await rdbStore.query(canonicalPredicatesInfo, ['_id']);
      if (canResultSet != undefined && canResultSet.goToFirstRow()) {
        resultRealId = canResultSet.getLong(canResultSet.getColumnIndex('_id'));
      }
      if (canResultSet != undefined) {
        canResultSet.close();
      }
    } catch (e) {
      LogUtils.w(TAG, 'query real id error : ' + JSON.stringify(e));
    }
    return resultRealId;
  }

  private buildPredicatesInfo(predicatesInfo: rdb.RdbPredicates, id: string, index: number): rdb.RdbPredicates {
    predicatesInfo.orderByAsc(id)
      .limitAs(DbConstants.QUERY_PAGE_NUM)
      .offsetAs(index);
    return predicatesInfo;
  }

  private async initDB(context: common.Context): Promise<rdb.RdbStore | undefined> {
    if (!context) {
      LogUtils.w(TAG, '[createDB] context is empty');
      return undefined;
    }
    if (this.rdbStoreHelper) {
      return Promise.resolve(this.rdbStoreHelper);
    }
    try {
      const OLD_CONFIG: rdb.StoreConfig = {
        name: DbConstants.OLD_DATABASE_NAME,
        securityLevel: rdb.SecurityLevel.S1,
      };
      LogUtils.i(TAG, '[createDB] OLD_CONFIG');
      this.rdbStoreHelper = await rdb.getRdbStore(context, OLD_CONFIG);
    } catch (error) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `initDB code:${error.code} msg:${error.message}`);
      LogUtils.w(TAG, '[createDB] failed : ' + JSON.stringify(error));
    }
    if (this.rdbStoreHelper) {
      return Promise.resolve(this.rdbStoreHelper);
    }
    return undefined;
  }

  private async initMaapDB(context: common.Context): Promise<rdb.RdbStore | undefined> {
    if (!context) {
      LogUtils.w(TAG, '[createDB] context is empty');
      return undefined;
    }
    if (this.rdbMaapStoreHelper) {
      return Promise.resolve(this.rdbMaapStoreHelper);
    }
    try {
      const OLD_CONFIG: rdb.StoreConfig = {
        name: 'maap.db',
        securityLevel: rdb.SecurityLevel.S1,
      };
      LogUtils.i(TAG, '[createDB] OLD_CONFIG');
      this.rdbMaapStoreHelper = await rdb.getRdbStore(context, OLD_CONFIG);
    } catch (error) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `initMaapDB code:${error.code} msg:${error.message}`);
      LogUtils.w(TAG, '[createDB] failed : ' + JSON.stringify(error));
    }
    if (this.rdbMaapStoreHelper) {
      return Promise.resolve(this.rdbMaapStoreHelper);
    }
    return undefined;
  }

  private extractRecordingTime(text: string): string | undefined {
    let str = text.toLowerCase();
    if (str.indexOf('audio') === -1) {
      return undefined;
    }
    let recordTime = str.substring(str.indexOf('dur="') + 5, str.indexOf('"', str.indexOf('dur="') + 5))
      .replace('ms', '');
    if (recordTime.indexOf('s') >= 0) {
      recordTime = String(Number(recordTime.replace('s', '')) * 1000);
    }
    return this.formatMilliseconds(recordTime);
  }

  private async setAudioTime(text: string, msgId: number): Promise<void> {
    const recordingTime = this.extractRecordingTime(text);
    if (recordingTime !== undefined) {
      LogUtils.i(TAG, 'recordingTime is:' + recordingTime + ', msgId = ' + msgId);
      MMS_ID_RECORDING_TIME_MAP.set(msgId, recordingTime);
    }
  }

  private formatMilliseconds(value: string): string {
    let second = Math.floor((Number(value) / 1000)); // second
    let minute = 0; // minute
    if (second >= 60) {
      minute = Math.floor(second / 60);
      second = second % 60;
    }
    let timeStr: string = ((minute >= 10) ? minute : ('0' + minute)) + ':' +
      ((second >= 10) ? second : ('0' + second));
    return timeStr;
  }

  public async updateRecordTime(context: common.Context) {
    let partId = 0;
    try {
      let resultSet = await newDbHelper.queryPart(context);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'No data exists in the part table, do nothing.');
        return;
      }
      do {
        partId = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.mmsPart.id));
        let msgId = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.mmsPart.msgId));
        let type = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.mmsPart.type));
        LogUtils.i(TAG, 'updateRecordTime msgId = ' + msgId + ', type = ' + type);
        if (MMS_ID_RECORDING_TIME_MAP.hasKey(msgId) && type === 3) {
          let upValues: ValuesBucket = {
            'recording_time': MMS_ID_RECORDING_TIME_MAP.get(msgId)
          }
          await newDbHelper.updatePartByRecordTime(context, msgId, type, upValues);
        }
      } while (resultSet.goToNextRow());
      LogUtils.i(TAG, 'update part table end.');
    } catch (error) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `updateRecordTime code:${error.code} msg:${error.message}`);
      LogUtils.w(TAG, 'update part table failed, partId = ' + partId);
    }
  }

  public async updateSmsMmsInfoByContent(context: common.Context) {
    try {
      let resultSet = await newDbHelper.querySms(context);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'No data exists in the SmsMms table, do nothing.');
        return;
      }
      do {
        let msgId = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.msgId));
        if (MMS_ID_TEXT_MAP.hasKey(msgId)) {
          let upValues: ValuesBucket = {
            'msg_content': MMS_ID_TEXT_MAP.get(msgId)
          }
          await newDbHelper.updateSmsMmsInfoByContent(context, msgId, upValues);
        }
      } while (resultSet.goToNextRow());
      LogUtils.i(TAG, 'update SmsMmsInfo table end.');
    } catch (error) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `updateSmsMmsInfoByContent code:${error.code} msg:${error.message}`);
      LogUtils.w(TAG, 'update SmsMmsInfo table failed ');
    }
  }

  public async updateSessionDraft(context: common.Context) {
    try {
      let resultSet = await newDbHelper.querySms(context);
      if (resultSet == undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'No data exists in the Session table, do nothing.');
        return;
      }
      do {
        let sessionId = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.sessionId));
        let msgType = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.msgType));
        let msgState = resultSet.getLong(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.msgState));
        let msgContent = resultSet.getString(resultSet.getColumnIndex(NextOldDBData.smsMmsInfo.msgContent));
        if (msgType == 1 && msgState == 3) {
          let upValues: ValuesBucket = {
            'content': msgContent
          }
          await newDbHelper.updateSession(context, sessionId, upValues);
        }
      } while (resultSet.goToNextRow());
      LogUtils.i(TAG, 'update Session Draft table end.');
    } catch {
      LogUtils.w(TAG, 'update Session Draft table failed ');
    }
  }

  public setVersion(version: string) {
    this.versionName = version;
    const backupRestoreResult = BackupRestoreResult.getInstance();
    backupRestoreResult.insertOrUpdateBackupInfo('sms', `versionName :${version}`);
  }

  public async queryUnreadCount(context: Context, progress: BackupRestoreProgress): Promise<number> {
    if (context == null) {
      LogUtils.w(TAG, `queryUnreadCount failed by null context`)
      return Promise.resolve(0);
    }
    let totalUnreadCount: number = 0;
    try {
      let rdbStore = await this.initDB(context);
      if (rdbStore === undefined) {
        LogUtils.e(TAG, `[queryUnreadCount] failed : rdbStore === undefined`)
        return Promise.resolve(0);
      }
      let sql: string = 'SELECT \n' +
        '( SELECT COUNT( * ) FROM chat WHERE read = 0 ) + \n' +
        '( SELECT COUNT( * ) FROM rcs_group_message WHERE read = 0 ) + \n' +
        '( SELECT COUNT( * ) FROM pdu WHERE read = 0 ) + \n' +
        '( SELECT COUNT( * ) FROM sms WHERE read = 0 ) AS total_count;';
      let resultSet: rdb.ResultSet = rdbStore.querySqlSync(sql);
      if (resultSet === undefined) {
        LogUtils.w(TAG, 'resultSet === undefined');
        return Promise.resolve(0);
      }
      if (resultSet != undefined && resultSet.goToFirstRow()) {
        totalUnreadCount = resultSet.getLong(resultSet.getColumnIndex('total_count'));
      }
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `ori_unread_count :${totalUnreadCount}`);
      return totalUnreadCount;
    } catch (err) {
      const backupRestoreResult = BackupRestoreResult.getInstance();
      backupRestoreResult.insertOrUpdateBackupInfo('sms',
        `queryUnreadCount code:${err.code} msg:${err.message}`);
      LogUtils.e(TAG, `queryUnreadCount failed, code is ${err.code},message is ${err.message}`);
      return Promise.resolve(0);
    }
  }

  public async queryTotalMsgCount(context: Context, localId: number, progress: BackupRestoreProgress) {
    let addressCount = await dBHelper.queryAddressCount(context);
    let threadsCount = await this.queryThreadsCount(context, localId);
    let rcsThreadsCount = await this.queryRcsThreadsCount(context, localId);
    let smsCount = await this.querySmsCount(context, localId);
    let pduCount = await this.queryPduCount(context, localId);
    let partCount = await this.queryPartCount(context);
    let chatCount = await this.queryChatCount(context, localId);
    let groupCount = await this.queryGroupCount(context);
    let maapCount = await this.queryMaapCount(context);
    let favSmsCount = await this.queryFavSmsCount(context);
    let favMmsCount = await this.queryFavMmsCount(context);
    let favorateCount = favSmsCount + favMmsCount;
    // The threadsCount value is calculated twice to indicate the unread progress of subsequent processing.
    let totalCount = addressCount + threadsCount + rcsThreadsCount + smsCount + pduCount + partCount + chatCount +
      groupCount + threadsCount + favorateCount + maapCount;
    // 迁移完成后清理无效会话、info提供count总量计数
    let invalidItemCount = threadsCount + rcsThreadsCount + smsCount + pduCount + chatCount + groupCount;
    totalCount = totalCount + invalidItemCount;
    let unSupportItemCount = rcsThreadsCount + pduCount;
    totalCount = totalCount + unSupportItemCount;
    progress.setInvalidItemCount(invalidItemCount);
    progress.setUnSupportItemCount(unSupportItemCount);
    progress.setAddressCount(addressCount);
    progress.setThreadCount(threadsCount);
    progress.setRcsThreadCount(rcsThreadsCount);
    progress.setSmsCount(smsCount);
    progress.setPduCount(pduCount);
    progress.setPartCount(partCount);
    progress.setRcsChatCount(chatCount);
    progress.setRcsGroupCount(groupCount);
    progress.setMaapCount(maapCount);
    progress.setFavoriteCount(favorateCount);
    progress.setFavSmsCount(favSmsCount);
    progress.setFavMmsCount(favMmsCount);
    let countList: string = 'addr[' + addressCount + '],thread[' + threadsCount + '],rcsThread[' + rcsThreadsCount +
      '],sms[' + smsCount + '],pdu[' + pduCount + '],part[' + partCount + '],rcsChat[' + chatCount +
      '],rcsGroup[' + groupCount + '],favorateCount[' + favorateCount + '],maapCount[' + maapCount +
      '],favSms[' + favSmsCount + '],favMms[' + favMmsCount + ']';
    LogUtils.i(TAG, `update progress total count ${totalCount}, listed: ${countList}`);
    const backupRestoreResult = BackupRestoreResult.getInstance();
    backupRestoreResult.insertOrUpdateBackupInfo('sms', `msg_count_list : ${countList}`);
    progress.setTotalCount(totalCount);
    return totalCount;
  }

  public async getUpgradeResult(context: common.Context, localId: number): Promise<string> {
    let smsCount = await this.querySmsCount(context, localId);
    let pduCount = await this.queryPduCount(context, localId);
    let rcsCount = await this.queryChatCount(context, localId);
    let rcsGroupCount = await this.queryGroupCount(context);
    let failCount =
      this.doMmsFailed + this.doPduFailed + this.doRcsChatFailed + this.doRcsGroupFailed + this.doRcsChatbotsFailed;
    failCount = failCount + this.doFavSmsFailed;
    let successCounts = smsCount + pduCount + rcsCount + rcsGroupCount - failCount;
    const backupRestoreResult = BackupRestoreResult.getInstance();
    backupRestoreResult.addErrorInfo(failCount <= 0 ? BackupRestoreResult.RESTORE_SUCCESS
      : BackupRestoreResult.RESTORE_COMMON_ERROR, '');
    backupRestoreResult.insertOrUpdateBackupInfo('sms', '', successCounts, 0, failCount);
    return backupRestoreResult.formatRestoreResult();
  }

  public async queryFavorateCount(context: common.Context): Promise<number> {
    if (context == null) {
      LogUtils.w(TAG, `queryFavoriteCount failed by null context`)
      return Promise.resolve(0);
    }
    let count = 0;
    try {
      let predicatesInfo = new rdb.RdbPredicates(DbConstants.TABLE_SMS_MMS_INFO);
      let rdbStore = await this.initDB(context);
      if (rdbStore === undefined) {
        LogUtils.e(TAG, `[queryPduCount] failed : rdbStore === undefined`)
        return Promise.resolve(0);
      }
      predicatesInfo.equalTo('is_collect', 1)
      let queryColumn = ['count(msg_id)'];
      let resultSet = await rdbStore.query(predicatesInfo, queryColumn);
      if (resultSet === undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'query no favorate found.');
        return Promise.resolve(0);
      }
      count = resultSet.getLong(0);
      LogUtils.i(TAG, '[queryFavoriteCount] count : ' + count);
      resultSet.close();
    } catch (error) {
      LogUtils.e(TAG, '[queryFavoriteCount] error : ' + JSON.stringify(error))
    }
    return Promise.resolve(count);
  }

  public getExceptionResult(): string {
    const backupRestoreResult = BackupRestoreResult.getInstance();
    backupRestoreResult.addErrorInfo(BackupRestoreResult.RESTORE_COMMON_ERROR, '');
    backupRestoreResult.insertOrUpdateBackupInfo('sms', '', 0, 0, 0);
    return backupRestoreResult.formatRestoreResult();
  }
}
export const dBHelper: DbHelper = new DbHelper();