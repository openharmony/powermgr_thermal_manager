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
constexpr const char* LEVEL_PATH = "/data/service/el0/thermal/config/configLevel";
const int MAX_PATH = 256;
}

bool ThermalPolicy::Init()
{
    SortLevel();
    if (g_service == nullptr) {
        return false;
    }
    handler_ = g_service->GetHandler();
    RegisterObserver();
    return true;
}

void ThermalPolicy::OnSensorInfoReported(const TypeTempMap& info)
{
    typeTempMap_ = info;

    LevelDecision();
    WriteLevel();
    PolicyDecision();
}

void ThermalPolicy::SetPolicyMap(PolicyConfigMap& pcm)
{
    clusterPolicyMap_ = pcm;
}

void ThermalPolicy::SetSensorClusterMap(SensorClusterMap& scm)
{
    sensorClusterMap_ = scm;
}

std::map<std::string, uint32_t> ThermalPolicy::GetClusterLevelMap()
{
    return clusterLevelMap_;
}


void ThermalPolicy::SortLevel()
{
    for (auto clusterPolicy = clusterPolicyMap_.begin(); clusterPolicy != clusterPolicyMap_.end(); clusterPolicy++) {
        sort(clusterPolicy->second.begin(), clusterPolicy->second.end(), LevelCompare);
    }
}

void ThermalPolicy::RegisterObserver()
{
    ThermalObserver::Callback callback = std::bind(&ThermalPolicy::OnSensorInfoReported, this, std::placeholders::_1);
    g_service->GetObserver()->SetRegisterCallback(callback);
}

void ThermalPolicy::LevelDecision()
{
    for (auto cluster = sensorClusterMap_.begin(); cluster != sensorClusterMap_.end(); cluster++) {
        THERMAL_HILOGD(COMP_SVC, "update [%{public}s] level", cluster->first.c_str());
        cluster->second->UpdateThermalLevel(typeTempMap_);
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

void ThermalPolicy::PolicyDecision()
{
    for (auto clusterPolicy = clusterPolicyMap_.begin(); clusterPolicy != clusterPolicyMap_.end(); clusterPolicy++) {
        const std::string& clusterName = clusterPolicy->first;
        const std::vector<PolicyConfig>& policyConfig = clusterPolicy->second;
        if (clusterName.empty() || policyConfig.empty()) {
            continue;
        }
        auto clusterIter = clusterLevelMap_.find(clusterName);
        if (clusterIter == clusterLevelMap_.end()) {
            continue;
        }
        uint32_t clusterCurrLev = clusterIter->second;
        for (auto levelAction = policyConfig.rbegin(); levelAction != policyConfig.rend(); levelAction++) {
            if (clusterCurrLev >= levelAction->level) {
                ActionDecision(levelAction->policyActionList);
                break;
            }
        }
    }

    PrintPolicyState();

    if (!ActionExecution()) {
        THERMAL_HILOGW(COMP_SVC, "failed to execute action");
        return;
    }

    ActionCharger::ExecuteCurrentLimit();
    ActionVoltage::ExecuteVoltageLimit();
}

void ThermalPolicy::ActionDecision(const std::vector<PolicyAction>& actionList)
{
    for (auto action = actionList.begin(); action != actionList.end(); action++) {
        ThermalActionManager::ThermalActionMap actionMap = g_service->GetActionManagerObj()->GetActionMap();
        auto actionIter = actionMap.find(action->actionName);
        if (actionIter == actionMap.end() || actionIter->second == nullptr) {
            THERMAL_HILOGE(COMP_SVC, "can't find action [%{public}s] ability", action->actionName.c_str());
            continue;
        }
        if (action->isProp) {
            if (StateMachineDecision(action->actionPropMap)) {
                actionIter->second->AddActionValue(action->actionValue);
                actionIter->second->SetXmlScene(action->actionPropMap.begin()->second, action->actionValue);
            }
        } else {
            actionIter->second->AddActionValue(action->actionValue);
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

    THERMAL_HILOGI(COMP_SVC, "g_service->GetObserver() is %{private}p", g_service->GetObserver().get());
    g_service->GetObserver()->FindSubscribeActionValue();
}

bool ThermalPolicy::StateMachineDecision(const std::map<std::string, std::string>& stateMap)
{
    for (auto prop = stateMap.begin(); prop != stateMap.end(); prop++) {
        StateMachine::StateMachineMap stateMachineMap = g_service->GetStateMachineObj()->GetStateCollectionMap();
        auto stateIter = stateMachineMap.find(prop->first);
        if (stateIter == stateMachineMap.end() || stateIter->second == nullptr) {
            THERMAL_HILOGE(COMP_SVC, "can't find state machine [%{public}s]", prop->first.c_str());
            return false;
        }
        if (stateIter->second->DecideState(prop->second)) {
            continue;
        } else {
            return false;
        }
    }
    return true;
}

bool ThermalPolicy::ActionExecution()
{
    auto actionMgr = g_service->GetActionManagerObj();
    if (actionMgr == nullptr) {
        return false;
    }

    ThermalActionManager::ThermalActionMap actionMap = actionMgr->GetActionMap();
    for (auto iter = actionMap.begin(); iter != actionMap.end(); iter++) {
        iter->second->Execute();
    }
    handler_->SendEvent(ThermalsrvEventHandler::SEND_ACTION_HUB_LISTENER, 0, 0);
    return true;
}

void ThermalPolicy::PrintPolicyState()
{
    std::string levInfo = "";
    for (auto clusterIter = clusterLevelMap_.begin(); clusterIter != clusterLevelMap_.end(); clusterIter++) {
        levInfo.append(clusterIter->first).append("-").append(std::to_string(clusterIter->second)).append(" ");
    }
    THERMAL_HILOGD(COMP_SVC, "current level: %{public}s", levInfo.c_str());
}
} // namespace PowerMgr
} // namespace OHOS
