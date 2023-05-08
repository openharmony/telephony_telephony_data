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

#ifndef DATA_STORAGE_SMS_MMS_ABILITY_H
#define DATA_STORAGE_SMS_MMS_ABILITY_H

#include "__functional_base"
#include "__mutex_base"
#include "ability.h"
#include "ability_lifecycle.h"
#include "iosfwd"
#include "map"
#include "memory"
#include "rdb_sms_mms_helper.h"
#include "string"
#include "telephony_permission.h"
#include "vector"

namespace OHOS {
namespace NativeRdb {
class AbsRdbPredicates;
class AbsSharedResultSet;
class DataAbilityPredicates;
class ValuesBucket;
}
namespace Telephony {
enum class MessageUriType {
    UNKNOW, SMS_MMS, THIRTY, MAX_GROUP, UNREAD_TOTAL, MMS_PROTOCOL, SMS_SUBSECTION, MMS_PART
};
class SmsMmsAbility : public AppExecFwk::Ability {
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
    virtual int BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values) override;

private:
    /**
     * Parse Uri Type
     *
     * @param uri Resource address
     * @return return MessageUriType
     */
    MessageUriType ParseUriType(Uri &uri);

    /**
     * Convert DataAbilityPredicates to AbsRdbPredicates
     *
     * @param predicates DataAbilityPredicates
     * @param absRdbPredicates AbsRdbPredicates
     */
    void ConvertPredicates(
        const NativeRdb::DataAbilityPredicates &predicates, NativeRdb::AbsRdbPredicates *absRdbPredicates);

    /**
    * Check whether the initialization succeeds
    *
    * @return true : succeed ,false : failed
    */
    bool IsInitOk();

    /**
    * Init UriMap
    */
    void InitUriMap();

private:
    RdbSmsMmsHelper helper_;
    std::mutex lock_;
    std::map<std::string, MessageUriType> smsMmsUriMap;
    bool initDatabaseDir = false;
    bool initRdbStore = false;
};
} // namespace Telephony
} // namespace OHOS
#endif // DATA_STORAGE_SMS_MMS_ABILITY_H
