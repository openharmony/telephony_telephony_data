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

export default class SessionItem {
  private time: number = 0;
  private telephone: string = '';
  private content: string = '';
  private contactsNum: number = 0;
  private unreadCount: number = 0;
  private smsType: number = 0;
  private sendingStatus: number = 0;
  private hasLock: number = 0;
  private hasDraft: number = 0;
  private messageCount: number = 0;
  private hasMms: number = 0;
  private hasAttachment: number = 0;
  private yellowPageId: string = '';
  private blockedType: number = 0;
  private pinningTime: number = 0;

  public setTime(time: number | undefined) {
    this.time = time != undefined ? time : this.time;
    return this;
  }

  public setTelephone(telephone: string | undefined) {
    this.telephone = telephone != undefined ? telephone : this.telephone;
    return this;
  }

  public setContent(content: string | undefined) {
    this.content = content != undefined ? content : this.content;
    return this;
  }

  public setContactsNum(contactsNum: number | undefined) {
    this.contactsNum = contactsNum != undefined ? contactsNum : this.contactsNum;
    return this;
  }

  public setUnreadCount(unreadCount: number | undefined) {
    this.unreadCount = unreadCount != undefined ? unreadCount : this.unreadCount;
    return this;
  }

  public setSmsType(smsType: number | undefined) {
    this.smsType = smsType != undefined ? smsType : this.smsType;
    return this;
  }

  public setSendingStatus(sendingStatus: number | undefined) {
    this.sendingStatus = sendingStatus != undefined ? sendingStatus : this.sendingStatus;
    return this;
  }

  public setHasLock(hasLock: number | undefined) {
    this.hasLock = hasLock != undefined ? hasLock : this.hasLock;
    return this;
  }

  public setHasDraft(hasDraft: number | undefined) {
    this.hasDraft = hasDraft != undefined ? hasDraft : this.hasDraft;
    return this;
  }

  public setMessageCount(messageCount: number | undefined) {
    this.messageCount = messageCount != undefined ? messageCount : this.messageCount;
    return this;
  }

  public setHasMms(hasMms: number | undefined) {
    this.hasMms = hasMms != undefined ? hasMms : this.hasMms;
    return this;
  }

  public setHasAttachment(hasAttachment: number | undefined) {
    this.hasAttachment = hasAttachment != undefined ? hasAttachment : this.hasAttachment;
    return this;
  }

  public setYellowPageId(yellowPageId: string | undefined) {
    this.yellowPageId = yellowPageId != undefined ? yellowPageId : this.yellowPageId;
    return this;
  }

  public setBlockedType(blockedType: number | undefined) {
    this.blockedType = blockedType != undefined ? blockedType : this.blockedType;
    return this;
  }
  
  public setPinningTime(pinningTime: number | undefined) {
    this.pinningTime = pinningTime != undefined ? pinningTime : this.pinningTime;
    return this;
  }

  public createValuesBucket(): ValuesBucket {
    const valuesBucket: ValuesBucket = {
      'time': this.time,
      'telephone': this.telephone,
      'content': this.content,
      'contacts_num': this.contactsNum,
      'unread_count': this.unreadCount,
      'sms_type': this.smsType,
      'sending_status': this.sendingStatus,
      'has_lock': this.hasLock,
      'has_draft': this.hasDraft,
      'message_count': this.messageCount,
      'has_mms': this.hasMms,
      'has_attachment': this.hasAttachment,
      'yellow_page_id': this.yellowPageId,
      'blocked_type': this.blockedType,
      'pinning_time': this.pinningTime
    };
    return valuesBucket;
  }
}