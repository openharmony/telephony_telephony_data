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

#include <iostream>
#include <vector>
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "data_ability_helper.h"
#include "data_ability_predicates.h"
#include "abs_shared_result_set.h"
#include "values_bucket.h"
#include "uri.h"
#include "resource_manager.h"

#include "sim_data.h"
#include "sms_mms_data.h"
#include "pdp_profile_data.h"
#include "opkey_data.h"
#include "data_storage_log_wrapper.h"

namespace OHOS {
namespace Telephony {
using CmdProcessFunc = int (*)(std::shared_ptr<AppExecFwk::DataAbilityHelper> helper);
std::map<char, CmdProcessFunc> g_simFuncMap;
std::map<char, CmdProcessFunc> g_smsFuncMap;
std::map<char, CmdProcessFunc> g_pdpProfileFuncMap;
std::map<char, CmdProcessFunc> g_opKeyFuncMap;
std::shared_ptr<AppExecFwk::DataAbilityHelper> simDataAbilityHelper = nullptr;
std::shared_ptr<AppExecFwk::DataAbilityHelper> smsDataAbilityHelper = nullptr;
std::shared_ptr<AppExecFwk::DataAbilityHelper> pdpProfileDataAbilityHelper = nullptr;
std::shared_ptr<AppExecFwk::DataAbilityHelper> opKeyDataAbilityHelper = nullptr;
std::shared_ptr<AppExecFwk::DataAbilityHelper> CreateDataAHelper(
    int32_t systemAbilityId, std::shared_ptr<Uri> dataAbilityUri)
{
    DATA_STORAGE_LOGI("DataSimRdbHelper::CreateDataAHelper ");
    auto saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        DATA_STORAGE_LOGE("DataSimRdbHelper Get system ability mgr failed.");
        return nullptr;
    }
    auto remoteObj = saManager->GetSystemAbility(systemAbilityId);
    if (remoteObj == nullptr) {
        DATA_STORAGE_LOGE("DataSimRdbHelper GetSystemAbility Service Failed.");
        return nullptr;
    }
    return AppExecFwk::DataAbilityHelper::Creator(remoteObj, dataAbilityUri);
}

void ApplyPermission()
{
    return;
}

void RemovePermission()
{
    return;
}

std::shared_ptr<AppExecFwk::DataAbilityHelper> CreateSimHelper()
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

std::shared_ptr<AppExecFwk::DataAbilityHelper> CreateSmsHelper()
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

std::shared_ptr<AppExecFwk::DataAbilityHelper> CreatePdpProfileHelper()
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

std::shared_ptr<AppExecFwk::DataAbilityHelper> CreateOpKeyHelper()
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

int SimSetCardByType(std::shared_ptr<AppExecFwk::DataAbilityHelper> helper)
{
    Uri uri("dataability:///com.ohos.simability/sim/sim_info/set_card");
    NativeRdb::ValuesBucket value;
    NativeRdb::DataAbilityPredicates predicates;
    value.PutInt(SimData::SLOT_INDEX, 1);
    value.PutInt(SimData::CARD_TYPE, 1);
    return helper->Update(uri, value, predicates);
}

int OpKeyInsert(std::shared_ptr<AppExecFwk::DataAbilityHelper> helper)
{
    std::string opkey = "1435";
    Uri uri("dataability:///com.ohos.opkeyability/opkey/opkey_info");
    NativeRdb::ValuesBucket value;
    value.PutInt(OpKeyData::ID, 1);
    value.PutString(OpKeyData::MCCMNC, "46000");
    value.PutString(OpKeyData::GID1, "gid1");
    value.PutString(OpKeyData::OPERATOR_NAME, "Mobile");
    value.PutString(OpKeyData::OPERATOR_KEY, opkey);
    return helper->Insert(uri, value);
}

int OpKeyUpdate(std::shared_ptr<AppExecFwk::DataAbilityHelper> helper)
{
    Uri uri("dataability:///com.ohos.opkeyability/opkey/opkey_info");
    NativeRdb::ValuesBucket values;
    values.PutString(OpKeyData::GID1, "gidd1");
    NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo(OpKeyData::OPERATOR_KEY, "1435");
    return helper->Update(uri, values, predicates);
}

int OpKeySelect(std::shared_ptr<AppExecFwk::DataAbilityHelper> helper)
{
    DATA_STORAGE_LOGI("OpKeySelect --- ");
    Uri uri("dataability:///com.ohos.opkeyability/opkey/opkey_info");
    std::vector<std::string> colume;
    NativeRdb::DataAbilityPredicates predicates;
    std::shared_ptr<NativeRdb::AbsSharedResultSet> resultSet = helper->Query(uri, colume, predicates);
    if (resultSet != nullptr) {
        int count;
        resultSet->GetRowCount(count);
        std::cout << "count is " << count;
        return count;
    }
    return -1;
}

int OpKeyDelete(std::shared_ptr<AppExecFwk::DataAbilityHelper> helper)
{
    Uri uri("dataability:///com.ohos.opkeyability/opkey/opkey_info");
    NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo(OpKeyData::ID, "1");
    return helper->Delete(uri, predicates);
}

int SimInsert(std::shared_ptr<AppExecFwk::DataAbilityHelper> helper)
{
    Uri uri("dataability:///com.ohos.simability/sim/sim_info");
    NativeRdb::ValuesBucket value;
    value.PutInt(SimData::SLOT_INDEX, 1);
    value.PutString(SimData::PHONE_NUMBER, "134xxxxxxxx");
    value.PutString(SimData::ICC_ID, "icc_id");
    value.PutString(SimData::CARD_ID, "card_id");
    return helper->Insert(uri, value);
}

int SimUpdate(std::shared_ptr<AppExecFwk::DataAbilityHelper> helper)
{
    Uri uri("dataability:///com.ohos.simability/sim/sim_info");
    std::string slot = std::to_string(1);
    NativeRdb::ValuesBucket values;
    values.PutString(SimData::SHOW_NAME, "China Mobile");
    NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo(SimData::SLOT_INDEX, slot);
    return helper->Update(uri, values, predicates);
}

int SimSelect(std::shared_ptr<AppExecFwk::DataAbilityHelper> helper)
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

int SimDelete(std::shared_ptr<AppExecFwk::DataAbilityHelper> helper)
{
    Uri uri("dataability:///com.ohos.simability/sim/sim_info");
    NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo(SimData::SLOT_INDEX, "1");
    return helper->Delete(uri, predicates);
}

int SmsBatchInsert(std::shared_ptr<AppExecFwk::DataAbilityHelper> helper)
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

int SmsInsert(std::shared_ptr<AppExecFwk::DataAbilityHelper> helper)
{
    Uri uri("dataability:///com.ohos.smsmmsability/sms_mms/sms_mms_info");
    NativeRdb::ValuesBucket value;
    value.PutString(SmsMmsInfo::RECEIVER_NUMBER, "134xxxxxxxx");
    value.PutString(SmsMmsInfo::MSG_CONTENT, "The first test text message content");
    value.PutInt(SmsMmsInfo::GROUP_ID, 1);
    return helper->Insert(uri, value);
}

int SmsUpdate(std::shared_ptr<AppExecFwk::DataAbilityHelper> helper)
{
    Uri uri("dataability:///com.ohos.smsmmsability/sms_mms/sms_mms_info");
    NativeRdb::ValuesBucket values;
    values.PutString(SmsMmsInfo::MSG_CONTENT, "The second test text message content");
    NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo(SmsMmsInfo::MSG_ID, "1");
    return helper->Update(uri, values, predicates);
}

int SmsSelect(std::shared_ptr<AppExecFwk::DataAbilityHelper> helper)
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

int SmsDelete(std::shared_ptr<AppExecFwk::DataAbilityHelper> helper)
{
    Uri uri("dataability:///com.ohos.smsmmsability/sms_mms/sms_mms_info");
    NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo(SmsMmsInfo::MSG_ID, "1");
    return helper->Delete(uri, predicates);
}

int PdpProfileReset(std::shared_ptr<AppExecFwk::DataAbilityHelper> helper)
{
    Uri uri("dataability:///com.ohos.pdpprofileability/net/pdp_profile/reset");
    NativeRdb::ValuesBucket values;
    NativeRdb::DataAbilityPredicates predicates;
    return helper->Update(uri, values, predicates);
}

int PdpProfileInsert(std::shared_ptr<AppExecFwk::DataAbilityHelper> helper)
{
    Uri uri("dataability:///com.ohos.pdpprofileability/net/pdp_profile");
    NativeRdb::ValuesBucket value;
    value.PutString(PdpProfileData::PROFILE_NAME, "frist_profile_name");
    value.PutString(PdpProfileData::MCC, "460");
    value.PutString(PdpProfileData::MNC, "91");
    return helper->Insert(uri, value);
}

int PdpProfileUpdate(std::shared_ptr<AppExecFwk::DataAbilityHelper> helper)
{
    Uri uri("dataability:///com.ohos.pdpprofileability/net/pdp_profile");
    NativeRdb::ValuesBucket values;
    values.PutString(PdpProfileData::PROFILE_NAME, "update_profile_name");
    NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo(PdpProfileData::PROFILE_ID, "1");
    return helper->Update(uri, values, predicates);
}

int PdpProfileDelete(std::shared_ptr<AppExecFwk::DataAbilityHelper> helper)
{
    Uri uri("dataability:///com.ohos.pdpprofileability/net/pdp_profile");
    NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo(PdpProfileData::PROFILE_ID, "1");
    return helper->Delete(uri, predicates);
}

int PdpProfileSelect(std::shared_ptr<AppExecFwk::DataAbilityHelper> helper)
{
    Uri uri("dataability:///com.ohos.pdpprofileability/net/pdp_profile");
    std::vector<std::string> colume;
    NativeRdb::DataAbilityPredicates predicates;
    std::shared_ptr<NativeRdb::AbsSharedResultSet> resultSet = helper->Query(uri, colume, predicates);
    if (resultSet != nullptr) {
        int count;
        resultSet->GetRowCount(count);
        std::cout << "count is " << count;
        return count;
    }
    return -1;
}

void Init()
{
    g_simFuncMap['q'] = SimInsert;
    g_simFuncMap['w'] = SimUpdate;
    g_simFuncMap['e'] = SimSelect;
    g_simFuncMap['r'] = SimDelete;
    g_simFuncMap['o'] = SimSetCardByType;
    g_smsFuncMap['t'] = SmsInsert;
    g_smsFuncMap['y'] = SmsUpdate;
    g_smsFuncMap['u'] = SmsSelect;
    g_smsFuncMap['i'] = SmsDelete;
    g_smsFuncMap['p'] = SmsBatchInsert;
    g_pdpProfileFuncMap['a'] = PdpProfileInsert;
    g_pdpProfileFuncMap['s'] = PdpProfileUpdate;
    g_pdpProfileFuncMap['d'] = PdpProfileDelete;
    g_pdpProfileFuncMap['f'] = PdpProfileSelect;
    g_pdpProfileFuncMap['h'] = PdpProfileReset;
    g_opKeyFuncMap['x'] = OpKeyInsert;
    g_opKeyFuncMap['c'] = OpKeyUpdate;
    g_opKeyFuncMap['v'] = OpKeySelect;
    g_opKeyFuncMap['b'] = OpKeyDelete;
    ApplyPermission();
}

int VerifyCmd(char inputCMD, std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper)
{
    auto itFunSim = g_simFuncMap.find(inputCMD);
    if (itFunSim != g_simFuncMap.end()) {
        auto memberFunc = itFunSim->second;
        if (memberFunc != nullptr) {
            helper = CreateSimHelper();
            if (helper != nullptr) {
                (*memberFunc)(helper);
            }
            return 0;
        }
    }
    auto itFunSms = g_smsFuncMap.find(inputCMD);
    if (itFunSms != g_smsFuncMap.end()) {
        auto memberFunc = itFunSms->second;
        if (memberFunc != nullptr) {
            helper = CreateSmsHelper();
            if (helper != nullptr) {
                (*memberFunc)(helper);
            }
            return 0;
        }
    }
    auto itFunPdpProfile = g_pdpProfileFuncMap.find(inputCMD);
    if (itFunPdpProfile != g_pdpProfileFuncMap.end()) {
        auto memberFunc = itFunPdpProfile->second;
        if (memberFunc != nullptr) {
            helper = CreatePdpProfileHelper();
            if (helper != nullptr) {
                (*memberFunc)(helper);
            }
            return 0;
        }
    }
    auto itFunOpKey = g_opKeyFuncMap.find(inputCMD);
    if (itFunOpKey != g_opKeyFuncMap.end()) {
        auto memberFunc = itFunOpKey->second;
        if (memberFunc != nullptr) {
            helper = CreateOpKeyHelper();
            if (helper != nullptr) {
                (*memberFunc)(helper);
            }
            return 0;
        }
    }
    return -1;
}

void ResourceTest()
{
    OHOS::Global::Resource::ResourceManager *rm = OHOS::Global::Resource::CreateResourceManager();
    std::string outValue;
    rm->GetStringByName("hello_ceshi", outValue);
    DATA_STORAGE_LOGI("DataSimRdbHelper::ResourceTest outValue = %{public}s", outValue.c_str());
    outValue = "";
    rm->GetStringByName("hello_telephony", outValue);
    DATA_STORAGE_LOGI("DataSimRdbHelper::ResourceTest outValue1 = %{public}s", outValue.c_str());
    free(rm);
    rm = nullptr;
}

void PrintfHint()
{
    printf("\n**********Unit Test Start**********\n"
           "usage: please input a cmd num:\n"
           "q:SimInsert()\n"
           "w:SimUpdate()\n"
           "e:SimSelect()\n"
           "r:SimDelete()\n"
           "o:SimSetCardByType()\n"
           "t:SmsInsert()\n"
           "y:SmsUpdate()\n"
           "u:SmsSelect()\n"
           "i:SmsDelete()\n"
           "p:SmsBatchInsert()\n"
           "a:PdpProfileInsert()\n"
           "s:PdpProfileUpdate()\n"
           "d:PdpProfileDelete()\n"
           "f:PdpProfileSelect()\n"
           "h:PdpProfileReset()\n"
           "g:ResourceTest()\n"
           "x:OpKeyInsert()\n"
           "c:OpKeyUpdate()\n"
           "v:OpKeySelect()\n"
           "b:OpKeyDelete()\n"
           "z:exit\n"
           "***********************************\n"
           "your choice: ");
}

void Looper()
{
    char inputCMD = '0';
    bool loopFlag = true;
    std::shared_ptr<AppExecFwk::DataAbilityHelper> helper = nullptr;
    Init();
    while (loopFlag) {
        PrintfHint();
        std::cin >> inputCMD;
        int ret = VerifyCmd(inputCMD, helper);
        if (ret == 0) {
            continue;
        }
        switch (inputCMD) {
            case 'g': {
                ResourceTest();
                break;
            }
            case 'z': {
                loopFlag = false;
                break;
            }
            default: {
                break;
            }
        }
    }
    RemovePermission();
}
} // namespace Telephony
} // namespace OHOS

using namespace OHOS;
int main()
{
    Telephony::Looper();
    return 0;
}
