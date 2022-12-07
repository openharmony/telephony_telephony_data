/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include "opkey_data.h"
#include "pdp_profile_data.h"
#include "sim_data.h"
#include "sms_mms_data.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
const int PERMS_NUM = 4;

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

std::shared_ptr<AppExecFwk::DataAbilityHelper> DataStorageGtest::CreateDataAHelper(
    int32_t systemAbilityId, std::shared_ptr<Uri> dataAbilityUri) const
{
    DATA_STORAGE_LOGI("DataStorageGtest::CreateDataAHelper");
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
    return AppExecFwk::DataAbilityHelper::Creator(remoteObj, dataAbilityUri);
}

std::shared_ptr<AppExecFwk::DataAbilityHelper> DataStorageGtest::CreateSimHelper()
{
    if (simDataAbilityHelper == nullptr) {
        std::shared_ptr<Uri> dataAbilityUri = std::make_shared<Uri>(SIM_URI);
        if (dataAbilityUri == nullptr) {
            DATA_STORAGE_LOGE("CreateSimHelper dataAbilityUri is nullptr");
            return nullptr;
        }
        simDataAbilityHelper = CreateDataAHelper(TELEPHONY_CORE_SERVICE_SYS_ABILITY_ID, dataAbilityUri);
    }
    return simDataAbilityHelper;
}

std::shared_ptr<AppExecFwk::DataAbilityHelper> DataStorageGtest::CreateSmsHelper()
{
    if (smsDataAbilityHelper == nullptr) {
        std::shared_ptr<Uri> dataAbilityUri = std::make_shared<Uri>(SMS_MMS_URI);
        if (dataAbilityUri == nullptr) {
            DATA_STORAGE_LOGE("CreateSmsHelper dataAbilityUri is nullptr");
            return nullptr;
        }
        smsDataAbilityHelper = CreateDataAHelper(TELEPHONY_SMS_MMS_SYS_ABILITY_ID, dataAbilityUri);
    }
    return smsDataAbilityHelper;
}

std::shared_ptr<AppExecFwk::DataAbilityHelper> DataStorageGtest::CreatePdpProfileHelper()
{
    if (pdpProfileDataAbilityHelper == nullptr) {
        std::shared_ptr<Uri> dataAbilityUri = std::make_shared<Uri>(PDP_PROFILE_URI);
        if (dataAbilityUri == nullptr) {
            DATA_STORAGE_LOGE("CreatePdpProfileHelper dataAbilityUri is nullptr");
            return nullptr;
        }
        pdpProfileDataAbilityHelper = CreateDataAHelper(TELEPHONY_SMS_MMS_SYS_ABILITY_ID, dataAbilityUri);
    }
    return pdpProfileDataAbilityHelper;
}

std::shared_ptr<AppExecFwk::DataAbilityHelper> DataStorageGtest::CreateOpKeyHelper()
{
    if (opKeyDataAbilityHelper == nullptr) {
        std::shared_ptr<Uri> dataAbilityUri = std::make_shared<Uri>(OPKEY_URI);
        if (dataAbilityUri == nullptr) {
            DATA_STORAGE_LOGE("CreateOpKeyHelper dataAbilityUri is nullptr");
            return nullptr;
        }
        opKeyDataAbilityHelper = CreateDataAHelper(TELEPHONY_SMS_MMS_SYS_ABILITY_ID, dataAbilityUri);
    }
    return opKeyDataAbilityHelper;
}

int DataStorageGtest::OpKeyInsert(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const
{
    std::string opkey = "110";
    Uri uri("dataability:///com.ohos.opkeyability/opkey/opkey_info");
    NativeRdb::ValuesBucket value;
    value.PutInt(OpKeyData::ID, 1);
    value.PutString(OpKeyData::MCCMNC, "460");
    value.PutString(OpKeyData::GID1, "gid1");
    value.PutString(OpKeyData::OPERATOR_NAME, "name");
    value.PutString(OpKeyData::OPERATOR_KEY, opkey);
    return helper->Insert(uri, value);
}

int DataStorageGtest::OpKeyUpdate(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const
{
    Uri uri("dataability:///com.ohos.opkeyability/opkey/opkey_info");
    NativeRdb::ValuesBucket values;
    values.PutString(OpKeyData::OPERATOR_NAME, "name2");
    NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo(OpKeyData::OPERATOR_KEY, "123");
    return helper->Update(uri, values, predicates);
}

int DataStorageGtest::OpKeySelect(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const
{
    Uri uri("dataability:///com.ohos.opkeyability/opkey/opkey_info");
    std::vector<std::string> colume;
    NativeRdb::DataAbilityPredicates predicates;
    std::shared_ptr<NativeRdb::AbsSharedResultSet> resultSet = helper->Query(uri, colume, predicates);
    if (resultSet != nullptr) {
        int count;
        resultSet->GetRowCount(count);
        return count;
    }
    return -1;
}

int DataStorageGtest::OpKeyDelete(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const
{
    Uri uri("dataability:///com.ohos.opkeyability/opkey/opkey_info");
    NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo(OpKeyData::ID, "1");
    return helper->Delete(uri, predicates);
}

int DataStorageGtest::SimInsert(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const
{
    Uri uri("dataability:///com.ohos.simability/sim/sim_info");
    NativeRdb::ValuesBucket value;
    value.PutInt(SimData::SLOT_INDEX, 1);
    value.PutString(SimData::PHONE_NUMBER, "134xxxxxxxx");
    value.PutString(SimData::ICC_ID, "icc_id");
    value.PutString(SimData::CARD_ID, "card_id");
    return helper->Insert(uri, value);
}

int DataStorageGtest::SimUpdate(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const
{
    Uri uri("dataability:///com.ohos.simability/sim/sim_info");
    std::string slot = std::to_string(1);
    NativeRdb::ValuesBucket values;
    values.PutString(SimData::SHOW_NAME, "China Mobile");
    NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo(SimData::SLOT_INDEX, slot);
    return helper->Update(uri, values, predicates);
}

int DataStorageGtest::SimSelect(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const
{
    Uri uri("dataability:///com.ohos.simability/sim/sim_info");
    std::vector<std::string> colume;
    NativeRdb::DataAbilityPredicates predicates;
    std::shared_ptr<NativeRdb::AbsSharedResultSet> resultSet = helper->Query(uri, colume, predicates);
    if (resultSet != nullptr) {
        int count;
        resultSet->GetRowCount(count);
        return count;
    }
    return -1;
}

int DataStorageGtest::SimDelete(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const
{
    Uri uri("dataability:///com.ohos.simability/sim/sim_info");
    NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo(SimData::SLOT_INDEX, "1");
    return helper->Delete(uri, predicates);
}

int DataStorageGtest::SmsBatchInsert(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const
{
    Uri uri("dataability:///com.ohos.smsmmsability/sms_mms/sms_mms_info");
    std::vector<NativeRdb::ValuesBucket> values;
    NativeRdb::ValuesBucket value;
    int batchNum = 100;
    for (int i = 0; i < batchNum; i++) {
        value.PutString(SmsMmsInfo::RECEIVER_NUMBER, "134xxxxxxxx");
        value.PutString(SmsMmsInfo::MSG_CONTENT, "The first test text message content");
        value.PutInt(SmsMmsInfo::GROUP_ID, 1);
        values.push_back(value);
        value.Clear();
    }
    return helper->BatchInsert(uri, values);
}

int DataStorageGtest::SmsInsert(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const
{
    Uri uri("dataability:///com.ohos.smsmmsability/sms_mms/sms_mms_info");
    NativeRdb::ValuesBucket value;
    value.PutString(SmsMmsInfo::RECEIVER_NUMBER, "134xxxxxxxx");
    value.PutString(SmsMmsInfo::MSG_CONTENT, "The first test text message content");
    value.PutInt(SmsMmsInfo::GROUP_ID, 1);
    value.PutInt(SmsMmsInfo::IS_SENDER, 0);
    return helper->Insert(uri, value);
}

int DataStorageGtest::SmsUpdate(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const
{
    Uri uri("dataability:///com.ohos.smsmmsability/sms_mms/sms_mms_info");
    NativeRdb::ValuesBucket values;
    values.PutString(SmsMmsInfo::MSG_CONTENT, "The second test text message content");
    NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo(SmsMmsInfo::MSG_ID, "1");
    return helper->Update(uri, values, predicates);
}

int DataStorageGtest::SmsSelect(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const
{
    Uri uri("dataability:///com.ohos.smsmmsability/sms_mms/sms_mms_info");
    std::vector<std::string> colume;
    NativeRdb::DataAbilityPredicates predicates;
    std::shared_ptr<NativeRdb::AbsSharedResultSet> resultSet = helper->Query(uri, colume, predicates);
    if (resultSet != nullptr) {
        int count;
        resultSet->GetRowCount(count);
        return count;
    }
    return -1;
}

int DataStorageGtest::SmsDelete(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const
{
    Uri uri("dataability:///com.ohos.smsmmsability/sms_mms/sms_mms_info");
    NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo(SmsMmsInfo::MSG_ID, "1");
    return helper->Delete(uri, predicates);
}

int DataStorageGtest::PdpProfileInsert(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const
{
    Uri uri("dataability:///com.ohos.pdpprofileability/net/pdp_profile");
    NativeRdb::ValuesBucket value;
    value.PutString(PdpProfileData::PROFILE_NAME, "frist_profile_name");
    value.PutString(PdpProfileData::MCC, "460");
    value.PutString(PdpProfileData::MNC, "91");
    return helper->Insert(uri, value);
}

int DataStorageGtest::PdpProfileUpdate(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const
{
    Uri uri("dataability:///com.ohos.pdpprofileability/net/pdp_profile");
    NativeRdb::ValuesBucket values;
    values.PutString(PdpProfileData::PROFILE_NAME, "update_profile_name");
    NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo(PdpProfileData::PROFILE_ID, "1");
    return helper->Update(uri, values, predicates);
}

int DataStorageGtest::PdpProfileSelect(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const
{
    Uri uri("dataability:///com.ohos.pdpprofileability/net/pdp_profile");
    std::vector<std::string> colume;
    NativeRdb::DataAbilityPredicates predicates;
    std::shared_ptr<NativeRdb::AbsSharedResultSet> resultSet = helper->Query(uri, colume, predicates);
    if (resultSet != nullptr) {
        int count;
        resultSet->GetRowCount(count);
        return count;
    }
    return -1;
}

int DataStorageGtest::PdpProfileDelete(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const
{
    Uri uri("dataability:///com.ohos.pdpprofileability/net/pdp_profile");
    NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo(PdpProfileData::PROFILE_ID, "1");
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
    std::shared_ptr<AppExecFwk::DataAbilityHelper> smsHelper = CreateSmsHelper();
    ASSERT_TRUE(smsHelper != nullptr);
    std::shared_ptr<AppExecFwk::DataAbilityHelper> simHelper = CreateSimHelper();
    ASSERT_TRUE(simHelper != nullptr);
}

/**
 * @tc.number   OpKeyInsert_001
 * @tc.name     insert opkey data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, OpKeyInsert_001, Function | MediumTest | Level1)
{
    std::shared_ptr<AppExecFwk::DataAbilityHelper> helper = CreateOpKeyHelper();
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
    std::shared_ptr<AppExecFwk::DataAbilityHelper> helper = CreateOpKeyHelper();
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
    std::shared_ptr<AppExecFwk::DataAbilityHelper> helper = CreateOpKeyHelper();
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
    std::shared_ptr<AppExecFwk::DataAbilityHelper> helper = CreateOpKeyHelper();
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
    std::shared_ptr<AppExecFwk::DataAbilityHelper> helper = CreateSimHelper();
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
    std::shared_ptr<AppExecFwk::DataAbilityHelper> helper = CreateSimHelper();
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
    std::shared_ptr<AppExecFwk::DataAbilityHelper> helper = CreateSimHelper();
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
    std::shared_ptr<AppExecFwk::DataAbilityHelper> helper = CreateSimHelper();
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
    std::shared_ptr<AppExecFwk::DataAbilityHelper> helper = CreateSmsHelper();
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
    std::shared_ptr<AppExecFwk::DataAbilityHelper> helper = CreateSmsHelper();
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
    std::shared_ptr<AppExecFwk::DataAbilityHelper> helper = CreateSmsHelper();
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
    std::shared_ptr<AppExecFwk::DataAbilityHelper> helper = CreateSmsHelper();
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
    std::shared_ptr<AppExecFwk::DataAbilityHelper> helper = CreateSmsHelper();
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
    std::shared_ptr<AppExecFwk::DataAbilityHelper> helper = CreatePdpProfileHelper();
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
    std::shared_ptr<AppExecFwk::DataAbilityHelper> helper = CreatePdpProfileHelper();
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
    std::shared_ptr<AppExecFwk::DataAbilityHelper> helper = CreatePdpProfileHelper();
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
    std::shared_ptr<AppExecFwk::DataAbilityHelper> helper = CreatePdpProfileHelper();
    ASSERT_TRUE(helper != nullptr);
    int ret = PdpProfileDelete(helper);
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
