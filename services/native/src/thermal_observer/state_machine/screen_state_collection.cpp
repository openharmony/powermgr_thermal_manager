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
    params_ = params;
    return true;
}

std::string ScreenStateCollection::GetState()
{
    THERMAL_HILOGD(COMP_SVC, "screen state = %{public}s", mockState_.c_str());
    auto tms = ThermalService::GetInstance();
    if (!tms->GetSimulationXml()) {
        return mockState_;
    } else {
        return state_;
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
    EventHandle handlerOn = std::bind(&ScreenStateCollection::HandleScreenOnCompleted, this, std::placeholders::_1);
    receiver->AddEvent(CommonEventSupport::COMMON_EVENT_SCREEN_ON, handlerOn);
    EventHandle handlerOff = std::bind(&ScreenStateCollection::HandleScreenOffCompleted, this, std::placeholders::_1);
    receiver->AddEvent(CommonEventSupport::COMMON_EVENT_SCREEN_OFF, handlerOff);
    return true;
}

void ScreenStateCollection::HandleScreenOnCompleted(const CommonEventData& data __attribute__((__unused__)))
{
    THERMAL_HILOGD(COMP_SVC, "received screen on event");
    state_ = ToString(SCREEN_ON);
}

void ScreenStateCollection::HandleScreenOffCompleted(const CommonEventData& data __attribute__((__unused__)))
{
    THERMAL_HILOGD(COMP_SVC, "received screen off event");
    state_ = ToString(SCREEN_OFF);
}

void ScreenStateCollection::SetState(const std::string& stateValue)
{
}

bool ScreenStateCollection::DecideState(const std::string& value)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    if ((value == ToString(SCREEN_ON) && powerMgrClient.IsScreenOn()) ||
        (value == ToString(SCREEN_OFF) && !powerMgrClient.IsScreenOn())) {
        return true;
    }
    return false;
}
} // namespace PowerMgr
} // namespace OHOS
