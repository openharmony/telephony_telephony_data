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

export default class MmsPartItem {
  private msgId: number = 0;
  private groupId: number = 0;
  private partIndex: number = 0;
  private partSize: string = '';
  private recordingTime: string = '';
  private type: number = 0;
  private ct: string = '';
  private locationPath: string = '';
  private encode: number = 0;
  private state: number = 0;
  private content: string = '';
  private rcsId: number = 0;
  private isFraud: number = 0;
  private referenceCount: number = -1;

  public setMsgId(msgId: number | undefined) {
    this.msgId = msgId != undefined ? msgId : this.msgId;
    return this;
  }

  public setGroupId(groupId: number | undefined) {
    this.groupId = groupId != undefined ? groupId : this.groupId;
    return this;
  }

  public setPartIndex(partIndex: number | undefined) {
    this.partIndex = partIndex != undefined ? partIndex : this.partIndex;
    return this;
  }

  public setPartSize(partSize: string | undefined) {
    this.partSize = partSize != undefined ? partSize : this.partSize;
    return this;
  }

  public setRecordingTime(recordingTime: string | undefined) {
    this.recordingTime = recordingTime != undefined ? recordingTime : this.recordingTime;
    return this;
  }

  public setType(type: number | undefined) {
    this.type = type != undefined ? type : this.type;
    return this;
  }

  public setCt(ct: string | undefined) {
    this.ct = ct != undefined ? ct : this.ct;
    return this;
  }

  public setLocationPath(locationPath: string | undefined) {
    this.locationPath = locationPath != undefined ? locationPath : this.locationPath;
    return this;
  }

  public setEncode(encode: number | undefined) {
    this.encode = encode ? encode : this.encode;
    return this;
  }

  public setState(state: number | undefined) {
    this.state = state != undefined ? state : this.state;
    return this;
  }

  public setContent(content: string = '') {
    this.content = content != undefined ? content : this.content;
    return this;
  }

  public setRcsId(rcsId: number | undefined) {
    this.rcsId = rcsId != undefined ? rcsId : this.rcsId;
    return this;
  }

  public setIsFraud(isFraud: number | undefined) {
    this.isFraud = isFraud != undefined ? isFraud : this.isFraud;
    return this;
  }

  public setReferenceCount(referenceCount: number | undefined) {
    this.referenceCount = referenceCount != undefined ? referenceCount : this.referenceCount;
    return this;
  }

  public createValuesBucket(): ValuesBucket {
    const valuesBucket: ValuesBucket = {
      'msg_id': this.msgId,
      'group_id': this.groupId,
      'part_index': this.partIndex,
      'part_size': this.partSize,
      'recording_time': this.recordingTime,
      'type': this.type,
      'ct': this.ct,
      'location_path': this.locationPath,
      'encode': this.encode,
      'state': this.state,
      'content': this.content,
      'rcs_id': this.rcsId,
      'is_fraud': this.isFraud,
      'reference_count': this.referenceCount
    };
    return valuesBucket;
  }
}