/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef PERMISSION_UTIL_H
#define PERMISSION_UTIL_H

#include <string>

namespace OHOS {
namespace Telephony {
namespace Permission {
/**
 * This permission indicates being allowed to set state of telephony.
 */
static constexpr const char *SET_TELEPHONY_STATE = "ohos.permission.SET_TELEPHONY_STATE";
/**
 * This permission indicates being allowed to get state of telephony.
 */
static constexpr const char *GET_TELEPHONY_STATE = "ohos.permission.GET_TELEPHONY_STATE";
/**
 * This permission indicates being allowed to read messages.
 */
static constexpr const char *READ_MESSAGES = "ohos.permission.READ_MESSAGES";
} // namespace Permission

class PermissionUtil {
public:
    /**
     * @brief Permission check by callingUid.
     *
     * @param permissionName permission name.
     * @return Return {@code true} on success, {@code false} on failure.
     */
    static bool CheckPermission(const std::string &permissionName);
};
} // namespace Telephony
} // namespace OHOS
#endif // PERMISSION_UTIL_H
