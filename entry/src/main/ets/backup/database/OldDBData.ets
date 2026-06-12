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

interface threads {
  id: string
  date: string;
  messageCount: string;
  recipientIds: string;
  snippet: string;
  snippetCs: string;
  read: string;
  archived: string;
  type: string;
  error: string;
  hasAttachment: string;
  subId: string;
  networkType: string;
  unreadCount: string;
  datePrivacy: string;
  messageCountPrivacy: string;
  snippetPrivacy: string;
  snippetCsPrivacy: string;
  readPrivacy: string;
  errorPrivacy: string;
  hasAttachmentPrivacy: string;
  subIdPrivacy: string;
  networkTypePrivacy: string;
  unreadCountPrivacy: string;
  visibleCountPrivacy: string;
  priorityPrivacy: string;
  isSecretPrivacy: string;
  messageCountSecretPrivacy: string;
  snippetSecretPrivacy: string;
  hasMetionedPeople: string;
  privacyMode: string;
  priority: string;
  numberType: string;
  isSecret: string;
  messageCountSecret: string;
  snippetSecret: string;
  visibleCount: string;
}

interface sms {
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
  creator: string,
  seen: string,
  siId: string,
  mid: string,
  created: string,
  mtype: string,
  isSatellite: string,
  privacyMode: string,
  groupId: string,
  addrBody: string,
  timeBody: string,
  riskUrlBody: string,
  isSecret: string,
  resentIm: string
}

interface canonicalAddresses {
  id: string,
  address: string,
  recent: string,
  CID: string,
  NAME: string,
  isSecret: string,
  countryIso: string,
  formattedAddress: string,
  peerName: string
}

interface pdu {
  id: string,
  threadId: string,
  date: string,
  dateSent: string,
  msgBox: string,
  read: string,
  mId: string,
  sub: string,
  subCs: string,
  ctT: string,
  ctL: string,
  exp: string,
  mCls: string,
  mType: string,
  v: string,
  mSize: string,
  pri: string,
  rr: string,
  rptA: string,
  respSt: string,
  st: string,
  trId: string,
  retrSt: string,
  retrTxt: string,
  retrTxtCs: string,
  readStatus: string,
  ctCls: string,
  respTxt: string,
  dTm: string,
  dRpt: string,
  locked: string,
  seen: string,
  subId: string,
  networkType: string,
  creator: string,
  textOnly: string,
  privacyMode: string,
  isSecret: string
}

interface part {
  id: string,
  mid: string,
  seq: string,
  ct: string,
  name: string,
  chset: string,
  cd: string,
  fn: string,
  cid: string,
  cl: string,
  cttS: string,
  cttT: string,
  data: string,
  text: string
}

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

interface rcsThreads {
  id: string,
  date: string,
  messageCount: string,
  recipientIds: string,
  snippet: string,
  snippetCs: string,
  read: string,
  type: string,
  error: string,
  hasAttachment: string,
  subId: string,
  networkType: string,
  unreadCount: string,
  privacyMode: string,
  visibleCount: string,
  priority: string,
  numberType: string,
  undeliveredStatus: string,
  fileType: string,
  hasMetionedPeople: string
}

interface favorateInfo {
  msgId: string,
  receiverNumber: string,
  senderNumber: string,
  keepTime: string,
  startTime: string,
  msgType: string,
  smsType: string,
  msgTitle: string,
  msgContent: string,
  sessionType: string,
  groupId: string,
  partType: string,
  partLocationPath: string,
  isSender: string,
  rcsId: string,
  detectResContent: string,
  sessionId: string,
  recordingTime: string,
  partSize: string
}

interface favSms {
  id: string,
  threadId: string,
  originId: string,
  dateFavadd: string,
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
  addrBody: string,
  timeBody: string,
  riskUrlBody: string,
  isSecret: string
}

interface favPdu {
  id: string,
  threadId: string,
  originId: string,
  dateFavadd: string,
  date: string,
  dateSent: string,
  msgBox: string,
  read: string,
  mId: string,
  sub: string,
  subCs: string,
  ctT: string,
  ctL: string,
  exp: string,
  mCls: string,
  mType: string,
  v: string,
  mSize: string,
  pri: string,
  rr: string,
  rptA: string,
  respSt: string,
  st: string,
  trId: string,
  retrSt: string,
  retrTxt: string,
  retrTxtCs: string,
  readStatus: string,
  ctCls: string,
  respTxt: string,
  dTm: string,
  dRpt: string,
  locked: string,
  seen: string,
  subId: string,
  networkType: string,
  textOnly: string,
  privacyMode: string
}

interface favPart {
  id: string,
  mid: string,
  seq: string,
  ct: string,
  name: string,
  chset: string,
  cd: string,
  fn: string,
  cid: string,
  cl: string,
  cttS: string,
  cttT: string,
  data: string,
  text: string
}

interface favAddr {
  id: string,
  msgId: string,
  contactId: string,
  address: string,
  type: string,
  charset: string
}

interface OldDBData {
  threads: threads
  sms: sms
  canonicalAddresses: canonicalAddresses
  pdu: pdu
  part: part
  chat: chat
  rcsThreads: rcsThreads
  rcsGroup: rcsGroup
  favorateInfo: favorateInfo
  favSms: favSms
  favPdu: favPdu
  favPart: favPart
  favAddr: favAddr
}

export default {
  threads: {
    id: '_id',
    date: 'date',
    messageCount: 'message_count',
    recipientIds: 'recipient_ids',
    snippet: 'snippet',
    snippetCs: 'snippet_cs',
    read: 'read',
    archived: 'archived',
    type: 'type',
    error: 'error',
    hasAttachment: 'has_attachment',
    subId: 'sub_id',
    networkType: 'network_type',
    unreadCount: 'unread_count',
    datePrivacy: 'date_privacy',
    messageCountPrivacy: 'message_count_privacy',
    snippetPrivacy: 'snippet_privacy',
    snippetCsPrivacy: 'snippet_cs_privacy',
    readPrivacy: 'read_privacy',
    errorPrivacy: 'error_privacy',
    hasAttachmentPrivacy: 'has_attachment_privacy',
    subIdPrivacy: 'sub_id_privacy',
    networkTypePrivacy: 'network_type_privacy',
    unreadCountPrivacy: 'unread_count_privacy',
    visibleCountPrivacy: 'visible_count_privacy',
    priorityPrivacy: 'priority_privacy',
    isSecretPrivacy: 'is_secret_privacy',
    messageCountSecretPrivacy: 'message_count_secret_privacy',
    snippetSecretPrivacy: 'snippet_secret_privacy',
    hasMetionedPeople: 'has_metioned_people',
    privacyMode: 'privacy_mode',
    priority: 'priority',
    numberType: 'number_type',
    isSecret: 'is_secret',
    messageCountSecret: 'message_count_secret',
    snippetSecret: 'snippet_secret',
    visibleCount: 'visible_count'
  },

  sms: {
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
    creator: 'creator',
    seen: 'seen',
    siId: 'si_id',
    mid: 'mid',
    created: 'created',
    mtype: 'mtype',
    isSatellite: 'is_satellite',
    privacyMode: 'privacy_mode',
    groupId: 'group_id',
    addrBody: 'addr_body',
    timeBody: 'time_body',
    riskUrlBody: 'risk_url_body',
    isSecret: 'is_secret',
    resentIm: 'resent_im'
  },

  canonicalAddresses: {
    id: '_id',
    address: 'address',
    recent: 'recent',
    CID: 'CID',
    NAME: 'NAME',
    isSecret: 'is_secret',
    countryIso: 'country_iso',
    formattedAddress: 'formatted_address',
    peerName: 'peer_name'
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
    creator: 'creator',
    textOnly: 'text_only',
    privacyMode: 'privacy_mode',
    isSecret: 'is_secret'
  },

  part: {
    id: '_id',
    mid: 'mid',
    seq: 'seq',
    ct: 'ct',
    name: 'name',
    chset: 'chset',
    cd: 'cd',
    fn: 'fn',
    cid: 'cid',
    cl: 'cl',
    cttS: 'ctt_s',
    cttT: 'ctt_t',
    data: '_data',
    text: 'text'
  },

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


  rcsThreads: {
    id: '_id',
    date: 'date',
    messageCount: 'message_count',
    recipientIds: 'recipient_ids',
    snippet: 'snippet',
    snippetCs: 'snippet_cs',
    read: 'read',
    type: 'type',
    error: 'error',
    hasAttachment: 'has_attachment',
    subId: 'sub_id',
    networkType: 'network_type',
    unreadCount: 'unread_count',
    privacyMode: 'privacy_mode',
    visibleCount: 'visible_count',
    priority: 'priority',
    numberType: 'number_type',
    undeliveredStatus: 'undelivered_status',
    fileType: 'file_type',
    hasMetionedPeople: 'has_metioned_people'
  },

  favorateInfo: {
    msgId: 'msg_id',
    receiverNumber: 'receiver_number',
    senderNumber: 'sender_number',
    keepTime: 'keep_time',
    startTime: 'start_time',
    msgType: 'msg_type',
    smsType: 'sms_type',
    msgTitle: 'msg_title',
    msgContent: 'msg_content',
    sessionType: 'session_type',
    groupId: 'group_id',
    partType: 'part_type',
    partLocationPath: 'part_location_path',
    isSender: 'is_sender',
    rcsId: 'rcs_id',
    detectResContent: 'detect_res_content',
    sessionId: 'session_id',
    recordingTime: 'recording_time',
    partSize: 'part_size'
  },

  favSms: {
    id: '_id',
    threadId: 'thread_id',
    originId: 'origin_id',
    dateFavadd: 'date_favadd',
    address: 'address',
    person:	'person',
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
    addrBody: 'addr_body',
    timeBody: 'time_body',
    riskUrlBody: 'risk_url_body',
    isSecret: 'is_secret',
  },

  favPdu: {
    id: '_id',
    threadId: 'thread_id',
    originId: 'origin_id',
    dateFavadd: 'date_favadd',
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
    textOnly: 'text_only',
    privacyMode: 'privacy_mode',
  },

  favPart: {
    id: '_id',
    mid: 'mid',
    seq: 'seq',
    ct: 'ct',
    name: 'name',
    chset: 'chset',
    cd: 'cd',
    fn: 'fn',
    cid: 'cid',
    cl: 'cl',
    cttS: 'ctt_s',
    cttT: 'ctt_t',
    data: '_data',
    text: 'text',
  },

  favAddr: {
    id: '_id',
    msgId: 'msg_id',
    contactId: 'contact_id',
    address: 'address',
    type: 'type',
    charset: 'charset',
  }
} as OldDBData