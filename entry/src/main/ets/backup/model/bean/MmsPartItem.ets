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
  private id: number = 0;
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

  public setId(id: number) {
    this.id = id != undefined ? id : this.id;
    return this;
  }

  public setMsgId(msgId: number) {
    this.msgId = msgId != undefined ? msgId : this.msgId;
    return this;
  }

  public setGroupId(groupId: number) {
    this.groupId = groupId != undefined ? groupId : this.groupId;
    return this;
  }

  public setPartIndex(partIndex: number) {
    this.partIndex = partIndex != undefined ? partIndex : this.partIndex;
    return this;
  }

  public setPartSize(partSize: string) {
    this.partSize = partSize != undefined ? partSize : this.partSize;
    return this;
  }

  public setRecordingTime(recordingTime: string) {
    this.recordingTime = recordingTime != undefined ? recordingTime : this.recordingTime;
    return this;
  }

  public setType(type: number) {
    this.type = type != undefined ? type : this.type;
    return this;
  }

  public getType(): number {
    return this.type;
  }

  public setCt(ct: string) {
    this.ct = ct != undefined ? ct : this.ct;
    return this;
  }

  public setLocationPath(locationPath: string) {
    this.locationPath = locationPath != undefined ? locationPath : this.locationPath;
    return this;
  }

  public getLocationPath(): string {
    return this.locationPath;
  }

  public getRecordTime(): string {
    return this.recordingTime;
  }

  public setEncode(encode: number) {
    this.encode = encode != undefined ? encode : this.encode;
    return this;
  }

  public setState(state: number) {
    this.state = state != undefined ? state : this.state;
    return this;
  }

  public setContent(content: string = '') {
    this.content = content != undefined ? content : this.content;
    return this;
  }

  public createValuesBucket(): ValuesBucket {
    const valuesBucket: ValuesBucket = {
      // 'id': this.id,
      'msg_id': this.msgId,
      'group_id': this.groupId,
      // 'part_index': this.partIndex,
      // 'part_size': this.partIndex,
      'recording_time': this.recordingTime,
      'type': this.type,
      'ct': this.ct,
      'location_path': this.locationPath,
      // 'encode': this.encode,
      // 'state': this.state,
      'content': this.content,
    };
    return valuesBucket;
  }
}