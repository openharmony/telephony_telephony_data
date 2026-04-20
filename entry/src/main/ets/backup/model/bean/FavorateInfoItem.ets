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

export default class FavorateInfoItem {
  private msgId: number = 0;
  private receiverNumber: string = '';
  private senderNumber: string = '';
  private keepTime: string = '';
  private startTime: string = '';
  private msgType: number = 0;
  private smsType: number = 0;
  private msgTitle: string = '';
  private msgContent: string = '';
  private sessionType: number = 0;
  private groupId: number = 0;
  private partType: number = 0;
  private partLocationPath: string = '';
  private isSender: number = 0;
  private rcsId: number = 0;
  private detectResContent: string = '';
  private sessionId: number = 0;
  private recordingTime: string = '';
  private partSize: string = '';

  public setMsgId(msgId: number | undefined) {
    this.msgId = msgId != undefined ? msgId : this.msgId;
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

  public setKeepTime(keepTime: string | undefined) {
    this.keepTime = keepTime != undefined ? keepTime : this.keepTime;
    return this;
  }

  public setStartTime(startTime: string | undefined) {
    this.startTime = startTime != undefined ? startTime : this.startTime;
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

  public setMsgTitle(msgTitle: string | undefined) {
    this.msgTitle = msgTitle != undefined ? msgTitle : this.msgTitle;
    return this;
  }

  public setMsgContent(msgContent: string | undefined) {
    this.msgContent = msgContent != undefined ? msgContent : this.msgContent;
    return this;
  }

  public setSessionType(sessionType: number | undefined) {
    this.sessionType = sessionType != undefined ? sessionType : this.sessionType;
    return this;
  }

  public setGroupId(groupId: number | undefined) {
    this.groupId = groupId != undefined ? groupId : this.groupId;
    return this;
  }

  public setPartType(partType: number | undefined) {
    this.partType = partType != undefined ? partType : this.partType;
    return this;
  }

  public setPartLocationPath(partLocationPath: string | undefined) {
    this.partLocationPath = partLocationPath != undefined ? partLocationPath : this.partLocationPath;
    return this;
  }

  public setIsSender(isSender: number | undefined) {
    this.isSender = isSender != undefined ? isSender : this.isSender;
    return this;
  }

  public setRcsId(rcsId: number | undefined) {
    this.rcsId = rcsId != undefined ? rcsId : this.rcsId;
    return this;
  }

  public setDetectResContent(detectResContent: string | undefined) {
    this.detectResContent = detectResContent != undefined ? detectResContent : this.detectResContent;
    return this;
  }

  public setSessionId(sessionId: number | undefined) {
    this.sessionId = sessionId != undefined ? sessionId : this.sessionId;
    return this;
  }

  public setRecordingTime(recordingTime: string | undefined) {
    this.recordingTime = recordingTime != undefined ? recordingTime : this.recordingTime;
    return this;
  }

  public setPartSize(partSize: string | undefined) {
    this.partSize = partSize != undefined ? partSize : this.partSize;
    return this;
  }

  public createValuesBucket(): ValuesBucket {
    const valuesBucket: ValuesBucket = {
      'msg_id': this.msgId,
      'receiver_number': this.receiverNumber,
      'sender_number': this.senderNumber,
      'keep_time': this.keepTime,
      'start_time': this.startTime,
      'msg_type': this.msgType,
      'sms_type': this.smsType,
      'msg_title': this.msgTitle,
      'msg_content': this.msgContent,
      'session_type': this.sessionType,
      'group_id': this.groupId,
      'part_type': this.partType,
      'part_location_path': this.partLocationPath,
      'is_sender': this.isSender,
      'rcs_id': this.rcsId,
      'detect_res_content': this.detectResContent,
      'session_id': this.sessionId,
      'recording_time': this.recordingTime,
      'part_size': this.partSize
    };
    return valuesBucket;
  }
}