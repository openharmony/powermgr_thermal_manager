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

#include "action_cpu_boost.h"

#include "constants.h"
#include "thermal_hisysevent.h"
#include "thermal_service.h"

namespace OHOS {
namespace PowerMgr {
namespace {
}

ActionCpuBoost::ActionCpuBoost(const std::string& actionName)
{
    actionName_ = actionName;
}

void ActionCpuBoost::InitParams(const std::string& params)
{
    (void)params;
}

void ActionCpuBoost::SetStrict(bool enable)
{
    isStrict_ = enable;
}

void ActionCpuBoost::SetEnableEvent(bool enable)
{
    enableEvent_ = enable;
}

void ActionCpuBoost::AddActionValue(std::string value)
{
    if (value.empty()) {
        return;
    }
    actionTag_ = static_cast<bool>(strtol(value.c_str(), nullptr, STRTOL_FORMART_DEC));
    isAction_ = true;
}

void ActionCpuBoost::Execute()
{
    auto tms = ThermalService::GetInstance();
    THERMAL_RETURN_IF (tms == nullptr);
    bool tag = GetActionValue();
    if (tag != lastTag_) {
        SocPerfRequest(tag);
        WriteActionTriggeredHiSysEvent(enableEvent_, actionName_, static_cast<int32_t>(tag));
        tms->GetObserver()->SetDecisionValue(actionName_, std::to_string(tag));
        lastTag_ = tag;
        THERMAL_HILOGD(COMP_SVC, "action execute: {%{public}s = %{public}d}", actionName_.c_str(), lastTag_);
    }
    isAction_ = false;
}

bool ActionCpuBoost::GetActionValue()
{
    auto tms = ThermalService::GetInstance();
    std::lock_guard<std::mutex> lock(sceneMutex_);
    std::string scene = tms->GetScene();
    auto iter = g_sceneMap.find(scene);
    if (iter != g_sceneMap.end()) {
        return static_cast<bool>(strtol(iter->second.c_str(), nullptr, STRTOL_FORMART_DEC));
    }
    return isAction_ ? actionTag_ : false;
}
} // namespace PowerMgr
} // namespace OHOS
