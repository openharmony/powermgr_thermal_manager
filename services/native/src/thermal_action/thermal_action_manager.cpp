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

#include "thermal_action_manager.h"

#include "file_operation.h"
#include "thermal_action_factory.h"
#include "securec.h"

namespace OHOS {
namespace PowerMgr {
namespace {
const int MAX_PATH = 256;
const int ARG_1 = 1;
}
bool ThermalActionManager::Init()
{
    THERMAL_HILOGD(COMP_SVC, "Enter");

    for (auto item = vActionItem_.begin(); item != vActionItem_.end(); ++item) {
        THERMAL_HILOGI(COMP_SVC, "ThermalActionManager name = %{public}s", item->name.c_str());
        std::shared_ptr<IThermalAction> thermalAction = ThermalActionFactory::Create(item->name);
        thermalAction->InitParams(item->params);
        thermalAction->SetStrict(item->strict);
        actionMap_.emplace(std::pair(item->name, thermalAction));
    }

    if (actionThermalLevel_ == nullptr) {
        actionThermalLevel_ = std::make_shared<ActionThermalLevel>();
        if (!actionThermalLevel_->Init()) {
            THERMAL_HILOGE(COMP_SVC, "failed to create level action");
        }
    }
    CreateActionMockFile();
    return true;
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
    std::string configDir = "/data/thermal/config/%s";
    std::string stateDir = "/data/thermal/state/%s";
    char fileBuf[MAX_PATH] = {0};
    char stateFileBuf[MAX_PATH] = {0};
    std::string action = "config";
    std::string state = "state";
    std::vector<std::string> actionValueList = {"lcd", "process_ctrl", "configLevel", "shut_down"};
    std::vector<std::string> stateValueList = {"scene", "screen", "charge"};
    int32_t ret = -1;
    for (auto iter : actionValueList) {
        THERMAL_HILOGD(COMP_SVC, "start create file");
        ret = snprintf_s(fileBuf, PATH_MAX, sizeof(fileBuf) - ARG_1, configDir.c_str(), iter.c_str());
        if (ret < ERR_OK) {
            return ret;
        }
        FileOperation::CreateNodeFile(static_cast<std::string>(fileBuf));
    }

    for (auto iter : stateValueList) {
        THERMAL_HILOGD(COMP_SVC, "start create file");
        ret = snprintf_s(stateFileBuf, PATH_MAX, sizeof(stateFileBuf) - ARG_1, stateDir.c_str(), iter.c_str());
        if (ret < ERR_OK) {
            return ret;
        }
        FileOperation::CreateNodeFile(static_cast<std::string>(stateFileBuf));
    }
    return ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS