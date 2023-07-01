/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "action_cpu_isolate.h"

#include "constants.h"
#include "thermal_hisysevent.h"
#include "thermal_service.h"

namespace OHOS {
namespace PowerMgr {
namespace {
auto g_service = DelayedSpSingleton<ThermalService>::GetInstance();
}

ActionCpuIsolate::ActionCpuIsolate(const std::string& actionName)
{
    actionName_ = actionName;
}

void ActionCpuIsolate::InitParams(const std::string& params)
{
    (void)params;
}

void ActionCpuIsolate::SetStrict(bool enable)
{
    isStrict_ = enable;
}

void ActionCpuIsolate::SetEnableEvent(bool enable)
{
    enableEvent_ = enable;
}

void ActionCpuIsolate::AddActionValue(std::string value)
{
    if (value.empty()) {
        return;
    }
    valueList_.push_back(static_cast<uint32_t>(strtol(value.c_str(), nullptr, STRTOL_FORMART_DEC)));
}

void ActionCpuIsolate::Execute()
{
    THERMAL_RETURN_IF (g_service == nullptr);
    uint32_t value = GetActionValue();
    if (value != lastValue_) {
        RequestCpuIsolate(value);
        WriteActionTriggeredHiSysEvent(enableEvent_, actionName_, value);
        g_service->GetObserver()->SetDecisionValue(actionName_, std::to_string(value));
        lastValue_ = value;
        THERMAL_HILOGD(COMP_SVC, "action execute: {%{public}s = %{public}u}", actionName_.c_str(), lastValue_);
    }
    valueList_.clear();
}

uint32_t ActionCpuIsolate::GetActionValue()
{
    std::string scene = g_service->GetScene();
    auto iter = g_sceneMap.find(scene);
    if (iter != g_sceneMap.end()) {
        return static_cast<uint32_t>(strtol(iter->second.c_str(), nullptr, STRTOL_FORMART_DEC));
    }
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

void ActionCpuIsolate::RequestCpuIsolate(uint32_t isolateNum)
{
    auto thermalInterface = g_service->GetThermalInterface();
    if (thermalInterface != nullptr) {
        int32_t ret = thermalInterface->IsolateCpu(isolateNum);
        if (ret != ERR_OK) {
            THERMAL_HILOGE(COMP_SVC, "failed to isolate cpu %u to thermal hdi", isolateNum);
            return;
        }
    }
    return;
}
} // namespace PowerMgr
} // namespace OHOS
