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

#include "data_storage_log_wrapper.h"
#include "sim_data.h"
#include "sms_mms_data.h"
#include "pdp_profile_data.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
void DataStorageGtest::SetUpTestCase(void)
{
    // step 3: Set Up Test Case
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
    while (remoteObj == nullptr) {
        DATA_STORAGE_LOGE("DataStorageGtest GetSystemAbility Service Failed.");
        return nullptr;
    }
    return AppExecFwk::DataAbilityHelper::Creator(remoteObj, dataAbilityUri);
}

std::shared_ptr<AppExecFwk::DataAbilityHelper> DataStorageGtest::CreateSimHelper()
{
    if (simDataAbilityHelper == nullptr) {
        std::shared_ptr<Uri> dataAbilityUri = std::make_shared<Uri>(SIM_URI);
        simDataAbilityHelper = CreateDataAHelper(TELEPHONY_CORE_SERVICE_SYS_ABILITY_ID, dataAbilityUri);
    }
    return simDataAbilityHelper;
}

std::shared_ptr<AppExecFwk::DataAbilityHelper> DataStorageGtest::CreateSmsHelper()
{
    if (smsDataAbilityHelper == nullptr) {
        std::shared_ptr<Uri> dataAbilityUri = std::make_shared<Uri>(SMS_MMS_URI);
        smsDataAbilityHelper = CreateDataAHelper(TELEPHONY_SMS_MMS_SYS_ABILITY_ID, dataAbilityUri);
    }
    return smsDataAbilityHelper;
}

std::shared_ptr<AppExecFwk::DataAbilityHelper> DataStorageGtest::CreatePdpProfileHelper()
{
    if (smsDataAbilityHelper == nullptr) {
        std::shared_ptr<Uri> dataAbilityUri = std::make_shared<Uri>(PDP_PROFILE_URI);
        pdpProfileDataAbilityHelper = CreateDataAHelper(TELEPHONY_SMS_MMS_SYS_ABILITY_ID, dataAbilityUri);
    }
    return pdpProfileDataAbilityHelper;
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
    value.PutString(PdpProfileData::PROFILE_NAME, "test_profile_name");
    value.PutString(PdpProfileData::MCC, "460");
    value.PutString(PdpProfileData::MNC, "91");
    return helper->Insert(uri, value);
}

int DataStorageGtest::PdpProfileUpdate(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const
{
    Uri uri("dataability:///com.ohos.pdpprofileability/net/pdp_profile");
    NativeRdb::ValuesBucket values;
    values.PutString(PdpProfileData::MCC, "461");
    values.PutString(PdpProfileData::MNC, "92");
    NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo(PdpProfileData::PROFILE_NAME, "test_profile_name");
    return helper->Update(uri, values, predicates);
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> DataStorageGtest::PdpProfileSelectOne(
    const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper,
    NativeRdb::DataAbilityPredicates &predicates) const
{
    Uri uri("dataability:///com.ohos.pdpprofileability/net/pdp_profile");
    std::vector<std::string> colume;
    colume.push_back(PdpProfileData::PROFILE_NAME);
    colume.push_back(PdpProfileData::MCC);
    colume.push_back(PdpProfileData::MNC);
    std::shared_ptr<NativeRdb::AbsSharedResultSet> resultSet = helper->Query(uri, colume, predicates);
    if (resultSet != nullptr) {
        return resultSet;
    }
    return nullptr;
}

int DataStorageGtest::PdpProfileSelectAll(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const
{
    Uri uri("dataability:///com.ohos.pdpprofileability/net/pdp_profile");
    std::vector<std::string> colume;
    colume.push_back(PdpProfileData::PROFILE_NAME);
    colume.push_back(PdpProfileData::MCC);
    colume.push_back(PdpProfileData::MNC);
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
    predicates.EqualTo(PdpProfileData::PROFILE_NAME, "test_profile_name");
    return helper->Delete(uri, predicates);
}

#ifndef TEL_TEST_UNSUPPORT
/**
 * @tc.number   DataStorage_001
 * @tc.name     create sim and sms DataAbilityHelper
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, DataStorage_001, TestSize.Level0)
{
    CreateSmsHelper();
    CreateSimHelper();
}

/**
 * @tc.number   SimInsert_001
 * @tc.name     insert sim data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, SimInsert_001, TestSize.Level1)
{
    std::shared_ptr<AppExecFwk::DataAbilityHelper> helper = CreateSimHelper();
    if (helper != nullptr) {
        SimInsert(helper);
    }
}

/**
 * @tc.number   SimUpdate_001
 * @tc.name     update sim data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, SimUpdate_001, TestSize.Level1)
{
    std::shared_ptr<AppExecFwk::DataAbilityHelper> helper = CreateSimHelper();
    if (helper != nullptr) {
        SimUpdate(helper);
    }
}

/**
 * @tc.number   SimSelect_001
 * @tc.name     select sim data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, SimSelect_001, TestSize.Level1)
{
    std::shared_ptr<AppExecFwk::DataAbilityHelper> helper = CreateSimHelper();
    if (helper != nullptr) {
        SimSelect(helper);
    }
}

/**
 * @tc.number   SimDelete_001
 * @tc.name     delete sim data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, SimDelete_001, TestSize.Level1)
{
    std::shared_ptr<AppExecFwk::DataAbilityHelper> helper = CreateSimHelper();
    if (helper != nullptr) {
        SimDelete(helper);
    }
}

/**
 * @tc.number   SmsInsert_001
 * @tc.name     insert sms data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, SmsInsert_001, TestSize.Level1)
{
    std::shared_ptr<AppExecFwk::DataAbilityHelper> helper = CreateSmsHelper();
    if (helper != nullptr) {
        SmsInsert(helper);
    }
}

/**
 * @tc.number   SmsUpdate_001
 * @tc.name     update sms data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, SmsUpdate_001, TestSize.Level1)
{
    std::shared_ptr<AppExecFwk::DataAbilityHelper> helper = CreateSmsHelper();
    if (helper != nullptr) {
        SmsUpdate(helper);
    }
}

/**
 * @tc.number   SmsSelect_001
 * @tc.name     select sms data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, SmsSelect_001, TestSize.Level1)
{
    std::shared_ptr<AppExecFwk::DataAbilityHelper> helper = CreateSmsHelper();
    if (helper != nullptr) {
        SmsSelect(helper);
    }
}

/**
 * @tc.number   SmsDelete_001
 * @tc.name     delete sms data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, SmsDelete_001, TestSize.Level1)
{
    std::shared_ptr<AppExecFwk::DataAbilityHelper> helper = CreateSmsHelper();
    if (helper != nullptr) {
        SmsDelete(helper);
    }
}

/**
 * @tc.number   PdpProfileInsert_001
 * @tc.name     insert apn data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, PdpProfileInsert_001, TestSize.Level1)
{
    std::shared_ptr<AppExecFwk::DataAbilityHelper> helper = CreatePdpProfileHelper();
    ASSERT_TRUE(helper != nullptr);

    int res = PdpProfileDelete(helper);
    ASSERT_EQ(res, 0);

    int count = PdpProfileSelectAll(helper);
    ASSERT_TRUE(count > 0);

    // insert test data into database
    int insertId = PdpProfileInsert(helper);
    printf("PdpProfileInsert insertId: %d\n", insertId);
    ASSERT_TRUE(insertId > 0);

    // query test data by profile_name
    NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo(PdpProfileData::PROFILE_NAME, "test_profile_name");
    std::shared_ptr<NativeRdb::AbsSharedResultSet> resultSet = PdpProfileSelectOne(helper, predicates);
    ASSERT_TRUE(resultSet != nullptr);
    resultSet->GetRowCount(count);
    ASSERT_EQ(count, 1);

    // check if profile_name, mcc, mnc is equal
    int index = 0;
    std::string resultProfileName;
    std::string resultMcc;
    std::string resultMnc;
    resultSet->GoToRow(0);
    resultSet->GetColumnIndex(PdpProfileData::PROFILE_NAME, index);
    resultSet->GetString(index, resultProfileName);
    printf("resultProfileName: %s\n", resultProfileName.c_str());
    ASSERT_EQ(resultProfileName, "test_profile_name");

    resultSet->GetColumnIndex(PdpProfileData::MCC, index);
    resultSet->GetString(index, resultMcc);
    printf("resultMcc: %s\n", resultMcc.c_str());
    ASSERT_EQ(resultMcc, "460");

    resultSet->GetColumnIndex(PdpProfileData::MNC, index);
    resultSet->GetString(index, resultMnc);
    printf("resultMnc: %s\n", resultMnc.c_str());
    ASSERT_EQ(resultMnc, "91");

    // delete test data
    res = PdpProfileDelete(helper);
    ASSERT_EQ(res, 0);

    resultSet = PdpProfileSelectOne(helper, predicates);
    ASSERT_TRUE(resultSet != nullptr);
    resultSet->GetRowCount(count);
    ASSERT_EQ(count, 0);
}

/**
 * @tc.number   PdpProfileUpdate_001
 * @tc.name     update apn data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, PdpProfileUpdate_001, TestSize.Level1)
{
    std::shared_ptr<AppExecFwk::DataAbilityHelper> helper = CreatePdpProfileHelper();
    ASSERT_TRUE(helper != nullptr);

    int res = PdpProfileDelete(helper);
    ASSERT_EQ(res, 0);

    int count = PdpProfileSelectAll(helper);
    ASSERT_TRUE(count > 0);
    
    // insert test data into database
    int insertId = PdpProfileInsert(helper);
    printf("PdpProfileInsert insertId: %d\n", insertId);
    ASSERT_TRUE(insertId > 0);

    // update test data
    res = PdpProfileUpdate(helper);
    ASSERT_EQ(res, 0);

    // query test data by profile_name
    NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo(PdpProfileData::PROFILE_NAME, "test_profile_name");
    std::shared_ptr<NativeRdb::AbsSharedResultSet> resultSet = PdpProfileSelectOne(helper, predicates);
    ASSERT_TRUE(resultSet != nullptr);
    resultSet->GetRowCount(count);
    ASSERT_EQ(count, 1);

    // check if profile_name, mcc, mnc is equal
    int index = 0;
    std::string resultProfileName;
    std::string resultMcc;
    std::string resultMnc;
    resultSet->GoToRow(0);
    resultSet->GetColumnIndex(PdpProfileData::PROFILE_NAME, index);
    resultSet->GetString(index, resultProfileName);
    printf("resultProfileName: %s\n", resultProfileName.c_str());
    ASSERT_EQ(resultProfileName, "test_profile_name");

    resultSet->GetColumnIndex(PdpProfileData::MCC, index);
    resultSet->GetString(index, resultMcc);
    printf("resultMcc: %s\n", resultMcc.c_str());
    ASSERT_EQ(resultMcc, "461");

    resultSet->GetColumnIndex(PdpProfileData::MNC, index);
    resultSet->GetString(index, resultMnc);
    printf("resultMnc: %s\n", resultMnc.c_str());
    ASSERT_EQ(resultMnc, "92");

    // delete test data
    res = PdpProfileDelete(helper);
    ASSERT_EQ(res, 0);

    resultSet = PdpProfileSelectOne(helper, predicates);
    ASSERT_TRUE(resultSet != nullptr);
    resultSet->GetRowCount(count);
    ASSERT_EQ(count, 0);
}

/**
 * @tc.number   PdpProfileSelect_001
 * @tc.name     select apn data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, PdpProfileSelect_001, TestSize.Level1)
{
    std::shared_ptr<AppExecFwk::DataAbilityHelper> helper = CreatePdpProfileHelper();
    ASSERT_TRUE(helper != nullptr);

    int res = PdpProfileDelete(helper);
    ASSERT_EQ(res, 0);

    int count = PdpProfileSelectAll(helper);
    ASSERT_TRUE(count > 0);

    // insert test data into database
    int insertId = PdpProfileInsert(helper);
    printf("PdpProfileInsert insertId: %d\n", insertId);
    ASSERT_TRUE(insertId > 0);

    // query test data by profile_name
    NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo(PdpProfileData::PROFILE_NAME, "test_profile_name");
    std::shared_ptr<NativeRdb::AbsSharedResultSet> resultSet = PdpProfileSelectOne(helper, predicates);
    ASSERT_TRUE(resultSet != nullptr);
    resultSet->GetRowCount(count);
    ASSERT_EQ(count, 1);

    int index = 0;
    std::string resultProfileName;
    std::string resultMcc;
    std::string resultMnc;
    resultSet->GoToRow(0);
    resultSet->GetColumnIndex(PdpProfileData::PROFILE_NAME, index);
    resultSet->GetString(index, resultProfileName);
    printf("resultProfileName: %s\n", resultProfileName.c_str());
    ASSERT_EQ(resultProfileName, "test_profile_name");

    resultSet->GetColumnIndex(PdpProfileData::MCC, index);
    resultSet->GetString(index, resultMcc);
    printf("resultMcc: %s\n", resultMcc.c_str());
    ASSERT_EQ(resultMcc, "460");

    resultSet->GetColumnIndex(PdpProfileData::MNC, index);
    resultSet->GetString(index, resultMnc);
    printf("resultMnc: %s\n", resultMnc.c_str());
    ASSERT_EQ(resultMnc, "91");

    // delete test data
    res = PdpProfileDelete(helper);
    ASSERT_EQ(res, 0);

    resultSet = PdpProfileSelectOne(helper, predicates);
    ASSERT_TRUE(resultSet != nullptr);
    resultSet->GetRowCount(count);
    ASSERT_EQ(count, 0);
}

/**
 * @tc.number   PdpProfileDelete_001
 * @tc.name     delete apn data
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, PdpProfileDelete_001, TestSize.Level1)
{
    std::shared_ptr<AppExecFwk::DataAbilityHelper> helper = CreatePdpProfileHelper();
    ASSERT_TRUE(helper != nullptr);

    int res = PdpProfileDelete(helper);
    ASSERT_EQ(res, 0);

    int count = PdpProfileSelectAll(helper);
    ASSERT_TRUE(count > 0);

    // insert test data into database
    int insertId = PdpProfileInsert(helper);
    printf("PdpProfileInsert insertId: %d\n", insertId);
    ASSERT_TRUE(insertId > 0);

    NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo(PdpProfileData::PROFILE_NAME, "test_profile_name");
    std::shared_ptr<NativeRdb::AbsSharedResultSet> resultSet = PdpProfileSelectOne(helper, predicates);
    ASSERT_TRUE(resultSet != nullptr);
    resultSet->GetRowCount(count);
    ASSERT_EQ(count, 1);

    // delete test data
    res = PdpProfileDelete(helper);
    ASSERT_EQ(res, 0);

    resultSet = PdpProfileSelectOne(helper, predicates);
    ASSERT_TRUE(resultSet != nullptr);
    resultSet->GetRowCount(count);
    ASSERT_EQ(count, 0);
}
#else // TEL_TEST_UNSUPPORT
/**
 * @tc.number   TelMockTest_001
 * @tc.name     Test for unsupport platform
 * @tc.desc     Function test
 */
HWTEST_F(DataStorageGtest, TelMockTest_001, TestSize.Level1)
{
    EXPECT_TRUE(true);
}
#endif // TEL_TEST_UNSUPPORT
} // namespace Telephony
} // namespace OHOS
