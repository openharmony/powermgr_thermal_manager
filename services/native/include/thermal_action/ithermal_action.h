/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef ITHERMAL_ACTION_H
#define ITHERMAL_ACTION_H

#include <map>
#include <string>
#include <vector>

#include "constants.h"
#include "thermal_common.h"
#include "thermal_timer.h"

namespace OHOS {
namespace PowerMgr {

struct PolicyDelayAction {
    bool isCompleted;
    int32_t intDelayValue;
    uint32_t uintDelayValue;
    float floatDelayValue;
    uint32_t delayTime;
    uint64_t delayTimerId;
};

class IThermalAction {
public:
    IThermalAction() = default;
    virtual ~IThermalAction() = default;

    virtual void InitParams(const std::string& params) = 0;
    virtual void SetStrict(bool enable) = 0;
    virtual void SetEnableEvent(bool enable) = 0;
    virtual void AddActionValue(uint32_t actionId, std::string value) = 0;
    virtual void ResetActionValue() = 0;

    virtual void AddActionValue(std::string value)
    {
        AddActionValue(0, value);
    }
 
    virtual void AddActionDelayTime(uint32_t actionId, const PolicyDelayAction& delayTime)
    {
        if (policyActionMap_.count(actionId) == 0) {
            policyActionMap_.insert(std::make_pair(actionId, delayTime));
        }
        actionIdList_.insert(actionId);
    }
 
    virtual void Execute()
    {
        auto thermalTimer = std::make_shared<ThermalTimer>();
        for (auto it = policyActionMap_.begin(); it != policyActionMap_.end();) {
            if (actionIdList_.count(it->first) == 0) {
                if (!it->second.isCompleted) {
                    thermalTimer->StopTimer(it->second.delayTimerId);
                    thermalTimer->DestroyTimer(it->second.delayTimerId);
                }
                it = policyActionMap_.erase(it);
            } else {
                ++it;
            }
        }
        actionIdList_.clear();
        int64_t curMsecTimestam = MiscServices::TimeServiceClient::GetInstance()->GetWallTimeMs();

        for (auto &policyAction : policyActionMap_) {
            if (policyAction.second.isCompleted || policyAction.second.delayTimerId > 0) {
                continue;
            }
            auto timerInfo = std::make_shared<ThermalTimerInfo>();
            timerInfo->SetType(ThermalTimer::TIMER_TYPE_WAKEUP | ThermalTimer::TIMER_TYPE_EXACT);
            auto callback = [this, &policyAction]() {
                policyAction.second.isCompleted = true;
                ExecuteInner();
                auto thermalTimer = std::make_shared<ThermalTimer>();
                thermalTimer->StopTimer(policyAction.second.delayTimerId);
                thermalTimer->DestroyTimer(policyAction.second.delayTimerId);
            };
            timerInfo->SetCallbackInfo(callback);
            policyAction.second.delayTimerId = thermalTimer->CreateTimer(timerInfo);
            THERMAL_HILOGI(COMP_SVC, "%{public}s startTimer, delayTime = %{public}d,", actionName_.c_str(),
                policyAction.second.delayTime);
            thermalTimer->StartTimer(policyAction.second.delayTimerId,
                static_cast<uint64_t>(policyAction.second.delayTime + curMsecTimestam));
        }
        ExecuteInner();
    }

    virtual void ExecuteInner() = 0;
protected:
    bool isStrict_ {true};
    bool enableEvent_ {false};
    std::string actionName_ = "";
    std::unordered_map<uint32_t, PolicyDelayAction> policyActionMap_;
    std::unordered_set<uint32_t> actionIdList_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // ITHERMAL_ACTION_H
