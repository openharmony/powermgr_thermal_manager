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
std::string mitigationNodeDir = "/data/thermal/%s";
std::string mitigationActionFile = "%s/%s";
std::string mitigationDir = "/data/thermal";
std::string stateDir = "/data/thermal/%s";
std::string stateFilePath = "%s/%s";
const int MAX_PATH = 256;
const int ARG_1 = 1;
const int ARG_3 = 3;
const int ARG_4 = 4;
}
bool ThermalActionManager::Init()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s ThermalActionManager enter", __func__);

    for (auto item = vActionItem_.begin(); item != vActionItem_.end(); ++item) {
        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s ThermalActionManager name = %{public}s",
            __func__, item->name.c_str());
        std::shared_ptr<IThermalAction> thermalAction = ThermalActionFactory::Create(item->name);
        thermalAction->InitParams(item->params);
        thermalAction->SetStrict(item->strict);
        actionMap_.emplace(std::pair(item->name, thermalAction));
    }

    if (actionThermalLevel_ == nullptr) {
        actionThermalLevel_ = std::make_shared<ActionThermalLevel>();
        if (!actionThermalLevel_->Init()) {
            THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "%{public}s failed to create level action", __func__);
        }
    }
    CreateActionMockFile();
    return true;
}

void ThermalActionManager::SubscribeThermalLevelCallback(const sptr<IThermalLevelCallback> &callback)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    if (actionThermalLevel_ != nullptr) {
        actionThermalLevel_->SubscribeThermalLevelCallback(callback);
    }
}

void ThermalActionManager::UnSubscribeThermalLevelCallback(const sptr<IThermalLevelCallback> &callback)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    if (actionThermalLevel_ != nullptr) {
        actionThermalLevel_->UnSubscribeThermalLevelCallback(callback);
    }
}

uint32_t ThermalActionManager::GetThermalLevel()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    return actionThermalLevel_->GetThermalLevel();
}

int32_t ThermalActionManager::CreateActionMockFile()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    char nodeBuf[MAX_PATH] = {0};
    char fileBuf[MAX_PATH] = {0};
    char stateBuf[MAX_PATH] = {0};
    char stateFileBuf[MAX_PATH] = {0};
    std::string action = "config";
    std::string state = "state";
    std::string actionValue[] = {"lcd", "process_ctrl", "configLevel", "shut_down"};
    std::vector<std::string> actionValueList(actionValue, actionValue + ARG_4);
    std::string stateValue[] = {"scene", "screen", "charge"};
    std::vector<std::string> stateValueList(stateValue, stateValue + ARG_3);
    int32_t ret = -1;
    FileOperation::CreateNodeDir(static_cast<std::string>(mitigationDir));
    ret = snprintf_s(nodeBuf, PATH_MAX, sizeof(nodeBuf) - ARG_1, mitigationNodeDir.c_str(), action.c_str());
    if (ret < ERR_OK) {
        return ret;
    }
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s start create dir nodeBuf=%{public}s", __func__, nodeBuf);
    FileOperation::CreateNodeDir(static_cast<std::string>(nodeBuf));

    for (auto iter : actionValueList) {
        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s start create file", __func__);
        ret = snprintf_s(fileBuf, PATH_MAX, sizeof(fileBuf) - ARG_1, mitigationActionFile.c_str(), nodeBuf,
            iter.c_str());
        if (ret < ERR_OK) {
            return ret;
        }
        FileOperation::CreateNodeFile(static_cast<std::string>(fileBuf));
    }

    ret = snprintf_s(stateBuf, PATH_MAX, sizeof(stateBuf) - ARG_1, stateDir.c_str(), state.c_str());
    if (ret < ERR_OK) {
        return ret;
    }
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s start create dir nodeBuf=%{public}s", __func__, stateBuf);
    FileOperation::CreateNodeDir(static_cast<std::string>(stateBuf));

    for (auto iter : stateValueList) {
        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s start create file", __func__);
        ret = snprintf_s(stateFileBuf, PATH_MAX, sizeof(stateFileBuf) - ARG_1, stateFilePath.c_str(), stateBuf,
            iter.c_str());
        if (ret < ERR_OK) {
            return ret;
        }
        FileOperation::CreateNodeFile(static_cast<std::string>(stateFileBuf));
    }
    return ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS