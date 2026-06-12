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

export default class SmsSubsectionItem {
  private slotId: number = 0;
  private smsSubsectionId: number = 0;
  private receiverNumber: string = '';
  private senderNumber: string = '';
  private isSender: number = 0;
  private startTime: string = '';
  private endTime: string = '';
  private rawPdu: string = '';
  private format: number = 0;
  private destPort: number = 0;
  private subsectionIndex: number = 0;
  private size: number = 0;

  public setSlotId(slotId: number | undefined) {
    this.slotId = slotId != undefined ? slotId : this.slotId;
    return this;
  }

  public setSmsSubsectionId(smsSubsectionId: number | undefined) {
    this.smsSubsectionId = smsSubsectionId != undefined ? smsSubsectionId : this.smsSubsectionId;
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

  public setStartTime(startTime: string | undefined) {
    this.startTime = startTime != undefined ? startTime : this.startTime;
    return this;
  }

  public setEndTime(endTime: string | undefined) {
    this.endTime = endTime != undefined ? endTime : this.endTime;
    return this;
  }

  public setRawPdu(rawPdu: string | undefined) {
    this.rawPdu = rawPdu != undefined ? rawPdu : this.rawPdu;
    return this;
  }

  public setFormat(format: number | undefined) {
    this.format = format != undefined ? format : this.format;
    return this;
  }

  public setDestPort(destPort: number | undefined) {
    this.destPort = destPort != undefined ? destPort : this.destPort;
    return this;
  }

  public setSubsectionIndex(subsectionIndex: number | undefined) {
    this.subsectionIndex = subsectionIndex != undefined ? subsectionIndex : this.subsectionIndex;
    return this;
  }

  public setSize(size: number | undefined) {
    this.size = size != undefined ? size : this.size;
    return this;
  }

  public createValuesBucket(): ValuesBucket {
    const valuesBucket: ValuesBucket = {
      'slot_id': this.slotId,
      'sms_subsection_id': this.smsSubsectionId,
      'receiver_number': this.receiverNumber,
      'sender_number': this.senderNumber,
      'is_sender': this.isSender,
      'start_time': this.startTime,
      'end_time': this.endTime,
      'raw_pdu': this.rawPdu,
      'format': this.format,
      'dest_port': this.destPort,
      'subsection_index': this.subsectionIndex,
      'size': this.size
    };
    return valuesBucket;
  }
}