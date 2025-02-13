/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

#include "charge_delay_state_collection.h"

#ifdef BATTERY_MANAGER_ENABLE
#include "battery_info.h"
#include "battery_srv_client.h"
#endif
#include "charger_state_collection.h"
#include "common_event_subscriber.h"
#include "common_event_data.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "file_operation.h"
#include "securec.h"
#include "string_ex.h"
#include "string_operation.h"
#include "thermal_service.h"
#include "thermal_common.h"

namespace OHOS {
namespace PowerMgr {
bool ChargeDelayStateCollection::Init()
{
    THERMAL_HILOGD(COMP_SVC, "ChargeDelayStateCollection Init...");
    return this->RegisterEvent();
}

bool ChargeDelayStateCollection::InitParam(std::string& params)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    params_ = params;
    THERMAL_HILOGD(COMP_SVC, "init charge delay time info");
    delayTime_ = static_cast<uint32_t>(strtol(params_.c_str(), nullptr, STRTOL_FORMART_DEC));
    return true;
}

std::string ChargeDelayStateCollection::GetState()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return ToString(criticalState_);
}

bool ChargeDelayStateCollection::RegisterEvent()
{
    auto tms = ThermalService::GetInstance();
    if (tms == nullptr) {
        return false;
    }
    auto receiver = tms->GetStateMachineObj()->GetCommonEventReceiver();
    if (receiver == nullptr) {
        return false;
    }

#ifdef BATTERY_MANAGER_ENABLE
    EventHandle batteryPowerConnectedHandler =
        [this](const EventFwk::CommonEventData& data) { this->HandlerPowerConnected(data); };
    receiver->AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED, batteryPowerConnectedHandler);

    EventHandle batteryPowerDisconnectedHandler =
        [this](const EventFwk::CommonEventData& data) { this->HandlerPowerDisconnected(data); };
    receiver->AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED, batteryPowerDisconnectedHandler);
#endif
    return true;
}

#ifdef BATTERY_MANAGER_ENABLE
void ChargeDelayStateCollection::HandlerPowerDisconnected(const EventFwk::CommonEventData& data)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto csc = ChargerStateCollection::GetInstance();
    csc->SetCharge(false);
    criticalState_ = CRITICAL_STATE;
    THERMAL_HILOGI(COMP_SVC, "ChargeDelayStateCollection HandlerPowerDisconnected");
    StopDelayTimer();
    StartDelayTimer();
}

void ChargeDelayStateCollection::HandlerPowerConnected(const EventFwk::CommonEventData& data)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (delayTimerId_ > 0) {
        StopDelayTimer();
    }
    auto csc = ChargerStateCollection::GetInstance();
    csc->SetCharge(true);
    criticalState_ = NON_CRITICAL_STATE;
    THERMAL_HILOGI(COMP_SVC, "ChargeDelayStateCollection HandlerPowerConnected");
}
#endif

bool ChargeDelayStateCollection::StartDelayTimer()
{
    auto thermalTimer = std::make_shared<ThermalTimer>();
    auto timerInfo = std::make_shared<ThermalTimerInfo>();
    timerInfo->SetType(ThermalTimer::TIMER_TYPE_WAKEUP | ThermalTimer::TIMER_TYPE_EXACT);
    timerInfo->SetCallbackInfo([this] { ResetState(); });
    
    delayTimerId_ = thermalTimer->CreateTimer(timerInfo);
    int64_t curMsecTimestam = MiscServices::TimeServiceClient::GetInstance()->GetWallTimeMs();

    return thermalTimer->StartTimer(delayTimerId_, static_cast<uint64_t>(delayTime_ + curMsecTimestam));
}

void ChargeDelayStateCollection::StopDelayTimer()
{
    if (delayTimerId_ > 0) {
        auto thermalTimer = std::make_shared<ThermalTimer>();
        if (!thermalTimer->StopTimer(delayTimerId_)) {
            THERMAL_HILOGE(COMP_SVC, "ChargeDelayStateCollection failed to stop delay timer");
        }
        thermalTimer->DestroyTimer(delayTimerId_);
        delayTimerId_ = 0;
    }
}

void ChargeDelayStateCollection::SetState(const std::string& stateValue)
{
}

void ChargeDelayStateCollection::ResetState()
{
    THERMAL_HILOGI(COMP_SVC, "ChargeDelayStateCollection ResetState");
    std::lock_guard<std::mutex> lock(mutex_);
    criticalState_ = NON_CRITICAL_STATE;
    delayTimerId_ = 0;
}

bool ChargeDelayStateCollection::DecideState(const std::string& value)
{
#ifdef BATTERY_MANAGER_ENABLE
    THERMAL_HILOGD(COMP_SVC, "Enter: Consider the influence of critical state of charge, %{public}d", criticalState_);
    std::lock_guard<std::mutex> lock(mutex_);
    if ((value == ToString(NON_CRITICAL_STATE) && criticalState_ == NON_CRITICAL_STATE) ||
        (value == ToString(CRITICAL_STATE) && criticalState_ == CRITICAL_STATE)) {
        THERMAL_HILOGD(COMP_SVC, "current critical state = %{public}d", criticalState_);
        return true;
    }
    return false;
#endif

    return true;
}
} // namespace PowerMgr
} // namespace OHOS
