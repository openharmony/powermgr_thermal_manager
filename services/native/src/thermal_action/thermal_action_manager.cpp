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

#include "thermal_action_manager.h"

#include "constants.h"
#include "file_operation.h"
#include "thermal_action_factory.h"
#include "securec.h"
#include "string_operation.h"

namespace OHOS {
namespace PowerMgr {
namespace {
const int MAX_PATH = 256;
}
bool ThermalActionManager::Init()
{
    ThermalActionFactory::InitFactory();
    for (auto item = vActionItem_.begin(); item != vActionItem_.end(); ++item) {
        std::shared_ptr<IThermalAction> thermalAction = nullptr;
        if (item->name == THERMAL_LEVEL_NAME) {
            actionThermalLevel_ = std::make_shared<ActionThermalLevel>(THERMAL_LEVEL_NAME);
            thermalAction = actionThermalLevel_;
        } else if (!item->protocol.empty()) {
            thermalAction = ThermalActionFactory::Create(item->protocol, item->name);
        } else {
            thermalAction = ThermalActionFactory::Create(item->name, item->name);
        }
        if (thermalAction == nullptr) {
            THERMAL_HILOGE(COMP_SVC, "failed to create action, name: %{public}s", item->name.c_str());
            continue;
        }
        thermalAction->InitParams(item->params);
        thermalAction->SetStrict(item->strict);
        thermalAction->SetEnableEvent(item->enableEvent);
        actionMap_.emplace(item->name, thermalAction);
        THERMAL_HILOGI(COMP_SVC, "add action, name: %{public}s", item->name.c_str());
    }

    CreateActionMockFile();
    return true;
}

void ThermalActionManager::SubscribeThermalLevelCallback(const sptr<IThermalLevelCallback> &callback)
{
    if (actionThermalLevel_ != nullptr) {
        actionThermalLevel_->SubscribeThermalLevelCallback(callback);
    } else {
        THERMAL_HILOGE(COMP_SVC, "thermal level action is uninitialized");
    }
}

void ThermalActionManager::UnSubscribeThermalLevelCallback(const sptr<IThermalLevelCallback> &callback)
{
    if (actionThermalLevel_ != nullptr) {
        actionThermalLevel_->UnSubscribeThermalLevelCallback(callback);
    } else {
        THERMAL_HILOGE(COMP_SVC, "thermal level action is uninitialized");
    }
}

uint32_t ThermalActionManager::GetThermalLevel()
{
    if (actionThermalLevel_ != nullptr) {
        return actionThermalLevel_->GetThermalLevel();
    }
    return 0;
}

int32_t ThermalActionManager::CreateActionMockFile()
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    std::string configDir = "/data/service/el0/thermal/config/%s";
    std::string stateDir = "/data/service/el0/thermal/state/%s";
    char fileBuf[MAX_PATH] = {0};
    char stateFileBuf[MAX_PATH] = {0};
    std::vector<std::string> actionValueList = {"lcd", "process_ctrl", "configLevel", "shut_down", "sc_current",
        "buck_current", "sc_voltage", "buck_voltage"};
    std::vector<std::string> stateValueList = {"scene", "screen", "charge"};
    int32_t ret;
    for (auto iter : actionValueList) {
        THERMAL_HILOGD(COMP_SVC, "start create file");
        ret = snprintf_s(fileBuf, MAX_PATH, sizeof(fileBuf) - ARG_1, configDir.c_str(), iter.c_str());
        if (ret < EOK) {
            return ret;
        }
        FileOperation::CreateNodeFile(static_cast<std::string>(fileBuf));
    }

    for (auto iter : stateValueList) {
        THERMAL_HILOGD(COMP_SVC, "start create file");
        ret = snprintf_s(stateFileBuf, MAX_PATH, sizeof(stateFileBuf) - ARG_1, stateDir.c_str(), iter.c_str());
        if (ret < EOK) {
            return ret;
        }
        FileOperation::CreateNodeFile(static_cast<std::string>(stateFileBuf));
    }
    return ERR_OK;
}

void ThermalActionManager::DumpAction(std::string& result)
{
    for (auto iter = vActionItem_.begin(); iter != vActionItem_.end(); ++iter) {
    result.append("name: ");
    result.append(iter->name);
    if (!iter->params.empty()) {
        result.append("\t");
        result.append("params: ");
        result.append(iter->params);
    }
    if (!iter->protocol.empty()) {
        result.append("\t");
        result.append("protocol: ");
        result.append(iter->protocol);
    }
    if (!iter->uid.empty()) {
        result.append("\t");
        result.append("uid: ");
        result.append(iter->uid);
    }
    result.append("\t");
    result.append("strict: ");
    result.append(std::to_string(iter->strict));
    result.append("\t");
    result.append("enableEvent: ");
    result.append(std::to_string(iter->enableEvent));
    result.append("\n");
    }
}

void ThermalActionManager::EnableMock(const std::string& actionName, void* mockAction)
{
    THERMAL_HILOGI(COMP_SVC, "EnableMock [%{public}s] ability", actionName.c_str());
    auto action = static_cast<IThermalAction*>(mockAction);
    auto actionIter = actionMap_.find(actionName);
    if (actionIter == actionMap_.end() || actionIter->second == nullptr) {
        THERMAL_HILOGE(COMP_SVC, "can't find action [%{public}s] ability", actionName.c_str());
        return;
    }
    std::unique_ptr<IThermalAction> mock(action);
    actionIter->second.reset();
    actionIter->second = std::move(mock);
}
} // namespace PowerMgr
} // namespace OHOS
