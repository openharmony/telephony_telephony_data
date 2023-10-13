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
#include "global_ecc_data.h"
#include "opkey_data.h"
#include "parameter.h"
#include "pdp_profile_data.h"
#include "sim_data.h"
#include "sms_mms_data.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
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

std::shared_ptr<DataShare::DataShareHelper> DataStorageGtest::CreateGlobalEccHelper()
{
    if (globalEccDataHelper_ == nullptr) {
        std::string uri(PDP_PROFILE_URI);
        if (uri.data() == nullptr) {
            DATA_STORAGE_LOGE("CreatePdpProfileHelper uri is nullptr");
            return nullptr;
        }
        globalEccDataHelper_ = CreateDataShareHelper(TELEPHONY_SMS_MMS_SYS_ABILITY_ID, uri);
    }
    return globalEccDataHelper_;
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
        DATA_STORAGE_LOGE("OpKeySelectret cnt: %{public}d", count);
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
    Uri uri("datashare:///com.ohos.pdpprofileability/net/pdp_profile");
    DataShare::DataShareValuesBucket value;
    value.Put(PdpProfileData::PROFILE_NAME, "frist_profile_name");
    value.Put(PdpProfileData::MCC, "460");
    value.Put(PdpProfileData::MNC, "91");
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

int DataStorageGtest::PdpProfileSelect(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    Uri uri("datashare:///com.ohos.pdpprofileability/net/pdp_profile");
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

int DataStorageGtest::GlobalEccInsert(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    Uri uri("datashare:///com.ohos.globaleccability/globalparams/ecc_list");
    DataShare::DataShareValuesBucket value;
    value.Put(GlobalEccData::MCC, "460");
    value.Put(GlobalEccData::MNC, "01");
    value.Put(GlobalEccData::NUMERIC, "46001");
    return helper->Insert(uri, value);
}

int DataStorageGtest::GlobalEccUpdate(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    Uri uri("datashare:///com.ohos.globaleccability/globalparams/ecc_list");
    DataShare::DataShareValuesBucket values;
    values.Put(GlobalEccData::NAME, "46001");
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(GlobalEccData::ID, "1");
    return helper->Update(uri, predicates, values);
}

int DataStorageGtest::GlobalEccSelect(const std::shared_ptr<DataShare::DataShareHelper> &helper) const
{
    Uri uri("datashare:///com.ohos.globaleccability/globalparams/ecc_list");
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
    Uri uri("datashare:///com.ohos.globaleccability/globalparams/ecc_list");
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(GlobalEccData::ID, "1");
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
    ASSERT_TRUE(smsHelper != nullptr);
    std::shared_ptr<DataShare::DataShareHelper> simHelper = CreateSimHelper();
    ASSERT_TRUE(simHelper != nullptr);
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
    ASSERT_TRUE(helper != nullptr);
    int ret = OpKeyInsert(helper);
    EXPECT_NE(DATA_STORAGE_ERROR, ret);
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
    ASSERT_TRUE(helper != nullptr);
    int ret = OpKeyUpdate(helper);
    EXPECT_NE(DATA_STORAGE_ERROR, ret);
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
    ASSERT_TRUE(helper != nullptr);
    int ret = OpKeySelect(helper);
    EXPECT_NE(DATA_STORAGE_ERROR, ret);
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
    ASSERT_TRUE(helper != nullptr);
    int ret = OpKeyDelete(helper);
    EXPECT_NE(DATA_STORAGE_ERROR, ret);
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
    ASSERT_TRUE(helper != nullptr);
    int ret = SimInsert(helper);
    EXPECT_NE(DATA_STORAGE_ERROR, ret);
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
    ASSERT_TRUE(helper != nullptr);
    int ret = SimUpdate(helper);
    EXPECT_NE(DATA_STORAGE_ERROR, ret);
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
    ASSERT_TRUE(helper != nullptr);
    int ret = SimSelect(helper);
    EXPECT_NE(DATA_STORAGE_ERROR, ret);
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
    ASSERT_TRUE(helper != nullptr);
    int ret = SimDelete(helper);
    EXPECT_NE(DATA_STORAGE_ERROR, ret);
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
    ASSERT_TRUE(helper != nullptr);
    int ret = SmsBatchInsert(helper);
    EXPECT_NE(DATA_STORAGE_ERROR, ret);
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
    ASSERT_TRUE(helper != nullptr);
    int ret = SmsInsert(helper);
    EXPECT_NE(DATA_STORAGE_ERROR, ret);
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
    ASSERT_TRUE(helper != nullptr);
    int ret = SmsUpdate(helper);
    EXPECT_NE(DATA_STORAGE_ERROR, ret);
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
    ASSERT_TRUE(helper != nullptr);
    int ret = SmsSelect(helper);
    EXPECT_NE(DATA_STORAGE_ERROR, ret);
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
    ASSERT_TRUE(helper != nullptr);
    int ret = SmsDelete(helper);
    EXPECT_NE(DATA_STORAGE_ERROR, ret);
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
    ASSERT_TRUE(helper != nullptr);
    int ret = PdpProfileInsert(helper);
    EXPECT_NE(DATA_STORAGE_ERROR, ret);
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
    ASSERT_TRUE(helper != nullptr);
    int ret = PdpProfileUpdate(helper);
    EXPECT_NE(DATA_STORAGE_ERROR, ret);
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
    ASSERT_TRUE(helper != nullptr);
    int ret = PdpProfileSelect(helper);
    EXPECT_NE(DATA_STORAGE_ERROR, ret);
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
    ASSERT_TRUE(helper != nullptr);
    int ret = PdpProfileDelete(helper);
    EXPECT_NE(DATA_STORAGE_ERROR, ret);
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
    std::shared_ptr<DataShare::DataShareHelper> helper = CreateGlobalEccHelper();
    ASSERT_TRUE(helper != nullptr);
    int ret = GlobalEccInsert(helper);
    EXPECT_NE(DATA_STORAGE_ERROR, ret);
}

/**
 * @tc.number   GlobalEccInsert_002
 * @tc.name     update ecc data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, GlobalEccInsert_002, Function | MediumTest | Level2)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreateGlobalEccHelper();
    ASSERT_TRUE(helper != nullptr);
    int ret = GlobalEccUpdate(helper);
    EXPECT_NE(DATA_STORAGE_ERROR, ret);
}

/**
 * @tc.number   GlobalEccInsert_003
 * @tc.name     select ecc data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, GlobalEccInsert_003, Function | MediumTest | Level1)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreateGlobalEccHelper();
    ASSERT_TRUE(helper != nullptr);
    int ret = GlobalEccSelect(helper);
    EXPECT_NE(DATA_STORAGE_ERROR, ret);
}

/**
 * @tc.number   GlobalEccInsert_004
 * @tc.name     delete ecc data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, GlobalEccInsert_004, Function | MediumTest | Level1)
{
    if (!HasVoiceCapability()) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = CreateGlobalEccHelper();
    ASSERT_TRUE(helper != nullptr);
    int ret = GlobalEccDelete(helper);
    EXPECT_NE(DATA_STORAGE_ERROR, ret);
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
