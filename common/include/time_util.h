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

#ifndef TELEPHONY_TIME_UTIL_H
#define TELEPHONY_TIME_UTIL_H

#include <ctime>
#include <cstring>

namespace OHOS {
namespace Telephony {
void FormatDate(std::string &date, const tm *ltm)
{
    if (ltm != nullptr) {
        int START_YEAR = 1900;
        int DOUBLE_DIGIT = 10;
        int year = START_YEAR + ltm->tm_year;
        int month = 1 + ltm->tm_mon;
        int day = ltm->tm_mday;
        int hour = ltm->tm_hour;
        int minute = ltm->tm_min;
        int second = ltm->tm_sec;
        date.append(std::to_string(year));
        date.append("-");
        if (month < DOUBLE_DIGIT) {
            date.append("0");
        }
        date.append(std::to_string(month));
        date.append("-");
        if (day < DOUBLE_DIGIT) {
            date.append("0");
        }
        date.append(std::to_string(day));
        date.append(" ");
        if (hour < DOUBLE_DIGIT) {
            date.append("0");
        }
        date.append(std::to_string(hour));
        date.append(":");
        if (minute < DOUBLE_DIGIT) {
            date.append("0");
        }
        date.append(std::to_string(minute));
        date.append(":");
        if (second < DOUBLE_DIGIT) {
            date.append("0");
        }
        date.append(std::to_string(second));
    }
}

void GetCurrentTime(std::string &date)
{
    time_t *DEFAULT_TIMER = 0;
    time_t now = time(DEFAULT_TIMER);
    if (now == -1) {
        return;
    }
    struct tm tm = { 0 };
    struct tm *ltm = localtime_r(&now, &tm);
    if (ltm == nullptr) {
        return;
    }
    FormatDate(date, ltm);
}

void GetTimeOfThirty(std::string &date)
{
    int64_t THIRTY_DAYS = 2592000;
    time_t *DEFAULT_TIMER = 0;
    time_t now = time(DEFAULT_TIMER) - THIRTY_DAYS;
    date = std::to_string(now);
}
} // namespace Telephony
} // namespace OHOS
#endif // TELEPHONY_TIME_UTIL_H
