/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
interface chat {
  id: string,
  threadId: string,
  address: string,
  person: string,
  date: string,
  dateSent: string,
  protocol: string,
  read: string,
  status: string,
  type: string,
  replyPathPresent: string,
  subject: string,
  body: string,
  serviceCenter: string,
  locked: string,
  subId: string,
  networkType: string,
  errorCode: string,
  seen: string,
  privacyMode: string,
  serviceKind: string,
  fileType: string,
  sdkSmsId: string,
  enrichedCallingType: string,
  ownerAddr: string,
  globalGroupId: string
}

interface rcsGroup {
  id: string,
  threadId: string,
  type: string,
  address: string,
  date: string,
  read: string,
  status: string,
  fileMode: string,
  globalDate: string,
  body: string,
  global_id: string,
  operateCode: string,
  errorCode: string,
  metaData: string,
  seen: string,
  serviceKind: string,
  fileType: string,
  sdkRcsGroupMessageId: string,
  mentionList: string,
}

interface Threads {
  threadId: string,
  address: string,
  recipientIds: string,
  pinningTime: string
}

interface Addresses {
  ID: string,
  address: string
}

interface Sms {
  subject: string,
  threadId: string,
  address: string,
  locked: string,
  protocol: string,
  status: string,
  subId: string,
  dateSent: string,
  networkType: string,
  body: string,
  date: string,
  read: string,
  seen: string,
  type: string,
  timeBody: string,
  groupId: string,
  addrBody: string,
  creator: string,
  errorCode: string,
  replyPathPresent: string,
  serviceCenter: string
}

interface Pdu {
  retrTxtCs: string,
  threadId: string,
  ctCls: string,
  locked: string,
  mSize: string,
  mType: string,
  subCs: string,
  subId: string,
  readStatus: string,
  respTxt: string,
  retrTxt: string,
  dateSent: string,
  networkType: string,
  v: string,
  rr: string,
  st: string,
  id: string,
  exp: string,
  pri: string,
  sub: string,
  ctL: string,
  ctT: string,
  dTm: string,
  date: string,
  mId: string,
  read: string,
  seen: string,
  dRpt: string,
  mCls: string,
  rptA: string,
  trId: string,
  creator: string,
  respSt: string,
  retrSt: string,
  msgBox: string
}

interface Part {
  cl: string,
  ct: string,
  fn: string,
  id: string,
  cid: string,
  mid: string,
  seq: string,
  name: string,
  text: string
  data: string,
  chset: string
}

interface Files {
  fileData: string
}

interface OldDBData {
  chat: chat,
  rcsGroup: rcsGroup,
  threads: Threads,
  addresses: Addresses,
  sms: Sms,
  pdu: Pdu,
  part: Part,
  files: Files
}

export default {
  chat: {
    id: '_id',
    threadId: 'thread_id',
    address: 'address',
    person: 'person',
    date: 'date',
    dateSent: 'date_sent',
    protocol: 'protocol',
    read: 'read',
    status: 'status',
    type: 'type',
    replyPathPresent: 'reply_path_present',
    subject: 'subject',
    body: 'body',
    serviceCenter: 'service_center',
    locked: 'locked',
    subId: 'sub_id',
    networkType: 'network_type',
    errorCode: 'error_code',
    seen: 'seen',
    privacyMode: 'privacy_mode',
    serviceKind: 'service_kind',
    fileType: 'file_type',
    sdkSmsId: 'sdk_sms_id',
    enrichedCallingType: 'enriched_calling_type',
    ownerAddr: 'owner_addr',
    globalGroupId: 'global_group_id'
  },

  rcsGroup: {
    id: '_id',
    threadId: 'thread_id',
    type: 'type',
    address: 'address',
    date: 'date',
    read: 'read',
    status: 'status',
    fileMode: 'file_mode',
    globalDate: 'global_date',
    body: 'body',
    global_id: 'global_id',
    operateCode: 'operate_code',
    errorCode: 'error_code',
    metaData: 'meta_data',
    seen: 'seen',
    serviceKind: 'service_kind',
    fileType: 'file_type',
    sdkRcsGroupMessageId: 'sdk_rcs_group_message_id',
    mentionList: 'mention_list',
  },

  threads: {
    threadId: 'thread_id',
    address: 'address',
    recipientIds: 'recipient_ids',
    pinningTime: 'pinning_time'
  },

  addresses: {
    ID: '_id',
    address: 'address'
  },

  sms: {
    threadId: 'thread_id',
    address: 'address',
    date: 'date',
    dateSent: 'date_sent',
    protocol: 'protocol',
    read: 'read',
    status: 'status',
    type: 'type',
    replyPathPresent: 'reply_path_present',
    subject: 'subject',
    body: 'body',
    serviceCenter: 'service_center',
    locked: 'locked',
    subId: 'sub_id',
    networkType: 'network_type',
    errorCode: 'error_code',
    creator: 'creator',
    seen: 'seen',
    groupId: 'group_id',
    addrBody: 'addr_body',
    timeBody: 'time_body'
  },

  pdu: {
    id: '_id',
    threadId: 'thread_id',
    date: 'date',
    dateSent: 'date_sent',
    msgBox: 'msg_box',
    read: 'read',
    mId: 'm_id',
    sub: 'sub',
    subCs: 'sub_cs',
    ctT: 'ct_t',
    ctL: 'ct_l',
    exp: 'exp',
    mCls: 'm_cls',
    mType: 'm_type',
    v: 'v',
    mSize: 'm_size',
    pri: 'pri',
    rr: 'rr',
    rptA: 'rpt_a',
    respSt: 'resp_st',
    st: 'st',
    trId: 'tr_id',
    retrSt: 'retr_st',
    retrTxt: 'retr_txt',
    retrTxtCs: 'retr_txt_cs',
    readStatus: 'read_status',
    ctCls: 'ct_cls',
    respTxt: 'resp_txt',
    dTm: 'd_tm',
    dRpt: 'd_rpt',
    locked: 'locked',
    seen: 'seen',
    subId: 'sub_id',
    networkType: 'network_type',
    creator: 'creator'
  },

  part: {
    id: '_id',
    mid: 'mid',
    seq: 'seq',
    ct: 'ct',
    name: 'name',
    chset: 'chset',
    fn: 'fn',
    cid: 'cid',
    cl: 'cl',
    data: '_data',
    text: 'text'
  },

  files: {
    fileData: 'file_data'
  }
} as OldDBData