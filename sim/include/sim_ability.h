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

#ifndef DATA_STORAGE_SIM_ABILITY_H
#define DATA_STORAGE_SIM_ABILITY_H

#include "__functional_base"
#include "__mutex_base"
#include "ability.h"
#include "ability_lifecycle.h"
#include "iosfwd"
#include "map"
#include "memory"
#include "rdb_sim_helper.h"
#include "string"
#include "telephony_permission.h"
#include "vector"

namespace OHOS {
namespace NativeRdb {
class AbsRdbPredicates;
class AbsSharedResultSet;
class DataAbilityPredicates;
class ValuesBucket;
class ValueObject;
}
namespace Telephony {
enum class SimUriType {
    UNKNOW, SIM_INFO, SET_CARD
};
class SimAbility : public AppExecFwk::Ability {
public:
    virtual void OnStart(const AppExecFwk::Want &want) override;
    virtual int Insert(const Uri &uri, const NativeRdb::ValuesBucket &value) override;
    virtual std::shared_ptr<NativeRdb::AbsSharedResultSet> Query(
        const Uri &uri, const std::vector<std::string> &columns,
        const NativeRdb::DataAbilityPredicates &predicates) override;
    virtual int Update(const Uri &uri, const NativeRdb::ValuesBucket &value,
        const NativeRdb::DataAbilityPredicates &predicates) override;
    virtual int Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates) override;
    virtual std::string GetType(const Uri &uri) override;
    virtual int OpenFile(const Uri &uri, const std::string &mode) override;

private:
    /**
     * Parse Uri Type
     *
     * @param uri Resource address
     * @return return SimUriType
     */
    SimUriType ParseUriType(Uri &uri);

    /**
     * Convert DataAbilityPredicates to AbsRdbPredicates
     *
     * @param predicates DataAbilityPredicates
     * @param absRdbPredicates AbsRdbPredicates
     */
    void ConvertPredicates(
        const NativeRdb::DataAbilityPredicates &predicates, NativeRdb::AbsRdbPredicates *absRdbPredicates);

    /**
    * Set card type
    *
    * @param value Table field data
    * @return Is it successful
    */

    int SetCard(const NativeRdb::ValuesBucket &value);

    /**
    * Check whether the initialization succeeds
    *
    * @return true : succeed ,false : failed
    */
    bool IsInitOk();

    /**
     * Init SimUriMap
     */
    void InitUriMap();

private:
    RdbSimHelper helper_;
    std::mutex lock_;
    std::map<std::string, SimUriType> simUriMap;
    bool initDatabaseDir = false;
    bool initRdbStore = false;
};
} // namespace Telephony
} // namespace OHOS
#endif // DATA_STORAGE_SIM_ABILITY_H
