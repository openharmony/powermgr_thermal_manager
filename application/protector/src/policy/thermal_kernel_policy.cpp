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
auto &g_service = ThermalKernelService::GetInsance();
const uint32_t FALLBACK_ACTION_VALUE = 0;
}
bool ThermalKernelPolicy::Init()
{
    SetCallback();
    Dump();
    return true;
}

void ThermalKernelPolicy::LevelDecision()
{
    THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s enter", __func__);
    for (auto tzIter = tzInfoMap_.begin(); tzIter != tzInfoMap_.end(); tzIter++) {
        auto typeIter = typeTempMap_.find(tzIter->first);
        if (typeIter != typeTempMap_.end()) {
            THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s start update thermal level", __func__);
            tzIter->second->UpdateThermalLevel(typeIter->second);
            uint32_t level = tzIter->second->GetThermlLevel();
            levelMap_.emplace(std::pair(typeIter->first, level));
            THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s final level = %{public}d", __func__, level);
        }
    }
}

void ThermalKernelPolicy::PolicyDecision()
{
    THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s enter", __func__);
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
    THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s enter", __func__);
    for (auto actionIter = vAction.begin(); actionIter != vAction.end(); actionIter++) {
        ThermalDeviceControl::ThermalActionMap vActionMap = g_service.GetControl()->GetThermalAction();
        auto nameIter = vActionMap.find(actionIter->name);
        if (nameIter != vActionMap.end()) {
            nameIter->second->AddActionValue(actionIter->value);
            executeActionList_.push_back(actionIter->name);
        }
    }
}

void ThermalKernelPolicy::ActionExecution()
{
    THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s enter", __func__);
    if (!actionFallbackSet_.empty()) {
        std::for_each(actionFallbackSet_.cbegin(), actionFallbackSet_.cend(),
            [this](const std::shared_ptr<IThermalAction> action) {
            preExecuteList_.insert(action);
        });
        actionFallbackSet_.clear();
    } else {
        for (auto name : executeActionList_) {
            ThermalDeviceControl::ThermalActionMap vActionMap = g_service.GetControl()->GetThermalAction();
            auto executeIter = vActionMap.find(name);
            if (executeIter != vActionMap.end()) {
                THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s action start execution", __func__);
                executeIter->second->Execute();
                actionFallbackSet_.insert(executeIter->second);
            }
            if (ActionFallbackDecision()) {
                THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "action fallback");
            }
        }
    }
}

bool ThermalKernelPolicy::ActionFallbackDecision()
{
    if (preExecuteList_.empty()) return false;
    std::for_each(preExecuteList_.cbegin(), preExecuteList_.cend(),
        [this](const std::shared_ptr<IThermalAction> action) {
        auto executeIter = actionFallbackSet_.find(action);
        if (executeIter != actionFallbackSet_.end()) {
            return false;
        } else {
            action->AddActionValue(FALLBACK_ACTION_VALUE);
            preExecuteList_.erase(action);
            return true;
        }
    });
    return false;
}

void ThermalKernelPolicy::SetCallback()
{
    ThermalSensorProvider::NotifyTask task = std::bind(&ThermalKernelPolicy::OnReceivedSensorsInfo,
        this, std::placeholders::_1);
    g_service.GetProvider()->RegisterTask(task);
}

void ThermalKernelPolicy::OnReceivedSensorsInfo(SensorsMap typeTempMap)
{
    THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s enter", __func__);
    typeTempMap_ = typeTempMap;
    LevelDecision();
    PolicyDecision();
}

void ThermalKernelPolicy::SetThermalZoneMap(ThermalZoneMap &tzInfoMap)
{
    tzInfoMap_ = tzInfoMap;
}

void ThermalKernelPolicy::SetLevelAction(std::vector<LevelAction> &levelAction)
{
    levelAction_ = levelAction;
}

std::vector<LevelAction> ThermalKernelPolicy::GetLevelAction()
{
    return levelAction_;
}

void ThermalKernelPolicy::Dump()
{
    THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s enter", __func__);
    for (auto tzIter : tzInfoMap_) {
        THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s type = %{public}s", __func__, tzIter.first.c_str());
        tzIter.second->Dump();
    }
}
} // namespace PowerMgr
} // namespace OHOS