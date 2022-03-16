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

#ifndef THERMAL_KERNEL_SERVICE_H
#define THERMAL_KERNEL_SERVICE_H

#include <memory>
#include "thermal_kernel_policy.h"
#include "thermal_sensor_provision.h"
#include "thermal_device_control.h"
#include "thermal_protector_timer.h"

namespace OHOS {
namespace PowerMgr {
class ThermalKernelService {
public:
    ThermalKernelService() {};
    ~ThermalKernelService() = default;
    ThermalKernelService(const ThermalKernelService&) = delete;
    ThermalKernelService& operator=(const ThermalKernelService&) = delete;
    static ThermalKernelService &GetInstance()
    {
        static ThermalKernelService instance;
        return instance;
    }

    void OnStart();
    void OnStop();

    std::shared_ptr<ThermalKernelPolicy> GetPolicy() const
    {
        return policy_;
    }

    std::shared_ptr<ThermalSensorProvision> GetProvision() const
    {
        return provision_;
    }

    std::shared_ptr<ThermalDeviceControl> GetControl() const
    {
        return control_;
    }

    std::shared_ptr<ThermalProtectorTimer> GetTimer() const
    {
        return timer_;
    }
private:
    bool Init();
    std::shared_ptr<ThermalKernelPolicy> policy_ {nullptr};
    std::shared_ptr<ThermalSensorProvision> provision_ {nullptr};
    std::shared_ptr<ThermalDeviceControl> control_ {nullptr};
    std::shared_ptr<ThermalProtectorTimer> timer_ {nullptr};
};
} // namespace PowerMgr
} // namespace OHOS
#endif
