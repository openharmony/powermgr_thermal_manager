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
#include "config_policy_utils.h"

namespace OHOS {
namespace PowerMgr {
namespace {
constexpr const char* THERMAL_KERNEL_CONFIG_PATH = "etc/thermal_config/thermal_kernel_config.xml";
constexpr const char* VENDOR_THERMAL_KERNEL_CONFIG_PATH = "/vendor/etc/thermal_config/thermal_kernel_config.xml";
constexpr const char* SYSTEM_THERMAL_KERNEL_CONFIG_PATH = "/system/etc/thermal_config/thermal_kernel_config.xml";
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

    char buf[MAX_PATH_LEN];
    bool parseConfigSuc = false;
    char* path = GetOneCfgFile(THERMAL_KERNEL_CONFIG_PATH, buf, MAX_PATH_LEN);
    if (path != nullptr && *path != '\0') {
        if (!ThermalKernelConfigFile::GetInstance().Init(path)) {
            THERMAL_HILOGE(FEATURE_PROTECTOR, "failed to parse config");
            return false;
        }
        parseConfigSuc = true;
    }

    if (!parseConfigSuc) {
        if (!ThermalKernelConfigFile::GetInstance().Init(VENDOR_THERMAL_KERNEL_CONFIG_PATH)) {
            THERMAL_HILOGE(FEATURE_PROTECTOR, "failed to parse vendor config");
            if (!ThermalKernelConfigFile::GetInstance().Init(SYSTEM_THERMAL_KERNEL_CONFIG_PATH)) {
                THERMAL_HILOGE(FEATURE_PROTECTOR, "failed to parse system config");
                return false;
            }
        }
    }

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
