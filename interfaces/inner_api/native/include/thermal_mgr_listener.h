/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef THERMAL_MGR_LISTENER_H
#define THERMAL_MGR_LISTENER_H

#include <memory>

#include "ithermal_level_callback.h"
#include "thermal_level_callback_stub.h"
#include "thermal_level_info.h"

namespace OHOS {
namespace PowerMgr {
class ThermalMgrListener : public std::enable_shared_from_this<ThermalMgrListener> {
public:
    ThermalMgrListener() {};
    ~ThermalMgrListener() {};
    class ThermalLevelEvent {
    public:
        virtual ~ThermalLevelEvent() = default;
        virtual void OnThermalLevelResult(const ThermalLevel& level) = 0;
    };

    class ThermalLevelCallback : public ThermalLevelCallbackStub {
    public:
        explicit ThermalLevelCallback(std::shared_ptr<ThermalMgrListener> listener) : listener_(listener) {};
        virtual ~ThermalLevelCallback() {};
        virtual bool GetThermalLevel(ThermalLevel level) override;

    private:
        std::weak_ptr<ThermalMgrListener> listener_;
    };

    int32_t SubscribeLevelEvent(const std::shared_ptr<ThermalLevelEvent>& levelEvent);
    int32_t UnSubscribeLevelEvent();
    ThermalLevel GetThermalLevel();
    friend class ThermalLevelCallback;

private:
    void RegisterServiceEvent();
    void UnRegisterServiceEvent();
    sptr<IThermalLevelCallback> callback_;
    std::shared_ptr<ThermalLevelEvent> levelEvent_;
};

enum class ChargeIdleEventCode : uint32_t {
    EVENT_CODE_CHARGE_IDLE_STATE = 0,
};
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_MGR_LISTENER_H
