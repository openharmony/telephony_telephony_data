/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "sms_mms_observer.h"
#include <filesystem>
#include <fstream>
#include "datashare_helper.h"
#include "datashare_predicates.h"
#include "datashare_log.h"
#include "datashare_errno.h"
#include "data_storage_log_wrapper.h"
#include "rdb_store_config.h"
#include "rdb_utils.h"
#include "sms_mms_ability.h"
#include "sms_mms_data.h"
#include "rdb_sms_mms_util.h"
#include "singleton.h"
 
namespace OHOS {
namespace Telephony {
SmsMmsObserver &SmsMmsObserver::GetInstance()
{
    static SmsMmsObserver instance;
    return instance;
}
SmsMmsObserver::SmsMmsObserver()
{
    DATA_STORAGE_LOGI("new SmsMmsObserver");
}
 
SmsMmsObserver::~SmsMmsObserver()
{
    if (statusChangeListener_ != nullptr) {
        auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgrProxy != nullptr) {
            samgrProxy->UnSubscribeSystemAbility(OHOS::COMMON_EVENT_SERVICE_ID, statusChangeListener_);
            statusChangeListener_ = nullptr;
        }
    }
    {
        std::lock_guard<std::mutex> lock(abilityLock_);
        if (smsMmsAbility_ != nullptr) {
            smsMmsAbility_ = nullptr;
        }
    }
    DATA_STORAGE_LOGI("~SmsMmsObserver");
}
 
void SmsMmsObserver::Init(std::shared_ptr<SmsMmsAbility> smsMmsAbility)
{
    if (smsMmsAbility_ == nullptr) {
        std::lock_guard<std::mutex> lock(abilityLock_);
        smsMmsAbility_ = smsMmsAbility;
    }
    DATA_STORAGE_LOGI("Init ability");
    auto smsMmsSystemAbilityListener = new (std::nothrow) SmsMmsSystemAbilityListener();
    if (smsMmsSystemAbilityListener == nullptr) {
        DATA_STORAGE_LOGE("failed to create statusChangeListener");
        return;
    }
    statusChangeListener_ = smsMmsSystemAbilityListener;
    auto managerPtr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (managerPtr == nullptr) {
        DATA_STORAGE_LOGE("get system ability manager error");
        return;
    }
    int32_t ret = managerPtr->SubscribeSystemAbility(OHOS::COMMON_EVENT_SERVICE_ID, statusChangeListener_);
    if (ret != 0) {
        // Failed to subscribe to the system capability.
        DATA_STORAGE_LOGE("failed to subscribe account manager service APP_KEY2!");
        return;
    }
}
 
bool SmsMmsObserver::IsInit()
{
    std::lock_guard<std::mutex> lock(abilityLock_);
    return  smsMmsAbility_ == nullptr ? false : true;
}
 
void SmsMmsObserver::ChangeStoreToEl5AndMoveData()
{
    std::lock_guard<std::mutex> lock(abilityLock_);
    if (smsMmsAbility_ != nullptr) {
        smsMmsAbility_->ChangeStoreToEl5AndMoveData();
    }
}

SmsMmsSystemAbilityListener::~SmsMmsSystemAbilityListener()
{
    if (databaseSwitchSubscriber_ != nullptr) {
        CommonEventManager::UnSubscribeCommonEvent(databaseSwitchSubscriber_);
        databaseSwitchSubscriber_ = nullptr;
    }
}
 
void SmsMmsSystemAbilityListener::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    DATA_STORAGE_LOGI("observe:%{public}d is added!", systemAbilityId);
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        DATA_STORAGE_LOGE("added APP_KEY2 is invalid!");
        return;
    }
    if (systemAbilityId != OHOS::COMMON_EVENT_SERVICE_ID) {
        DATA_STORAGE_LOGE("added APP_KEY2 is not accoubt manager service, ignored.");
        return;
    }
    DATA_STORAGE_LOGD("begine detect events, observe:%{public}d is added!", systemAbilityId);
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_SECOND_MOUNTED);
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetThreadMode(EventFwk::CommonEventSubscribeInfo::COMMON);
    databaseSwitchSubscriber_ = std::make_shared<DatabaseSwitchEventSubscriber>(subscriberInfo);
    bool subRet = CommonEventManager::SubscribeCommonEvent(databaseSwitchSubscriber_);
    if (!subRet) {
        DATA_STORAGE_LOGE("Failed to subscribe to the lock screen switchover event!");
    }
}
 
void SmsMmsSystemAbilityListener::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        DATA_STORAGE_LOGE("removed APP_KEY2 is invalid!");
        return;
    }
    if (systemAbilityId != OHOS::COMMON_EVENT_SERVICE_ID) {
        DATA_STORAGE_LOGE("removed APP_KEY2 is not account manager service, ignored.");
        return;
    }
    if (databaseSwitchSubscriber_ != nullptr) {
        bool subRet = CommonEventManager::UnSubscribeCommonEvent(databaseSwitchSubscriber_);
        if (!subRet) {
            DATA_STORAGE_LOGE("UnSubscribe user switched event failed!");
        }
        databaseSwitchSubscriber_ = nullptr;
    }
}
 
DatabaseSwitchEventSubscriber::~DatabaseSwitchEventSubscriber()
{
    DATA_STORAGE_LOGI("~DatabaseSwitchEventSubscriber");
}
 
void DatabaseSwitchEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    if (!SmsMmsObserver::GetInstance().IsInit()) {
        DATA_STORAGE_LOGW("SmsMmsObserver not initialized");
        return;
    }
    OHOS::EventFwk::Want want = data.GetWant();
    std::string action = data.GetWant().GetAction();
    int32_t codeType = data.GetCode();
    DATA_STORAGE_LOGI("action = %{public}s, codeType = %{public}d", action.c_str(), codeType);
    if (action == CommonEventSupport::COMMON_EVENT_SECOND_MOUNTED) {
        SmsMmsObserver::GetInstance().ChangeStoreToEl5AndMoveData();
    }
}
} // namespace Telephony
} // namespace OHOS