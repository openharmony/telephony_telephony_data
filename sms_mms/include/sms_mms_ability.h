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
#include "sms_mms_observer.h"
#include "hi_audit.h"
#include "../../common/include/rdb_sms_mms_dfx_adapter.h"

namespace OHOS {
namespace NativeRdb {
class AbsRdbPredicates;
class ResultSet;
class DataAbilityPredicates;
class ValuesBucket;
}
namespace Telephony {
class SmsMmsObserver;
enum class MessageUriType {
    UNKNOW,
    SMS_MMS,
    RCS,
    THIRTY,
    MAX_GROUP,
    UNREAD_TOTAL,
    MMS_PROTOCOL,
    SMS_SUBSECTION,
    MMS_PART,
    SESSION,
    MMS_PDU,
    RISK_URL_RECORD,
    MMS,
    SELECT_INFO_COUNT,
    SELECT_CONTACT_ID,
    CLEAN_INVALID_SESSION,
    DELETE_SESSION,
    QUERY_MMS_PATHS_WITH_SESSIONIDS,
	QUERY_MMS_INFO_VIEW_CUSTOMIZED,
    MMS_PDU_LT,
    CHATBOTS,
    SPECIFICCHATBOTS,
    SMS_MMS_FAVORITE,
    SMC,
    TEMPORARY_DB,
    CONVERSION_SESSION,
    CONVERSION_NO_NOTIFY_SESSION,
    MATCHED_RECIPIENT,
    BATCH_MARK_READ,
    SINGLE_MARK_READ,
    MAG_SEARCH_INDEX,
    INVALID_SESSION,
    INVALID_SMS_INFO,
    INVALID_RCS_INFO,
    INVALID_RCS_INFO_WITH_MSG,
    INVALID_PART_INFO,
};

enum class MarkReadType : int {
    SINGLE = 1,
    INFO = 2,
    ALL = 3
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
    bool GetBundleNameByUid(int32_t uid, std::string &bundleName);
    static std::string bundleName_;
    void UpdateScreenLockStatus(bool isLocked);
    void ChangeStoreToEl5AndMoveData();

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
    NativeRdb::AbsRdbPredicates *GetPredicatesEx(MessageUriType messageUriType);
    std::shared_ptr<DataShare::DataShareResultSet> GetResultSet(MessageUriType messageUriType);
	std::shared_ptr<DataShare::DataShareResultSet> GetQueryMmsInfoResultSet(
        const DataShare::DataSharePredicates &predicates);
    int InsertEx(MessageUriType type, const NativeRdb::ValuesBucket &values);
    NativeRdb::AbsRdbPredicates *CreateAbsRdbPredicatesEx(MessageUriType messageUriType);
    bool IsRetryable(int32_t errCode);
    void InsertRetry(int64_t *id, const NativeRdb::ValuesBucket &values, const char *tableName);
    OHOS::AuditLog MakeAuditLog(std::string operationType);
    bool IsScreenLocked();
    std::shared_ptr<DataShare::DataShareResultSet> GetQueryConversionListResult(
        const DataShare::DataSharePredicates &predicates, bool isQueryNotifiyMsg);
    std::shared_ptr<DataShare::DataShareResultSet> QueryMmsPathsWithSessionIdsResult(
        const DataShare::DataSharePredicates &predicates);
    std::shared_ptr<DataShare::DataShareResultSet> GetQueryMatchedRecipientResult(
        const DataShare::DataSharePredicates &predicates);
    int BatchProcessMarkRead(const DataShare::DataSharePredicates &predicates);
    int UpdateSessionUnreadCount(const DataShare::DataSharePredicates &predicates);
    std::shared_ptr<DataShare::DataShareResultSet> GetMsgCoordinates(
        const DataShare::DataSharePredicates &predicates);
    std::shared_ptr<DataShare::DataShareResultSet> GetQueryInvalidData(const MessageUriType &messageUriType);
    std::shared_ptr<DataShare::DataShareResultSet> HandleCommonQuery(
        MessageUriType messageUriType, const Uri &uri, const DataShare::DataSharePredicates &predicates,
        std::vector<std::string> &columns);
    std::unordered_map<MessageUriType, std::function<std::shared_ptr<DataShare::DataShareResultSet>(
        const DataShare::DataSharePredicates&)>> queryHandlers_;
    void InitializeQueryHandlers();
    int64_t GetSystemTime();
    void CheckAndReportTimeoutError(RDBOperation operation, const int64_t &startMs, const Uri &uri);
    int ProcessRdbUpdate(const Uri &uri, const DataShare::DataSharePredicates &predicates,
    const DataShare::DataShareValuesBucket &value, int64_t startMs);
private:
    RdbSmsMmsHelper helper_;
    std::mutex lock_;
    bool initDatabaseDir = false;
    bool initRdbStore = false;
    std::mutex initMtx_;
    std::string dbPath = "/data/storage/el1/database/";
    std::string dbPathE = "/data/storage/el2/database/";
    std::shared_ptr<SmsMmsObserver> smsMmsObserver_;
    RdbSmsMmsDFXAdapter dfxAdapter_;
};
} // namespace Telephony
} // namespace OHOS
#endif // DATA_STORAGE_SMS_MMS_ABILITY_H
