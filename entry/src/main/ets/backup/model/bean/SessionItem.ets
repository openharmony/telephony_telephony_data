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
  private smsType: number = 0;
  private unreadCount: number = 0;
  private messageCount: number = 0;
  private hasAttachment: number = 0;
  private sendingStatus: number = 0;


  public setTime(time: number) {
    this.time = time != undefined ? time : this.time;
    return this;
  }

  public getTime(): number {
    return this.time;
  }

  public setTelephone(telephone: string) {
    this.telephone = telephone != undefined ? telephone : this.telephone;
    return this;
  }

  public setContent(content: string) {
    this.content = content != undefined ? content : this.content;
    return this;
  }

  public setContactsNum(contactsNum: number) {
    this.contactsNum = contactsNum != undefined ? contactsNum : this.contactsNum;
    return this;
  }

  public setSmsType(smsType: number) {
    this.smsType = smsType != undefined ? smsType : this.smsType;
    return this;
  }

  public setUnreadCount(unreadCount: number) {
    this.unreadCount = unreadCount != undefined ? unreadCount : this.unreadCount;
    return this;
  }

  public setMessageCount(messageCount: number) {
    this.messageCount = messageCount != undefined ? messageCount : this.messageCount;
    return this;
  }

  public setHasAttachment(hasAttachment: number) {
    this.hasAttachment = hasAttachment != undefined ? hasAttachment : this.hasAttachment;
    return this;
  }

  public setSendingStatus(sendingStatus: number) {
    this.sendingStatus = sendingStatus != undefined ? sendingStatus : this.sendingStatus;
    return this;
  }

  public createValuesBucket(): ValuesBucket {
    const valuesBucket: ValuesBucket = {
      'time': this.time,
      'telephone': this.telephone,
      'content': this.content,
      'contacts_num': this.contactsNum,
      'sms_type': this.smsType,
      'unread_count': this.unreadCount,
      'message_count': this.messageCount,
      'has_attachment': this.hasAttachment,
      'sending_status': this.sendingStatus
    };
    return valuesBucket;
  }
}