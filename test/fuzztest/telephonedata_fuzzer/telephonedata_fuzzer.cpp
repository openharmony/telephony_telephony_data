/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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

#include "telephonedata_fuzzer.h"

#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>
#include "parcel.h"
#include "singleton.h"
#include "sms_mms_ability.h"


#undef private
namespace OHOS {
namespace Telephony {

const std::vector<std::string>  smsMmsUri = {
    "/sms_mms/sms_mms_info",
    "/sms_mms/sms_mms_info/thirty",
    "/sms_mms/sms_mms_info/max_group",
    "/sms_mms/sms_mms_info/unread_total",
    "/sms_mms/mms_info",
    "/sms_mms/mms_protocol",
    "/sms_mms/sms_subsection",
    "/sms_mms/rcs_info",
    "/sms_mms/mms_part",
    "/sms_mms/session",
    "/sms_mms/mms_pdu"
};

void FuzzSmsMmsAbilityInsert(Parcel &parcel)
{
    Uri uri(smsMmsUri[parcel.ReadUint32() % smsMmsUri.size()]);
    DataShare::DataShareValuesBucket value;
    DelayedSingleton<SmsMmsAbility>::GetInstance()->DoInit();
    DelayedSingleton<SmsMmsAbility>::GetInstance()->Insert(uri, value);
}

void FuzzSmsMmsAbilityUpdate(Parcel &parcel)
{
    Uri uri(smsMmsUri[parcel.ReadUint32() % smsMmsUri.size()]);
    DataShare::DataSharePredicates predicates;
    DataShare::DataShareValuesBucket value;
    DelayedSingleton<SmsMmsAbility>::GetInstance()->DoInit();
    DelayedSingleton<SmsMmsAbility>::GetInstance()->Update(uri, predicates, value);
}

void FuzzSmsMmsAbilityDelete(Parcel &parcel)
{
    Uri uri(smsMmsUri[parcel.ReadUint32() % smsMmsUri.size()]);
    DataShare::DataSharePredicates predicates;
    DelayedSingleton<SmsMmsAbility>::GetInstance()->DoInit();
    DelayedSingleton<SmsMmsAbility>::GetInstance()->Delete(uri, predicates);
}


using FuzzFunc = decltype(FuzzSmsMmsAbilityInsert);
FuzzFunc *g_fuzzFuncs[] = {
    FuzzSmsMmsAbilityInsert,
    FuzzSmsMmsAbilityUpdate,
    FuzzSmsMmsAbilityDelete
};

void RcsFuzzTest(const uint8_t *data, size_t size)
{
    Parcel parcel;
    parcel.WriteBuffer(data, size);
    parcel.RewindRead(0);
    uint32_t index = parcel.ReadUint32() % (sizeof(g_fuzzFuncs)) / sizeof(FuzzFunc *);
    auto fuzzFunc = g_fuzzFuncs[index];
    fuzzFunc(parcel);
    return;
}
} // namespace Telephony
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::Telephony::RcsFuzzTest(data, size);
    return 0;
}
