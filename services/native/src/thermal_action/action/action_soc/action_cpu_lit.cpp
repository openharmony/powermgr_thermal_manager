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

#include "action_cpu_lit.h"

#include "constants.h"
#include "thermal_hisysevent.h"
#include "thermal_service.h"

namespace OHOS {
namespace PowerMgr {
namespace {
constexpr int32_t LIM_CPU_LIT_ID = 1001;
auto g_service = DelayedSpSingleton<ThermalService>::GetInstance();
}

ActionCpuLit::ActionCpuLit(const std::string& actionName)
{
    actionName_ = actionName;
}

void ActionCpuLit::InitParams(const std::string& params)
{
    (void)params;
}

void ActionCpuLit::SetStrict(bool enable)
{
    isStrict_ = enable;
}

void ActionCpuLit::SetEnableEvent(bool enable)
{
    enableEvent_ = enable;
}

void ActionCpuLit::AddActionValue(std::string value)
{
    if (value.empty()) {
        return;
    }
    valueList_.push_back(static_cast<uint32_t>(strtol(value.c_str(), nullptr, STRTOL_FORMART_DEC)));
}

void ActionCpuLit::Execute()
{
    THERMAL_RETURN_IF (g_service == nullptr);
    uint32_t value = GetActionValue();
    if (value != lastValue_) {
        CpuRuquest(value);
        WriteActionTriggeredHiSysEvent(enableEvent_, actionName_, value);
        g_service->GetObserver()->SetDecisionValue(actionName_, std::to_string(value));
        lastValue_ = value;
        THERMAL_HILOGD(COMP_SVC, "action execute: {%{public}s = %{public}u}", actionName_.c_str(), lastValue_);
    }
    valueList_.clear();
}

uint32_t ActionCpuLit::GetActionValue()
{
    std::string scene = g_service->GetScene();
    auto iter = g_sceneMap.find(scene);
    if (iter != g_sceneMap.end()) {
        return static_cast<uint32_t>(strtol(iter->second.c_str(), nullptr, STRTOL_FORMART_DEC));
    }
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

void ActionCpuLit::CpuRuquest(uint32_t freq)
{
    if (!g_service->GetSimulationXml()) {
        SocLimitRequest(LIM_CPU_LIT_ID, freq);
    } else {
        auto thermalInterface = g_service->GetThermalInterface();
        if (thermalInterface != nullptr) {
            thermalInterface->SetCpuFreq(freq);
        }
    }
}
} // namespace PowerMgr
} // namespace OHOS
