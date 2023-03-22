/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

using namespace OHOS::AAFwk;
using namespace OHOS::EventFwk;
using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace PowerMgr {
namespace {
constexpr const char* TASK_UNREG_THERMAL_LEVEL_CALLBACK = "ThermalLevel_UnRegThermalLevelpCB";
auto g_service = DelayedSpSingleton<ThermalService>::GetInstance();
}
int32_t ActionThermalLevel::level_ = static_cast<int32_t>(ThermalLevel::COOL);
std::set<const sptr<IThermalLevelCallback>, ActionThermalLevel::classcomp> ActionThermalLevel::thermalLevelListeners_;

ActionThermalLevel::ActionThermalLevel(const wptr<ThermalService>& tms) : tms_(tms), laststValue_(0) {}
ActionThermalLevel::~ActionThermalLevel() {}

ActionThermalLevel::ActionThermalLevel(const std::string& actionName)
{
    actionName_ = actionName;
}

void ActionThermalLevel::InitParams(const std::string& params)
{
}

void ActionThermalLevel::SetStrict(bool flag)
{
    flag_ = flag;
}

void ActionThermalLevel::SetEnableEvent(bool enable)
{
    enableEvent_ = enable;
}

void ActionThermalLevel::AddActionValue(std::string value)
{
    THERMAL_HILOGD(COMP_SVC, "value=%{public}s", value.c_str());
    if (value.empty()) return;
    valueList_.push_back(atoi(value.c_str()));
    for (auto iter : valueList_) {
        THERMAL_HILOGD(COMP_SVC, "thermallevel=%{public}d", iter);
    }
}

void ActionThermalLevel::Execute()
{
    THERMAL_HILOGD(COMP_SVC, "valueList_.size=%{public}zu", valueList_.size());
    THERMAL_RETURN_IF (g_service == nullptr);
    uint32_t value;
    std::string scene = g_service->GetScene();
    auto iter = g_sceneMap.find(scene);
    if (iter != g_sceneMap.end()) {
        value = static_cast<uint32_t>(atoi(iter->second.c_str()));
        if (value != laststValue_) {
            LevelRequest(value);
            WriteActionTriggeredHiSysEvent(enableEvent_, actionName_, value);
            g_service->GetObserver()->SetDecisionValue(actionName_, iter->second);
            laststValue_ = value;
            valueList_.clear();
        }
        return;
    }

    if (valueList_.empty()) {
        value = 0;
    } else {
        if (flag_) {
            value = *max_element(valueList_.begin(), valueList_.end());
        } else {
            value = *min_element(valueList_.begin(), valueList_.end());
        }
        valueList_.clear();
    }

    if (value != laststValue_) {
        LevelRequest(value);
        WriteActionTriggeredHiSysEvent(enableEvent_, actionName_, value);
        g_service->GetObserver()->SetDecisionValue(actionName_, std::to_string(value));
        laststValue_ = value;
    }
}

bool ActionThermalLevel::Init()
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    if (thermalLevelCBDeathRecipient_ == nullptr) {
        thermalLevelCBDeathRecipient_ = new ThermalLevelCallbackDeathRecipient();
    }

    THERMAL_HILOGD(COMP_SVC, "Exit");
    return true;
}

int32_t ActionThermalLevel::GetThermalLevel()
{
    return level_;
}

uint32_t ActionThermalLevel::LevelRequest(int32_t level)
{
    THERMAL_HILOGD(COMP_SVC, "level = %{public}d", level);
    if (level_ != level) {
        THERMAL_HILOGI(COMP_SVC, "level changed, notify");
        NotifyThermalLevelChanged(level);
    }
    return ERR_OK;
}

/**
 * @brief Callback of subscription temp level
 *
 * @param callback Thermal level callback object
 */
void ActionThermalLevel::SubscribeThermalLevelCallback(const sptr<IThermalLevelCallback>& callback)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    std::lock_guard lock(mutex_);
    THERMAL_RETURN_IF(callback == nullptr);
    auto object = callback->AsObject();
    THERMAL_RETURN_IF(object == nullptr);
    auto retIt = thermalLevelListeners_.insert(callback);
    if (retIt.second) {
        object->AddDeathRecipient(thermalLevelCBDeathRecipient_);
    }
    for (auto& listener : thermalLevelListeners_) {
        listener->GetThermalLevel(static_cast<ThermalLevel>(level_));
    }
    THERMAL_HILOGI(COMP_SVC, "listeners.size=%{public}d, insertOk=%{public}d",
        static_cast<unsigned int>(thermalLevelListeners_.size()), retIt.second);
}

void ActionThermalLevel::UnSubscribeThermalLevelCallback(const sptr<IThermalLevelCallback>& callback)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    std::lock_guard lock(mutex_);
    THERMAL_RETURN_IF(callback == nullptr);
    auto object = callback->AsObject();
    THERMAL_RETURN_IF(object == nullptr);
    size_t eraseNum = thermalLevelListeners_.erase(callback);
    if (eraseNum != 0) {
        object->RemoveDeathRecipient(thermalLevelCBDeathRecipient_);
    }
    THERMAL_HILOGI(COMP_SVC, "listeners.size=%{public}d, eraseNum=%{public}zu",
        static_cast<unsigned int>(thermalLevelListeners_.size()), eraseNum);
}

void ActionThermalLevel::ThermalLevelCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    if (remote == nullptr || remote.promote() == nullptr) {
        return;
    }
    auto tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        return;
    }

    auto handler = tms->GetHandler();
    if (handler == nullptr) {
        return;
    }
    sptr<IThermalLevelCallback> callback = iface_cast<IThermalLevelCallback>(remote.promote());
    std::function<void()> unRegFunc = std::bind(&ThermalService::UnSubscribeThermalLevelCallback, tms, callback);
    handler->PostTask(unRegFunc, TASK_UNREG_THERMAL_LEVEL_CALLBACK);
}
/**
 * @brief notify level
 *
 * @param level
 */
void ActionThermalLevel::NotifyThermalLevelChanged(int32_t level)
{
    THERMAL_HILOGD(COMP_SVC, "level = %{public}d, listeners.size = %{public}d",
        level, static_cast<unsigned int>(thermalLevelListeners_.size()));
    // Get Thermal Level
    level_ = level;
    THERMAL_HILOGI(COMP_SVC, "level = %{public}d", level_);
    // Send Notification event
    PublishLevelChangedEvents(ThermalCommonEventCode::CODE_THERMAL_LEVEL_CHANGED, level_);

    // Call back all level listeners
    std::lock_guard lock(mutex_);
    for (auto& listener : thermalLevelListeners_) {
        listener->GetThermalLevel(static_cast<ThermalLevel>(level_));
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
