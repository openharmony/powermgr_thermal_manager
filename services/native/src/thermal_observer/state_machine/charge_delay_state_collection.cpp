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

#include "charge_delay_state_collection.h"
 
#ifdef BATTERY_MANAGER_ENABLE
#include "battery_info.h"
#include "battery_srv_client.h"
#endif
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

using namespace OHOS::EventFwk;
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
        [this](const CommonEventData& data) { this->HandlerPowerConnected(data); };
    receiver->AddEvent(CommonEventSupport::COMMON_EVENT_POWER_CONNECTED, batteryPowerConnectedHandler);

    EventHandle batteryPowerDisconnectedHandler =
        [this](const CommonEventData& data) { this->HandlerPowerDisconnected(data); };
    receiver->AddEvent(CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED, batteryPowerDisconnectedHandler);
#endif
    return true;
}

#ifdef BATTERY_MANAGER_ENABLE
void ChargeDelayStateCollection::HandlerPowerDisconnected(const CommonEventData& data __attribute__((__unused__)))
{
    std::lock_guard<std::mutex> lock(mutex_);
    criticalState_ = CRITICAL_STATE;
    StartDelayTimer();
}
 
void ChargeDelayStateCollection::HandlerPowerConnected(const CommonEventData& data __attribute__((__unused__)))
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (delayTimerId_ > 0) {
        StopDelayAction();
    }
    criticalState_ = NON_CRITICAL_STATE;
}
#endif

bool ChargeDelayStateCollection::StartDelayTimer()
{
    auto thermalTimer = std::make_shared<ThermalTimer>();
    auto timerInfo = std::make_shared<ThermalTimerInfo>();
    timerInfo->SetType(timerInfo->TIMER_TYPE_WAKEUP | timerInfo->TIMER_TYPE_EXACT);
    timerInfo->SetCallbackInfo([this] { ResetState(); });

    delayTimerId_ = thermalTimer->CreateTimer(timerInfo);
    int64_t curMsecTimestam = MiscServices::TimeServiceClient::GetInstance()->GetWallTimeMs();

    return thermalTimer->StartTimer(delayTimerId_, static_cast<uint64_t>(delayTime_ + curMsecTimestam));
}

void ChargeDelayStateCollection::StopDelayAction()
{
    if (delayTimerId_ > 0) {
        auto thermalTimer = std::make_shared<ThermalTimer>();
        if (!thermalTimer->StopTimer(delayTimerId_)) {
            THERMAL_HILOGE(COMP_SVC, "failed to stop delay timer, timerId = %{public}llu", delayTimerId_);
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
    std::lock_guard<std::mutex> lock(mutex_);
    criticalState_ = NON_CRITICAL_STATE;
    delayTimerId_ = 0;
}

bool ChargeDelayStateCollection::InitDelayTime(std::string& delaytime)
{
    THERMAL_HILOGD(COMP_SVC, "init charge delay time info");
    delayTime_ = static_cast<uint32_t>(strtol(delaytime.c_str(), nullptr, STRTOL_FORMART_DEC));
    return true;
}

bool ChargeDelayStateCollection::DecideState(const std::string& value)
{
#ifdef BATTERY_MANAGER_ENABLE
    THERMAL_HILOGD(COMP_SVC, "Enter: Consider the influence of critical state of charge");
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