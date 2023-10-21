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

#include <iostream>
#include <vector>
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "datashare_helper.h"
#include "datashare_predicates.h"
#include "abs_shared_result_set.h"
#include "values_bucket.h"
#include "uri.h"
#include "resource_manager.h"

#include "global_params_data.h"
#include "sim_data.h"
#include "sms_mms_data.h"
#include "pdp_profile_data.h"
#include "opkey_data.h"
#include "data_storage_log_wrapper.h"

namespace OHOS {
namespace Telephony {
using CmdProcessFunc = int (*)(std::shared_ptr<DataShare::DataShareHelper> helper);
std::map<char, CmdProcessFunc> g_simFuncMap;
std::map<char, CmdProcessFunc> g_smsFuncMap;
std::map<char, CmdProcessFunc> g_pdpProfileFuncMap;
std::map<char, CmdProcessFunc> g_opKeyFuncMap;
std::map<char, CmdProcessFunc> g_globalParamsFuncMap;
std::shared_ptr<DataShare::DataShareHelper> simDataHelper = nullptr;
std::shared_ptr<DataShare::DataShareHelper> smsDataHelper = nullptr;
std::shared_ptr<DataShare::DataShareHelper> pdpProfileDataHelper = nullptr;
std::shared_ptr<DataShare::DataShareHelper> opKeyDataHelper = nullptr;
std::shared_ptr<DataShare::DataShareHelper> globalParamsDataHelper = nullptr;
std::shared_ptr<DataShare::DataShareHelper> CreateDataShareHelper(
    int32_t systemAbilityId, std::string &uri)
{
    DATA_STORAGE_LOGI("DataSimRdbHelper::CreateDataShareHelper ");
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
    return DataShare::DataShareHelper::Creator(remoteObj, uri);
}

void ApplyPermission()
{
    return;
}

void RemovePermission()
{
    return;
}

std::shared_ptr<DataShare::DataShareHelper> CreateSimHelper()
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

std::shared_ptr<DataShare::DataShareHelper> CreateSmsHelper()
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

std::shared_ptr<DataShare::DataShareHelper> CreatePdpProfileHelper()
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

std::shared_ptr<DataShare::DataShareHelper> CreateOpKeyHelper()
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

std::shared_ptr<DataShare::DataShareHelper> CreateGlobalParamsHelper()
{
    if (globalParamsDataHelper == nullptr) {
        std::string uri(OPKEY_URI);
        if (uri.data() == nullptr) {
            DATA_STORAGE_LOGE("CreateGlobalParamsHelper uri is nullptr");
            return nullptr;
        }
        globalParamsDataHelper = CreateDataShareHelper(TELEPHONY_SMS_MMS_SYS_ABILITY_ID, uri);
    }
    return globalParamsDataHelper;
}

int SimSetCardByType(std::shared_ptr<DataShare::DataShareHelper> helper)
{
    Uri uri("datashare:///com.ohos.simability/sim/sim_info/set_card");
    DataShare::DataShareValuesBucket value;
    DataShare::DataSharePredicates predicates;
    value.Put(SimData::SIM_ID, 1);
    value.Put(SimData::CARD_TYPE, 1);
    return helper->Update(uri, predicates, value);
}

int OpKeyInsert(std::shared_ptr<DataShare::DataShareHelper> helper)
{
    std::string opkey = "1435";
    Uri uri("datashare:///com.ohos.opkeyability/opkey/opkey_info");
    DataShare::DataShareValuesBucket value;
    value.Put(OpKeyData::ID, 1);
    value.Put(OpKeyData::MCCMNC, "46000");
    value.Put(OpKeyData::GID1, "gid1");
    value.Put(OpKeyData::OPERATOR_NAME, "Mobile");
    value.Put(OpKeyData::OPERATOR_KEY, opkey);
    return helper->Insert(uri, value);
}

int OpKeyUpdate(std::shared_ptr<DataShare::DataShareHelper> helper)
{
    Uri uri("datashare:///com.ohos.opkeyability/opkey/opkey_info");
    DataShare::DataShareValuesBucket values;
    values.Put(OpKeyData::GID1, "gidd1");
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(OpKeyData::OPERATOR_KEY, "1435");
    return helper->Update(uri, predicates, values);
}

int OpKeySelect(std::shared_ptr<DataShare::DataShareHelper> helper)
{
    DATA_STORAGE_LOGI("OpKeySelect --- ");
    Uri uri("datashare:///com.ohos.opkeyability/opkey/opkey_info");
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

int OpKeyDelete(std::shared_ptr<DataShare::DataShareHelper> helper)
{
    Uri uri("datashare:///com.ohos.opkeyability/opkey/opkey_info");
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(OpKeyData::ID, "1");
    return helper->Delete(uri, predicates);
}

int SimInsert(std::shared_ptr<DataShare::DataShareHelper> helper)
{
    Uri uri("datashare:///com.ohos.simability/sim/sim_info");
    DataShare::DataShareValuesBucket value;
    value.Put(SimData::SIM_ID, 1);
    value.Put(SimData::PHONE_NUMBER, "134xxxxxxxx");
    value.Put(SimData::ICC_ID, "icc_id");
    value.Put(SimData::CARD_ID, "card_id");
    return helper->Insert(uri, value);
}

int SimUpdate(std::shared_ptr<DataShare::DataShareHelper> helper)
{
    Uri uri("datashare:///com.ohos.simability/sim/sim_info");
    std::string slot = std::to_string(1);
    DataShare::DataShareValuesBucket values;
    values.Put(SimData::SHOW_NAME, "China Mobile");
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SimData::SIM_ID, slot);
    return helper->Update(uri, predicates, values);
}

int SimSelect(std::shared_ptr<DataShare::DataShareHelper> helper)
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

int SimDelete(std::shared_ptr<DataShare::DataShareHelper> helper)
{
    Uri uri("datashare:///com.ohos.simability/sim/sim_info");
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SimData::SIM_ID, "1");
    return helper->Delete(uri, predicates);
}

int SmsBatchInsert(std::shared_ptr<DataShare::DataShareHelper> helper)
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

int SmsInsert(std::shared_ptr<DataShare::DataShareHelper> helper)
{
    Uri uri("datashare:///com.ohos.smsmmsability/sms_mms/sms_mms_info");
    DataShare::DataShareValuesBucket value;
    value.Put(SmsMmsInfo::RECEIVER_NUMBER, "134xxxxxxxx");
    value.Put(SmsMmsInfo::MSG_CONTENT, "The first test text message content");
    value.Put(SmsMmsInfo::GROUP_ID, 1);
    return helper->Insert(uri, value);
}

int SmsUpdate(std::shared_ptr<DataShare::DataShareHelper> helper)
{
    Uri uri("datashare:///com.ohos.smsmmsability/sms_mms/sms_mms_info");
    DataShare::DataShareValuesBucket values;
    values.Put(SmsMmsInfo::MSG_CONTENT, "The second test text message content");
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SmsMmsInfo::MSG_ID, "1");
    return helper->Update(uri, predicates, values);
}

int SmsSelect(std::shared_ptr<DataShare::DataShareHelper> helper)
{
    Uri uri("datashare:///com.ohos.smsmmsability/sms_mms/sms_mms_info");
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

int SmsDelete(std::shared_ptr<DataShare::DataShareHelper> helper)
{
    Uri uri("datashare:///com.ohos.smsmmsability/sms_mms/sms_mms_info");
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SmsMmsInfo::MSG_ID, "1");
    return helper->Delete(uri, predicates);
}

int PdpProfileReset(std::shared_ptr<DataShare::DataShareHelper> helper)
{
    Uri uri("datashare:///com.ohos.pdpprofileability/net/pdp_profile/reset");
    DataShare::DataShareValuesBucket values;
    DataShare::DataSharePredicates predicates;
    return helper->Update(uri, predicates, values);
}

int PdpProfileInsert(std::shared_ptr<DataShare::DataShareHelper> helper)
{
    Uri uri("datashare:///com.ohos.pdpprofileability/net/pdp_profile");
    DataShare::DataShareValuesBucket value;
    value.Put(PdpProfileData::PROFILE_NAME, "frist_profile_name");
    value.Put(PdpProfileData::MCC, "460");
    value.Put(PdpProfileData::MNC, "91");
    return helper->Insert(uri, value);
}

int PdpProfileUpdate(std::shared_ptr<DataShare::DataShareHelper> helper)
{
    Uri uri("datashare:///com.ohos.pdpprofileability/net/pdp_profile");
    DataShare::DataShareValuesBucket values;
    values.Put(PdpProfileData::PROFILE_NAME, "update_profile_name");
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(PdpProfileData::PROFILE_ID, "1");
    return helper->Update(uri, predicates, values);
}

int PdpProfileDelete(std::shared_ptr<DataShare::DataShareHelper> helper)
{
    Uri uri("datashare:///com.ohos.pdpprofileability/net/pdp_profile");
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(PdpProfileData::PROFILE_ID, "1");
    return helper->Delete(uri, predicates);
}

int PdpProfileSelect(std::shared_ptr<DataShare::DataShareHelper> helper)
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

int GlobalEccInsert(std::shared_ptr<DataShare::DataShareHelper> helper)
{
    Uri uri("datashare:///com.ohos.globalparamsability/globalparams/ecc_data");
    DataShare::DataShareValuesBucket value;
    value.Put(EccData::MCC, "460");
    value.Put(EccData::MNC, "01");
    value.Put(EccData::NUMERIC, "46001");
    return helper->Insert(uri, value);
}

int GlobalEccUpdate(std::shared_ptr<DataShare::DataShareHelper> helper)
{
    Uri uri("datashare:///com.ohos.globalparamsability/globalparams/ecc_data");
    DataShare::DataShareValuesBucket values;
    values.Put(EccData::NAME, "46001");
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(EccData::ID, "1");
    return helper->Update(uri, predicates, values);
}

int GlobalEccDelete(std::shared_ptr<DataShare::DataShareHelper> helper)
{
    Uri uri("datashare:///com.ohos.globalparamsability/globalparams/ecc_data");
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(EccData::ID, "1");
    return helper->Delete(uri, predicates);
}

int GlobalEccSelect(std::shared_ptr<DataShare::DataShareHelper> helper)
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
    g_globalParamsFuncMap['j'] = GlobalEccInsert;
    g_globalParamsFuncMap['k'] = GlobalEccUpdate;
    g_globalParamsFuncMap['l'] = GlobalEccSelect;
    g_globalParamsFuncMap['m'] = GlobalEccDelete;
    ApplyPermission();
}

int VerifyCmd(char inputCMD, std::shared_ptr<DataShare::DataShareHelper> &helper)
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
    auto itFunGlobalEcc = g_globalParamsFuncMap.find(inputCMD);
    if (itFunGlobalEcc != g_globalParamsFuncMap.end()) {
        auto memberFunc = itFunGlobalEcc->second;
        if (memberFunc != nullptr) {
            helper = CreateGlobalParamsHelper();
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
           "j:GlobalInsert()\n"
           "k:GlobalUpdate()\n"
           "l:GlobalSelect()\n"
           "m:GlobalDelete()\n"
           "z:exit\n"
           "***********************************\n"
           "your choice: ");
}

void Looper()
{
    char inputCMD = '0';
    bool loopFlag = true;
    std::shared_ptr<DataShare::DataShareHelper> helper = nullptr;
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
