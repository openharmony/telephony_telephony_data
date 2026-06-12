/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <vector>
#include <string>
#include "sms_mms_tables_gtest.h"
#include "sms_mms_tables.h"
#include "rdb_sms_mms_callback.h"
#include "rdb_store_config.h"
#include "rdb_errno.h"
#include "rdb_helper.h"
#include "rdb_store.h"
#include "sms_mms_data.h"
#include "data_storage_log_wrapper.h"

namespace OHOS {
namespace Telephony {

using namespace testing::ext;
const std::string SMS_DB_DIR = "/data/app/el1/100/database/com.ohos.telephonydataability/";

void RdbSmsMmsTablesGtest::SetUpTestCase(void)
{
}

void RdbSmsMmsTablesGtest::TearDownTestCase(void)
{
}

void RdbSmsMmsTablesGtest::SetUp(void)
{
}

void RdbSmsMmsTablesGtest::TearDown(void)
{
    NativeRdb::RdbHelper::ClearCache();
    NativeRdb::RdbHelper::DeleteRdbStore(SMS_DB_DIR + "sms_mms_test.db");
}

std::shared_ptr<NativeRdb::RdbStore> RdbSmsMmsTablesGtest::Subject(int version) {
    RdbSmsMmsTables tables;
    std::vector<std::string> createTableVec = tables.InitCreateTableVec();
    RdbSmsMmsCallback callback(createTableVec);

    NativeRdb::RdbStoreConfig config(SMS_DB_DIR + "sms_mms_test.db");
    config.SetBundleName("com.ohos.telephonydataability");
    config.SetArea(0);
    config.SetSearchable(true);
    config.SetSecurityLevel(OHOS::NativeRdb::SecurityLevel::S1);
    int errCode = -1;
    int versionIn = 33;
    auto store = NativeRdb::RdbHelper::GetRdbStore(config, versionIn, callback, errCode);
    return store;
}

bool RdbSmsMmsTablesGtest::IsTableExist(NativeRdb::RdbStore &rdb, const std::string& tableName)
{
    std::shared_ptr<NativeRdb::ResultSet> resultSet =
        rdb.QuerySql("SELECT name FROM sqlite_master WHERE type='table' AND name='" + tableName + "';");
    if (resultSet == nullptr) {
        DATA_STORAGE_LOGE("Query table exist is null!");
        return false;
    }
    int count;
    resultSet->GetRowCount(count);
    resultSet->Close();
    if (count <= 0) {
        DATA_STORAGE_LOGE("talbe not exist %{public}s.", tableName.c_str());
        return false;
    }
    return true;
}

bool RdbSmsMmsTablesGtest::IsTriggerExist(NativeRdb::RdbStore &rdb, const std::string& triggerName)
{
    std::shared_ptr<NativeRdb::ResultSet> resultSet =
        rdb.QuerySql("SELECT name FROM sqlite_master WHERE type='trigger' AND name='" + triggerName + "';");
    if (resultSet == nullptr) {
        DATA_STORAGE_LOGE("Query table exist is null!");
        return false;
    }
    int count;
    resultSet->GetRowCount(count);
    resultSet->Close();
    if (count <= 0) {
        DATA_STORAGE_LOGE("trigger not exist %{public}s.", triggerName.c_str());
        return false;
    }
    return true;
}

/**
 * @tc.number   InitCreateTableVec_001
 * @tc.name     create rdb table, view and trigger
 * @tc.desc     Function test
 */
HWTEST_F(RdbSmsMmsTablesGtest, InitCreateTableVec_001, TestSize.Level0)
{
    // Given
    int versionIn = 33;
    auto store = Subject(versionIn);

    // When
    int versionOut = -1;
    int ret = store->GetVersion(versionOut);

    // Then
    EXPECT_EQ(NativeRdb::E_OK, ret);
    EXPECT_EQ(versionIn, versionOut);
}

/**
 * @tc.number   InitCreateTableVec_MessageInfoTable_001
 * @tc.name     create rdb table, view and trigger
 * @tc.desc     Function test
 */
HWTEST_F(RdbSmsMmsTablesGtest, InitCreateTableVec_SmsMmsInfoInfoTable_001, TestSize.Level0)
{
    // Given
    int versionIn = 33;
    auto store = Subject(versionIn);

    // When & Then
    EXPECT_TRUE(IsTableExist(*store, TABLE_SMS_MMS_INFO));
}

/**
 * @tc.number   InitCreateTableVec_MmsProtocolTable_001
 * @tc.name     create rdb table, view and trigger
 * @tc.desc     Function test
 */
HWTEST_F(RdbSmsMmsTablesGtest, InitCreateTableVec_MmsProtocolTable_001, TestSize.Level0)
{
    // Given
    int versionIn = 33;
    auto store = Subject(versionIn);

    // When & Then
    EXPECT_TRUE(IsTableExist(*store, TABLE_MMS_PROTOCOL));
}

/**
 * @tc.number   InitCreateTableVec_RcsInfoTable_001
 * @tc.name     create rdb table, view and trigger
 * @tc.desc     Function test
 */
HWTEST_F(RdbSmsMmsTablesGtest, InitCreateTableVec_RcsInfoTable_001, TestSize.Level0)
{
    // Given
    int versionIn = 33;
    auto store = Subject(versionIn);

    // When & Then
    EXPECT_TRUE(IsTableExist(*store, TABLE_RCS_INFO));
}

/**
 * @tc.number   InitCreateTableVec_InsertSmsUnreadCount_001 DTS2507160012551
 * @tc.name     create rdb table, view and trigger
 * @tc.desc     Function test
 */
HWTEST_F(RdbSmsMmsTablesGtest, InitCreateTableVec_InsertSmsUnreadCount_001, TestSize.Level0)
{
    // Given
    int versionIn = 33;
    auto store = Subject(versionIn);

    // When & Then
    EXPECT_TRUE(IsTriggerExist(*store, "insertSms_unread_count"));
}
}
}