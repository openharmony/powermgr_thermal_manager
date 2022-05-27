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

#include "thermal_common_event_receiver.h"
#include <common_event_manager.h>
#include <common_event_support.h>
#include <unistd.h>
#include "thermal_common.h"

using namespace OHOS::AAFwk;
using namespace OHOS::EventFwk;

namespace OHOS {
namespace PowerMgr {
namespace {
const int32_t DELAY_TIME = 50000;
}
ThermalCommonEventReceiver::~ThermalCommonEventReceiver()
{
    if (subscriber_ == nullptr) {
        CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    }
}

bool ThermalCommonEventReceiver::Start(std::string eventName, EventHandle callback)
{
    THERMAL_HILOGD(COMP_SVC, "eventName=%{public}s", eventName.c_str());
    InitEventHandles(eventName, callback);
    return RegisterSubscriber(GetSubscribeInfo());
}

void ThermalCommonEventReceiver::InitEventHandles(std::string eventName, EventHandle callback)
{
    THERMAL_HILOGD(COMP_SVC, "Add Event: %{public}s", eventName.c_str());
    eventHandles_.insert(std::make_pair(eventName, callback));
}

sptr<CesInfo> ThermalCommonEventReceiver::GetSubscribeInfo() const
{
    MatchingSkills skill;
    for (auto &eh : eventHandles_) {
        skill.AddEvent(eh.first);
    }
    sptr<CesInfo> info = new CesInfo(skill);
    return info;
}

bool ThermalCommonEventReceiver::RegisterSubscriber(const sptr<CesInfo>& info)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    static const int32_t MAX_RETRY_TIMES = 2;

    auto succeed = false;
    std::shared_ptr<Ces> s = std::make_shared<EventSubscriber>(info, eventHandles_);
    for (int32_t tryTimes = 0; tryTimes < MAX_RETRY_TIMES; tryTimes++) {
        THERMAL_HILOGI(COMP_SVC, "start subscribe");
        succeed = CommonEventManager::SubscribeCommonEvent(s);
        if (succeed) {
            break;
        }
        THERMAL_HILOGE(COMP_SVC, "Sleep for a while and retry to register subscriber");
        usleep(DELAY_TIME);
    }
    if (!succeed) {
        THERMAL_HILOGE(COMP_SVC, "Failed to register subscriber");
        return false;
    }
    subscriber_ = s;
    THERMAL_HILOGI(COMP_SVC, "Succeed to register subscriber");
    return true;
}

void ThermalCommonEventReceiver::HandleEventChanged(const CommonEventData& __attribute__((unused))data) const
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
}

void ThermalCommonEventReceiver::EventSubscriber::HandleEvent(const OHOS::EventFwk::CommonEventData &data)
{
    auto action = data.GetWant().GetAction();
    auto it = eventHandles_.find(action);
    if (it == eventHandles_.end()) {
        THERMAL_HILOGE(COMP_SVC, "Ignore event: %{public}s", action.c_str());
        return;
    }
    THERMAL_HILOGI(COMP_SVC, "Handle Event: %{public}s", action.c_str());
    it->second(data);
}
} // namespace PowerMgr
} // namespace OHOS