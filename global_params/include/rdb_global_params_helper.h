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

#ifndef DATA_STORAGE_RDB_GLOBAL_PARAMS_HELPER_H
#define DATA_STORAGE_RDB_GLOBAL_PARAMS_HELPER_H

#include "iosfwd"
#include "rdb_base_helper.h"
#include "string"
#include "vector"

namespace OHOS {
namespace Telephony {
class RdbGlobalParamsHelper : public RdbBaseHelper {
public:
    RdbGlobalParamsHelper();
    ~RdbGlobalParamsHelper() = default;

    /**
     * Init dataBase
     *
     * @return 0 is succeed else failed
     */
    int Init();

    /**
     * Update dataBase path
     *
     * @param path path
     */
    void UpdateDbPath(const std::string &path);

private:
    void CreateGlobalParamsTableStr(std::string &createTableStr, const std::string &tableName);
    /**
     * Create number_match table
     *
     * @param createTableStr Create table statement
     */
    void CreateNumMatchTableStr(std::string &createTableStr);
    
    /**
     * Create number_match index
     *
     * @param createIndexStr Create index statement
     */
    void CreateNumMatchIndexStr(std::string &createIndexStr);
    
    /**
     * Create ecc_data table
     *
     * @param createTableStr Create table statement
     */
    void CreateEccDataTableStr(std::string &createTableStr);
    int CommitTransactionAction();

private:
    const std::string DB_NAME = "globalparams.db";
    std::string dbPath_ = FOLDER_PATH + DB_NAME;
    const int VERSION = 1;
};
} // namespace Telephony
} // namespace OHOS
#endif // DATA_STORAGE_RDB_GLOBAL_PARAMS_HELPER_H
