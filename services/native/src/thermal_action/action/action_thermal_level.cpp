/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "action_thermal_level.h"

#include <common_event_data.h>
#include <common_event_manager.h>
#include <common_event_support.h>
#include <map>

#include "constants.h"
#include "thermal_hisysevent.h"
#include "thermal_service.h"
#include "thermal_common.h"
#include "ffrt_utils.h"

using namespace OHOS::AAFwk;
using namespace OHOS::EventFwk;
using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace PowerMgr {
namespace {
constexpr int32_t MIN_THERMAL_LEVEL = static_cast<int32_t>(ThermalLevel::COOL);
constexpr int32_t MAX_THERMAL_LEVEL = static_cast<int32_t>(ThermalLevel::ESCAPE);
}
int32_t ActionThermalLevel::lastValue_ = INT_MIN;

ActionThermalLevel::ActionThermalLevel(const std::string& actionName)
{
    actionName_ = actionName;
}

void ActionThermalLevel::InitParams(const std::string& params)
{
    if (thermalLevelCBDeathRecipient_ == nullptr) {
        thermalLevelCBDeathRecipient_ = new ThermalLevelCallbackDeathRecipient();
    }
}

void ActionThermalLevel::SetStrict(bool enable)
{
    isStrict_ = enable;
}

void ActionThermalLevel::SetEnableEvent(bool enable)
{
    enableEvent_ = enable;
}

void ActionThermalLevel::AddActionValue(std::string value)
{
    if (value.empty()) {
        return;
    }
    valueList_.push_back(static_cast<uint32_t>(strtol(value.c_str(), nullptr, STRTOL_FORMART_DEC)));
}

void ActionThermalLevel::Execute()
{
    auto tms = ThermalService::GetInstance();
    THERMAL_RETURN_IF (tms == nullptr);
    int32_t value = GetActionValue();
    if (value != lastValue_) {
        LevelRequest(value);
        WriteActionTriggeredHiSysEvent(enableEvent_, actionName_, value);
        tms->GetObserver()->SetDecisionValue(actionName_, std::to_string(value));
        lastValue_ = value;
        THERMAL_HILOGD(COMP_SVC, "action execute: {%{public}s = %{public}u}", actionName_.c_str(), lastValue_);
    }
    valueList_.clear();
}

int32_t ActionThermalLevel::GetActionValue()
{
    int32_t value = MIN_THERMAL_LEVEL;
    if (!valueList_.empty()) {
        if (isStrict_) {
            value = *min_element(valueList_.begin(), valueList_.end());
        } else {
            value = *max_element(valueList_.begin(), valueList_.end());
        }
    }
    return value;
}

int32_t ActionThermalLevel::GetThermalLevel()
{
    return lastValue_;
}

void ActionThermalLevel::LevelRequest(int32_t level)
{
    if (level > MAX_THERMAL_LEVEL) {
        level = MAX_THERMAL_LEVEL;
    }
    if (level < MIN_THERMAL_LEVEL) {
        level = MIN_THERMAL_LEVEL;
    }
    NotifyThermalLevelChanged(level);
}

void ActionThermalLevel::SubscribeThermalLevelCallback(const sptr<IThermalLevelCallback>& callback)
{
    std::lock_guard lock(mutex_);
    THERMAL_RETURN_IF(callback == nullptr);
    auto object = callback->AsObject();
    THERMAL_RETURN_IF(object == nullptr);
    auto retIt = thermalLevelListeners_.insert(callback);
    if (retIt.second) {
        object->AddDeathRecipient(thermalLevelCBDeathRecipient_);
        callback->OnThermalLevelChanged(static_cast<ThermalLevel>(lastValue_));
    }
    THERMAL_HILOGI(COMP_SVC, "listeners.size=%{public}zu, insertOk=%{public}d",
        thermalLevelListeners_.size(), retIt.second);
}

void ActionThermalLevel::UnSubscribeThermalLevelCallback(const sptr<IThermalLevelCallback>& callback)
{
    std::lock_guard lock(mutex_);
    THERMAL_RETURN_IF(callback == nullptr);
    auto object = callback->AsObject();
    THERMAL_RETURN_IF(object == nullptr);
    size_t eraseNum = thermalLevelListeners_.erase(callback);
    if (eraseNum != 0) {
        object->RemoveDeathRecipient(thermalLevelCBDeathRecipient_);
    }
    THERMAL_HILOGI(COMP_SVC, "listeners.size=%{public}zu, eraseNum=%{public}zu",
        thermalLevelListeners_.size(), eraseNum);
}

void ActionThermalLevel::ThermalLevelCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    if (remote == nullptr || remote.promote() == nullptr) {
        return;
    }
    auto tms = ThermalService::GetInstance();
    if (tms == nullptr) {
        return;
    }
    sptr<IThermalLevelCallback> callback = iface_cast<IThermalLevelCallback>(remote.promote());
    FFRTTask task = std::bind(&ThermalService::UnSubscribeThermalLevelCallback, tms, callback);
    FFRTUtils::SubmitTask(task);
}

void ActionThermalLevel::NotifyThermalLevelChanged(int32_t level)
{
    THERMAL_HILOGI(COMP_SVC, "thermal level changed, new lev: %{public}d, old lev: %{public}d", level, lastValue_);

    // Send Notification event
    PublishLevelChangedEvents(ThermalCommonEventCode::CODE_THERMAL_LEVEL_CHANGED, level);

    // Call back all level listeners
    std::lock_guard lock(mutex_);
    for (auto& listener : thermalLevelListeners_) {
        listener->OnThermalLevelChanged(static_cast<ThermalLevel>(level));
    }

    // Notify thermal level change event to battery statistics
    WriteLevelChangedHiSysEvent(enableEvent_, level);
}

bool ActionThermalLevel::PublishLevelChangedEvents(ThermalCommonEventCode code, int32_t level)
{
    Want want;
    want.SetParam(ToString(static_cast<int32_t>(code)), level);
    want.SetAction(CommonEventSupport::COMMON_EVENT_THERMAL_LEVEL_CHANGED);
    CommonEventData commonData;
    commonData.SetWant(want);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(false);
    if (!CommonEventManager::PublishCommonEvent(commonData, publishInfo)) {
        THERMAL_HILOGE(COMP_SVC, "failed to publish thermal level change event");
        return false;
    }
    return true;
}
} // namespace PowerMgr
} // namespace OHOS
