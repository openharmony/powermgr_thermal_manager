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

#include "startup_delay_state_collection.h"

#include "file_operation.h"
#include "securec.h"
#include "string_ex.h"
#include "string_operation.h"
#include "sysparam.h"
#include "thermal_service.h"
#include "thermal_common.h"

namespace OHOS {
namespace PowerMgr {

bool StartupDelayStateCollection::Init()
{
    THERMAL_HILOGI(COMP_SVC, "StartupDelayStateCollection Init...,start to init startup delay state");
    if (delayTimerId_ > 0) {
        StopDelayTimer();
    }
    state_ = STARTUP_DELAY_STATE;
    StartDelayTimer();
    return true;
}

bool StartupDelayStateCollection::InitParam(std::string& params)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    params_ = params;
    THERMAL_HILOGI(COMP_SVC, "init power on delay time info");
    delayTime_ = static_cast<uint32_t>(strtol(params_.c_str(), nullptr, STRTOL_FORMART_DEC));
    return true;
}

std::string StartupDelayStateCollection::GetState()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return ToString(state_);
}

bool StartupDelayStateCollection::StartDelayTimer()
{
    auto thermalTimer = std::make_shared<ThermalTimer>();
    auto timerInfo = std::make_shared<ThermalTimerInfo>();
    timerInfo->SetType(ThermalTimer::TIMER_TYPE_WAKEUP | ThermalTimer::TIMER_TYPE_EXACT);
    timerInfo->SetCallbackInfo([this] { ResetState(); });
    
    delayTimerId_ = thermalTimer->CreateTimer(timerInfo);
    int64_t curMsecTimestam = MiscServices::TimeServiceClient::GetInstance()->GetWallTimeMs();

    return thermalTimer->StartTimer(delayTimerId_, static_cast<uint64_t>(delayTime_ + curMsecTimestam));
}

void StartupDelayStateCollection::StopDelayTimer()
{
    if (delayTimerId_ > 0) {
        auto thermalTimer = std::make_shared<ThermalTimer>();
        if (!thermalTimer->StopTimer(delayTimerId_)) {
            THERMAL_HILOGE(COMP_SVC, "StartupDelayStateCollection failed to stop delay timer");
        }
        thermalTimer->DestroyTimer(delayTimerId_);
        delayTimerId_ = 0;
    }
}

void StartupDelayStateCollection::SetState(const std::string& stateValue)
{
}

void StartupDelayStateCollection::ResetState()
{
    std::lock_guard<std::mutex> lock(mutex_);
    THERMAL_HILOGI(COMP_SVC, "StartupDelayStateCollection ResetState");
    state_ = NON_STARTUP_DELAY_STATE;
    StopDelayTimer();
}

bool StartupDelayStateCollection::DecideState(const std::string& value)
{
    THERMAL_HILOGD(COMP_SVC, "Enter: DecideState the impact of the power-on delay status.");
    std::lock_guard<std::mutex> lock(mutex_);
    if ((value == ToString(NON_STARTUP_DELAY_STATE) && state_ == NON_STARTUP_DELAY_STATE) ||
        (value == ToString(STARTUP_DELAY_STATE) && state_ == STARTUP_DELAY_STATE)) {
        THERMAL_HILOGD(COMP_SVC, "current power on delay state = %{public}d", state_);
        return true;
    }
    return false;
}
} // namespace PowerMgr
} // namespace OHOS
