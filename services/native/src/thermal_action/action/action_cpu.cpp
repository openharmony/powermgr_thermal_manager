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

#include "action_cpu.h"

#include "thermal_service.h"

namespace OHOS {
namespace PowerMgr {
namespace {
auto g_service = DelayedSpSingleton<ThermalService>::GetInstance();
}
bool ActionCpu::InitParams(const std::string &params)
{
    params_ = params;
    return true;
}

void ActionCpu::SetStrict(bool flag)
{
    flag_ = flag;
}

void ActionCpu::AddActionValue(std::string value)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, " %{public}s value=%{public}s", __func__, value.c_str());
    if (value.empty()) return;
    valueList_.push_back(atoi(value.c_str()));
}

void ActionCpu::Execute()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, " %{public}s enter", __func__);
    int value = lastValue_;
    if (valueList_.empty()) {
        value = 0;
    } else {
        if (flag_) {
            value = *max_element(valueList_.begin(), valueList_.end());
        } else {
            value = *min_element(valueList_.begin(), valueList_.end());
        }
        valueList_.clear();
    }

    if (value != lastValue_) {
        CpuRuquest(value);
        lastValue_ = value;
    }
}

int32_t ActionCpu::CpuRuquest(uint32_t freq)
{
    THERMAL_HILOGD(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    auto thermalInterface = g_service->GetThermalInterface();
    if (thermalInterface != nullptr) {
        int32_t ret = thermalInterface->SetCpuFreq(freq);
        if (ret != ERR_OK) {
            THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "%{public}s failed to set cpu freq", __func__);
            return ret;
        }
    }
    return ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS