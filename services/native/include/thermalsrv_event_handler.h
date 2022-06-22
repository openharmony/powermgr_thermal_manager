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

#ifndef THERMALSRV_EVENT_HANDLER
#define THERMALSRV_EVENT_HANDLER

#include <set>
#include <event_handler.h>
#include "delayed_sp_singleton.h"
#include "ithermal_action.h"

namespace OHOS {
namespace PowerMgr {
class ThermalService;
class ThermalsrvEventHandler : public AppExecFwk::EventHandler {
public:
    enum {
        SEND_THERMAL_LEVEL_MSG,
        SEND_EXECUTION_ACTION_MSG,
        SEND_POPUP_MSG,
        SEND_REGISTER_THERMAL_HDI_CALLBACK,
        SEND_RETRY_REGISTER_HDI_STATUS_LISTENER,
        SEND_ACTION_HUB_LISTENER,
    };

    ThermalsrvEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner,
        const wptr<ThermalService> &service);
    ~ThermalsrvEventHandler() = default;
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;

private:
    wptr<ThermalService> service_;
};
} // namespace PowerMgr
} // namespace OHOS

#endif // THERMALSRV_EVENT_HANDLER
