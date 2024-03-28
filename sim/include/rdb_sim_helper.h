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

#ifndef DATA_STORAGE_RDB_SIM_HELPER_H
#define DATA_STORAGE_RDB_SIM_HELPER_H

#include <stdint.h>

#include "iosfwd"
#include "rdb_base_helper.h"
#include "string"

namespace OHOS {
namespace Telephony {
class RdbSimHelper : public RdbBaseHelper {
public:
    enum class SimCardType {
        MAIN = 0, VOICE, MESSAGE, CELLULAR_DATA
    };

    RdbSimHelper();
    ~RdbSimHelper() = default;

    /**
     * Update dataBase path
     *
     * @param path path
     */
    void UpdateDbPath(const std::string &path);

    /**
     * Clear sim_info table data
     * @return 0 is succeed else failed
     */
    int32_t ClearData();

    /**
     * Init dataBase
     *
     * @return 0 is succeed else failed
     */
    int Init();

    /**
     * Set default card by card type
     *
     * @param simId  simId
     * @param type card type
     * @return 0 is succeed else failed
     */
    int32_t SetDefaultCardByType(int32_t simId, int32_t type);

private:
    /**
     * Create SimInfo table
     *
     * @param createTableStr Create table statement
     */
    void CreateSimInfoTableStr(std::string &createTableStr);

    /**
     * Update sim_info table card state by card type
     *
     * @param type card type
     * @param updateState Updated value that is 0 or 1
     * @param whereSate Previous value
     * @return 0 is succeed else failed
     */
    int32_t UpdateCardStateByType(int32_t type, int32_t updateState, int32_t whereSate);

    /**
     * Commit the transaction action
     *
     * @return 0 is succeed else failed
     */
    int CommitTransactionAction();

private:
    const std::string DB_NAME = "sim.db";
    std::string dbPath_ = FOLDER_PATH + DB_NAME;
    const int VERSION = 3;
};
} // namespace Telephony
} // namespace OHOS
#endif // DATA_STORAGE_RDB_SIM_HELPER_H