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

#ifndef THERMAL_DEVICE_CONTROL_H
#define THERMAL_DEVICE_CONTROL_H

#include <map>
#include <memory>
#include <string>

#include "ithermal_action.h"

namespace OHOS {
namespace PowerMgr {
class ThermalDeviceControl {
public:
    using ThermalActionMap = std::map<std::string, std::shared_ptr<IThermalAction>>;
    ThermalDeviceControl() {};
    ~ThermalDeviceControl() {};
    bool Init();
    ThermalActionMap GetThermalAction()
    {
        return thermalActionMap_;
    }
private:
    ThermalActionMap thermalActionMap_;
    std::shared_ptr<IThermalAction> thermalAction_;
    std::string preName_;
};
} // namespace PowerMgr
} // namespace OHOs

#endif // THERMAL_DEVICE_CONTROL_H