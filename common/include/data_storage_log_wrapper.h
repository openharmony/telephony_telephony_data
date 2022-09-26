/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_DATA_STORAGE_LOG_WRAPPER_H
#define OHOS_DATA_STORAGE_LOG_WRAPPER_H

#include "hilog/log_c.h"
#include "hilog/log_cpp.h"
#include "iosfwd"

namespace OHOS {
namespace Telephony {
enum class DataStorageLogLevel {
    DEBUG = 0,
    INFO,
    WARN,
    ERROR,
    FATAL,
};

class DataStorageLogWrapper {
public:
    static bool JudgeLevel(const DataStorageLogLevel &level);

    static void SetLogLevel(const DataStorageLogLevel &level)
    {
        level_ = level;
    }

    static const DataStorageLogLevel &GetLogLevel()
    {
        return level_;
    }

    static std::string GetBriefFileName(const std::string &file);

private:
    static DataStorageLogLevel level_;
};

#define CONFIG_HILOG
#ifdef CONFIG_HILOG

#ifndef TELEPHONY_LOG_TAG
#define TELEPHONY_LOG_TAG "DataStorage"
#endif

#ifndef LOG_DOMAIN
#define LOG_DOMAIN 0xD001F08
#endif

static constexpr OHOS::HiviewDFX::HiLogLabel DATA_STORAGE_LABEL = {LOG_CORE, LOG_DOMAIN,
    TELEPHONY_LOG_TAG};

#define DATA_STORAGE_FILENAME (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

#define OHOS_DEBUG
#ifndef OHOS_DEBUG
#define PRINT_DATASTORAGE_LOG(op, fmt, ...) (void)OHOS::HiviewDFX::HiLog::op(DATA_STORAGE_LABEL, fmt, ##__VA_ARGS__)
#else
#define PRINT_DATASTORAGE_LOG(op, fmt, ...) \
    (void)OHOS::HiviewDFX::HiLog::op(DATA_STORAGE_LABEL, "[%{public}s-(%{public}s:%{public}d)] " fmt, __FUNCTION__, \
        DATA_STORAGE_FILENAME, __LINE__, ##__VA_ARGS__)
#endif

#define DATA_STORAGE_LOGE(fmt, ...) PRINT_DATASTORAGE_LOG(Error, fmt, ##__VA_ARGS__)
#define DATA_STORAGE_LOGW(fmt, ...) PRINT_DATASTORAGE_LOG(Warn, fmt, ##__VA_ARGS__)
#define DATA_STORAGE_LOGI(fmt, ...) PRINT_DATASTORAGE_LOG(Info, fmt, ##__VA_ARGS__)
#define DATA_STORAGE_LOGF(fmt, ...) PRINT_DATASTORAGE_LOG(Fatal, fmt, ##__VA_ARGS__)

#else
#endif // CONFIG_HILOG
} // namespace Telephony
} // namespace OHOS
#endif // OHOS_DATA_STORAGE_LOG_WRAPPER_H
