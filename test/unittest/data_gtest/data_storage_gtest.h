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

#ifndef DATA_STORAGE_GTEST_H
#define DATA_STORAGE_GTEST_H

#include <iostream>
#include <securec.h>

#include "abs_shared_result_set.h"
#include "accesstoken_kit.h"
#include "data_ability_helper.h"
#include "data_ability_predicates.h"
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
        (const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const;
    std::map<char, RequestFuncType> requestFuncMap_;

    std::shared_ptr<AppExecFwk::DataAbilityHelper> CreateDataAHelper(
        int32_t systemAbilityId, std::shared_ptr<Uri> dataAbilityUri) const;
    std::shared_ptr<AppExecFwk::DataAbilityHelper> CreateSimHelper();
    std::shared_ptr<AppExecFwk::DataAbilityHelper> CreateSmsHelper();
    std::shared_ptr<AppExecFwk::DataAbilityHelper> CreatePdpProfileHelper();
    std::shared_ptr<AppExecFwk::DataAbilityHelper> CreateOpKeyHelper();

    int SimInsert(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const;
    int SimUpdate(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const;
    int SimSelect(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const;
    int SimDelete(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const;
    int SmsInsert(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const;
    int SmsBatchInsert(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const;
    int SmsUpdate(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const;
    int SmsSelect(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const;
    int SmsDelete(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const;
    int PdpProfileInsert(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const;
    int PdpProfileUpdate(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const;
    int PdpProfileSelect(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const;
    int PdpProfileDelete(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const;
    int OpKeyInsert(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const;
    int OpKeyUpdate(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const;
    int OpKeySelect(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const;
    int OpKeyDelete(const std::shared_ptr<AppExecFwk::DataAbilityHelper> &helper) const;

private:
    std::shared_ptr<AppExecFwk::DataAbilityHelper> simDataAbilityHelper = nullptr;
    std::shared_ptr<AppExecFwk::DataAbilityHelper> smsDataAbilityHelper = nullptr;
    std::shared_ptr<AppExecFwk::DataAbilityHelper> pdpProfileDataAbilityHelper = nullptr;
    std::shared_ptr<AppExecFwk::DataAbilityHelper> opKeyDataAbilityHelper = nullptr;
};
} // namespace Telephony
} // namespace OHOS
#endif // STATE_REGISTER_GTEST_H