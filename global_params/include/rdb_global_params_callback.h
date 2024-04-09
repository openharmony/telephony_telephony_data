/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef DATA_STORAGE_RDB_GLOBAL_PARAMS_CALLBACK_H
#define DATA_STORAGE_RDB_GLOBAL_PARAMS_CALLBACK_H

#include "iosfwd"
#include "rdb_base_callback.h"
#include "vector"

namespace OHOS {
namespace NativeRdb {
class RdbStore;
}
namespace Telephony {
class RdbGlobalParamsCallback : public RdbBaseCallBack {
public:
    explicit RdbGlobalParamsCallback(const std::vector<std::string> &createTableVec);
    ~RdbGlobalParamsCallback() = default;

    int OnUpgrade(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion) override;
    int OnDowngrade(NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion) override;
    int OnCreate(NativeRdb::RdbStore &rdbStore) override;
    int OnOpen(NativeRdb::RdbStore &rdbStore) override;
private:
    void InitData(NativeRdb::RdbStore &rdbStore, const std::string &tableName, const bool hashCheck);
    void InitNumMatchData(NativeRdb::RdbStore &rdbStore, const std::string &tableName, const bool hashCheck);
    void InitEccData(NativeRdb::RdbStore &rdbStore, const std::string &tableName, const bool hashCheck);
    int ClearData(NativeRdb::RdbStore &rdbStore, const std::string &tableName);
};
} // namespace Telephony
} // namespace OHOS
#endif // DATA_STORAGE_RDB_GLOBAL_PARAMS_CALLBACK_H
