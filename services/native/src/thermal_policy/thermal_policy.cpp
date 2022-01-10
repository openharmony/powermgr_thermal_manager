/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "thermal_policy.h"

#include <algorithm>
#include "thermal_common.h"
#include "thermal_service.h"
#include "string_operation.h"
#include "constants.h"

using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace PowerMgr {
namespace {
auto g_service = DelayedSpSingleton<ThermalService>::GetInstance();
TypeTempMap typeTempMap;
}

ThermalPolicy::ThermalPolicy() {};

bool ThermalPolicy::Init()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, " %{public}s enter", __func__);
    DumpConfigInfo();
    SortLevel();
    if (g_service == nullptr) {
        return false;
    }
    handler_ = g_service->GetHandler();
    RegisterObserver();
    return true;
}

void ThermalPolicy::RegisterObserver()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "RegisterObserver: Enter");
    ThermalObserver::Callback callback = std::bind(&ThermalPolicy::GetSensorInfomation, this, std::placeholders::_1);
    g_service->GetObserver()->SetRegisterCallback(callback);
}

void ThermalPolicy::GetSensorInfomation(TypeTempMap info)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, " %{public}s enter", __func__);
    typeTempMap = info;
    for (auto it : typeTempMap) {
        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, " %{public}s type:%{public}s temp:%{public}d",
            __func__, it.first.c_str(), it.second);
    }
    LevelDecision();

    PolicyDecision(clusterLevelMap_);
}

void ThermalPolicy::LevelDecision()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, " %{public}s enter", __func__);
    for (auto cluster = msc_.begin(); cluster != msc_.end(); cluster++) {
        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, " %{public}s: name: %{public}s", __func__, cluster->first.c_str());
        cluster->second->UpdateThermalLevel(typeTempMap);
        uint32_t level = cluster->second->GetCurrentLevel();
        clusterLevelMap_[cluster->first] = level;
        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, " %{public}s cluster level %{public}d", __func__, level);
    }

    for (auto iter = clusterLevelMap_.begin(); iter != clusterLevelMap_.end(); iter++) {
        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, " %{public}s name = %{public}s, level = %{public}d",
            __func__, iter->first.c_str(), iter->second);
    }
}

void ThermalPolicy::PolicyDecision(std::map<std::string, uint32_t> &clusterLevelMap)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, " %{public}s policySize = %{public}d, clusterSize = %{public}d",
        __func__, clusterPolicyMap_.size(), clusterLevelMap.size());
    for (auto clusterPolicy = clusterPolicyMap_.begin(); clusterPolicy != clusterPolicyMap_.end(); clusterPolicy++) {
        if (clusterPolicy->first.empty() && clusterPolicy->second.empty()) {
            continue;
        }
        auto clusterIter = clusterLevelMap.find(clusterPolicy->first);
        if (clusterIter != clusterLevelMap.end()) {
            for (auto levelAction = clusterPolicy->second.rbegin(); levelAction != clusterPolicy->second.rend();
                levelAction++) {
                THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE,
                    "%{public}s levelAction->level = %{public}d, clusterIter->second = %{public}d",
                    __func__, levelAction->level, clusterIter->second);
                if (clusterIter->second >= levelAction->level) {
                    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, " %{public}s start to action decision", __func__);

                    ActionDecision(levelAction->vPolicyAction);
                    break;
                }
            }
        } else {
            THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, " %{public}s failed to find policy of cluster", __func__);
            continue;
        }
    }

    /* Action Execute */
    if (!ActionExecution()) {
        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, " %{public}s failed to execute action", __func__);
        return;
    }
}

void ThermalPolicy::ActionDecision(std::vector<PolicyAction> &vAction)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, " %{public}s action.size=%{public}d", __func__, vAction.size());

    for (auto action = vAction.begin(); action != vAction.end(); action++) {
        ThermalActionManager::ThermalActionMap actionMap = g_service->GetActionManagerObj()->GetActionMap();
        auto actionIter = actionMap.find(action->actionName);
        if (actionIter != actionMap.end()) {
            THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, " %{public}s: actoinIterName = %{public}s",
                __func__, actionIter->first.c_str());
            if (actionIter->second == nullptr) {
                THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, " %{public}s: action instance is nullptr", __func__);
                continue;
            }
            if (action->isProp) {
                THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s: start state decision", __func__);
                if (StateMachineDecision(action->mActionProp)) {
                    actionIter->second->AddActionValue(action->actionValue);
                } else {
                    THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, " %{public}s: failed to decide state", __func__);
                }
            } else {
                THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s: add action value", __func__);
                actionIter->second->AddActionValue(action->actionValue);
            }
        } else {
            THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, " %{public}s failed to find action", __func__);
            continue;
        }
    }
}

bool ThermalPolicy::StateMachineDecision(std::map<std::string, std::string> &stateMap)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, " %{public}s enter", __func__);

    bool ret = true;
    for (auto prop = stateMap.begin(); prop != stateMap.end(); prop++) {
        StateMachine::StateMachineMap stateMachineMap = g_service->GetStateMachineObj()->GetStateCollectionMap();
        auto stateIter = stateMachineMap.find(prop->first);
        std::vector<std::string> stateList;
        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE,
            " %{public}s statename = %{public}s stateItername = %{public}s",
            __func__, prop->first.c_str(), stateIter->first.c_str());
        if (stateIter != stateMachineMap.end()) {
            if (stateIter->second == nullptr) {
                THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, " %{public}s: state instance is nullptr", __func__);
                continue;
            }
            THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "state.size=%{public}d", stateList.size());
            if (stateIter->second->DecideState(prop->second)) {
                continue;
            } else {
                ret = false;
                break;
            }
        }
    }
    return ret;
}

bool ThermalPolicy::ActionExecution()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s action execution", __func__);
    auto actionMgr = g_service->GetActionManagerObj();
    if (actionMgr == nullptr) {
        return false;
    }

    if (!actionFallbackSet_.empty()) {
        for (auto currentIter = actionFallbackSet_.begin(); currentIter != actionFallbackSet_.end(); currentIter++) {
            preExecuteList_.insert(*currentIter);
        }
        actionFallbackSet_.clear();
    }

    ThermalActionManager::ThermalActionMap actionMap = actionMgr->GetActionMap();
    for (auto iter = actionMap.begin(); iter != actionMap.end(); iter++) {
        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s action start execution", __func__);
        iter->second->Execute();
        actionFallbackSet_.insert(iter->second);
    }
    return true;
}

bool ThermalPolicy::ActionFallbackDecision()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, " %{public}s enter", __func__);
    if (!preExecuteList_.empty()) {
        for (auto preAction = preExecuteList_.begin(); preAction != preExecuteList_.end(); preAction++) {
            auto currentAction = actionFallbackSet_.find(*preAction);
            if (currentAction == actionFallbackSet_.end()) {
                (*currentAction)->AddActionValue(FALLBACK_ACTION_VALUE);
                (*currentAction)->Execute();
                preExecuteList_.erase(preAction);
                return true;
            } else {
                return false;
            }
        }
    }
    return false;
}

void ThermalPolicy::SortLevel()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, " %{public}s enter", __func__);
    for (auto clusterPolicy = clusterPolicyMap_.begin(); clusterPolicy != clusterPolicyMap_.end(); clusterPolicy++) {
        sort(clusterPolicy->second.begin(), clusterPolicy->second.end(), LevelCompare);
    }
}

void ThermalPolicy::SetPolicyMap(PolicyConfigMap &policyConfigMap)
{
    clusterPolicyMap_ = policyConfigMap;
}

void ThermalPolicy::SetSensorClusterMap(std::map<std::string, std::shared_ptr<ThermalConfigSensorCluster>> &msc)
{
    msc_ = msc;
}

std::map<std::string, uint32_t> ThermalPolicy::GetClusterLevelMap()
{
    return clusterLevelMap_;
}

void ThermalPolicy::DumpConfigInfo()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, " %{public}s enter", __func__);
    for (auto cluster : msc_) {
        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, " %{public}s: name: %{public}s", __func__, cluster.first.c_str());
        for (auto levelinfo : cluster.second->GetSensorInfoList()) {
            THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, " %{public}s type %{public}s", __func__, levelinfo.first.c_str());
            for (auto item : levelinfo.second) {
                THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE,
                    "%{public}s:threshold:%{public}d, clr:%{public}d, tempRiseRate:%{public}f, level:%{public}d",
                    __func__, item.threshold, item.thresholdClr, item.tempRiseRate, item.level);
            }
        }
        for (auto auxlevelinfo : cluster.second->GetAuxSensorInfoList()) {
            THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, " %{public}s: type: %{public}s",
                __func__, auxlevelinfo.first.c_str());
            for (auto item : auxlevelinfo.second) {
                THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE,
                    " %{public}s: lowerTemp: %{public}d, upperTemp: %{public}d",
                    __func__, item.lowerTemp, item.upperTemp);
            }
        }
    }

    for (auto policy : clusterPolicyMap_) {
        if (policy.first.empty() && policy.second.empty()) continue;
        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, " %{public}s: clusterName = %{public}s",
            __func__, policy.first.c_str());
        for (auto policyConfig : policy.second) {
            THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, " %{public}s: level = %{public}d",
                __func__, policyConfig.level);
            for (auto action : policyConfig.vPolicyAction) {
                THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE,
                    " %{public}s: actionName = %{public}s, actionValue = %{public}s, prop = %{public}d",
                    __func__, action.actionName.c_str(), action.actionValue.c_str(), action.isProp);
                for (auto prop : action.mActionProp) {
                    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE,
                        " %{public}s: propName = %{public}s, propValue = %{public}s",
                        __func__, prop.first.c_str(), prop.second.c_str());
                }
            }
        }
    }
}
} // namespace PowerMgr
} // namespace OHOS