/**
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

export default class UpdateSessionItem {
  private sessionId: number = 0;
  private isRead: number = 1;
  private startTime: number = 0;
  private msgTitle: string = '';
  private msgState: number = 0;
  private smsType: number = 0;
  private hasDraft: number = 0;

  constructor(sessionId: number, isRead: number, startTime: number, msgTitle: string, msgState: number, smsType: number,
              hasDraft: number) {
    this.sessionId = sessionId;
    this.isRead = isRead;
    this.startTime = startTime;
    this.msgTitle = msgTitle;
    this.msgState = msgState;
    this.smsType = smsType;
    this.hasDraft = hasDraft;
  }

  public getSessionId(): number {
    return this.sessionId;
  }

  public getIsRead(): number {
    return this.isRead;
  }

  public getStartTime(): number {
    return this.startTime;
  }

  public getMsgTitle(): string {
    return this.msgTitle;
  }

  public getMsgState(): number {
    return this.msgState;
  }

  public getSmsType(): number {
    return this.smsType;
  }

  public getHasDraft(): number {
    return this.hasDraft;
  }
}