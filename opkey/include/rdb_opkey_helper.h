/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef DATA_STORAGE_RDB_OPKEY_HELPER_H
#define DATA_STORAGE_RDB_OPKEY_HELPER_H

#include "iosfwd"
#include "rdb_base_helper.h"
#include "string"

namespace OHOS {
namespace Telephony {
class RdbOpKeyHelper : public RdbBaseHelper {
public:
    RdbOpKeyHelper();
    ~RdbOpKeyHelper() = default;

    /**
     * Update dataBase path
     *
     * @param path path
     */
    void UpdateDbPath(const std::string &path);

    /**
     * Init dataBase
     *
     * @return 0 is succeed else failed
     */
    int Init();

private:
    /**
     * Create OpKeyInfo table
     *
     * @param createTableStr Create table statement
     */
    void CreateOpKeyInfoTableStr(std::string &createTableStr);

    /**
     * Create OpKeyInfo index
     *
     * @param createIndexStr Create index statement
     */
    void CreateOpKeyInfoIndexStr(std::string &createIndexStr);

    /**
     * End the transaction action
     *
     * @return 0 is succeed else failed
     */
    int EndTransactionAction();

    /**
     * Commit the transaction action
     *
     * @return 0 is succeed else failed
     */
    int CommitTransactionAction();

private:
    const std::string DB_NAME = "opkey.db";
    std::string dbPath_ = FOLDER_PATH + DB_NAME;
};
} // namespace Telephony
} // namespace OHOS
#endif // DATA_STORAGE_RDB_OPKEY_HELPER_H