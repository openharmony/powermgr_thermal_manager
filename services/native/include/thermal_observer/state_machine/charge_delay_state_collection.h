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

#ifndef CHARGE_DELAY_STATE_COLLECTION_H
#define CHARGE_DELAY_STATE_COLLECTION_H

#include "istate_collection.h"
#include "thermal_common_event_receiver.h"
#include "thermal_mgr_listener.h"
#include "thermal_timer.h"

namespace OHOS {
namespace PowerMgr {
class ChargeDelayStateCollection : public IStateCollection {
public:
    enum {
        NON_CRITICAL_STATE = 0,
        CRITICAL_STATE,
    };
    ChargeDelayStateCollection() = default;
    ~ChargeDelayStateCollection() = default;
    bool Init() override;
    bool InitParam(std::string& params) override;
    std::string GetState() override;
    bool DecideState(const std::string& value) override;
    void HandlerPowerDisconnected(const EventFwk::CommonEventData& data);
    void HandlerPowerConnected(const EventFwk::CommonEventData& data);
    bool RegisterEvent();
    bool StartDelayTimer();
    void StopDelayTimer();
    void ResetState();
    virtual void SetState(const std::string& stateValue) override;

#ifndef THERMAL_OBSERVER_UT_TEST
private:
#endif
    int32_t criticalState_ {NON_CRITICAL_STATE};
    std::string params_;
    std::mutex mutex_;
    uint64_t delayTime_ {0};
    uint64_t delayTimerId_ {0};
};
} // namespace PowerMgr
} // namespace OHOS
#endif // CHARGE_DELAY_STATE_COLLECTION_H
