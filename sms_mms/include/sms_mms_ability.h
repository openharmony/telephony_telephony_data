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

#ifndef DATA_STORAGE_SMS_MMS_ABILITY_H
#define DATA_STORAGE_SMS_MMS_ABILITY_H

#include <functional>

#include "__mutex_base"
#include "ability.h"
#include "ability_lifecycle.h"
#include "abs_shared_result_set.h"
#include "datashare_ext_ability.h"
#include "datashare_ext_ability_context.h"
#include "datashare_predicates.h"
#include "datashare_values_bucket.h"
#include "iosfwd"
#include "map"
#include "memory"
#include "rdb_predicates.h"
#include "rdb_sms_mms_helper.h"
#include "string"
#include "vector"
#include "want.h"

namespace OHOS {
namespace NativeRdb {
class AbsRdbPredicates;
class ResultSet;
class DataAbilityPredicates;
class ValuesBucket;
}
namespace Telephony {
enum class MessageUriType {
    UNKNOW,
    SMS_MMS,
    THIRTY,
    MAX_GROUP,
    UNREAD_TOTAL,
    MMS_PROTOCOL,
    SMS_SUBSECTION,
    MMS_PART,
    SESSION,
    MMS_PDU,
};
class SmsMmsAbility : public DataShare::DataShareExtAbility {
public:
    SmsMmsAbility();
    virtual ~SmsMmsAbility() override;
    static SmsMmsAbility* Create();
    void DoInit();
    sptr<IRemoteObject> OnConnect(const AAFwk::Want &want) override;
    virtual void OnStart(const AppExecFwk::Want &want) override;
    virtual int Insert(const Uri &uri, const DataShare::DataShareValuesBucket &value) override;
    virtual std::shared_ptr<DataShare::DataShareResultSet> Query(const Uri &uri,
        const DataShare::DataSharePredicates &predicates, std::vector<std::string> &columns,
        DataShare::DatashareBusinessError &businessError) override;
    virtual int Update(const Uri &uri, const DataShare::DataSharePredicates &predicates,
        const DataShare::DataShareValuesBucket &value) override;
    virtual int Delete(const Uri &uri, const DataShare::DataSharePredicates &predicates) override;
    virtual std::string GetType(const Uri &uri) override;
    virtual int OpenFile(const Uri &uri, const std::string &mode) override;
    virtual int BatchInsert(const Uri &uri, const std::vector<DataShare::DataShareValuesBucket> &values) override;

private:
    /**
     * Parse Uri Type
     *
     * @param uri Resource address
     * @return return MessageUriType
     */
    MessageUriType ParseUriType(Uri &uri);

    /**
     * Convert DataSharePredicates to RdbPredicates
     *
     * @param tableName table name of the predicates
     * @param predicates DataSharePredicates
     */
    OHOS::NativeRdb::RdbPredicates ConvertPredicates(
        const std::string &tableName, const DataShare::DataSharePredicates &predicates);

    /**
    * Check whether the initialization succeeds
    *
    * @return true : succeed ,false : failed
    */
    bool IsInitOk();

    NativeRdb::AbsRdbPredicates *CreateAbsRdbPredicates(MessageUriType messageUriType, int &result, Uri uri);
    NativeRdb::AbsRdbPredicates *GetPredicates(MessageUriType messageUriType, const Uri &uri);
    std::shared_ptr<DataShare::DataShareResultSet> GetResultSet(MessageUriType messageUriType, const Uri &uri);

private:
    RdbSmsMmsHelper helper_;
    std::mutex lock_;
    bool initDatabaseDir = false;
    bool initRdbStore = false;
    std::mutex initMtx_;
};
} // namespace Telephony
} // namespace OHOS
#endif // DATA_STORAGE_SMS_MMS_ABILITY_H
