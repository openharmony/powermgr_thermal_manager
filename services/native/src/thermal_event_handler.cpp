/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "thermal_event_handler.h"
#include "thermal_service.h"

namespace OHOS {
namespace PowerMgr {
namespace {
auto g_service = DelayedSpSingleton<ThermalService>::GetInstance();
}

ThermalEventHandler::ThermalEventHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner)
    : AppExecFwk::EventHandler(runner)
{
    eventFuncMap_ = {
        { THERMAL_UNKNOWN_EVENT, &ThermalEventHandler::HandleUnknownEvent },
        { THERMAL_TEMPERATURE_CHANGED_EVENT, &ThermalEventHandler::HandleTemperatureChanged },
        { THERMAL_STATE_CHANGED_EVENT, &ThermalEventHandler::HandleTemperatureChanged },
    };
    THERMAL_HILOGI(COMP_SVC, "ThermalEventHandler is created");
}

ThermalEventHandler::~ThermalEventHandler()
{
    THERMAL_HILOGW(COMP_SVC, "ThermalEventHandler is destroyed");
}

void ThermalEventHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    if (!event) {
        return;
    }
    uint32_t eventId = event->GetInnerEventId();
    auto funcIter = eventFuncMap_.find(eventId);
    if (funcIter == eventFuncMap_.end()) {
        THERMAL_HILOGW(COMP_SVC, "unexpected eventId: %{public}u", eventId);
        return;
    }
    auto func = funcIter->second;
    if (func) {
        (this->*func)(event);
    }
}

void ThermalEventHandler::HandleUnknownEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    THERMAL_HILOGW(COMP_SVC, "eventId is unknown");
}

void ThermalEventHandler::HandleTemperatureChanged(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto data = event->GetSharedObject<TemperatureData>();
    if (!data || data->typeTempMap.empty()) {
        return;
    }
    g_service->GetSubscriber()->OnTemperatureChanged(data->typeTempMap);
}

void ThermalEventHandler::HandleStateChanged(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto data = event->GetSharedObject<StateData>();
    if (!data || data->tag.empty() || data->val.empty()) {
        return;
    }
    g_service->GetStateMachineObj()->UpdateState(data->tag, data->val);
    if (data->isImmed) {
        g_service->GetPolicy()->ExecutePolicy();
    }
}

} // OHOS
} // PowerMgr