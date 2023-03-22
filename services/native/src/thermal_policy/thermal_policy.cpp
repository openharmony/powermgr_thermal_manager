/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "action_charger.h"
#include "action_voltage.h"
#include "constants.h"
#include "file_operation.h"
#include "thermal_common.h"
#include "thermal_service.h"
#include "securec.h"
#include "string_operation.h"

using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace PowerMgr {
namespace {
auto g_service = DelayedSpSingleton<ThermalService>::GetInstance();
TypeTempMap typeTempMap;
constexpr const char* LEVEL_PATH = "/data/service/el0/thermal/config/configLevel";
const int MAX_PATH = 256;
}

ThermalPolicy::ThermalPolicy() {};

bool ThermalPolicy::Init()
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
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
    THERMAL_HILOGD(COMP_SVC, "Enter");
    ThermalObserver::Callback callback = std::bind(&ThermalPolicy::GetSensorInfomation, this, std::placeholders::_1);
    g_service->GetObserver()->SetRegisterCallback(callback);
}

void ThermalPolicy::GetSensorInfomation(TypeTempMap info)
{
    typeTempMap = info;

    LevelDecision();
    WriteLevel();
    PolicyDecision(clusterLevelMap_);
}

void ThermalPolicy::LevelDecision()
{
    for (auto cluster = msc_.begin(); cluster != msc_.end(); cluster++) {
        cluster->second->UpdateThermalLevel(typeTempMap);
        uint32_t level = cluster->second->GetCurrentLevel();
        clusterLevelMap_[cluster->first] = level;
    }
}

void ThermalPolicy::WriteLevel()
{
    std::list<uint32_t> levelList;
    int32_t ret = -1;
    char levelBuf[MAX_PATH] = {0};
    for (auto iter = clusterLevelMap_.begin(); iter != clusterLevelMap_.end(); iter++) {
        levelList.push_back(iter->second);
    }
    auto level = *max_element(levelList.begin(), levelList.end());

    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, LEVEL_PATH);
    if (ret < EOK) {
        return;
    }
    std::string valueString = std::to_string(level) + "\n";
    ret = FileOperation::WriteFile(levelBuf, valueString, valueString.length());
    if (ret != ERR_OK) {
        return;
    }
}

void ThermalPolicy::PolicyDecision(std::map<std::string, uint32_t> &clusterLevelMap)
{
    THERMAL_HILOGD(COMP_SVC, "policySize = %{public}zu, clusterSize = %{public}zu",
        clusterPolicyMap_.size(), clusterLevelMap.size());
    for (auto clusterPolicy = clusterPolicyMap_.begin(); clusterPolicy != clusterPolicyMap_.end(); clusterPolicy++) {
        if (clusterPolicy->first.empty() && clusterPolicy->second.empty()) {
            continue;
        }
        auto clusterIter = clusterLevelMap.find(clusterPolicy->first);
        if (clusterIter != clusterLevelMap.end()) {
            for (auto levelAction = clusterPolicy->second.rbegin(); levelAction != clusterPolicy->second.rend();
                levelAction++) {
                if (clusterIter->second >= levelAction->level) {
                    ActionDecision(levelAction->vPolicyAction);
                    break;
                }
            }
        } else {
            continue;
        }
    }

    /* Action Execute */
    if (!ActionExecution()) {
        THERMAL_HILOGW(COMP_SVC, "failed to execute action");
        return;
    }

    ActionCharger::ExecuteCurrentLimit();
    ActionVoltage::ExecuteVoltageLimit();
}

void ThermalPolicy::ActionDecision(std::vector<PolicyAction> &vAction)
{
    THERMAL_HILOGD(COMP_SVC, "action.size=%{public}zu", vAction.size());
    for (auto action = vAction.begin(); action != vAction.end(); action++) {
        THERMAL_HILOGD(COMP_SVC, "actionName: %{public}s", action->actionName.c_str());
        ThermalActionManager::ThermalActionMap actionMap = g_service->GetActionManagerObj()->GetActionMap();
        auto actionIter = actionMap.find(action->actionName);
        if (actionIter != actionMap.end()) {
            THERMAL_HILOGD(COMP_SVC, "action Iter Name = %{public}s", actionIter->first.c_str());
            if (actionIter->second == nullptr) {
                THERMAL_HILOGE(COMP_SVC, "action instance is nullptr");
                continue;
            }
            if (action->isProp) {
                THERMAL_HILOGD(COMP_SVC, "start state decision");
                if (StateMachineDecision(action->mActionProp)) {
                    actionIter->second->AddActionValue(action->actionValue);
                    actionIter->second->SetXmlScene(action->mActionProp.begin()->second, action->actionValue);
                } else {
                    THERMAL_HILOGW(COMP_SVC, "failed to decide state");
                }
            } else {
                THERMAL_HILOGD(COMP_SVC, "add action value");
                actionIter->second->AddActionValue(action->actionValue);
            }
        } else {
            THERMAL_HILOGE(COMP_SVC, "failed to find action");
            continue;
        }
    }
}

void ThermalPolicy::FindSubscribeActionValue()
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    if (g_service == nullptr) {
        THERMAL_HILOGI(COMP_SVC, "g_service is nullptr");
        return;
    }
    if (g_service->GetObserver() ==nullptr) {
        THERMAL_HILOGI(COMP_SVC, "g_service->GetObserver() is nullptr");
        return;
    }
    
    g_service->GetObserver()->FindSubscribeActionValue();
}

bool ThermalPolicy::StateMachineDecision(std::map<std::string, std::string> &stateMap)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    bool ret = true;

    for (auto prop = stateMap.begin(); prop != stateMap.end(); prop++) {
        StateMachine::StateMachineMap stateMachineMap = g_service->GetStateMachineObj()->GetStateCollectionMap();
        auto stateIter = stateMachineMap.find(prop->first);
        THERMAL_HILOGD(COMP_SVC, "statename = %{public}s stateItername = %{public}s",
            prop->first.c_str(), stateIter->first.c_str());
        if (stateIter != stateMachineMap.end()) {
            if (stateIter->second == nullptr) {
                THERMAL_HILOGE(COMP_SVC, "state instance is nullptr");
                continue;
            }
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
    THERMAL_HILOGD(COMP_SVC, "Enter");
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
        iter->second->Execute();
        actionFallbackSet_.insert(iter->second);
    }
    handler_->SendEvent(ThermalsrvEventHandler::SEND_ACTION_HUB_LISTENER, 0, 0);
    return true;
}

bool ThermalPolicy::ActionFallbackDecision()
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    if (!preExecuteList_.empty()) {
        for (auto preAction = preExecuteList_.begin(); preAction != preExecuteList_.end(); preAction++) {
            auto currentAction = actionFallbackSet_.find(*preAction);
            if (currentAction != actionFallbackSet_.end()) {
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
    THERMAL_HILOGD(COMP_SVC, "Enter");
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
    THERMAL_HILOGD(COMP_SVC, "Enter");
    for (auto cluster : msc_) {
        THERMAL_HILOGD(COMP_SVC, "name: %{public}s", cluster.first.c_str());
        for (auto levelinfo : cluster.second->GetSensorInfoList()) {
            THERMAL_HILOGD(COMP_SVC, "type %{public}s", levelinfo.first.c_str());
            for (auto item : levelinfo.second) {
                THERMAL_HILOGD(COMP_SVC,
                    "threshold:%{public}d, clr:%{public}d, tempRiseRate:%{public}f, level:%{public}d",
                    item.threshold, item.thresholdClr, item.tempRiseRate, item.level);
            }
        }
        for (auto auxlevelinfo : cluster.second->GetAuxSensorInfoList()) {
            THERMAL_HILOGD(COMP_SVC, "type: %{public}s", auxlevelinfo.first.c_str());
            for (auto item : auxlevelinfo.second) {
                THERMAL_HILOGD(COMP_SVC, "lowerTemp: %{public}d, upperTemp: %{public}d",
                    item.lowerTemp, item.upperTemp);
            }
        }
    }

    for (auto policy : clusterPolicyMap_) {
        if (policy.first.empty() && policy.second.empty()) continue;
        THERMAL_HILOGD(COMP_SVC, "clusterName = %{public}s", policy.first.c_str());
        for (auto policyConfig : policy.second) {
            THERMAL_HILOGD(COMP_SVC, "level = %{public}d", policyConfig.level);
            for (auto action : policyConfig.vPolicyAction) {
                THERMAL_HILOGD(COMP_SVC, "actionName = %{public}s, actionValue = %{public}s, prop = %{public}d",
                    action.actionName.c_str(), action.actionValue.c_str(), action.isProp);
                for (auto prop : action.mActionProp) {
                    THERMAL_HILOGD(COMP_SVC, "propName = %{public}s, propValue = %{public}s",
                        prop.first.c_str(), prop.second.c_str());
                }
            }
        }
    }
}
} // namespace PowerMgr
} // namespace OHOS
