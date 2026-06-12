/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#ifndef SMS_MMS_OBSERVE_H
#define SMS_MMS_OBSERVE_H

#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "common_event_support.h"
#include "iservice_registry.h"
#include "sms_mms_ability.h"
#include "system_ability_definition.h"
#include "system_ability_status_change_stub.h"

namespace OHOS {
namespace Telephony {
using namespace OHOS::EventFwk;
using CommonEventSubscribeInfo = OHOS::EventFwk::CommonEventSubscribeInfo;
using CommonEventSubscriber = OHOS::EventFwk::CommonEventSubscriber;
class SmsMmsAbility;
class DatabaseSwitchEventSubscriber : public CommonEventSubscriber {
public:
    explicit DatabaseSwitchEventSubscriber(const CommonEventSubscribeInfo &info) : CommonEventSubscriber(info) {}
    ~DatabaseSwitchEventSubscriber();
    void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data) override;
};

class SmsMmsSystemAbilityListener : public SystemAbilityStatusChangeStub {
public:
    SmsMmsSystemAbilityListener() = default;
    ~SmsMmsSystemAbilityListener();
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;

private:
    std::shared_ptr<DatabaseSwitchEventSubscriber> databaseSwitchSubscriber_ = nullptr;
};

class SmsMmsObserver {
public:
    static SmsMmsObserver &GetInstance();
    ~SmsMmsObserver();
    void Init(std::shared_ptr<SmsMmsAbility> smsMmsAbility);
    bool IsInit();
    void ChangeStoreToEl5AndMoveData();
private:
    SmsMmsObserver();
 
private:
    std::mutex abilityLock_;
    std::shared_ptr<SmsMmsAbility> smsMmsAbility_ = nullptr;
    sptr<ISystemAbilityStatusChange> statusChangeListener_ = nullptr;
};
}
}
#endif // SMS_MMS_OBSERVE_H