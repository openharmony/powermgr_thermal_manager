/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "action_cpu_big.h"

#include "constants.h"
#include "thermal_hisysevent.h"
#include "thermal_service.h"

namespace OHOS {
namespace PowerMgr {
namespace {
}

ActionCpuBig::ActionCpuBig(const std::string& actionName)
{
    actionName_ = actionName;
    SocActionBase::SocSet.insert(actionName_);
}

void ActionCpuBig::InitParams(const std::string& params)
{
    if (params == "multi-core") {
        multiCoreflag_ = true;
    }
}

void ActionCpuBig::SetStrict(bool enable)
{
    isStrict_ = enable;
}

void ActionCpuBig::SetEnableEvent(bool enable)
{
    enableEvent_ = enable;
}

void ActionCpuBig::AddActionValue(uint32_t actionId, std::string value)
{
    if (value.empty()) {
        return;
    }
    if (actionId > 0) {
        auto iter = policyActionMap_.find(actionId);
        if (iter != policyActionMap_.end()) {
            iter->second.uintDelayValue = static_cast<uint32_t>(strtol(value.c_str(),
                nullptr, STRTOL_FORMART_DEC));
        }
    } else {
        valueList_.push_back(static_cast<uint32_t>(strtol(value.c_str(), nullptr, STRTOL_FORMART_DEC)));
    }
}

void ActionCpuBig::ExecuteInner()
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
        if (multiCoreflag_) {
            SocLimitRequest(LIM_CPU_BIG2_ID, value);
            SocLimitRequest(LIM_CPU_BIG3_ID, value);
            SocLimitRequest(LIM_CPU_BIG4_ID, value);
        }
        SocLimitRequest(LIM_CPU_BIG_ID, value);
        WriteActionTriggeredHiSysEvent(enableEvent_, actionName_, value);
        tms->GetObserver()->SetDecisionValue(actionName_, std::to_string(value));
        lastValue_ = value;
        THERMAL_HILOGD(COMP_SVC, "action execute: {%{public}s = %{public}u}", actionName_.c_str(), lastValue_);
    }
    valueList_.clear();
}

void ActionCpuBig::ResetActionValue()
{
    lastValue_ = 0;
}

uint32_t ActionCpuBig::GetActionValue()
{
    uint32_t value = FALLBACK_VALUE_UINT_SOC;
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
