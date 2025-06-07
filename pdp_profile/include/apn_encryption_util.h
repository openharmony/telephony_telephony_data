/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#ifndef DATA_STORAGE_APN_ENCRYPTION_UTIL_H
#define DATA_STORAGE_APN_ENCRYPTION_UTIL_H

#include <string>
#include <vector>
#include "hks_api.h"
#include "hks_type.h"
#include "hks_param.h"

namespace OHOS {
namespace Telephony {
static const uint32_t AES_COMMON_SIZE = 1024;
static const std::string APN_PWD_KEY_ALIAS = "ApnPwdSecret";

std::vector<uint8_t> EncryptData(std::string encryptData);

std::string DecryptData(std::string decryptData);

std::string DecryptVecData(std::vector<uint8_t> decryptData);
} // namespace Telephony
} // namespace OHOS

#endif // DATA_STORAGE_APN_ENCRYPTION_UTIL_H