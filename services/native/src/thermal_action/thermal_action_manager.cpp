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

#include "thermal_action_factory.h"

namespace OHOS {
namespace PowerMgr {
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

    if (popup_ == nullptr) {
        popup_ = std::make_shared<ActionPopup>();
    }
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
} // namespace PowerMgr
} // namespace OHOS