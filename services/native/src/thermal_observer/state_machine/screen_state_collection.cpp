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
auto g_service = DelayedSpSingleton<ThermalService>::GetInstance();
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
    if (!g_service->GetFlag()) {
        return mockState_;
    } else {
        return state_;
    }
}

bool ScreenStateCollection::RegisterEvent()
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    if (g_service == nullptr) return false;
    auto receiver = g_service->GetStateMachineObj()->GetCommonEventReceiver();
    if (receiver == nullptr) return false;
    THERMAL_HILOGI(COMP_SVC, "register screen on event");
    EventHandle handlerOn = std::bind(&ScreenStateCollection::HandleScreenOnCompleted, this, std::placeholders::_1);
    bool on = receiver->Start(CommonEventSupport::COMMON_EVENT_SCREEN_ON, handlerOn);
    if (!on) {
        THERMAL_HILOGE(COMP_SVC, "fail to COMMON_EVENT_SCREEN_ON");
        return false;
    }
    THERMAL_HILOGI(COMP_SVC, "register screen off event");
    EventHandle handlerOff = std::bind(&ScreenStateCollection::HandleScreenOffCompleted, this, std::placeholders::_1);
    bool off = receiver->Start(CommonEventSupport::COMMON_EVENT_SCREEN_OFF, handlerOff);
    if (!off) {
        THERMAL_HILOGE(COMP_SVC, "fail to COMMON_EVENT_SCREEN_OFF");
        return false;
    }
    return true;
}

void ScreenStateCollection::HandleScreenOnCompleted(const CommonEventData& data __attribute__((__unused__)))
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    state_ = ToString(SCREEN_ON);
}

void ScreenStateCollection::HandleScreenOffCompleted(const CommonEventData& data __attribute__((__unused__)))
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    state_ = ToString(SCREEN_OFF);
}

void ScreenStateCollection::SetState()
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
