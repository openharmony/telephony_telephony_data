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

#include "permission_util.h"

#include "accesstoken_kit.h"
#include "data_storage_log_wrapper.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace Telephony {
using namespace Security::AccessToken;

/**
 * @brief Permission check by callingUid.
 * @param permissionName permission name.
 * @return Returns true on success, false on failure.
 */
bool PermissionUtil::CheckPermission(const std::string &permissionName)
{
    if (permissionName.empty()) {
        DATA_STORAGE_LOGD("permission check failed, permission name is empty.");
        return false;
    }

    auto callerToken = IPCSkeleton::GetCallingTokenID();
    int result = AccessTokenKit::VerifyAccessToken(callerToken, permissionName);
    if (result != PermissionState::PERMISSION_GRANTED) {
        DATA_STORAGE_LOGD("permission check failed, permission:%{public}s, callerToken:%{public}u",
            permissionName.c_str(), callerToken);
        return false;
    }
    return true;
}
} // namespace Telephony
} // namespace OHOS
