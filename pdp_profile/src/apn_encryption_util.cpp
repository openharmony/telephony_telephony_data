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
#include "apn_encryption_util.h"

#include <iterator>
#include <securec.h>
#include <sstream>

#include "data_storage_log_wrapper.h"
#include "data_storage_errors.h"

namespace OHOS {
namespace Telephony {

constexpr uint32_t IV_SIZE = 16;
const struct HksParam g_genEncDecParams[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_AES
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_AES_KEY_SIZE_256
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_PKCS7
    }, {
        .tag = HKS_TAG_BLOCK_MODE,
        .uint32Param = HKS_MODE_CBC
    }, {
        .tag = HKS_TAG_AUTH_STORAGE_LEVEL,
        .uint32Param = HKS_AUTH_STORAGE_LEVEL_DE
    }
};

int InitParamSet(struct HksParamSet **paramSet, const struct HksParam *params, uint32_t paramCount)
{
    int ret = HksInitParamSet(paramSet);
    if (ret != HKS_SUCCESS) {
        DATA_STORAGE_LOGE("HksInitParamSet failed, error code: %{public}d", ret);
        return ret;
    }
    ret = HksAddParams(*paramSet, params, paramCount);
    if (ret != HKS_SUCCESS) {
        DATA_STORAGE_LOGE("HksAddParams failed, error code: %{public}d", ret);
        return ret;
    }
    return HksBuildParamSet(paramSet);
}

int AesCbcPkcs7EncryptInner(struct HksBlob *keyAlias, struct HksBlob *inData, struct HksBlob *outData)
{
    uint8_t iv[IV_SIZE] = { 0 };
    struct HksBlob ivBlob = { IV_SIZE, iv };
    int ret = HksGenerateRandom(nullptr, &ivBlob);
    if (ret != HKS_SUCCESS) {
        DATA_STORAGE_LOGE("HksGenerateRandom failed, error code: %{public}d", ret);
        return ret;
    }
    struct HksParam encryptParams[] = {
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT },
        { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256 },
        { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_PKCS7 },
        { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
        { .tag = HKS_TAG_IV, .blob = { .size = IV_SIZE, .data = iv }},
        { .tag = HKS_TAG_AUTH_STORAGE_LEVEL, .uint32Param = HKS_AUTH_STORAGE_LEVEL_DE }};
    struct HksParamSet *encryptParamSet = nullptr;
    ret = InitParamSet(&encryptParamSet, encryptParams, sizeof(encryptParams) / sizeof(HksParam));
    if (ret != HKS_SUCCESS) {
        DATA_STORAGE_LOGE("init encrypt param set failed, error code: %{public}d", ret);
        HksFreeParamSet(&encryptParamSet);
        return ret;
    }
    uint8_t handleE[sizeof(uint64_t)] = {0};
    struct HksBlob handleEncrypt = {sizeof(uint64_t), handleE};
    ret = HksInit(keyAlias, encryptParamSet, &handleEncrypt, nullptr);
    if (ret != HKS_SUCCESS) {
        DATA_STORAGE_LOGE("hks init encrypt invoke failed, error code: %{public}d", ret);
        HksFreeParamSet(&encryptParamSet);
        return ret;
    }
    ret = HksFinish(&handleEncrypt, encryptParamSet, inData, outData);
    HksFreeParamSet(&encryptParamSet);
    if (ret != HKS_SUCCESS) {
        DATA_STORAGE_LOGE("hks finish encrypt invoke failed, error code: %{public}d", ret);
        return ret;
    }
    if (memcpy_s(outData->data + outData->size, IV_SIZE, iv, IV_SIZE) != EOK) {
        DATA_STORAGE_LOGE("copy iv failed!");
        return -1;
    }
    outData->size += IV_SIZE;
    return ret;
}

int AesCbcPkcs7Encrypt(const std::string &alias, struct HksBlob *inData, struct HksBlob *outData)
{
    if (inData == nullptr || outData == nullptr || outData->data == nullptr || outData->size < IV_SIZE) {
        DATA_STORAGE_LOGE("encrypt input error");
        return -1;
    }
    struct HksParamSet *genParamSet = nullptr;
    int ret = InitParamSet(&genParamSet, g_genEncDecParams, sizeof(g_genEncDecParams) / sizeof(HksParam));
    if (ret != HKS_SUCCESS) {
        HksFreeParamSet(&genParamSet);
        return ret;
    }
    struct HksBlob keyAlias = { alias.length(), (uint8_t *)alias.c_str() };
    ret = HksKeyExist(&keyAlias, genParamSet);
    if (ret != HKS_SUCCESS) {
        ret = HksGenerateKey(&keyAlias, genParamSet, nullptr);
        if (ret != HKS_SUCCESS) {
            DATA_STORAGE_LOGE("generate key failed, error code: %{public}d", ret);
            HksFreeParamSet(&genParamSet);
            return ret;
        }
    }
    HksFreeParamSet(&genParamSet);
    return AesCbcPkcs7EncryptInner(&keyAlias, inData, outData);
}

int AesCbcPkcs7Decrypt(const std::string &alias, struct HksBlob *inData, struct HksBlob *outData)
{
    if (inData == nullptr || outData == nullptr || inData->data == nullptr || inData->size < IV_SIZE) {
        DATA_STORAGE_LOGE("decrypt input error");
        return -1;
    }
    struct HksBlob keyAlias = { alias.length(), (uint8_t *)alias.c_str() };
    struct HksParamSet *genParamSet = nullptr;
    struct HksParamSet *decryptParamSet = nullptr;
    int32_t ret = InitParamSet(&genParamSet, g_genEncDecParams, sizeof(g_genEncDecParams) / sizeof(HksParam));
    if (ret != HKS_SUCCESS) {
        HksFreeParamSet(&genParamSet);
        DATA_STORAGE_LOGE("init gen param set failed, error code: %{public}d", ret);
        return ret;
    }
    ret = HksKeyExist(&keyAlias, genParamSet);
    HksFreeParamSet(&genParamSet);
    if (ret != HKS_SUCCESS) {
        DATA_STORAGE_LOGE("hks key is not exist, error code: %{public}d", ret);
        return ret;
    }
    struct HksParam decryptParams[] = {
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_DECRYPT },
        { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256 },
        { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_PKCS7 },
        { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_CBC },
        { .tag = HKS_TAG_IV, .blob = { .size = IV_SIZE, .data = inData->data + inData->size - IV_SIZE }},
        { .tag = HKS_TAG_AUTH_STORAGE_LEVEL, .uint32Param = HKS_AUTH_STORAGE_LEVEL_DE }};
    ret = InitParamSet(&decryptParamSet, decryptParams, sizeof(decryptParams) / sizeof(HksParam));
    if (ret != HKS_SUCCESS) {
        DATA_STORAGE_LOGE("init decrypt param set failed, error code: %{public}d", ret);
        HksFreeParamSet(&decryptParamSet);
        return ret;
    }
    uint8_t handleD[sizeof(uint64_t)] = {0};
    struct HksBlob handleDecrypt = {sizeof(uint64_t), handleD};
    ret = HksInit(&keyAlias, decryptParamSet, &handleDecrypt, nullptr);
    if (ret != HKS_SUCCESS) {
        HksFreeParamSet(&decryptParamSet);
        DATA_STORAGE_LOGE("hks init decrypt invoke failed, error code: %{public}d", ret);
        return ret;
    }
    struct HksBlob temp = { .size = inData->size - IV_SIZE, .data = inData->data };
    ret = HksFinish(&handleDecrypt, decryptParamSet, &temp, outData);
    HksFreeParamSet(&decryptParamSet);
    if (ret != HKS_SUCCESS) {
        DATA_STORAGE_LOGE("hks finish decrypt invoke failed, error code: %{public}d", ret);
    }
    return ret;
}

std::vector<uint8_t> EncryptData(std::string encryptData)
{
    std::vector<uint8_t> result;
    result.clear();
    if (encryptData.empty()) {
        DATA_STORAGE_LOGI("encryptData is empty");
        return result;
    }
    struct HksBlob inDataBlob = { encryptData.length(), (uint8_t *)encryptData.c_str() };
    uint8_t outData[AES_COMMON_SIZE] = {0};
    struct HksBlob outDataBlob = {AES_COMMON_SIZE, outData};
    int32_t ret = AesCbcPkcs7Encrypt(APN_PWD_KEY_ALIAS.c_str(), &inDataBlob, &outDataBlob);
    if (ret != 0) {
        DATA_STORAGE_LOGE("EncryptDataerror ret=%{public}d", ret);
        return result;
    }
    for (size_t i = 0; i < outDataBlob.size; i++) {
        result.emplace_back(outDataBlob.data[i]);
     }
    return result;
}

std::string DecryptData(std::string decryptData)
{
    if (decryptData.empty()) {
        DATA_STORAGE_LOGI("decryptData is empty");
        return "";
    }
    struct HksBlob inDataBlob = { decryptData.length(), (uint8_t *)decryptData.c_str() };
    uint8_t outData[AES_COMMON_SIZE] = {0};
    struct HksBlob outDataBlob = {AES_COMMON_SIZE, outData};
    int32_t ret = AesCbcPkcs7Decrypt(APN_PWD_KEY_ALIAS.c_str(), &inDataBlob, &outDataBlob);
    if (ret != 0) {
        DATA_STORAGE_LOGE("DecryptData error ret=%{public}d", ret);
        return "";
    }
    std::string result(reinterpret_cast<const char*>(outDataBlob.data), outDataBlob.size);
    return result;
}

std::string DecryptVecData(std::vector<uint8_t> decryptVecData)
{
    if (decryptVecData.size() == 0) {
        DATA_STORAGE_LOGI("DecryptVecData is empty");
        return "";
    }
    struct HksBlob inDataBlob = { decryptVecData.size(), &decryptVecData[0] };
    uint8_t outData[AES_COMMON_SIZE] = {0};
    struct HksBlob outDataBlob = {AES_COMMON_SIZE, outData};
    int32_t ret = AesCbcPkcs7Decrypt(APN_PWD_KEY_ALIAS.c_str(), &inDataBlob, &outDataBlob);
    if (ret != 0) {
        DATA_STORAGE_LOGE("DecryptVecData error ret=%{public}d", ret);
        return "";
    }
    std::string result(reinterpret_cast<const char*>(outDataBlob.data), outDataBlob.size);
    return result;
}
}  // namespace Telephony
}  // namespace OHOS