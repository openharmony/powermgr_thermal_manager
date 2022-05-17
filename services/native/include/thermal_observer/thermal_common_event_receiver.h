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

#ifndef THERMAL_COMMON_EVENT_RECEIVER_H
#define THERMAL_COMMON_EVENT_RECEIVER_H

#include <functional>
#include <map>
#include <memory>
#include <string>

#include <common_event_subscriber.h>
#include <want.h>

using IntentWant = OHOS::AAFwk::Want;
using Ces = OHOS::EventFwk::CommonEventSubscriber;
using CesInfo = OHOS::EventFwk::CommonEventSubscribeInfo;
using EventHandle = std::function<void(const OHOS::EventFwk::CommonEventData &data)>;

namespace OHOS {
namespace PowerMgr {
class ThermalCommonEventReceiver {
public:
    ThermalCommonEventReceiver() = default;
    ~ThermalCommonEventReceiver();

    bool Start(std::string eventName, EventHandle callback);

private:
    class EventSubscriber : public Ces {
    public:
        EventSubscriber(const sptr<CesInfo>& info, const std::map<std::string, EventHandle>& handles) : Ces(*info),
            eventHandles_(handles) {}
        void OnReceiveEvent(const EventFwk::CommonEventData &data) override
        {
            HandleEvent(data);
        }
        ~EventSubscriber() override = default;
    private:
        void HandleEvent(const OHOS::EventFwk::CommonEventData &data);

        const std::map<std::string, EventHandle>& eventHandles_;
    };

    void InitEventHandles(std::string eventName, EventHandle callback);
    sptr<CesInfo> GetSubscribeInfo() const;
    bool RegisterSubscriber(const sptr<CesInfo>& info);
    void HandleEventChanged(const OHOS::EventFwk::CommonEventData &data) const;
    void HandleBatteryChargerCompleted(const OHOS::EventFwk::CommonEventData &data) const;
    std::shared_ptr<Ces> subscriber_;
    std::map<std::string, EventHandle> eventHandles_;
};
} // namespace PowerMgr
} // namespace OHOS

#endif // THERMAL_COMMON_EVENT_RECEIVER_H