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

export default class RcsInfoItem {
  private slotId: number = 0;
  private receiverNumber: string = '';
  private senderNumber: string = '';
  private startTime: string = '';
  private endTime: string = '';
  private rcsType: number = 0;
  private msgState: number = 0;
  private msgTitle: string = '';
  private msgContent: string = '';
  private isLock: number = 0;
  private isRead: number = 1;
  private isSender: number = 0;
  private sessionId: number = -1;
  private groupId: number = -1;
  private msgId: string = '';
  private enrichedCallingType: number = 0;
  private errorCode: number = 0;
  private networkType: string = '';
  private ownerAddr: string = '';
  private privacyMode: number = 0;
  private protocol: number = 0;
  private replyPathPresent: number = 0;
  private seen: number = 0;
  private serviceCenter: string = '';
  private serviceKind: string = '';
  private phoneNumber: string = '';
  private formatPhoneNumber: string = '';
  private isBlocked: number = 0;
  private blockedReason: number = 0;
  private blockedSources: string = '';
  private blockedType: number = 0;
  private blockedTypeText: string = '';
  private clurSize: number = 0;
  private totalSize: number = 0;
  private riskUrlBody: string = '';
  private hasBeReportedAsSpam: number = 0;
  private isReport: number = 0;
  private detectResContent: string = '';
  private blockedPrefNumber: string = '';
  private isCollect: number = 0;

  public setSlotId(slotId: number | undefined) {
    this.slotId = slotId != undefined ? slotId : this.slotId;
    return this;
  }

  public setIsCollect(isCollect: number | undefined) {
    this.isCollect = isCollect != undefined ? isCollect : this.isCollect;
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

  public setStartTime(startTime: string | undefined) {
    this.startTime = startTime != undefined ? startTime : this.startTime;
    return this;
  }

  public setEndTime(endTime: string | undefined) {
    this.endTime = endTime != undefined ? endTime : this.endTime;
    return this;
  }

  public setRcsType(rcsType: number | undefined) {
    this.rcsType = rcsType != undefined ? rcsType : this.rcsType;
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

  public setIsLock(isLock: number | undefined) {
    this.isLock = isLock != undefined ? isLock : this.isLock;
    return this;
  }

  public setIsRead(isRead: number | undefined) {
    this.isRead = isRead != undefined ? isRead : this.isRead;
    return this;
  }

  public setIsSender(isSender: number | undefined) {
    this.isSender = isSender != undefined ? isSender : this.isSender;
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

  public setMsgId(msgId: string | undefined) {
    this.msgId = msgId != undefined ? msgId : this.msgId;
    return this;
  }

  public setEnrichedCallingType(enrichedCallingType: number | undefined) {
    this.enrichedCallingType = enrichedCallingType != undefined ? enrichedCallingType : this.enrichedCallingType;
    return this;
  }

  public setErrorCode(errorCode: number | undefined) {
    this.errorCode = errorCode != undefined ? errorCode : this.errorCode;
    return this;
  }

  public setNetworkType(networkType: string | undefined) {
    this.networkType = networkType != undefined ? networkType : this.networkType;
    return this;
  }

  public setOwnerAddr(ownerAddr: string | undefined) {
    this.ownerAddr = ownerAddr != undefined ? ownerAddr : this.ownerAddr;
    return this;
  }

  public setPrivacyMode(privacyMode: number | undefined) {
    this.privacyMode = privacyMode != undefined ? privacyMode : this.privacyMode;
    return this;
  }

  public setProtocol(protocol: number | undefined) {
    this.protocol = protocol != undefined ? protocol : this.protocol;
    return this;
  }

  public setReplyPathPresent(replyPathPresent: number | undefined) {
    this.replyPathPresent = replyPathPresent != undefined ? replyPathPresent : this.replyPathPresent;
    return this;
  }

  public setSeen(seen: number | undefined) {
    this.seen = seen != undefined ? seen : this.seen;
    return this;
  }

  public setServiceCenter(serviceCenter: string | undefined) {
    this.serviceCenter = serviceCenter != undefined ? serviceCenter : this.serviceCenter;
    return this;
  }

  public setServiceKind(serviceKind: string | undefined) {
    this.serviceKind = serviceKind != undefined ? serviceKind : this.serviceKind;
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

  public setIsBlocked(isBlocked: number | undefined) {
    this.isBlocked = isBlocked != undefined ? isBlocked : this.isBlocked;
    return this;
  }

  public setBlockedReason(blockedReason: number | undefined) {
    this.blockedReason = blockedReason != undefined ? blockedReason : this.blockedReason;
    return this;
  }

  public setBlockedSources(blockedSources: string | undefined) {
    this.blockedSources = blockedSources != undefined ? blockedSources : this.blockedSources;
    return this;
  }

  public setBlockedType(blockedType: number | undefined) {
    this.blockedType = blockedType != undefined ? blockedType : this.blockedType;
    return this;
  }

  public setBlockedTypeText(blockedTypeText: string | undefined) {
    this.blockedTypeText = blockedTypeText != undefined ? blockedTypeText : this.blockedTypeText;
    return this;
  }

  public setClurSize(clurSize: number | undefined) {
    this.clurSize = clurSize != undefined ? clurSize : this.clurSize;
    return this;
  }

  public setTotalSize(totalSize: number | undefined) {
    this.totalSize = totalSize != undefined ? totalSize : this.totalSize;
    return this;
  }

  public setRiskUrlBody(riskUrlBody: string | undefined) {
    this.riskUrlBody = riskUrlBody != undefined ? riskUrlBody : this.riskUrlBody;
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

  public setDetectResContent(detectResContent: string | undefined) {
    this.detectResContent = detectResContent != undefined ? detectResContent : this.detectResContent;
    return this;
  }

  public setBlockedPrefNumber(blockedPrefNumber: string | undefined) {
    this.blockedPrefNumber = blockedPrefNumber != undefined ? blockedPrefNumber : this.blockedPrefNumber;
    return this;
  }

  public createValuesBucket(): ValuesBucket {
    const valuesBucket: ValuesBucket = {
      'slot_id': this.slotId,
      'receiver_number': this.receiverNumber,
      'sender_number': this.senderNumber,
      'start_time': this.startTime,
      'end_time': this.endTime,
      'rcs_type': this.rcsType,
      'msg_state': this.msgState,
      'msg_title': this.msgTitle,
      'msg_content': this.msgContent,
      'is_lock': this.isLock,
      'is_read': this.isRead,
      'is_sender': this.isSender,
      'session_id': this.sessionId,
      'group_id': this.groupId,
      'msg_id': this.msgId,
      'enriched_calling_type': this.enrichedCallingType,
      'error_code': this.errorCode,
      'network_type': this.networkType,
      'owner_addr': this.ownerAddr,
      'privacy_mode': this.privacyMode,
      'protocol': this.protocol,
      'reply_path_present': this.replyPathPresent,
      'seen': this.seen,
      'service_center': this.serviceCenter,
      'service_kind': this.serviceKind,
      'phone_number': this.phoneNumber,
      'format_phone_number': this.formatPhoneNumber,
      'is_blocked': this.isBlocked,
      'blocked_reason': this.blockedReason,
      'blocked_sources': this.blockedSources,
      'blocked_type': this.blockedType,
      'blocked_type_text':this.blockedTypeText,
      'clur_size':this.clurSize,
      'total_size':this.totalSize,
      'risk_url_body':this.riskUrlBody,
      'has_be_reported_as_spam':this.hasBeReportedAsSpam,
      'is_report':this.isReport,
      'detect_res_content':this.detectResContent,
      'blocked_pref_number':this.blockedPrefNumber,
      'is_collect':this.isCollect
    };
    return valuesBucket;
  }
}