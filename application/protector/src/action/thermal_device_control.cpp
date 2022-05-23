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
#include <set>
#include "thermal_common.h"
#include "device_control_factory.h"
#include "thermal_kernel_service.h"

namespace OHOS {
namespace PowerMgr {
namespace {
auto &service = ThermalKernelService::GetInstance();
}
bool ThermalDeviceControl::Init()
{
    THERMAL_HILOGD(FEATURE_PROTECTOR, "Enter");
    auto policy = service.GetPolicy();
    if (policy == nullptr) {
        return false;
    }

    auto actionList = policy->GetLevelAction();
    if (actionList.empty()) {
        return false;
    }

    std::set<std::string> actionNameSet;
    for (auto &level : actionList) {
        for (auto &action : level.vAction) {
            actionNameSet.insert(action.name);
        }
    }

    for (auto &iter : actionNameSet) {
        thermalActionMap_.insert(std::make_pair(iter, DeviceControlFactory::Create(iter)));
    }
    return true;
}
} // namespace PowerMgr
} // namespace OHOs