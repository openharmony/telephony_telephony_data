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

#include "opkey_ability.h"

#include "ability_context.h"
#include "ability_loader.h"
#include "abs_rdb_predicates.h"
#include "abs_shared_result_set.h"
#include "data_storage_errors.h"
#include "data_storage_log_wrapper.h"
#include "datashare_ext_ability.h"
#include "datashare_predicates.h"
#include "new"
#include "opkey_data.h"
#include "permission_util.h"
#include "rdb_errno.h"
#include "rdb_utils.h"
#include "telephony_datashare_stub_impl.h"
#include "uri.h"
#include "utility"

namespace OHOS {
using AppExecFwk::AbilityLoader;
using AppExecFwk::Ability;
namespace Telephony {
const int32_t CHANGED_ROWS = 0;
static const std::map<std::string, OpKeyUriType> opKeyUriMap_ = {
    { "/opkey/opkey_info", OpKeyUriType::OPKEY_INFO },
    {"/opkey/opkey_init", OpKeyUriType::OPKEY_INIT},
};

OpKeyAbility::OpKeyAbility() : DataShareExtAbility() {}

OpKeyAbility::~OpKeyAbility() {}

OpKeyAbility* OpKeyAbility::Create()
{
    DATA_STORAGE_LOGI("OpKeyAbility::Create begin.");
    auto self =  new OpKeyAbility();
    self->DoInit();
    return self;
}

void OpKeyAbility::DoInit()
{
    if (initDatabaseDir_ && initRdbStore_) {
        DATA_STORAGE_LOGI("DoInit has done");
        return;
    }
    auto abilityContext = AbilityRuntime::Context::GetApplicationContext();
    if (abilityContext == nullptr) {
        DATA_STORAGE_LOGE("DoInit GetAbilityContext is null");
        return;
    }
    // switch database dir to el1 for init before unlock
    abilityContext->SwitchArea(0);
    std::string path = abilityContext->GetDatabaseDir();
    DATA_STORAGE_LOGI("GetDatabaseDir: %{public}s", path.c_str());
    if (!path.empty()) {
        initDatabaseDir_ = true;
        path.append("/");
        helper_.UpdateDbPath(path);
        int rdbInitCode = helper_.Init();
        if (rdbInitCode == NativeRdb::E_OK) {
            initRdbStore_ = true;
        } else {
            DATA_STORAGE_LOGE("DoInit rdb init failed!");
            initRdbStore_ = false;
        }
    } else {
        DATA_STORAGE_LOGE("DoInit##databaseDir is empty!");
        initDatabaseDir_ = false;
    }
}

sptr<IRemoteObject> OpKeyAbility::OnConnect(const AAFwk::Want &want)
{
    DATA_STORAGE_LOGI("OpKeyAbility::OnConnect begin.");
    Extension::OnConnect(want);
    sptr<DataShare::TelephonyDataShareStubImpl> remoteObject =
        new (std::nothrow) DataShare::TelephonyDataShareStubImpl();
    if (remoteObject == nullptr) {
        DATA_STORAGE_LOGE("%{public}s No memory allocated for DataShareStubImpl", __func__);
        return nullptr;
    }
    remoteObject->SetOpKeyAbility(std::static_pointer_cast<OpKeyAbility>(shared_from_this()));
    DATA_STORAGE_LOGI("OpKeyAbility %{public}s end.", __func__);
    return remoteObject->AsObject();
}

void OpKeyAbility::OnStart(const AppExecFwk::Want &want)
{
    DATA_STORAGE_LOGI("OpKeyAbility::OnStart");
    Extension::OnStart(want);
    DoInit();
}

int OpKeyAbility::BatchInsert(const Uri &uri, const std::vector<DataShare::DataShareValuesBucket> &values)
{
    if (!PermissionUtil::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        DATA_STORAGE_LOGE("Permission denied!");
        return DATA_STORAGE_ERR_PERMISSION_ERR;
    }
    if (!IsInitOk()) {
        return DATA_STORAGE_ERROR;
    }
    std::lock_guard<std::mutex> guard(lock_);
    Uri tempUri = uri;
    OpKeyUriType opKeyUriType = ParseUriType(tempUri);
    if (opKeyUriType == OpKeyUriType::OPKEY_INIT) {
        return helper_.InitOpKeyDatabase();
    }
    return DATA_STORAGE_ERROR;
}

int OpKeyAbility::Insert(const Uri &uri, const DataShare::DataShareValuesBucket &value)
{
    if (!PermissionUtil::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        DATA_STORAGE_LOGE("Permission denied!");
        return DATA_STORAGE_ERR_PERMISSION_ERR;
    }
    if (!IsInitOk()) {
        return DATA_STORAGE_ERROR;
    }
    std::lock_guard<std::mutex> guard(lock_);
    Uri tempUri = uri;
    OpKeyUriType opKeyUriType = ParseUriType(tempUri);
    int64_t id = DATA_STORAGE_ERROR;
    if (opKeyUriType == OpKeyUriType::OPKEY_INFO) {
        OHOS::NativeRdb::ValuesBucket values = RdbDataShareAdapter::RdbUtils::ToValuesBucket(value);
        helper_.Insert(id, values, TABLE_OPKEY_INFO);
    } else {
        DATA_STORAGE_LOGE("OpKeyAbility::Insert failed##uri = %{public}s", uri.ToString().c_str());
    }
    return id;
}

std::shared_ptr<DataShare::DataShareResultSet> OpKeyAbility::Query(const Uri &uri,
    const DataShare::DataSharePredicates &predicates, std::vector<std::string> &columns,
    DataShare::DatashareBusinessError &businessError)
{
    if (!PermissionUtil::CheckPermission(Permission::GET_TELEPHONY_STATE)) {
        DATA_STORAGE_LOGE("Permission denied!");
        return nullptr;
    }
    std::shared_ptr<DataShare::DataShareResultSet> sharedPtrResult = nullptr;
    if (!IsInitOk()) {
        return nullptr;
    }
    Uri tempUri = uri;
    OpKeyUriType opKeyUriType = ParseUriType(tempUri);
    if (opKeyUriType == OpKeyUriType::OPKEY_INFO) {
        NativeRdb::AbsRdbPredicates *absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_OPKEY_INFO);
        if (absRdbPredicates != nullptr) {
            NativeRdb::RdbPredicates rdbPredicates = ConvertPredicates(absRdbPredicates->GetTableName(), predicates);
            auto resultSet = helper_.Query(rdbPredicates, columns);
            if (resultSet == nullptr) {
                DATA_STORAGE_LOGE("OpKeyAbility::Query  NativeRdb::ResultSet is null!");
                delete absRdbPredicates;
                absRdbPredicates = nullptr;
                return nullptr;
            }
            auto queryResultSet = RdbDataShareAdapter::RdbUtils::ToResultSetBridge(resultSet);
            sharedPtrResult = std::make_shared<DataShare::DataShareResultSet>(queryResultSet);
            delete absRdbPredicates;
            absRdbPredicates = nullptr;
        } else {
            DATA_STORAGE_LOGE("OpKeyAbility::Query  NativeRdb::AbsRdbPredicates is null!");
        }
    } else {
        DATA_STORAGE_LOGE("OpKeyAbility::Query failed##uri = %{public}s", uri.ToString().c_str());
    }
    return sharedPtrResult;
}

int OpKeyAbility::Update(
    const Uri &uri, const DataShare::DataSharePredicates &predicates,
    const DataShare::DataShareValuesBucket &value)
{
    if (!PermissionUtil::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        DATA_STORAGE_LOGE("Permission denied!");
        return DATA_STORAGE_ERR_PERMISSION_ERR;
    }
    int result = DATA_STORAGE_ERROR;
    if (!IsInitOk()) {
        return result;
    }
    std::lock_guard<std::mutex> guard(lock_);
    Uri tempUri = uri;
    OpKeyUriType opKeyUriType = ParseUriType(tempUri);
    NativeRdb::AbsRdbPredicates *absRdbPredicates = nullptr;
    switch (opKeyUriType) {
        case OpKeyUriType::OPKEY_INFO: {
            absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_OPKEY_INFO);
            break;
        }
        default:
            DATA_STORAGE_LOGE("OpKeyAbility::Update failed##uri = %{public}s", uri.ToString().c_str());
            break;
    }
    if (absRdbPredicates != nullptr) {
        int changedRows = CHANGED_ROWS;
        NativeRdb::RdbPredicates rdbPredicates = ConvertPredicates(absRdbPredicates->GetTableName(), predicates);
        OHOS::NativeRdb::ValuesBucket values = RdbDataShareAdapter::RdbUtils::ToValuesBucket(value);
        result = helper_.Update(changedRows, values, rdbPredicates);
        delete absRdbPredicates;
        absRdbPredicates = nullptr;
    } else if (result == DATA_STORAGE_ERROR) {
        DATA_STORAGE_LOGE("OpKeyAbility::Update  NativeRdb::AbsRdbPredicates is null!");
    }
    return result;
}

int OpKeyAbility::Delete(const Uri &uri, const DataShare::DataSharePredicates &predicates)
{
    if (!PermissionUtil::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        DATA_STORAGE_LOGE("Permission denied!");
        return DATA_STORAGE_ERR_PERMISSION_ERR;
    }
    int result = DATA_STORAGE_ERROR;
    if (!IsInitOk()) {
        return result;
    }
    std::lock_guard<std::mutex> guard(lock_);
    Uri tempUri = uri;
    OpKeyUriType opKeyUriType = ParseUriType(tempUri);
    if (opKeyUriType == OpKeyUriType::OPKEY_INFO) {
        NativeRdb::AbsRdbPredicates *absRdbPredicates = new NativeRdb::AbsRdbPredicates(TABLE_OPKEY_INFO);
        if (absRdbPredicates != nullptr) {
            NativeRdb::RdbPredicates rdbPredicates = ConvertPredicates(absRdbPredicates->GetTableName(), predicates);
            int deletedRows = CHANGED_ROWS;
            result = helper_.Delete(deletedRows, rdbPredicates);
            delete absRdbPredicates;
            absRdbPredicates = nullptr;
        } else {
            DATA_STORAGE_LOGE("OpKeyAbility::Delete  NativeRdb::AbsRdbPredicates is null!");
        }
    } else {
        DATA_STORAGE_LOGE("OpKeyAbility::Delete failed##uri = %{public}s", uri.ToString().c_str());
    }
    return result;
}

bool OpKeyAbility::IsInitOk()
{
    if (!initDatabaseDir_) {
        DATA_STORAGE_LOGE("OpKeyAbility::IsInitOk initDatabaseDir_ failed!");
        return false;
    }
    if (!initRdbStore_) {
        DATA_STORAGE_LOGE("OpKeyAbility::IsInitOk initRdbStore_ failed!");
        return false;
    }
    return true;
}

std::string OpKeyAbility::GetType(const Uri &uri)
{
    DATA_STORAGE_LOGI("OpKeyAbility::GetType##uri = %{public}s", uri.ToString().c_str());
    std::string retval(uri.ToString());
    return retval;
}

int OpKeyAbility::OpenFile(const Uri &uri, const std::string &mode)
{
    DATA_STORAGE_LOGI("OpKeyAbility::OpenFile##uri = %{public}s", uri.ToString().c_str());
    Uri tempUri = uri;
    OpKeyUriType opKeyUriType = ParseUriType(tempUri);
    return static_cast<int>(opKeyUriType);
}

OpKeyUriType OpKeyAbility::ParseUriType(Uri &uri)
{
    DATA_STORAGE_LOGI("OpKeyAbility::ParseUriType start");
    OpKeyUriType opKeyUriType = OpKeyUriType::UNKNOW;
    std::string uriPath = uri.ToString();
    if (!uriPath.empty()) {
        helper_.ReplaceAllStr(uriPath, ":///", "://");
        Uri tempUri(uriPath);
        std::string path = tempUri.GetPath();
        if (!path.empty() && !opKeyUriMap_.empty()) {
            DATA_STORAGE_LOGI("OpKeyAbility::ParseUriType##path = %{public}s", path.c_str());
            auto it = opKeyUriMap_.find(path);
            if (it != opKeyUriMap_.end()) {
                opKeyUriType = it->second;
                DATA_STORAGE_LOGI("OpKeyAbility::ParseUriType##opKeyUriType = %{public}d",
                    opKeyUriType);
            }
        }
    }
    return opKeyUriType;
}

OHOS::NativeRdb::RdbPredicates OpKeyAbility::ConvertPredicates(
    const std::string &tableName, const DataShare::DataSharePredicates &predicates)
{
    OHOS::NativeRdb::RdbPredicates res = RdbDataShareAdapter::RdbUtils::ToPredicates(predicates, tableName);
    return res;
}
} // namespace Telephony
} // namespace OHOS
