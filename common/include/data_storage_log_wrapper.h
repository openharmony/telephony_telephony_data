/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#define DATA_STORAGE_FILENAME (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

#define OHOS_DEBUG
#ifndef OHOS_DEBUG
#define DATA_STORAGE_LOGE(fmt, ...) \
    (void)HILOG_IMPL(LOG_CORE, LOG_ERROR, LOG_DOMAIN, TELEPHONY_LOG_TAG, fmt, ##__VA_ARGS__)
#define DATA_STORAGE_LOGW(fmt, ...) \
    (void)HILOG_IMPL(LOG_CORE, LOG_WARN, LOG_DOMAIN, TELEPHONY_LOG_TAG, fmt, ##__VA_ARGS__)
#define DATA_STORAGE_LOGI(fmt, ...) \
    (void)HILOG_IMPL(LOG_CORE, LOG_INFO, LOG_DOMAIN, TELEPHONY_LOG_TAG, fmt, ##__VA_ARGS__)
#define DATA_STORAGE_LOGF(fmt, ...) \
    (void)HILOG_IMPL(LOG_CORE, LOG_FATAL, LOG_DOMAIN, TELEPHONY_LOG_TAG, fmt, ##__VA_ARGS__)
#define DATA_STORAGE_LOGD(fmt, ...) \
    (void)HILOG_IMPL(LOG_CORE, LOG_DEBUG, LOG_DOMAIN, TELEPHONY_LOG_TAG, fmt, ##__VA_ARGS__)
#else
#define DATA_STORAGE_LOGE(fmt, ...)                                                                                   \
    (void)HILOG_IMPL(LOG_CORE, LOG_ERROR, LOG_DOMAIN, TELEPHONY_LOG_TAG, "[%{public}s-(%{public}s:%{public}d)] " fmt, \
    __FUNCTION__, DATA_STORAGE_FILENAME, __LINE__, ##__VA_ARGS__)
#define DATA_STORAGE_LOGW(fmt, ...)                                                                                  \
    (void)HILOG_IMPL(LOG_CORE, LOG_WARN, LOG_DOMAIN, TELEPHONY_LOG_TAG, "[%{public}s-(%{public}s:%{public}d)] " fmt, \
    __FUNCTION__, DATA_STORAGE_FILENAME, __LINE__, ##__VA_ARGS__)
#define DATA_STORAGE_LOGI(fmt, ...)                                                                                  \
    (void)HILOG_IMPL(LOG_CORE, LOG_INFO, LOG_DOMAIN, TELEPHONY_LOG_TAG, "[%{public}s-(%{public}s:%{public}d)] " fmt, \
    __FUNCTION__, DATA_STORAGE_FILENAME, __LINE__, ##__VA_ARGS__)
#define DATA_STORAGE_LOGF(fmt, ...)                                                                                   \
    (void)HILOG_IMPL(LOG_CORE, LOG_FATAL, LOG_DOMAIN, TELEPHONY_LOG_TAG, "[%{public}s-(%{public}s:%{public}d)] " fmt, \
    __FUNCTION__, DATA_STORAGE_FILENAME, __LINE__, ##__VA_ARGS__)
#define DATA_STORAGE_LOGD(fmt, ...)                                                                                   \
    (void)HILOG_IMPL(LOG_CORE, LOG_DEBUG, LOG_DOMAIN, TELEPHONY_LOG_TAG, "[%{public}s-(%{public}s:%{public}d)] " fmt, \
    __FUNCTION__, DATA_STORAGE_FILENAME, __LINE__, ##__VA_ARGS__)
#endif
#else
#endif // CONFIG_HILOG
} // namespace Telephony
} // namespace OHOS
#endif // OHOS_DATA_STORAGE_LOG_WRAPPER_H
