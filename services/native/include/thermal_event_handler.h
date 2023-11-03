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

#ifndef THERMAL_EVENT_HANDLER_H
#define THERMAL_EVENT_HANDLER_H

#include <string>
#include <unordered_map>
#include <map>

#include "event_handler.h"

namespace OHOS {
namespace PowerMgr {
enum ThermalEvent : uint32_t {
    THERMAL_UNKNOWN_EVENT = 0,
    THERMAL_TEMPERATURE_CHANGED_EVENT,
    THERMAL_STATE_CHANGED_EVENT,
};

struct TemperatureData {
    std::map<std::string, int32_t> typeTempMap;
};

struct StateData {
    bool isImmed;
    std::string tag;
    std::string val;
};

class ThermalEventHandler : public AppExecFwk::EventHandler {
public:
    explicit ThermalEventHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner);
    virtual ~ThermalEventHandler() override;

    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event) override;
private:
    using EventHandleFunc = void (ThermalEventHandler::*)(const AppExecFwk::InnerEvent::Pointer& event);
    void HandleUnknownEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void HandleTemperatureChanged(const AppExecFwk::InnerEvent::Pointer& event);
    void HandleStateChanged(const AppExecFwk::InnerEvent::Pointer& event);

    std::unordered_map<uint32_t, EventHandleFunc> eventFuncMap_;
};
} // OHOS
} // PowerMgr
#endif // POWERMGR_THERMAL_EVENT_HANDLER_H