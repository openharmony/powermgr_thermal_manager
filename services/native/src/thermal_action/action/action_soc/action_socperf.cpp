/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "action_socperf.h"

#include "string_operation.h"
#include "thermal_hisysevent.h"
#include "thermal_service.h"

namespace OHOS {
namespace PowerMgr {
ActionSocPerf::ActionSocPerf(const std::string& actionName)
{
    actionName_ = actionName;
    SocActionBase::SocSet.insert(actionName_);
}

void ActionSocPerf::InitParams(const std::string& params)
{
    StrToInt(params, cmdId_);
}

void ActionSocPerf::SetStrict(bool enable)
{
    isStrict_ = enable;
}

void ActionSocPerf::SetEnableEvent(bool enable)
{
    enableEvent_ = enable;
}

void ActionSocPerf::AddActionValue(uint32_t actionId, std::string value)
{
    if (value.empty()) {
        THERMAL_HILOGD(COMP_SVC, "value empty");
        return;
    }
    uint32_t newValue = 0;
    if (!StringOperation::StrToUint(value, newValue)) {
        return;
    }
    if (actionId > 0) {
        auto iter = policyActionMap_.find(actionId);
        if (iter != policyActionMap_.end()) {
            iter->second.uintDelayValue = newValue;
        }
    } else {
        valueList_.push_back(newValue);
    }
}

void ActionSocPerf::ExecuteInner()
{
    auto tms = ThermalService::GetInstance();
    THERMAL_RETURN_IF (tms == nullptr);
    for (auto &policyAction : policyActionMap_) {
        if (policyAction.second.isCompleted) {
            valueList_.push_back(policyAction.second.uintDelayValue);
        }
    }

    uint32_t value = GetActionValue();
    if (value != lastValue_) {
#ifdef SOC_PERF_ENABLE
        SocPerfRequestEx(cmdId_, value > 0);
#endif
        WriteActionTriggeredHiSysEvent(enableEvent_, actionName_, value);
        if (tms->GetObserver() != nullptr) {
            tms->GetObserver()->SetDecisionValue(actionName_, std::to_string(value));
        }
        lastValue_ = value;
        THERMAL_HILOGD(COMP_SVC, "action execute: {%{public}s = %{public}u}", actionName_.c_str(), lastValue_);
    }
    valueList_.clear();
}

void ActionSocPerf::ResetActionValue()
{
    lastValue_ = 0;
}

uint32_t ActionSocPerf::GetActionValue()
{
    uint32_t value = FALLBACK_VALUE_UINT_ZERO;
    if (!valueList_.empty()) {
        if (isStrict_) {
            value = *min_element(valueList_.begin(), valueList_.end());
        } else {
            value = *max_element(valueList_.begin(), valueList_.end());
        }
    }
    return value;
}
} // namespace PowerMgr
} // namespace OHOS
