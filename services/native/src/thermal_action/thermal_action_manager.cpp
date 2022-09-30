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
    THERMAL_HILOGD(COMP_SVC, "Enter");
    ThermalActionFactory::InitFactory();
    for (auto item = vActionItem_.begin(); item != vActionItem_.end(); ++item) {
        THERMAL_HILOGI(COMP_SVC, "ThermalActionManager name = %{public}s", item->name.c_str());
        if (!item->protocol.empty()) {
            std::vector<std::string> protocolList;
            StringOperation::SplitString(item->protocol, protocolList, ",");
            if (protocolList.empty()) {
                THERMAL_HILOGW(COMP_SVC, "protocolList is empty");
                continue;
            }

            for (auto& iter : protocolList) {
                std::string str = item->name;
                std::string combinedActionName = str.append("_").append(iter.c_str());
                InsertActionMap(combinedActionName, iter, item->strict, item->enableEvent);
            }
        } else {
            InsertActionMap(item->name, item->protocol, item->strict, item->enableEvent);
        }
    }

    if (actionThermalLevel_ == nullptr) {
        actionThermalLevel_ = std::make_shared<ActionThermalLevel>(THERMAL_LEVEL_NAME);
        if (!actionThermalLevel_->Init()) {
            THERMAL_HILOGE(COMP_SVC, "failed to create level action");
        }
    }
    CreateActionMockFile();
    return true;
}

void ThermalActionManager::InsertActionMap(const std::string& actionName, const std::string& protocol, bool strict,
    bool enableEvent)
{
    std::shared_ptr<IThermalAction> thermalAction = ThermalActionFactory::Create(actionName);
    if (thermalAction == nullptr) {
        THERMAL_HILOGE(COMP_SVC, "failed to create action");
        return;
    }
    thermalAction->InitParams(protocol);
    thermalAction->SetStrict(strict);
    thermalAction->SetEnableEvent(enableEvent);
    actionMap_.emplace(actionName, thermalAction);
}

void ThermalActionManager::SubscribeThermalLevelCallback(const sptr<IThermalLevelCallback> &callback)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    if (actionThermalLevel_ != nullptr) {
        actionThermalLevel_->SubscribeThermalLevelCallback(callback);
    }
}

void ThermalActionManager::UnSubscribeThermalLevelCallback(const sptr<IThermalLevelCallback> &callback)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    if (actionThermalLevel_ != nullptr) {
        actionThermalLevel_->UnSubscribeThermalLevelCallback(callback);
    }
}

uint32_t ThermalActionManager::GetThermalLevel()
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    return actionThermalLevel_->GetThermalLevel();
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
} // namespace PowerMgr
} // namespace OHOS
