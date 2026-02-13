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

export default class MmsProtocolItem {
  private msgId: number = -1;
  private bcc: string = '';
  private cc: string = '';
  private contentLocation: string = '';
  private date: string = '';
  private deliveryReport: number = 0;
  private deliveryTime: string = '';
  private expiry: number = 0;
  private type: number = 0;
  private serialNumber: string = '';
  private category: string = '';
  private version: number = 0;
  private size: number = 0;
  private priority: number = 0;
  private readReply: number = 0;
  private reportAllowed: number = 0;
  private responseStatus: number = 0;
  private responseText: string = '';
  private senderVisibility: number = 0;

  public setMsgId(msgId: number | undefined) {
    this.msgId = msgId != undefined ? msgId : this.msgId;
    return this;
  }

  public setBcc(bcc: string | undefined) {
    this.bcc = bcc != undefined ? bcc : this.bcc;
    return this;
  }

  public setCc(cc: string | undefined) {
    this.cc = cc != undefined ? cc : this.cc;
    return this;
  }

  public setContentLocation(contentLocation: string | undefined) {
    this.contentLocation = contentLocation != undefined ? contentLocation : this.contentLocation;
    return this;
  }

  public setDate(date: string | undefined) {
    this.date = date != undefined ? date : this.date;
    return this;
  }

  public setDeliveryReport(deliveryReport: number | undefined) {
    this.deliveryReport = deliveryReport != undefined ? deliveryReport : this.deliveryReport;
    return this;
  }

  public setDeliveryTime(deliveryTime: string | undefined) {
    this.deliveryTime = deliveryTime != undefined ? deliveryTime : this.deliveryTime;
    return this;
  }

  public setExpiry(expiry: number | undefined) {
    this.expiry = expiry != undefined ? expiry : this.expiry;
    return this;
  }

  public setType(type: number | undefined) {
    this.type = type != undefined ? type : this.type;
    return this;
  }

  public setSerialNumber(serialNumber: string | undefined) {
    this.serialNumber = serialNumber != undefined ? serialNumber : this.serialNumber;
    return this;
  }

  public setCategory(category: string | undefined) {
    this.category = category != undefined ? category : this.category;
    return this;
  }

  public setVersion(version: number | undefined) {
    this.version = version != undefined ? version : this.version;
    return this;
  }

  public setSize(size: number | undefined) {
    this.size = size != undefined ? size : this.size;
    return this;
  }

  public setPriority(priority: number | undefined) {
    this.priority = priority != undefined ? priority : this.priority;
    return this;
  }

  public setReadReply(readReply: number | undefined) {
    this.readReply = readReply != undefined ? readReply : this.readReply;
    return this;
  }

  public setReportAllowed(reportAllowed: number | undefined) {
    this.reportAllowed = reportAllowed != undefined ? reportAllowed : this.reportAllowed;
    return this;
  }

  public setResponseStatus(responseStatus: number | undefined) {
    this.responseStatus = responseStatus != undefined ? responseStatus : this.responseStatus;
    return this;
  }

  public setResponseText(responseText: string | undefined) {
    this.responseText = responseText != undefined ? responseText : this.responseText;
    return this;
  }

  public setSenderVisibility(senderVisibility: number | undefined) {
    this.senderVisibility = senderVisibility != undefined ? senderVisibility : this.senderVisibility;
    return this;
  }


  public createValuesBucket(): ValuesBucket {
    const valuesBucket: ValuesBucket = {
      'msg_id': this.msgId,
      'bcc': this.bcc,
      'cc': this.cc,
      'content_location': this.contentLocation,
      'date': this.date,
      'delivery_report': this.deliveryReport,
      'delivery_time': this.deliveryTime,
      'expiry': this.expiry,
      'type': this.type,
      'serial_number': this.serialNumber,
      'category': this.category,
      'version': this.version,
      'size': this.size,
      'priority': this.priority,
      'read_reply': this.readReply,
      'report_allowed': this.reportAllowed,
      'response_status': this.responseStatus,
      'response_text': this.responseText,
      'sender_visibility': this.senderVisibility
    };
    return valuesBucket;
  }
}