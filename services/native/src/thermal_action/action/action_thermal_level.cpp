/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "thermal_service.h"
#include "thermal_common.h"

using namespace OHOS::AAFwk;
using namespace OHOS::EventFwk;

namespace OHOS {
namespace PowerMgr {
namespace {
const std::string TASK_UNREG_THERMAL_LEVEL_CALLBACK = "ThermalLevel_UnRegThermalLevelpCB";
}
int32_t ActionThermalLevel::level_ = static_cast<int32_t>(ThermalLevel::INVALID_LEVEL);
std::set<const sptr<IThermalLevelCallback>, ActionThermalLevel::classcomp> ActionThermalLevel::thermalLevelListeners_;

ActionThermalLevel::ActionThermalLevel(const wptr<ThermalService>& tms) : tms_(tms) {}
ActionThermalLevel::~ActionThermalLevel() {}

bool ActionThermalLevel::InitParams(const std::string &params)
{
    params_ = params;
    return true;
}

void ActionThermalLevel::SetStrict(bool flag)
{
    flag_ = flag;
}

void ActionThermalLevel::AddActionValue(std::string value)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, " %{public}s value=%{public}s", __func__, value.c_str());
    if (value.empty()) return;
    valueList_.push_back(atoi(value.c_str()));
    for (auto iter : valueList_) {
        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s thermallevel=%{public}d", __func__, iter);
    }
}

void ActionThermalLevel::Execute()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s valueList_.size=%{public}zu", __func__, valueList_.size());

    int value = laststValue_;
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
        laststValue_ = value;
    }
}

bool ActionThermalLevel::Init()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s ActionThermalLevel enter", __func__);
    if (thermalLevelCBDeathRecipient_ == nullptr) {
        thermalLevelCBDeathRecipient_ = new ThermalLevelCallbackDeathRecipient();
    }

    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "ActionThermalLevel: Init Success");
    return true;
}

int32_t ActionThermalLevel::GetThermalLevel()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s level = %{public}d", __func__, level_);
    if (level_ != static_cast<int32_t>(ThermalLevel::INVALID_LEVEL)) {
        return level_;
    } else {
        return static_cast<int32_t>(ThermalLevel::INVALID_LEVEL);
    }
}

uint32_t ActionThermalLevel::LevelRequest(int32_t level)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s level = %{public}d", __func__, level);
    if (level_ != level) {
        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s level changed, notify", __func__);
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
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    std::lock_guard lock(mutex_);
    THERMAL_RETURN_IF(callback == nullptr);
    auto object = callback->AsObject();
    THERMAL_RETURN_IF(object == nullptr);
    auto retIt = thermalLevelListeners_.insert(callback);
    if (retIt.second) {
        object->AddDeathRecipient(thermalLevelCBDeathRecipient_);
    }
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE,
        "%{public}s, object=%{public}p, callback=%{public}p, listeners.size=%{public}d, insertOk=%{public}d",
        __func__, object.GetRefPtr(), callback.GetRefPtr(),
        static_cast<unsigned int>(thermalLevelListeners_.size()), retIt.second);
    NotifyThermalLevelChanged(level_);
}

void ActionThermalLevel::UnSubscribeThermalLevelCallback(const sptr<IThermalLevelCallback>& callback)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    std::lock_guard lock(mutex_);
    THERMAL_RETURN_IF(callback == nullptr);
    auto object = callback->AsObject();
    THERMAL_RETURN_IF(object == nullptr);
    size_t eraseNum = thermalLevelListeners_.erase(callback);
    if (eraseNum != 0) {
        object->RemoveDeathRecipient(thermalLevelCBDeathRecipient_);
    }
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE,
        "%{public}s, object=%{public}p, callback=%{public}p, listeners.size=%{public}d, eraseNum=%{public}zu",
        __func__, object.GetRefPtr(), callback.GetRefPtr(),
        static_cast<unsigned int>(thermalLevelListeners_.size()), eraseNum);
}

void ActionThermalLevel::ThermalLevelCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    if (remote == nullptr || remote.promote() == nullptr) {
        return;
    }
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "OnRemoteDied::%{public}s remote = %{public}p",
        __func__, remote.promote().GetRefPtr());
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
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE,
        "%{public}s level = %{public}d, listeners.size = %{public}d",
        __func__, level, static_cast<unsigned int>(thermalLevelListeners_.size()));
    // Get Thermal Level
    level_ = level;
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s level = %{public}d", __func__, level_);
    // Send Notification event
    SendThermalLevelEvents(level_);

    // Call back all level listeners
    for (auto& listener : thermalLevelListeners_) {
        listener->GetThermalLevel(static_cast<ThermalLevel>(level_));
    }
}

/**
 * @brief publish thermal level to other subsystem
 *
 * @param eventAction thermal level change common event
 * @param code thermal level mode
 * @param data thermal level value
 */
bool ActionThermalLevel::PublishEvent(const std::string &eventAction, const int &code, const std::string &data)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s publish event: %{public}s, code: %{public}d",
        __func__, eventAction.c_str(), code);
    Want want;
    want.SetAction(eventAction);
    CommonEventData commonData;
    commonData.SetWant(want);
    commonData.SetCode(code);
    commonData.SetData(data);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(false);
    if (!CommonEventManager::PublishCommonEvent(commonData, publishInfo)) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE,
            "failed to publish thermal level change event: %{public}s, code:%{public}d",
            eventAction.c_str(), code);
        return false;
    }
    return true;
}

bool ActionThermalLevel::PushlishLevelChangedEvents(const int &code, const std::string &data)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    bool ret = PublishEvent(CommonEventSupport::COMMON_EVENT_THERMAL_LEVEL_CHANGED, code, data);
    if (!ret) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "%{public}s to publish level event", __func__);
        return false;
    }
    return true;
}

void ActionThermalLevel::SendThermalLevelEvents(int32_t level)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    switch (level) {
        case static_cast<int32_t>(ThermalLevel::COOL): {
            PushlishLevelChangedEvents(level, std::to_string(level));
            break;
        }
        case static_cast<int32_t>(ThermalLevel::NORMAL): {
            PushlishLevelChangedEvents(level, std::to_string(level));
            break;
        }
        case static_cast<int32_t>(ThermalLevel::WARM): {
            PushlishLevelChangedEvents(level, std::to_string(level));
            break;
        }
        case static_cast<int32_t>(ThermalLevel::HOT): {
            PushlishLevelChangedEvents(level, std::to_string(level));
            break;
        }
        case static_cast<int32_t>(ThermalLevel::OVERHEATED): {
            PushlishLevelChangedEvents(level, std::to_string(level));
            break;
        }
        case static_cast<int32_t>(ThermalLevel::WARNING): {
            PushlishLevelChangedEvents(level, std::to_string(level));
            break;
        }
        case static_cast<int32_t>(ThermalLevel::EMERGENCY): {
            PushlishLevelChangedEvents(level, std::to_string(level));
            break;
        }
        case static_cast<int32_t>(ThermalLevel::INVALID_LEVEL): {
            break;
        }
        default: {
            break;
        }
    }
}
} // namespace PowerMgr
} // namespace OHOS