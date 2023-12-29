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

#ifndef DATA_STORAGE_GTEST_H
#define DATA_STORAGE_GTEST_H

#include <iostream>
#include <securec.h>

#include "abs_shared_result_set.h"
#include "accesstoken_kit.h"
#include "datashare_helper.h"
#include "datashare_predicates.h"
#include "gtest/gtest.h"
#include "iservice_registry.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "token_setproc.h"
#include "values_bucket.h"

namespace OHOS {
namespace Telephony {
class DataStorageGtest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    using RequestFuncType = int (DataStorageGtest::*)
        (const std::shared_ptr<DataShare::DataShareHelper> &helper) const;
    std::map<char, RequestFuncType> requestFuncMap_;

    std::shared_ptr<DataShare::DataShareHelper> CreateDataShareHelper(
        int32_t systemAbilityId, std::string &uri) const;
    std::shared_ptr<DataShare::DataShareHelper> CreateSimHelper();
    std::shared_ptr<DataShare::DataShareHelper> CreateSmsHelper();
    std::shared_ptr<DataShare::DataShareHelper> CreatePdpProfileHelper();
    std::shared_ptr<DataShare::DataShareHelper> CreateOpKeyHelper();
    std::shared_ptr<DataShare::DataShareHelper> CreateGlobalParamsHelper();

    int SimInsert(const std::shared_ptr<DataShare::DataShareHelper> &helper) const;
    int SimUpdate(const std::shared_ptr<DataShare::DataShareHelper> &helper) const;
    int SimSelect(const std::shared_ptr<DataShare::DataShareHelper> &helper) const;
    int SimDelete(const std::shared_ptr<DataShare::DataShareHelper> &helper) const;
    int SmsInsert(const std::shared_ptr<DataShare::DataShareHelper> &helper) const;
    int SmsBatchInsert(const std::shared_ptr<DataShare::DataShareHelper> &helper) const;
    int SmsUpdate(const std::shared_ptr<DataShare::DataShareHelper> &helper) const;
    int SmsSelect(const std::shared_ptr<DataShare::DataShareHelper> &helper) const;
    int SmsDelete(const std::shared_ptr<DataShare::DataShareHelper> &helper) const;
    int PdpProfileInsert(const std::shared_ptr<DataShare::DataShareHelper> &helper) const;
    int PdpProfileUpdate(const std::shared_ptr<DataShare::DataShareHelper> &helper) const;
    int PdpProfileUpdate2(const std::shared_ptr<DataShare::DataShareHelper> &helper) const;
    int PdpProfileUpdate3(const std::shared_ptr<DataShare::DataShareHelper> &helper) const;
    int PdpProfileUpdate4(const std::shared_ptr<DataShare::DataShareHelper> &helper) const;
    int PdpProfileSelect(const std::shared_ptr<DataShare::DataShareHelper> &helper) const;
    int PdpProfileSelect2(const std::shared_ptr<DataShare::DataShareHelper> &helper) const;
    int PdpProfileDelete(const std::shared_ptr<DataShare::DataShareHelper> &helper) const;
    int OpKeyInsert(const std::shared_ptr<DataShare::DataShareHelper> &helper) const;
    int OpKeyUpdate(const std::shared_ptr<DataShare::DataShareHelper> &helper) const;
    int OpKeySelect(const std::shared_ptr<DataShare::DataShareHelper> &helper) const;
    int OpKeyDelete(const std::shared_ptr<DataShare::DataShareHelper> &helper) const;
    int GlobalParamsNumMatchInsert(const std::shared_ptr<DataShare::DataShareHelper> &helper) const;
    int GlobalParamsNumMatchUpdate(const std::shared_ptr<DataShare::DataShareHelper> &helper) const;
    int GlobalParamsNumMatchSelect(const std::shared_ptr<DataShare::DataShareHelper> &helper) const;
    int GlobalParamsNumMatchDelete(const std::shared_ptr<DataShare::DataShareHelper> &helper) const;
    int GlobalEccInsert(const std::shared_ptr<DataShare::DataShareHelper> &helper) const;
    int GlobalEccUpdate(const std::shared_ptr<DataShare::DataShareHelper> &helper) const;
    int GlobalEccSelect(const std::shared_ptr<DataShare::DataShareHelper> &helper) const;
    int GlobalEccDelete(const std::shared_ptr<DataShare::DataShareHelper> &helper) const;

private:
    std::shared_ptr<DataShare::DataShareHelper> simDataHelper = nullptr;
    std::shared_ptr<DataShare::DataShareHelper> smsDataHelper = nullptr;
    std::shared_ptr<DataShare::DataShareHelper> pdpProfileDataHelper = nullptr;
    std::shared_ptr<DataShare::DataShareHelper> opKeyDataHelper = nullptr;
    std::shared_ptr<DataShare::DataShareHelper> globalParamsDataHelper = nullptr;
};
} // namespace Telephony
} // namespace OHOS
#endif // STATE_REGISTER_GTEST_H