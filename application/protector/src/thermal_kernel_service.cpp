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

#include "thermal_kernel_service.h"
#include "thermal_common.h"
#include "thermal_kernel_config_file.h"


namespace OHOS {
namespace PowerMgr {
namespace {
constexpr const char* CONFIG_FILE_PATH = "/system/etc/thermal_config/thermal_kernel_config.xml";
}
void ThermalKernelService::OnStart()
{
    if (!Init()) {
        THERMAL_HILOGE(FEATURE_PROTECTOR, "failed to init service");
    }
}

bool ThermalKernelService::Init()
{
    if (provision_ == nullptr) {
        provision_ = std::make_shared<ThermalSensorProvision>();
    }

    if (control_ == nullptr) {
        control_ = std::make_shared<ThermalDeviceControl>();
    }

    if (policy_ == nullptr) {
        policy_ = std::make_shared<ThermalKernelPolicy>();
    }

    if (timer_ == nullptr) {
        timer_ = std::make_shared<ThermalProtectorTimer>(provision_);
    }

    ThermalKernelConfigFile::GetInstance().Init(CONFIG_FILE_PATH);

    if (!policy_->Init()) {
        THERMAL_HILOGE(FEATURE_PROTECTOR, "failed to init policy");
        return false;
    }

    if (!control_->Init()) {
        THERMAL_HILOGE(FEATURE_PROTECTOR, "failed to init device control");
        return false;
    }

    provision_->InitProvision();

    timer_->Init();
    return true;
}
} // namespace PowerMgr
} // namespace OHOS
