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

import { ValuesBucket } from '@ohos.data.ValuesBucket';

export default class SmsMmsInfoItem {
  private msgId: number = 0;
  private slotId: number = 0;
  private receiverNumber: string = '';
  private senderNumber: string = '';
  private isSender: number = 0;
  private msgType: number = 0;
  private smsType: number = 0;
  private startTime: string = '';
  private endTime: string = '';
  private msgState: number = 0;
  private msgTitle: string = '';
  private msgContent: string = '';
  private operatorServiceNumber: string = '';
  private isLock: number = 0;
  private isCollect: number = 0;
  private isRead: number = 1;
  private sessionType: number = 0;
  private retryNumber: number = 0;
  private sessionId: number = -1;
  private groupId: number = -1;
  private deviceId: number = 0;
  private isSubsection: number = 0;
  private isSendReport: number = 0;
  private msgCode: number = 0;
  private rcsId : number = 0;
  private phoneNumber: string = '';
  private formatPhoneNumber: string = '';

  public setRcsId(rcsId: number) {
    this.rcsId = rcsId != undefined ? rcsId : this.rcsId;
    return this;
  }

  public setMsgId(msgId: number) {
    this.msgId = msgId != undefined ? msgId : this.msgId;
    return this;
  }

  public setSlotId(slotId: number) {
    this.slotId = slotId != undefined ? slotId : this.slotId;
    return this;
  }

  public setReceiverNumber(receiverNumber: string) {
    this.receiverNumber = receiverNumber != undefined ? receiverNumber : this.receiverNumber;
    return this;
  }

  public setSenderNumber(senderNumber: string) {
    this.senderNumber = senderNumber != undefined ? senderNumber : this.senderNumber;
    return this;
  }

  public setIsSender(isSender: number) {
    this.isSender = isSender != undefined ? isSender : this.isSender;
    return this;
  }

  public setMsgType(msgType: number) {
    this.msgType = msgType != undefined ? msgType : this.msgType;
    return this;
  }

  public setSmsType(smsType: number) {
    this.smsType = smsType != undefined ? smsType : this.smsType;
    return this;
  }

  public setStartTime(startTime: number) {
    this.startTime = startTime != undefined ? startTime.toString() : this.startTime;
    return this;
  }

  public setEndTime(endTime: number) {
    this.endTime = endTime != undefined ? endTime.toString() : this.endTime;
    return this;
  }

  public setMsgState(msgState: number) {
    this.msgState = msgState != undefined ? msgState : this.msgState;
    return this;
  }

  public setMsgTitle(msgTitle: string) {
    this.msgTitle = msgTitle != undefined ? msgTitle : this.msgTitle;
    return this;
  }

  public setMsgContent(msgContent: string) {
    this.msgContent = msgContent != undefined ? msgContent : this.msgContent;
    return this;
  }

  public setOperatorServiceNumber(operatorServiceNumber: string) {
    this.operatorServiceNumber = operatorServiceNumber != undefined ?
      operatorServiceNumber : this.operatorServiceNumber;
    return this;
  }

  public setIsLock(isLock: number) {
    this.isLock = isLock != undefined ? isLock : this.isLock;
    return this;
  }

  public setIsCollect(isCollect: number) {
    this.isCollect = isCollect != undefined ? isCollect : this.isCollect;
    return this;
  }

  public setIsRead(isRead: number) {
    this.isRead = isRead != undefined ? isRead : this.isRead;
    return this;
  }

  public setSessionType(sessionType: number) {
    this.sessionType = sessionType != undefined ? sessionType : this.sessionType;
    return this;
  }

  public setRetryNumber(retryNumber: number) {
    this.retryNumber = retryNumber != undefined ? retryNumber : this.retryNumber;
    return this;
  }

  public setSessionId(sessionId: number) {
    this.sessionId = sessionId != undefined ? sessionId : this.sessionId;
    return this;
  }

  public setGroupId(groupId: number) {
    this.groupId = groupId != undefined ? groupId : this.groupId;
    return this;
  }

  public setDeviceId(deviceId: number) {
    this.deviceId = deviceId != undefined ? deviceId : this.deviceId;
    return this;
  }

  public setIsSubsection(isSubsection: number) {
    this.isSubsection = isSubsection != undefined ? isSubsection : this.isSubsection;
    return this;
  }

  public setIsSendReport(isSendReport: number) {
    this.isSendReport = isSendReport != undefined ? isSendReport : this.isSendReport;
    return this;
  }

  public setMsgCode(msgCode: number) {
    this.msgCode = msgCode != undefined ? msgCode : this.msgCode;
    return this;
  }

  public setPhoneNumber(phoneNumber: string) {
    this.phoneNumber = phoneNumber != undefined ? phoneNumber : this.phoneNumber;
    return this;
  }

  public setFormatPhoneNumber(formatPhoneNumber: string) {
    this.formatPhoneNumber = formatPhoneNumber != undefined ? formatPhoneNumber : this.formatPhoneNumber;
    return this;
  }

  public createValuesBucket(): ValuesBucket {
    const valuesBucket: ValuesBucket = {
      // 'msg_id': this.msgId,
      'slot_id': this.slotId,
      'receiver_number': this.receiverNumber,
      'sender_number': this.senderNumber,
      'is_sender': this.isSender,
      'msg_type': this.msgType,
      'sms_type': this.smsType,
      'start_time': this.startTime,
      'end_time': this.endTime,
      'msg_state': this.msgState,
      'msg_title': this.msgTitle,
      'msg_content': this.msgContent,
      // 'operator_service_number': this.operatorServiceNumber,
      'is_lock': this.isLock,
      'is_collect': this.isCollect,
      'is_read': this.isRead,
      // 'session_type': this.sessionType,
      // 'retry_number': this.retryNumber,
      'session_id': this.sessionId,
      'group_id': this.groupId,
      // 'device_id': this.deviceId,
      // 'is_subsection': this.isSubsection,
      // 'is_send_report': this.isSendReport,
      // 'msg_code': this.msgCode
      'rcs_id': this.rcsId,
      'phone_number': this.phoneNumber,
      'format_phone_number': this.formatPhoneNumber,
    };
    return valuesBucket;
  }
}