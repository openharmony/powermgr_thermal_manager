/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "thermal_hisysevent.h"

namespace OHOS {
namespace PowerMgr {
namespace {
constexpr int32_t LIM_GPU_ID = 2001;
}
ActionGpu::ActionGpu(const std::string& actionName)
{
    actionName_ = actionName;
}

void ActionGpu::InitParams(const std::string& params)
{
}

void ActionGpu::SetStrict(bool enable)
{
    isStrict_ = enable;
}

void ActionGpu::SetEnableEvent(bool enable)
{
    enableEvent_ = enable;
}

void ActionGpu::AddActionValue(std::string value)
{
    valueList_.push_back(atoi(value.c_str()));
}

void ActionGpu::Execute()
{
    uint32_t value = fallbackValue;
    if (!valueList_.empty()) {
        if (isStrict_) {
            value = *min_element(valueList_.begin(), valueList_.end());
        } else {
            value = *max_element(valueList_.begin(), valueList_.end());
        }
        valueList_.clear();
    }

    if (value != lastValue_) {
        SocLimitRequest(LIM_GPU_ID, value);
        WriteActionTriggeredHiSysEvent(enableEvent_, actionName_, value);
        lastValue_ = value;
        THERMAL_HILOGI(COMP_SVC, "action execute: {%{public}s = %{public}u}", actionName_.c_str(), lastValue_);
    }
}
} // namespace PowermMgr
} // namespace OHOS
