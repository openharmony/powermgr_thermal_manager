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

#include "thermalsrv_event_handler.h"
#include "thermal_common.h"
#include "thermal_service.h"
#include "ithermal_action.h"

namespace OHOS {
namespace PowerMgr {
ThermalsrvEventHandler::ThermalsrvEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner,
    const wptr<ThermalService> &service) : AppExecFwk::EventHandler(runner), service_(service)
{
    THERMAL_HILOGD(COMP_SVC, "ThermalsrvEventHandler::ThermalsrvEventHandler instance created.");
}

void ThermalsrvEventHandler::ProcessEvent([[maybe_unused]] const AppExecFwk::InnerEvent::Pointer &event)
{
    auto tmsptr = service_.promote();
    if (tmsptr == nullptr) {
        return;
    }
    THERMAL_HILOGI(COMP_SVC, "eventid = %{public}d", event->GetInnerEventId());
    tmsptr->HandleEvent(event->GetInnerEventId());
}
} // namespace PowerMgr
} // namespace OHOS