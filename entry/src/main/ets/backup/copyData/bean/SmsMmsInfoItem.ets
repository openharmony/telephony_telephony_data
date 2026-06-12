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
import LogUtils from '../../../common/utils/LogUtils';

export default class SmsMmsInfoItem {
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
  private sessionType: number = 3;
  private retryNumber: number = 0;
  private sessionId: number = -1;
  private groupId: number = -1;
  private deviceId: number = 0;
  private isSubsection: number = 0;
  private isSendReport: number = 0;
  private msgCode: number = 0;
  private isAdvancedSecurity: number = 0;
  private phoneNumber: string = '';
  private formatPhoneNumber: string = '';
  private rcsId : number = 0;
  private expiresTime: number = 0;
  private mmsPdu: string = '';
  private isBlocked: number = 0;
  private blockedReason: number = 0;
  private blockedSources: string = '';
  private blockedType: number = 0;
  private blockedTypeText: string = '';
  private riskUrlBody: string = '';
  private hasBeReportedAsSpam: number = 0;
  private isReport: number = 0;
  private detectResContent: string = '';
  private msgCodeStr: string = '';
  private blockedPrefNumber: string = '';

  public setSlotId(slotId: number | undefined) {
    this.slotId = slotId != undefined ? slotId : this.slotId;
    return this;
  }

  public setReceiverNumber(receiverNumber: string | undefined) {
    this.receiverNumber = receiverNumber != undefined ? receiverNumber : this.receiverNumber;
    return this;
  }

  public setSenderNumber(senderNumber: string | undefined) {
    this.senderNumber = senderNumber != undefined ? senderNumber : this.senderNumber;
    return this;
  }

  public setIsSender(isSender: number | undefined) {
    this.isSender = isSender != undefined ? isSender : this.isSender;
    return this;
  }

  public setMsgType(msgType: number | undefined) {
    this.msgType = msgType != undefined ? msgType : this.msgType;
    return this;
  }

  public setSmsType(smsType: number | undefined) {
    this.smsType = smsType != undefined ? smsType : this.smsType;
    return this;
  }

  public setStartTime(startTime: number | undefined) {
    this.startTime = startTime != undefined ? startTime.toString() : this.startTime;
    return this;
  }

  public setEndTime(endTime: number | undefined) {
    this.endTime = endTime != undefined ? endTime.toString() : this.endTime;
    return this;
  }

  public setMsgState(msgState: number | undefined) {
    this.msgState = msgState != undefined ? msgState : this.msgState;
    return this;
  }

  public setMsgTitle(msgTitle: string | undefined) {
    this.msgTitle = msgTitle != undefined ? msgTitle : this.msgTitle;
    return this;
  }

  public setMsgContent(msgContent: string | undefined) {
    this.msgContent = msgContent != undefined ? msgContent : this.msgContent;
    return this;
  }

  public setOperatorServiceNumber(operatorServiceNumber: string | undefined) {
    this.operatorServiceNumber = operatorServiceNumber != undefined ?
      operatorServiceNumber : this.operatorServiceNumber;
    return this;
  }

  public setIsLock(isLock: number | undefined) {
    this.isLock = isLock != undefined ? isLock : this.isLock;
    return this;
  }

  public setIsCollect(isCollect: number | undefined) {
    this.isCollect = isCollect != undefined ? isCollect : this.isCollect;
    return this;
  }

  public setIsRead(isRead: number | undefined) {
    this.isRead = isRead != undefined ? isRead : this.isRead;
    return this;
  }

  public setSessionType(sessionType: number | undefined) {
    this.sessionType = sessionType != undefined ? sessionType : this.sessionType;
    return this;
  }

  public setRetryNumber(retryNumber: number | undefined) {
    this.retryNumber = retryNumber != undefined ? retryNumber : this.retryNumber;
    return this;
  }

  public setSessionId(sessionId: number | undefined) {
    this.sessionId = sessionId != undefined ? sessionId : this.sessionId;
    return this;
  }

  public setGroupId(groupId: number | undefined) {
    this.groupId = groupId != undefined ? groupId : this.groupId;
    return this;
  }

  public setDeviceId(deviceId: number | undefined) {
    this.deviceId = deviceId != undefined ? deviceId : this.deviceId;
    return this;
  }

  public setIsSubsection(isSubsection: number | undefined) {
    this.isSubsection = isSubsection != undefined ? isSubsection : this.isSubsection;
    return this;
  }

  public setIsSendReport(isSendReport: number | undefined) {
    this.isSendReport = isSendReport != undefined ? isSendReport : this.isSendReport;
    return this;
  }

  public setMsgCode(msgCode: number | undefined) {
    this.msgCode = msgCode != undefined ? msgCode : this.msgCode;
    return this;
  }

  public setIsAdvancedSecurity(isAdvancedSecurity: number | undefined) {
    this.isAdvancedSecurity = isAdvancedSecurity != undefined ? isAdvancedSecurity : this.isAdvancedSecurity;
    return this;
  }

  public setPhoneNumber(phoneNumber: string | undefined) {
    this.phoneNumber = phoneNumber != undefined ? phoneNumber : this.phoneNumber;
    return this;
  }

  public setFormatPhoneNumber(formatPhoneNumber: string | undefined) {
    this.formatPhoneNumber = formatPhoneNumber != undefined ? formatPhoneNumber : this.formatPhoneNumber;
    return this;
  }

  public setRcsId(rcsId: number | undefined) {
    this.rcsId = rcsId != undefined ? rcsId : this.rcsId;
    return this;
  }

  public setExpiresTime(expiresTime: number | undefined) {
    this.expiresTime = expiresTime != undefined ? expiresTime : this.expiresTime;
    return this;
  }

  public setIsBlocked(isBlocked: number | undefined) {
    this.isBlocked = isBlocked != undefined ? isBlocked : this.isBlocked;
    return this;
  }

  public setMmsPdu(mmsPdu: string | undefined) {
    this.mmsPdu = mmsPdu != undefined ? mmsPdu : this.mmsPdu;
    return this;
  }

  public setBlockedReason(blockedReason: number | undefined) {
    this.blockedReason = blockedReason != undefined ? blockedReason : this.blockedReason;
    return this;
  }

  public setBlockedType(blockedType: number | undefined) {
    this.blockedType = blockedType != undefined ? blockedType : this.blockedType;
    return this;
  }

  public setHasBeReportedAsSpam(hasBeReportedAsSpam: number | undefined) {
    this.hasBeReportedAsSpam = hasBeReportedAsSpam != undefined ? hasBeReportedAsSpam : this.hasBeReportedAsSpam;
    return this;
  }

  public setIsReport(isReport: number | undefined) {
    this.isReport = isReport != undefined ? isReport : this.isReport;
    return this;
  }

  public setBlockedSources(blockedSources: string | undefined) {
    this.blockedSources = blockedSources != undefined ? blockedSources : this.blockedSources;
    return this;
  }

  public setBlockedTypeText(blockedTypeText: string | undefined) {
    this.blockedTypeText = blockedTypeText != undefined ? blockedTypeText : this.blockedTypeText;
    return this;
  }

  public setRiskUrlBody(riskUrlBody: string | undefined) {
    this.riskUrlBody = riskUrlBody != undefined ? riskUrlBody : this.riskUrlBody;
    return this;
  }

  public setDetectResContent(detectResContent: string | undefined) {
    this.detectResContent = detectResContent != undefined ? detectResContent : this.detectResContent;
    return this;
  }

  public setBlockedPrefNumber(blockedPrefNumber: string | undefined) {
    this.blockedPrefNumber = blockedPrefNumber != undefined ? blockedPrefNumber : this.blockedPrefNumber;
    return this;
  }

  public setMsgCodeStr(msgCodeStr: string | undefined) {
    this.msgCodeStr = msgCodeStr != undefined ? msgCodeStr : this.msgCodeStr;
    return this;
  }

  public createValuesBucket(): ValuesBucket {
    const valuesBucket: ValuesBucket = {
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
      'operator_service_number': this.operatorServiceNumber,
      'is_lock': this.isLock,
      'is_collect': this.isCollect,
      'is_read': this.isRead,
      'session_type': this.sessionType,
      'retry_number': this.retryNumber,
      'session_id': this.sessionId,
      'group_id': this.groupId,
      'device_id': this.deviceId,
      'is_subsection': this.isSubsection,
      'is_send_report': this.isSendReport,
      'msg_code': this.msgCode,
      'is_advanced_security': this.isAdvancedSecurity,
      'phone_number': this.phoneNumber,
      'rcs_id': this.rcsId,
      'format_phone_number': this.formatPhoneNumber,
      'expires_time' : this.expiresTime,
      'mms_pdu' : this.mmsPdu,
      'is_blocked' : this.isBlocked,
      'blocked_reason' : this.blockedReason,
      'blocked_sources' : this.blockedSources,
      'blocked_type' : this.blockedType,
      'blocked_type_text' : this.blockedTypeText,
      'risk_url_body' : this.riskUrlBody,
      'has_be_reported_as_spam' : this.hasBeReportedAsSpam,
      'is_report' : this.isReport,
      'detect_res_content' : this.detectResContent,
      'msg_code_str' :this.msgCodeStr,
      'blocked_pref_number':this.blockedPrefNumber
    };
    return valuesBucket;
  }
}