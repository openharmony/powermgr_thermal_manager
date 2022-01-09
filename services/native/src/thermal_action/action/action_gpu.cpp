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

#include "action_gpu.h"

#include "thermal_client.h"

namespace OHOS {
namespace PowerMgr {
bool ActionGpu::InitParams(const std::string &params)
{
    return true;
}

void ActionGpu::SetStrict(bool flag)
{
    flag_ = flag;
}

void ActionGpu::AddActionValue(std::string value)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, " %{public}s value=%{public}s", __func__, value.c_str());
    valueList_.push_back(atoi(value.c_str()));
}

void ActionGpu::Execute()
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
        GpuRequest(value);
        lastValue_ = value;
    }
}

int32_t ActionGpu::GpuRequest(uint32_t freq)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    int32_t ret = ThermalClient::SetGPUFreq(freq);
    if (ret != ERR_OK) {
        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s failed to set gpu freq to thermal hdf", __func__);
        return ret;
    }
    return ERR_OK;
}
} // namespace PowermMgr
} // namespace OHOS