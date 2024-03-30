/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "data_storage_gtest.h"

#include "data_storage_errors.h"
#include "data_storage_log_wrapper.h"
#include "datashare_helper.h"
#include "datashare_predicates.h"
#include "global_params_data.h"
#include "opkey_data.h"
#include "parameter.h"
#include "pdp_profile_data.h"
#include "sim_data.h"
#include "sms_mms_data.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
const int NUM_MATCH_SHORT_EIGHT = 8;
const int NUM_MATCH_ELEVEN = 11;
const int PERMS_NUM = 4;
const int32_t VOICECALL_CAP_VAL_LEN = 6;
const std::string KEY_VOICECALL_CAP = "const.telephony.voice.capable";

bool HasVoiceCapability()
{
    char retValue[VOICECALL_CAP_VAL_LEN + 1] = { "true" };
    int retLen = GetParameter(KEY_VOICECALL_CAP.c_str(), "true", retValue, VOICECALL_CAP_VAL_LEN);
    DATA_STORAGE_LOGI("HasVoiceCapability retValue %{public}s, retLen %{public}d", retValue, retLen);
    if (strcmp(retValue, "false") == 0) {
        return false;
    }
    return true;
}

void DataStorageGtest::SetUpTestCase(void)
{
    // step 3: Set Up Test Case
    const char *perms[PERMS_NUM] = {
        "ohos.permission.READ_MESSAGES",
        "ohos.permission.ANSWER_CALL",
        "ohos.permission.SET_TELEPHONY_STATE",
        "ohos.permission.GET_TELEPHONY_STATE",
    };

    NativeTokenInfoParams testCallInfoParams = {
        .dcapsNum = 0,
        .permsNum = PERMS_NUM,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "telephony_data_gtest",
        .aplStr = "system_basic",
    };
    Security::AccessToken::AccessTokenID currentID_ = GetAccessTokenId(&testCallInfoParams);
    SetSelfTokenID(currentID_);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

void DataStorageGtest::TearDownTestCase(void)
{
    // step 3: Tear Down Test Case
}

void DataStorageGtest::SetUp(void)
{
    // step 3: input testcase setup step
}

void DataStorageGtest::TearDown(void)
{
    // step 3: input testcase teardown step
}

std::shared_ptr<DataShare::DataShareHelper> DataStorageGtest::CreateDataShareHelper(
    int32_t systemAbilityId, std::string &uri) const
{
    DATA_STORAGE_LOGI("DataStorageGtest::CreateDataShareHelper");
    auto saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        DATA_STORAGE_LOGE("DataStorageGtest Get system ability mgr failed.");
        return nullptr;
    }
    auto remoteObj = saManager->GetSystemAbility(systemAbilityId);
    if (remoteObj == nullptr) {
        DATA_STORAGE_LOGE("DataStorageGtest GetSystemAbility Service Failed.");
        return nullptr;
    }
    return DataShare::DataShareHelper::Creator(remoteObj, uri);
}

std::shared_ptr<DataShare::DataShareHelper> DataStorageGtest::CreateSimHelper()
{
    if (simDataHelper == nullptr) {
        std::string uri(SIM_URI);
        if (uri.data() == nullptr) {
            DATA_STORAGE_LOGE("CreateSimHelper uri is nullptr");
            return nullptr;
        }
        simDataHelper = CreateDataShareHelper(TELEPHONY_CORE_SERVICE_SYS_ABILITY_ID, uri);
    }
    return simDataHelper;
}

std::shared_ptr<DataShare::DataShareHelper> DataStorageGtest::CreateSmsHelper()
{
    if (smsDataHelper == nullptr) {
        std::string uri(SMS_MMS_URI);
        if (uri.data() == nullptr) {
            DATA_STORAGE_LOGE("CreateSmsHelper uri is nullptr");
            return nullptr;
        }
        smsDataHelper = CreateDataShareHelper(TELEPHONY_SMS_MMS_SYS_ABILITY_ID, uri);
    }
    return smsDataHelper;
}

std::shared_ptr<DataShare::DataShareHelper> DataStorageGtest::CreatePdpProfileHelper()
{
    if (pdpProfileDataHelper == nullptr) {
        std::string uri(PDP_PROFILE_URI);
        if (uri.data() == nullptr) {
            DATA_STORAGE_LOGE("CreatePdpProfileHelper uri is nullptr");
            return nullptr;
        }
        pdpProfileDataHelper = CreateDataShareHelper(TELEPHONY_SMS_MMS_SYS_ABILITY_ID, uri);
    }
    return pdpProfileDataHelper;
}

std::shared_ptr<DataShare::DataShareHelper> DataStorageGtest::CreateOpKeyHelper()
{
    if (opKeyDataHelper == nullptr) {
        std::string uri(OPKEY_URI);
        if (uri.data() == nullptr) {
            DATA_STORAGE_LOGE("CreateOpKeyHelper uri is nullptr");
            return nullptr;
        }
        opKeyDataHelper = CreateDataShareHelper(TELEPHONY_SMS_MMS_SYS_ABILITY_ID, uri);
    }
    return opKeyDataHelper;
}

std::shared_ptr<DataShare::DataShareHelper> DataStorageGtest::CreateGlobalParamsHelper()
{
    if (globalParamsDataHelper == nullptr) {
        std::string uri(GLOBAL_PARAMS_URI);
        if (uri.data() == nullptr) {
            DATA_STORAGE_LOGE("CreateGlobalParamsHelper uri is nullptr");
            return nullptr;
        }
        globalParamsDataHelper = CreateDataShareHelper(TELEPHONY_SMS_MMS_SYS_ABILITY_ID, uri);
    }
    return globalParamsDataHelper;
}

int DataStorageGtest::OpKeyInsert(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    std::string opkey = "110";
    Uri uri("datashare:///com.ohos.opkeyability/opkey/opkey_info");
    DataShare::DataShareValuesBucket value;
    value.Put(OpKeyData::ID, 1);
    value.Put(OpKeyData::MCCMNC, "460");
    value.Put(OpKeyData::GID1, "gid1");
    value.Put(OpKeyData::OPERATOR_NAME, "name");
    value.Put(OpKeyData::OPERATOR_KEY, opkey);
    auto ret = helper->Insert(uri, value);
    DATA_STORAGE_LOGI("OpKeyInsert ret: %{public}d", ret);
    return ret;
}

int DataStorageGtest::OpKeyUpdate(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    Uri uri("datashare:///com.ohos.opkeyability/opkey/opkey_info");
    DataShare::DataShareValuesBucket values;
    values.Put(OpKeyData::OPERATOR_NAME, "name2");
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(OpKeyData::OPERATOR_KEY, "123");
    return helper->Update(uri, predicates, values);
}

int DataStorageGtest::OpKeySelect(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    DATA_STORAGE_LOGI("OpKeySelect begin.");
    Uri uri("datashare:///com.ohos.opkeyability/opkey/opkey_info");
    std::vector<std::string> columns;
    DataShare::DataSharePredicates predicates;
    std::shared_ptr<DataShare::DataShareResultSet> resultSet = helper->Query(uri, predicates, columns);
    if (resultSet != nullptr) {
        int count;
        resultSet->GetRowCount(count);
        DATA_STORAGE_LOGI("OpKeySelectret cnt: %{public}d", count);
        return count;
    }
    return -1;
}

int DataStorageGtest::OpKeyDelete(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    Uri uri("datashare:///com.ohos.opkeyability/opkey/opkey_info");
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(OpKeyData::ID, "1");
    return helper->Delete(uri, predicates);
}

int DataStorageGtest::SimInsert(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    Uri uri("datashare:///com.ohos.simability/sim/sim_info");
    DataShare::DataShareValuesBucket value;
    value.Put(SimData::SLOT_INDEX, 1);
    value.Put(SimData::PHONE_NUMBER, "134xxxxxxxx");
    value.Put(SimData::ICC_ID, "icc_id");
    value.Put(SimData::CARD_ID, "card_id");
    return helper->Insert(uri, value);
}

int DataStorageGtest::SimUpdate(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    Uri uri("datashare:///com.ohos.simability/sim/sim_info");
    std::string slot = std::to_string(1);
    DataShare::DataShareValuesBucket values;
    values.Put(SimData::SHOW_NAME, "China Mobile");
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SimData::SLOT_INDEX, slot);
    return helper->Update(uri, predicates, values);
}

int DataStorageGtest::SimSelect(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    Uri uri("datashare:///com.ohos.simability/sim/sim_info");
    std::vector<std::string> columns;
    DataShare::DataSharePredicates predicates;
    std::shared_ptr<DataShare::DataShareResultSet> resultSet = helper->Query(uri, predicates, columns);
    if (resultSet != nullptr) {
        int count;
        resultSet->GetRowCount(count);
        return count;
    }
    return -1;
}

int DataStorageGtest::SimDelete(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    Uri uri("datashare:///com.ohos.simability/sim/sim_info");
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SimData::SLOT_INDEX, "1");
    return helper->Delete(uri, predicates);
}

int DataStorageGtest::SmsBatchInsert(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    Uri uri("datashare:///com.ohos.smsmmsability/sms_mms/sms_mms_info");
    std::vector<DataShare::DataShareValuesBucket> values;
    DataShare::DataShareValuesBucket value;
    int batchNum = 100;
    for (int i = 0; i < batchNum; i++) {
        value.Put(SmsMmsInfo::RECEIVER_NUMBER, "134xxxxxxxx");
        value.Put(SmsMmsInfo::MSG_CONTENT, "The first test text message content");
        value.Put(SmsMmsInfo::GROUP_ID, 1);
        values.push_back(value);
        value.Clear();
    }
    return helper->BatchInsert(uri, values);
}

int DataStorageGtest::SmsInsert(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    Uri uri("datashare:///com.ohos.smsmmsability/sms_mms/sms_mms_info");
    DataShare::DataShareValuesBucket value;
    value.Put(SmsMmsInfo::RECEIVER_NUMBER, "134xxxxxxxx");
    value.Put(SmsMmsInfo::MSG_CONTENT, "The first test text message content");
    value.Put(SmsMmsInfo::GROUP_ID, 1);
    value.Put(SmsMmsInfo::IS_SENDER, 0);
    return helper->Insert(uri, value);
}

int DataStorageGtest::SmsUpdate(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    Uri uri("datashare:///com.ohos.smsmmsability/sms_mms/sms_mms_info");
    DataShare::DataShareValuesBucket values;
    values.Put(SmsMmsInfo::MSG_CONTENT, "The second test text message content");
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SmsMmsInfo::MSG_ID, "1");
    return helper->Update(uri, predicates, values);
}

int DataStorageGtest::SmsSelect(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    Uri uri("datashare:///com.ohos.smsmmsability/sms_mms/sms_mms_info");
    std::vector<std::string> columns;
    DataShare::DataSharePredicates predicates;
    std::shared_ptr<DataShare::DataShareResultSet> resultSet = helper->Query(uri, predicates, columns);
    if (resultSet != nullptr) {
        int count;
        resultSet->GetRowCount(count);
        DATA_STORAGE_LOGI("SmsSelect count: %{public}d", count);
        return count;
    }
    return -1;
}

int DataStorageGtest::SmsDelete(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    Uri uri("datashare:///com.ohos.smsmmsability/sms_mms/sms_mms_info");
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SmsMmsInfo::MSG_ID, "1");
    return helper->Delete(uri, predicates);
}

int DataStorageGtest::PdpProfileInsert(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    Uri uri("datashare:///com.ohos.pdpprofileability/net/pdp_profile?simId=0");
    DataShare::DataShareValuesBucket value;
    value.Put(PdpProfileData::PROFILE_NAME, "frist_profile_name");
    value.Put(PdpProfileData::MCC, "460");
    value.Put(PdpProfileData::MNC, "91");
    value.Put(PdpProfileData::MVNO_TYPE, "gid1");
    value.Put(PdpProfileData::MVNO_MATCH_DATA, "FFF");
    value.Put(PdpProfileData::EDITED_STATUS, "1");
    value.Put(PdpProfileData::SERVER, "pdpserver");
    value.Put(PdpProfileData::BEARING_SYSTEM_TYPE, "1");
    value.Put(PdpProfileData::IS_ROAMING_APN, "0");
    value.Put(PdpProfileData::APN_PROTOCOL, "IP");
    value.Put(PdpProfileData::APN_ROAM_PROTOCOL, "IPV6");
    return helper->Insert(uri, value);
}

int DataStorageGtest::PdpProfileUpdate(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    Uri uri("datashare:///com.ohos.pdpprofileability/net/pdp_profile");
    DataShare::DataShareValuesBucket values;
    values.Put(PdpProfileData::PROFILE_NAME, "update_profile_name");
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(PdpProfileData::PROFILE_ID, "1");
    return helper->Update(uri, predicates, values);
}

int DataStorageGtest::PdpProfileUpdate2(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    Uri uri("datashare:///com.ohos.pdpprofileability/net/pdp_profile/preferapn");
    DataShare::DataShareValuesBucket values;
    DataShare::DataSharePredicates predicates;
    double testId = 1110;
    double testSImId = 1;
    values.Put(PdpProfileData::PROFILE_ID, testId);
    values.Put(PdpProfileData::SIM_ID, testSImId);
    return helper->Update(uri, predicates, values);
}

int DataStorageGtest::PdpProfileUpdate3(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    Uri uri("datashare:///com.ohos.pdpprofileability/net/pdp_profile/preferapn");
    DataShare::DataShareValuesBucket values;
    DataShare::DataSharePredicates predicates;
    values.Put(PdpProfileData::PROFILE_ID, -1);
    values.Put(PdpProfileData::SIM_ID, 0);
    return helper->Update(uri, predicates, values);
}

int DataStorageGtest::PdpProfileUpdate4(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    Uri uri("datashare:///com.ohos.pdpprofileability/net/pdp_profile/reset?simId=0");
    DataShare::DataShareValuesBucket values;
    DataShare::DataSharePredicates predicates;
    return helper->Update(uri, predicates, values);
}

int DataStorageGtest::PdpProfileSelect(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    Uri uri("datashare:///com.ohos.pdpprofileability/net/pdp_profile?simId=0");
    std::vector<std::string> columns;
    DataShare::DataSharePredicates predicates;
    std::shared_ptr<DataShare::DataShareResultSet> resultSet = helper->Query(uri, predicates, columns);
    if (resultSet != nullptr) {
        int count;
        resultSet->GetRowCount(count);
        std::cout << "count is " << count;
        return count;
    }
    return -1;
}

int DataStorageGtest::PdpProfileSelect2(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    Uri uri("datashare:///com.ohos.pdpprofileability/net/pdp_profile/preferapn?Proxy=true&simId=0");
    std::vector<std::string> columns;
    DataShare::DataSharePredicates predicates;
    std::shared_ptr<DataShare::DataShareResultSet> resultSet = helper->Query(uri, predicates, columns);
    if (resultSet != nullptr) {
        int count;
        resultSet->GetRowCount(count);
        std::cout << "count is " << count;
        return count;
    }
    return -1;
}

int DataStorageGtest::PdpProfileDelete(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    Uri uri("datashare:///com.ohos.pdpprofileability/net/pdp_profile");
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(PdpProfileData::PROFILE_ID, "1");
    return helper->Delete(uri, predicates);
}

int DataStorageGtest::GlobalParamsNumMatchInsert(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    Uri uri("datashare:///com.ohos.globalparamsability/globalparams/num_matchs");
    DataShare::DataShareValuesBucket value;
    value.Put(NumMatchData::NAME, "frist_numMatch_name");
    value.Put(NumMatchData::MCC, "460");
    value.Put(NumMatchData::MNC, "91");
    value.Put(NumMatchData::MCCMNC, "46091");
    value.Put(NumMatchData::NUM_MATCH, NUM_MATCH_ELEVEN);
    value.Put(NumMatchData::NUM_MATCH_SHORT, NUM_MATCH_SHORT_EIGHT);
    return helper->Insert(uri, value);
}

int DataStorageGtest::GlobalParamsNumMatchUpdate(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    Uri uri("datashare:///com.ohos.globalparamsability/globalparams/num_matchs");
    DataShare::DataShareValuesBucket values;
    values.Put(NumMatchData::NAME, "update_name");
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(NumMatchData::MCCMNC, "46091");
    return helper->Update(uri, predicates, values);
}

static void DumpNumMatchData(std::shared_ptr<DataShare::DataShareResultSet> resultSet)
{
    if (resultSet == nullptr) {
        std::cout << "resultSet is NULL, count = 0." << std::endl;
        return;
    }
    int count;
    resultSet->GetRowCount(count);
    std::cout << "Dump NumMatchTable: count is " << count << std::endl;
    for (int row = 0; row < count; row++) {
        int columnIndex;
        int id;
        int match_long;
        int match_short;
        std::string name;
        std::string mcc;
        std::string mnc;
        std::string numeric;
        resultSet->GoToRow(row);
        resultSet->GetColumnIndex(NumMatchData::ID, columnIndex);
        resultSet->GetInt(columnIndex, id);
        resultSet->GetColumnIndex(NumMatchData::NUM_MATCH, columnIndex);
        resultSet->GetInt(columnIndex, match_long);
        resultSet->GetColumnIndex(NumMatchData::NUM_MATCH_SHORT, columnIndex);
        resultSet->GetInt(columnIndex, match_short);
        resultSet->GetColumnIndex(NumMatchData::NAME, columnIndex);
        resultSet->GetString(columnIndex, name);
        resultSet->GetColumnIndex(NumMatchData::MCC, columnIndex);
        resultSet->GetString(columnIndex, mcc);
        resultSet->GetColumnIndex(NumMatchData::MNC, columnIndex);
        resultSet->GetString(columnIndex, mnc);
        resultSet->GetColumnIndex(NumMatchData::MCCMNC, columnIndex);
        resultSet->GetString(columnIndex, numeric);
        std::cout << " Row: " << row << ", id: " << id << ", name: " << name << ", mcc: " << mcc
                  << ", mnc: " << mnc << ", numeric: " << numeric << ", num_match: " << match_long
                  << ", num_match_short: " << match_short << std::endl;
    }
}

int DataStorageGtest::GlobalParamsNumMatchSelect(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    DATA_STORAGE_LOGI("GlobalParamsNumMatchSelect ---");
    Uri uri("datashare:///com.ohos.globalparamsability/globalparams/num_matchs");
    std::vector<std::string> columns;
    DataShare::DataSharePredicates predicates;
    std::shared_ptr<DataShare::DataShareResultSet> resultSet = helper->Query(uri, predicates, columns);
    if (resultSet != nullptr) {
        int count;
        resultSet->GetRowCount(count);
        std::cout << "count is " << count;
        DumpNumMatchData(resultSet);
        resultSet->Close();
        return count;
    }
    return -1;
}

int DataStorageGtest::GlobalParamsNumMatchDelete(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    Uri uri("datashare:///com.ohos.globalparamsability/globalparams/num_matchs");
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(NumMatchData::MCCMNC, "46091");
    return helper->Delete(uri, predicates);
}
int DataStorageGtest::GlobalEccInsert(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    Uri uri("datashare:///com.ohos.globalparamsability/globalparams/ecc_data");
    DataShare::DataShareValuesBucket value;
    value.Put(EccData::MCC, "460");
    value.Put(EccData::MNC, "01");
    value.Put(EccData::NUMERIC, "46001");
    return helper->Insert(uri, value);
}

int DataStorageGtest::GlobalEccUpdate(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    Uri uri("datashare:///com.ohos.globalparamsability/globalparams/ecc_data");
    DataShare::DataShareValuesBucket values;
    values.Put(EccData::NAME, "46001");
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(EccData::ID, "1");
    return helper->Update(uri, predicates, values);
}

int DataStorageGtest::GlobalEccSelect(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    Uri uri("datashare:///com.ohos.globalparamsability/globalparams/ecc_data");
    std::vector<std::string> columns;
    DataShare::DataSharePredicates predicates;
    std::shared_ptr<DataShare::DataShareResultSet> resultSet = helper->Query(uri, predicates, columns);
    if (resultSet != nullptr) {
        int count;
        resultSet->GetRowCount(count);
        std::cout << "count is " << count;
        return count;
    }
    return -1;
}

int DataStorageGtest::GlobalEccDelete(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    Uri uri("datashare:///com.ohos.globalparamsability/globalparams/ecc_data");
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(EccData::ID, "1");
    return helper->Delete(uri, predicates);
}

#ifndef TEL_TEST_UNSUPPORT
/**
 * @tc.number   DataStorage_001
 * @tc.name     create sim and sms DataAbilityHelper
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, DataStorage_001, Function | MediumTest | Level0)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> smsHelper = CreateSmsHelper();
    std::shared_ptr<DataShare::DataShareHelper> simHelper = CreateSimHelper();
    if (smsHelper == nullptr || simHelper == nullptr) {
        DATA_STORAGE_LOGE("CreateDataShareHelper occur error");
        return;
    }
    bool retSms = (smsHelper != nullptr);
    bool retSim = (simHelper != nullptr);
    smsHelper = nullptr;
    simHelper = nullptr;
    EXPECT_TRUE(retSms);
    EXPECT_TRUE(retSim);
}

/**
 * @tc.number   OpKeyInsert_001
 * @tc.name     insert opkey data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, OpKeyInsert_001, Function | MediumTest | Level1)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreateOpKeyHelper();
    if (helper == nullptr) {
        DATA_STORAGE_LOGE("CreateDataShareHelper occur error");
        return;
    }
    int ret = OpKeyInsert(helper);
    helper = nullptr;
    EXPECT_GE(ret, DATA_STORAGE_ERROR);
}

/**
 * @tc.number   OpKeyUpdate_001
 * @tc.name     update opkey data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, OpKeyUpdate_001, Function | MediumTest | Level2)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreateOpKeyHelper();
    if (helper == nullptr) {
        DATA_STORAGE_LOGE("CreateDataShareHelper occur error");
        return;
    }
    int ret = OpKeyUpdate(helper);
    helper = nullptr;
    EXPECT_GE(ret, DATA_STORAGE_ERROR);
}

/**
 * @tc.number   OpKeySelect_001
 * @tc.name     select opkey data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, OpKeySelect_001, Function | MediumTest | Level1)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreateOpKeyHelper();
    if (helper == nullptr) {
        DATA_STORAGE_LOGE("CreateDataShareHelper occur error");
        return;
    }
    int ret = OpKeySelect(helper);
    helper = nullptr;
    EXPECT_GE(ret, DATA_STORAGE_ERROR);
}

/**
 * @tc.number   OpKeyDelete_001
 * @tc.name     delete opkey data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, OpKeyDelete_001, Function | MediumTest | Level1)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreateOpKeyHelper();
    if (helper == nullptr) {
        DATA_STORAGE_LOGE("CreateDataShareHelper occur error");
        return;
    }
    int ret = OpKeyDelete(helper);
    helper = nullptr;
    EXPECT_GE(ret, DATA_STORAGE_ERROR);
}

/**
 * @tc.number   SimInsert_001
 * @tc.name     insert sim data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, SimInsert_001, Function | MediumTest | Level1)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreateSimHelper();
    if (helper == nullptr) {
        DATA_STORAGE_LOGE("CreateDataShareHelper occur error");
        return;
    }
    int ret = SimInsert(helper);
    helper = nullptr;
    EXPECT_GE(ret, DATA_STORAGE_ERROR);
}

/**
 * @tc.number   SimUpdate_001
 * @tc.name     update sim data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, SimUpdate_001, Function | MediumTest | Level1)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreateSimHelper();
    if (helper == nullptr) {
        DATA_STORAGE_LOGE("CreateDataShareHelper occur error");
        return;
    }
    int ret = SimUpdate(helper);
    helper = nullptr;
    EXPECT_GE(ret, DATA_STORAGE_ERROR);
}

/**
 * @tc.number   SimSelect_001
 * @tc.name     select sim data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, SimSelect_001, Function | MediumTest | Level1)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreateSimHelper();
    if (helper == nullptr) {
        DATA_STORAGE_LOGE("CreateDataShareHelper occur error");
        return;
    }
    int ret = SimSelect(helper);
    helper = nullptr;
    EXPECT_GE(ret, DATA_STORAGE_ERROR);
}

/**
 * @tc.number   SimDelete_001
 * @tc.name     delete sim data
 * @tc.desc     Function test
 * @tc.require: I5LACE
 */
HWTEST_F(DataStorageGtest, SimDelete_001, Function | MediumTest | Level1)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreateSimHelper();
    if (helper == nullptr) {
        DATA_STORAGE_LOGE("CreateDataShareHelper occur error");
        return;
    }
    int ret = SimDelete(helper);
    helper = nullptr;
    EXPECT_GE(ret, DATA_STORAGE_ERROR);
}

/**
 * @tc.number   SmsBatchInsert_001
 * @tc.name     batch insert sms data
 * @tc.desc     Function test
 * @tc.require: I5LACE
 */
HWTEST_F(DataStorageGtest, SmsBatchInsert_001, Function | MediumTest | Level1)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreateSmsHelper();
    if (helper == nullptr) {
        DATA_STORAGE_LOGE("CreateDataShareHelper occur error");
        return;
    }
    int ret = SmsBatchInsert(helper);
    helper = nullptr;
    EXPECT_GE(ret, DATA_STORAGE_ERROR);
}

/**
 * @tc.number   SmsInsert_001
 * @tc.name     insert sms data
 * @tc.desc     Function test
 * @tc.require: I5LACE
 */
HWTEST_F(DataStorageGtest, SmsInsert_001, Function | MediumTest | Level1)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreateSmsHelper();
    if (helper == nullptr) {
        DATA_STORAGE_LOGE("CreateDataShareHelper occur error");
        return;
    }
    int ret = SmsInsert(helper);
    helper = nullptr;
    EXPECT_GE(ret, DATA_STORAGE_ERROR);
}

/**
 * @tc.number   SmsUpdate_001
 * @tc.name     update sms data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, SmsUpdate_001, Function | MediumTest | Level1)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreateSmsHelper();
    if (helper == nullptr) {
        DATA_STORAGE_LOGE("CreateDataShareHelper occur error");
        return;
    }
    int ret = SmsUpdate(helper);
    helper = nullptr;
    EXPECT_GE(ret, DATA_STORAGE_ERROR);
}

/**
 * @tc.number   SmsSelect_001
 * @tc.name     select sms data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, SmsSelect_001, Function | MediumTest | Level1)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreateSmsHelper();
    if (helper == nullptr) {
        DATA_STORAGE_LOGE("CreateDataShareHelper occur error");
        return;
    }
    int ret = SmsSelect(helper);
    helper = nullptr;
    EXPECT_GE(ret, DATA_STORAGE_ERROR);
}

/**
 * @tc.number   SmsDelete_001
 * @tc.name     delete sms data
 * @tc.desc     Function test
 * @tc.require: I5LACE
 */
HWTEST_F(DataStorageGtest, SmsDelete_001, Function | MediumTest | Level1)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreateSmsHelper();
    if (helper == nullptr) {
        DATA_STORAGE_LOGE("CreateDataShareHelper occur error");
        return;
    }
    int ret = SmsDelete(helper);
    helper = nullptr;
    EXPECT_GE(ret, DATA_STORAGE_ERROR);
}

/**
 * @tc.number   PdpProfileInsert_001
 * @tc.name     insert apn data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, PdpProfileInsert_001, Function | MediumTest | Level1)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreatePdpProfileHelper();
    if (helper == nullptr) {
        DATA_STORAGE_LOGE("CreateDataShareHelper occur error");
        return;
    }
    int ret = PdpProfileInsert(helper);
    helper = nullptr;
    EXPECT_GE(ret, DATA_STORAGE_ERROR);
}

/**
 * @tc.number   PdpProfileUpdate_001
 * @tc.name     update apn data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, PdpProfileUpdate_001, Function | MediumTest | Level2)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreatePdpProfileHelper();
    if (helper == nullptr) {
        DATA_STORAGE_LOGE("CreateDataShareHelper occur error");
        return;
    }
    int ret = PdpProfileUpdate(helper);
    helper = nullptr;
    EXPECT_GE(ret, DATA_STORAGE_ERROR);
}

/**
 * @tc.number   PdpProfileUpdate_002
 * @tc.name     update apn data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, PdpProfileUpdate_002, Function | MediumTest | Level2)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreatePdpProfileHelper();
    if (helper == nullptr) {
        DATA_STORAGE_LOGE("CreateDataShareHelper occur error");
        return;
    }
    int ret = PdpProfileUpdate2(helper);
    helper = nullptr;
    EXPECT_GE(ret, DATA_STORAGE_ERROR);
}

/**
 * @tc.number   PdpProfileUpdate_003
 * @tc.name     update apn data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, PdpProfileUpdate_003, Function | MediumTest | Level2)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreatePdpProfileHelper();
    if (helper == nullptr) {
        DATA_STORAGE_LOGE("CreateDataShareHelper occur error");
        return;
    }
    int ret = PdpProfileUpdate3(helper);
    helper = nullptr;
    EXPECT_GE(ret, DATA_STORAGE_ERROR);
}

/**
 * @tc.number   PdpProfileUpdate_004
 * @tc.name     update apn data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, PdpProfileUpdate_004, Function | MediumTest | Level2)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreatePdpProfileHelper();
    if (helper == nullptr) {
        DATA_STORAGE_LOGE("CreateDataShareHelper occur error");
        return;
    }
    int ret = PdpProfileUpdate4(helper);
    helper = nullptr;
    EXPECT_GE(ret, DATA_STORAGE_ERROR);
}

/**
 * @tc.number   PdpProfileSelect_001
 * @tc.name     select apn data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, PdpProfileSelect_001, Function | MediumTest | Level1)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreatePdpProfileHelper();
    if (helper == nullptr) {
        DATA_STORAGE_LOGE("CreateDataShareHelper occur error");
        return;
    }
    int ret = PdpProfileSelect(helper);
    helper = nullptr;
    EXPECT_GE(ret, DATA_STORAGE_ERROR);
}

/**
 * @tc.number   PdpProfileSelect_002
 * @tc.name     select apn data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, PdpProfileSelect_002, Function | MediumTest | Level1)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreatePdpProfileHelper();
    if (helper == nullptr) {
        DATA_STORAGE_LOGE("CreateDataShareHelper occur error");
        return;
    }
    int ret = PdpProfileSelect2(helper);
    helper = nullptr;
    EXPECT_GE(ret, DATA_STORAGE_ERROR);
}

/**
 * @tc.number   PdpProfileDelete_001
 * @tc.name     delete apn data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, PdpProfileDelete_001, Function | MediumTest | Level1)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreatePdpProfileHelper();
    if (helper == nullptr) {
        DATA_STORAGE_LOGE("CreateDataShareHelper occur error");
        return;
    }
    int ret = PdpProfileDelete(helper);
    helper = nullptr;
    EXPECT_GE(ret, DATA_STORAGE_ERROR);
}

/**
 * @tc.number   GlobalParamsNumMatchInsert_001
 * @tc.name     insert NumMatch data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, GlobalParamsNumMatchInsert_001, Function | MediumTest | Level1)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreateGlobalParamsHelper();
    if (helper == nullptr) {
        DATA_STORAGE_LOGE("CreateDataShareHelper occur error");
        return;
    }
    int ret = GlobalParamsNumMatchInsert(helper);
    helper = nullptr;
    EXPECT_GE(ret, DATA_STORAGE_ERROR);
}

/**
 * @tc.number   GlobalParamsNumMatchUpdate_001
 * @tc.name     update NumMatch data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, GlobalParamsNumMatchUpdate_001, Function | MediumTest | Level1)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreateGlobalParamsHelper();
    if (helper == nullptr) {
        DATA_STORAGE_LOGE("CreateDataShareHelper occur error");
        return;
    }
    int ret = GlobalParamsNumMatchUpdate(helper);
    helper = nullptr;
    EXPECT_GE(ret, DATA_STORAGE_ERROR);
}

/**
 * @tc.number   GlobalParamsNumMatchSelect_001
 * @tc.name     select NumMatch data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, GlobalParamsNumMatchSelect_001, Function | MediumTest | Level1)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreateGlobalParamsHelper();
    if (helper == nullptr) {
        DATA_STORAGE_LOGE("CreateDataShareHelper occur error");
        return;
    }
    int ret = GlobalParamsNumMatchSelect(helper);
    helper = nullptr;
    EXPECT_GE(ret, DATA_STORAGE_ERROR);
}

/**
 * @tc.number   GlobalParamsNumMatchDelete_001
 * @tc.name     delete NumMatch data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, GlobalParamsNumMatchDelete_001, Function | MediumTest | Level1)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreateGlobalParamsHelper();
    if (helper == nullptr) {
        DATA_STORAGE_LOGE("CreateDataShareHelper occur error");
        return;
    }
    int ret = GlobalParamsNumMatchDelete(helper);
    helper = nullptr;
    EXPECT_GE(ret, DATA_STORAGE_ERROR);
}
/**
 * @tc.number   GlobalEccInsert_001
 * @tc.name     insert ecc data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, GlobalEccInsert_001, Function | MediumTest | Level1)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreateGlobalParamsHelper();
    if (helper == nullptr) {
        DATA_STORAGE_LOGE("CreateDataShareHelper occur error");
        return;
    }
    int ret = GlobalEccInsert(helper);
    helper = nullptr;
    EXPECT_GE(ret, DATA_STORAGE_ERROR);
}

/**
 * @tc.number   GlobalEccUpdate_002
 * @tc.name     update ecc data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, GlobalEccUpdate_002, Function | MediumTest | Level2)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreateGlobalParamsHelper();
    if (helper == nullptr) {
        DATA_STORAGE_LOGE("CreateDataShareHelper occur error");
        return;
    }
    int ret = GlobalEccUpdate(helper);
    helper = nullptr;
    EXPECT_GE(ret, DATA_STORAGE_ERROR);
}

/**
 * @tc.number   GlobalEccSelect_003
 * @tc.name     select ecc data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, GlobalEccSelect_003, Function | MediumTest | Level1)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreateGlobalParamsHelper();
    if (helper == nullptr) {
        DATA_STORAGE_LOGE("CreateDataShareHelper occur error");
        return;
    }
    int ret = GlobalEccSelect(helper);
    helper = nullptr;
    EXPECT_GE(ret, DATA_STORAGE_ERROR);
}

/**
 * @tc.number   GlobalEccDelete_004
 * @tc.name     delete ecc data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, GlobalEccDelete_004, Function | MediumTest | Level1)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreateGlobalParamsHelper();
    if (helper == nullptr) {
        DATA_STORAGE_LOGE("CreateDataShareHelper occur error");
        return;
    }
    int ret = GlobalEccDelete(helper);
    helper = nullptr;
    EXPECT_GE(ret, DATA_STORAGE_ERROR);
}
#else // TEL_TEST_UNSUPPORT
/**
 * @tc.number   TelMockTest_001
 * @tc.name     Test for unsupport platform
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, TelMockTest_001, Function | MediumTest | Level1)
{
    EXPECT_TRUE(true);
}
#endif // TEL_TEST_UNSUPPORT
} // namespace Telephony
} // namespace OHOS
