/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "thermal_kernel_policy.h"


#include "thermal_common.h"
#include "thermal_device_control.h"
#include "thermal_kernel_service.h"

namespace OHOS {
namespace PowerMgr {
namespace {
auto &g_service = ThermalKernelService::GetInstance();
constexpr int32_t NUM_ZERO = 0;
}
bool ThermalKernelPolicy::Init()
{
    SetCallback();
    SetMultiples();
    Dump();
    return true;
}

void ThermalKernelPolicy::LevelDecision()
{
    THERMAL_HILOGD(FEATURE_PROTECTOR, "Enter");
    for (auto tzIter = tzInfoMap_.begin(); tzIter != tzInfoMap_.end(); tzIter++) {
        auto typeIter = typeTempMap_.find(tzIter->first);
        if (typeIter != typeTempMap_.end()) {
            THERMAL_HILOGD(FEATURE_PROTECTOR, "start update thermal level");
            tzIter->second->UpdateThermalLevel(typeIter->second);
            uint32_t level = tzIter->second->GetThermalLevel();
            levelMap_.emplace(std::pair(typeIter->first, level));
            THERMAL_HILOGI(FEATURE_PROTECTOR, "final level = %{public}d", level);
        }
    }
}

void ThermalKernelPolicy::PolicyDecision()
{
    THERMAL_HILOGD(FEATURE_PROTECTOR, "Enter");
    for (auto policy = levelAction_.begin(); policy != levelAction_.end(); policy++) {
        auto levelIter = levelMap_.find(policy->name);
        if (levelIter != levelMap_.end()) {
            if (levelIter->second == policy->level) {
                ActionDecision(policy->vAction);
            }
        }
    }
    ActionExecution();
}

void ThermalKernelPolicy::ActionDecision(std::vector<ActionItem> &vAction)
{
    THERMAL_HILOGD(FEATURE_PROTECTOR, "Enter");
    for (auto actionIter = vAction.begin(); actionIter != vAction.end(); actionIter++) {
        ThermalDeviceControl::ThermalActionMap actionMap = g_service.GetControl()->GetThermalAction();
        auto nameIter = actionMap.find(actionIter->name);
        if (nameIter != actionMap.end()) {
            if (nameIter->second == nullptr) {
                continue;
            }
            nameIter->second->AddActionValue(actionIter->value);
            executeActionList_.push_back(actionIter->name);
        }
    }
}

void ThermalKernelPolicy::ActionExecution()
{
    THERMAL_HILOGD(FEATURE_PROTECTOR, "Enter");

    if (executeActionList_.empty()) {
        THERMAL_HILOGI(FEATURE_PROTECTOR, "executeActionList_ is empty");
        return;
    }

    ThermalDeviceControl::ThermalActionMap actionMap = g_service.GetControl()->GetThermalAction();
    THERMAL_HILOGI(FEATURE_PROTECTOR, "size: %{public}zu", actionMap.size());
    for (auto name : executeActionList_) {
        auto executeIter = actionMap.find(name);
        if (executeIter != actionMap.end()) {
            if (executeIter->second == nullptr) {
                continue;
            }
            executeIter->second->Execute();
        }
    }
}

void ThermalKernelPolicy::SetCallback()
{
    ThermalProtectorTimer::NotifyTask task = std::bind(&ThermalKernelPolicy::OnReceivedSensorsInfo,
        this, std::placeholders::_1);
    g_service.GetTimer()->RegisterTask(task);
}

void ThermalKernelPolicy::OnReceivedSensorsInfo(SensorsMap typeTempMap)
{
    THERMAL_HILOGD(FEATURE_PROTECTOR, "Enter");
    typeTempMap_ = typeTempMap;
    LevelDecision();
    PolicyDecision();
}

int32_t ThermalKernelPolicy::GetMaxCommonDivisor(int32_t a, int32_t b)
{
    if (b == 0) {
        return NUM_ZERO;
    }

    if (a % b == 0) {
        return b;
    } else {
        return GetMaxCommonDivisor(b, a % b);
    }
}

void ThermalKernelPolicy::SetMultiples()
{
    CalculateMaxCd();
    if (maxCd_ == NUM_ZERO) {
        return;
    }
    for (auto &tzIter : tzInfoMap_) {
        int32_t multiple = tzIter.second->GetInterval() / maxCd_;
        tzIter.second->SetMultiple(multiple);
    }
}

int32_t ThermalKernelPolicy::GetIntervalCommonDivisor(std::vector<int32_t> intervalList)
{
    if (intervalList.empty()) {
        return NUM_ZERO;
    }

    uint32_t count = intervalList.size();
    int32_t commonDivisor = intervalList[0];
    for (uint32_t i = 1; i < count; i++) {
        commonDivisor = GetMaxCommonDivisor(commonDivisor, intervalList[i]);
    }
    return commonDivisor;
}

void ThermalKernelPolicy::CalculateMaxCd()
{
    std::vector<int32_t> intervalList;
    if (tzInfoMap_.empty()) {
        THERMAL_HILOGI(FEATURE_PROTECTOR, "info list is null");
    }
    for (auto &tzIter : tzInfoMap_) {
        int32_t interval = tzIter.second->GetInterval();
        intervalList.push_back(interval);
    }
    maxCd_ = GetIntervalCommonDivisor(intervalList);
    THERMAL_HILOGI(FEATURE_PROTECTOR, "maxCd_ %{public}d", maxCd_);
}

void ThermalKernelPolicy::SetThermalZoneMap(ThermalZoneMap &tzInfoMap)
{
    tzInfoMap_ = tzInfoMap;
}

void ThermalKernelPolicy::SetLevelAction(std::vector<LevelAction> &levelAction)
{
    levelAction_ = levelAction;
}

ThermalKernelPolicy::ThermalZoneMap ThermalKernelPolicy::GetThermalZoneMap()
{
    return tzInfoMap_;
}

std::vector<LevelAction> ThermalKernelPolicy::GetLevelAction()
{
    return levelAction_;
}

int32_t ThermalKernelPolicy::GetMaxCd()
{
    return maxCd_;
}

void ThermalKernelPolicy::Dump()
{
    THERMAL_HILOGD(FEATURE_PROTECTOR, "Enter");
    for (auto &tzIter : tzInfoMap_) {
        THERMAL_HILOGI(FEATURE_PROTECTOR, "type = %{public}s", tzIter.first.c_str());
        tzIter.second->Dump();
    }
    for (auto &actionIter : levelAction_) {
        THERMAL_HILOGI(FEATURE_PROTECTOR, "name:%{public}s, level:%{public}d",
            actionIter.name.c_str(), actionIter.level);
        for (auto &item : actionIter.vAction) {
            THERMAL_HILOGI(FEATURE_PROTECTOR, "actionName:%{public}s, actionValue:%{public}d",
                item.name.c_str(), item.value);
        }
    }
}
} // namespace PowerMgr
} // namespace OHOS
