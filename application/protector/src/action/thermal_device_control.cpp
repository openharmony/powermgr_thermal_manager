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

#include "thermal_device_control.h"
#include "thermal_common.h"
#include "device_control_factory.h"

namespace OHOS {
namespace PowerMgr {
bool ThermalDeviceControl::Init()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    if (CreateInstance()) {
        thermalActionMap_.emplace(std::pair(CPU_ACTION, cpuAction_));
        thermalActionMap_.emplace(std::pair(CURRENT_ACTION, currentAction_));
        thermalActionMap_.emplace(std::pair(VOLTAGE_ACTION, voltageAction_));
    }
    return true;
}

bool ThermalDeviceControl::CreateInstance()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    cpuAction_ = DeviceControlFactory::Create(DeviceControlFactory::MODE_CPU);
    if (!cpuAction_) {
        THERMAL_HILOGE(MODULE_THERMAL_PROTECTOR, "failed to create");
        return false;
    }
    currentAction_ = DeviceControlFactory::Create(DeviceControlFactory::MODE_CURRENT);
    if (!currentAction_) {
        THERMAL_HILOGE(MODULE_THERMAL_PROTECTOR, "failed to create");
        return false;
    }
    voltageAction_ = DeviceControlFactory::Create(DeviceControlFactory::MODE_VOLTAGE);
    if (!voltageAction_) {
        THERMAL_HILOGE(MODULE_THERMAL_PROTECTOR, "failed to create");
        return false;
    }
    return true;
}
} // namespace PowerMgr
} // namespace OHOs