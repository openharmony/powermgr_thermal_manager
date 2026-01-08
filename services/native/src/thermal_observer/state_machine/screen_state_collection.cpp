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

#include "screen_state_collection.h"

#include "common_event_support.h"
#include "file_operation.h"
#include "power_mgr_client.h"
#include "securec.h"
#include "string_ex.h"
#include "string_operation.h"
#include "thermal_service.h"
#include "thermal_common.h"
#include "thermal_timer.h"

using namespace OHOS::EventFwk;
namespace OHOS {
namespace PowerMgr {
namespace {
const uint32_t SCREEN_ON = 1;
const uint32_t SCREEN_OFF = 0;
}
bool ScreenStateCollection::Init()
{
    if (!RegisterEvent()) {
        return false;
    }
    return true;
}
bool ScreenStateCollection::InitParam(std::string& params)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    StringOperation::StrToUint(params, delayTime_);
    return true;
}

std::string ScreenStateCollection::GetState()
{
    THERMAL_HILOGD(COMP_SVC, "screen state = %{public}s", mockState_.c_str());
    auto tms = ThermalService::GetInstance();
    if (!tms->GetSimulationXml()) {
        return mockState_;
    } else {
        return ToString(state_);
    }
}

bool ScreenStateCollection::RegisterEvent()
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    auto tms = ThermalService::GetInstance();
    if (tms == nullptr) {
        return false;
    }
    auto receiver = tms->GetStateMachineObj()->GetCommonEventReceiver();
    if (receiver == nullptr) {
        return false;
    }
    EventHandle handlerOn = [this](const CommonEventData& data) { this->HandleScreenOnCompleted(data); };
    receiver->AddEvent(CommonEventSupport::COMMON_EVENT_SCREEN_ON, handlerOn);
    EventHandle handlerOff = [this](const CommonEventData& data) { this->HandleScreenOffCompleted(data); };
    receiver->AddEvent(CommonEventSupport::COMMON_EVENT_SCREEN_OFF, handlerOff);
    return true;
}

void ScreenStateCollection::HandleScreenOnCompleted(const CommonEventData& data __attribute__((__unused__)))
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    THERMAL_HILOGD(COMP_SVC, "received screen on event");
    if (delayTime_ > 0) {
        state_ = SCREEN_OFF;
        StopDelayTimer();
        StartDelayTimer();
    } else {
        state_ = SCREEN_ON;
    }
}

void ScreenStateCollection::HandleScreenOffCompleted(const CommonEventData& data __attribute__((__unused__)))
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    THERMAL_HILOGD(COMP_SVC, "received screen off event");
    state_ = SCREEN_OFF;
    StopDelayTimer();
}

void ScreenStateCollection::SetState(const std::string& stateValue)
{
}

bool ScreenStateCollection::DecideState(const std::string& value)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    std::lock_guard<ffrt::mutex> lock(mutex_);
    state_ = delayTime_ > 0 ? state_ : powerMgrClient.IsScreenOn(false);
    if ((value == ToString(SCREEN_ON) && state_ == SCREEN_ON) ||
        (value == ToString(SCREEN_OFF) && !powerMgrClient.IsScreenOn(false))) {
        return true;
    }
    return false;
}

void ScreenStateCollection::ResetState()
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    THERMAL_HILOGI(COMP_SVC, "ScreenStateCollection ResetState");
    state_ = SCREEN_ON;
    StopDelayTimer();
}

bool ScreenStateCollection::StartDelayTimer()
{
    auto thermalTimer = std::make_shared<ThermalTimer>();
    auto timerInfo = std::make_shared<ThermalTimerInfo>();
    timerInfo->SetType(ThermalTimer::TIMER_TYPE_WAKEUP | ThermalTimer::TIMER_TYPE_EXACT);
    timerInfo->SetCallbackInfo([this] { ResetState(); });

    delayTimerId_ = thermalTimer->CreateTimer(timerInfo);
    int64_t curMsecTimestam = MiscServices::TimeServiceClient::GetInstance()->GetWallTimeMs();

    return thermalTimer->StartTimer(delayTimerId_, static_cast<uint64_t>(delayTime_ + curMsecTimestam));
}

void ScreenStateCollection::StopDelayTimer()
{
    if (delayTimerId_ > 0) {
        auto thermalTimer = std::make_shared<ThermalTimer>();
        if (!thermalTimer->StopTimer(delayTimerId_)) {
            THERMAL_HILOGE(COMP_SVC, "ScreenStateCollection failed to stop delay timer");
        }
        thermalTimer->DestroyTimer(delayTimerId_);
        delayTimerId_ = 0;
    }
}
} // namespace PowerMgr
} // namespace OHOS
