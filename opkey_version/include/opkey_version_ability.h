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

#ifndef OPKEY_VERSION_ABILITY
#define OPKEY_VERSION_ABILITY

#include "__mutex_base"
#include "ability.h"
#include "datashare_ext_ability.h"
#include "datashare_ext_ability_context.h"
#include "string"
#include "vector"
#include "uri.h"
#include "want.h"

namespace OHOS {
namespace Telephony {
class OpkeyVersionAbility : public DataShare::DataShareExtAbility {
public:
    OpkeyVersionAbility();
    virtual ~OpkeyVersionAbility() override;
    static OpkeyVersionAbility* Create();
    virtual std::shared_ptr<DataShare::DataShareResultSet> Query(const Uri &uri,
         const DataShare::DataSharePredicates &predicates, std::vector<std::string> &columns,
         DataShare::DatashareBusinessError &businessError) override;
};
} // namespace Telephony
} // namespace OHOS

#endif // OPKEY_VERSION_ABILITY