/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "opkey_version_ability.h"

#include "core_service_client.h"
#include "data_storage_errors.h"
#include "data_storage_log_wrapper.h"
#include "new"
#include "opkey_version_result_set_bridge.h"
#include "telephony_datashare_stub_impl.h"
#include "uri.h"
#include "utility"

namespace OHOS {
namespace Telephony {
OpkeyVersionAbility::OpkeyVersionAbility() : DataShareExtAbility()
{
    ParserUtil util;
    util.GetJsonItemStringVaule(PARAM_CONFIG_PATH, "CARRIER_CONFIG", "cfgInfoUri", custParam_);
    util.GetJsonItemStringVaule(PARAM_CONFIG_PATH, "CHIP_CONFIG", "cfgInfoUri", chipParam_);
}

OpkeyVersionAbility::~OpkeyVersionAbility() {}

OpkeyVersionAbility* OpkeyVersionAbility::Create()
{
    auto self = new OpkeyVersionAbility();
    return self;
}

std::shared_ptr<DataShare::DataShareResultSet> OpkeyVersionAbility::Query(const Uri &uri,
    const DataShare::DataSharePredicates &predicates, std::vector<std::string> &columns,
    DataShare::DatashareBusinessError &businessError)
{
    DATA_STORAGE_LOGD("start uri=%{public}s", uri.ToString().c_str());
    std::string versionInfo;
    int32_t ret = DATA_STORAGE_ERROR;
    if (uri.ToString() == custParam_) {
        ret = DelayedRefSingleton<CoreServiceClient>::GetInstance().GetOpkeyVersion(versionInfo);
    } else if (uri.ToString() == chipParam_) {
        ret = DelayedRefSingleton<CoreServiceClient>::GetInstance().GetOpnameVersion(versionInfo);
    } else {
        DATA_STORAGE_LOGE("uri=%{public}s not match", uri.ToString().c_str());
        return nullptr;
    }
    if (ret != DATA_STORAGE_SUCCESS) {
        DATA_STORAGE_LOGE("uri=%{public}s get versionInfo failed!", uri.ToString().c_str());
        return nullptr;
    }
    std::shared_ptr<DataShare::ResultSetBridge> resultSet =
             std::make_shared<OpkeyVersionResultSetBridge>(versionInfo);
    std::shared_ptr<DataShare::DataShareResultSet> sharedPtrResult =
        std::make_shared<DataShare::DataShareResultSet>(resultSet);
    return sharedPtrResult;
}
} // Telephony
} // OHOS