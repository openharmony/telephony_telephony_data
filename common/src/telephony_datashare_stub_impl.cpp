/*
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

#include "dataobs_mgr_client.h"
#include "datashare_ext_ability_context.h"
#include "data_storage_errors.h"
#include "data_storage_log_wrapper.h"
#include "opkey_ability.h"
#include "pdp_profile_ability.h"
#include "sim_ability.h"
#include "sms_mms_ability.h"
#include "telephony_datashare_stub_impl.h"

namespace OHOS {
namespace DataShare {
using DataObsMgrClient = OHOS::AAFwk::DataObsMgrClient;
using namespace OHOS::Telephony;

std::shared_ptr<DataShareExtAbility> TelephonyDataShareStubImpl::GetTelephonyDataAbility()
{
    return telephonyDataAbility_;
}

std::shared_ptr<DataShareExtAbility> TelephonyDataShareStubImpl::GetOpKeyAbility()
{
    if (opKeyAbility_ == nullptr) {
        opKeyAbility_.reset(OpKeyAbility::Create());
    }
    return opKeyAbility_;
}

std::shared_ptr<DataShareExtAbility> TelephonyDataShareStubImpl::GetPdpProfileAbility()
{
    if (pdpProfileAbility_ == nullptr) {
        pdpProfileAbility_.reset(PdpProfileAbility::Create());
    }
    return pdpProfileAbility_;
}

std::shared_ptr<DataShareExtAbility> TelephonyDataShareStubImpl::GetSimAbility()
{
    if (simAbility_ == nullptr) {
        simAbility_.reset(SimAbility::Create());
    }
    return simAbility_;
}

std::shared_ptr<DataShareExtAbility> TelephonyDataShareStubImpl::GetSmsMmsAbility()
{
    if (smsMmsAbility_ == nullptr) {
        smsMmsAbility_.reset(SmsMmsAbility::Create());
    }
    return smsMmsAbility_;
}

std::shared_ptr<DataShareExtAbility> TelephonyDataShareStubImpl::GetOwner(const Uri &uri)
{
    OHOS::Uri uriTemp = uri;
    std::string path = uriTemp.GetPath();
    DATA_STORAGE_LOGI("GetOwner uri: %{public}s", path.c_str());
    if (path.find("com.ohos.telephonydataability") != std::string::npos) {
        return GetTelephonyDataAbility();
    }
    if (path.find("com.ohos.opkeyability") != std::string::npos) {
        return GetOpKeyAbility();
    }
    if (path.find("com.ohos.pdpprofileability") != std::string::npos) {
        return GetPdpProfileAbility();
    }
    if (path.find("com.ohos.simability") != std::string::npos) {
        return GetSimAbility();
    }
    if (path.find("com.ohos.smsmmsability") != std::string::npos) {
        return GetSmsMmsAbility();
    }
    return nullptr;
}

int TelephonyDataShareStubImpl::Insert(const Uri &uri, const DataShareValuesBucket &value)
{
    DATA_STORAGE_LOGI("Insert begin.");
    int ret = 0;
    auto client = sptr<TelephonyDataShareStubImpl>(this);
    auto extension = client->GetOwner(uri);
    if (extension == nullptr) {
        DATA_STORAGE_LOGE("Insert failed, extension is null.");
        return ret;
    }
    ret = extension->Insert(uri, value);
    DATA_STORAGE_LOGI("Insert end successfully. ret: %{public}d", ret);
    if (ret != Telephony::OPERATION_ERROR) {
        NotifyChange(uri);
    }
    return ret;
}

int TelephonyDataShareStubImpl::Update(const Uri &uri, const DataSharePredicates &predicates,
    const DataShareValuesBucket &value)
{
    DATA_STORAGE_LOGI("Update begin.");
    int ret = 0;
    auto client = sptr<TelephonyDataShareStubImpl>(this);
    auto extension = client->GetOwner(uri);
    if (extension == nullptr) {
        DATA_STORAGE_LOGE("Update failed, extension is null.");
        return ret;
    }
    ret = extension->Update(uri, predicates, value);
    DATA_STORAGE_LOGI("Update end successfully. ret: %{public}d", ret);
    if (ret != Telephony::OPERATION_ERROR) {
        NotifyChange(uri);
    }
    return ret;
}

int TelephonyDataShareStubImpl::Delete(const Uri &uri, const DataSharePredicates &predicates)
{
    DATA_STORAGE_LOGI("Delete begin.");
    int ret = 0;
    auto client = sptr<TelephonyDataShareStubImpl>(this);
    auto extension = client->GetOwner(uri);
    if (extension == nullptr) {
        DATA_STORAGE_LOGE("Delete failed, extension is null.");
        return ret;
    }
    ret = extension->Delete(uri, predicates);
    DATA_STORAGE_LOGI("Delete end successfully. ret: %{public}d", ret);
    if (ret != Telephony::OPERATION_ERROR) {
        NotifyChange(uri);
    }
    return ret;
}

std::shared_ptr<DataShareResultSet> TelephonyDataShareStubImpl::Query(const Uri &uri,
    const DataSharePredicates &predicates, std::vector<std::string> &columns, DatashareBusinessError &businessError)
{
    DATA_STORAGE_LOGI("Query begin.");
    std::shared_ptr<DataShareResultSet> resultSet = nullptr;
    auto client = sptr<TelephonyDataShareStubImpl>(this);
    auto extension = client->GetOwner(uri);
    if (extension == nullptr) {
        DATA_STORAGE_LOGE("Query failed, extension is null.");
        return nullptr;
    }
    resultSet = extension->Query(uri, predicates, columns);
    DATA_STORAGE_LOGI("Query end successfully.");
    return resultSet;
}

int TelephonyDataShareStubImpl::BatchInsert(const Uri &uri, const std::vector<DataShareValuesBucket> &values)
{
    DATA_STORAGE_LOGI("BatchInsert begin.");
    int ret = 0;
    auto client = sptr<TelephonyDataShareStubImpl>(this);
    auto extension = client->GetOwner(uri);
    if (extension == nullptr) {
        DATA_STORAGE_LOGE("BatchInsert failed, extension is null.");
        return ret;
    }
    ret = extension->BatchInsert(uri, values);
    DATA_STORAGE_LOGI("BatchInsert end successfully. ret: %{public}d", ret);
    if (ret != Telephony::OPERATION_ERROR) {
        NotifyChange(uri);
    }
    return ret;
}

std::vector<std::string> TelephonyDataShareStubImpl::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    DATA_STORAGE_LOGI("GetFileTypes not supported.");
    std::vector<std::string> result;
    return result;
}

int TelephonyDataShareStubImpl::OpenFile(const Uri &uri, const std::string &mode)
{
    DATA_STORAGE_LOGI("OpenFile not supported.");
    return -1;
}

int TelephonyDataShareStubImpl::OpenRawFile(const Uri &uri, const std::string &mode)
{
    DATA_STORAGE_LOGI("OpenRawFile not supported.");
    return -1;
}

std::string TelephonyDataShareStubImpl::GetType(const Uri &uri)
{
    DATA_STORAGE_LOGI("GetType not supported.");
    return "";
}

bool TelephonyDataShareStubImpl::RegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    DATA_STORAGE_LOGI("%{public}s begin.", __func__);
    auto obsMgrClient = DataObsMgrClient::GetInstance();
    if (obsMgrClient == nullptr) {
        DATA_STORAGE_LOGE("%{public}s obsMgrClient is nullptr", __func__);
        return false;
    }

    ErrCode ret = obsMgrClient->RegisterObserver(uri, dataObserver);
    if (ret != ERR_OK) {
        DATA_STORAGE_LOGE("%{public}s obsMgrClient->RegisterObserver error return %{public}d", __func__, ret);
        return false;
    }
    return true;
}

bool TelephonyDataShareStubImpl::UnregisterObserver(const Uri &uri,
    const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    DATA_STORAGE_LOGI("%{public}s begin.", __func__);
    auto obsMgrClient = DataObsMgrClient::GetInstance();
    if (obsMgrClient == nullptr) {
        DATA_STORAGE_LOGE("%{public}s obsMgrClient is nullptr", __func__);
        return false;
    }

    ErrCode ret = obsMgrClient->UnregisterObserver(uri, dataObserver);
    if (ret != ERR_OK) {
        DATA_STORAGE_LOGE("%{public}s obsMgrClient->UnregisterObserver error return %{public}d", __func__, ret);
        return false;
    }
    return true;
}

bool TelephonyDataShareStubImpl::NotifyChange(const Uri &uri)
{
    auto obsMgrClient = DataObsMgrClient::GetInstance();
    if (obsMgrClient == nullptr) {
        DATA_STORAGE_LOGE("%{public}s obsMgrClient is nullptr", __func__);
        return false;
    }

    ErrCode ret = obsMgrClient->NotifyChange(uri);
    if (ret != ERR_OK) {
        DATA_STORAGE_LOGE("%{public}s obsMgrClient->NotifyChange error return %{public}d", __func__, ret);
        return false;
    }
    return true;
}

Uri TelephonyDataShareStubImpl::NormalizeUri(const Uri &uri)
{
    DATA_STORAGE_LOGI("NormalizeUri not supported.");
    return uri;
}

Uri TelephonyDataShareStubImpl::DenormalizeUri(const Uri &uri)
{
    DATA_STORAGE_LOGI("DenormalizeUri not supported.");
    return uri;
}
} // namespace DataShare
} // namespace OHOS