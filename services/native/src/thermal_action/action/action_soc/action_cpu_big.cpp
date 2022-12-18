/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "action_cpu_big.h"

#include "thermal_hisysevent.h"

namespace OHOS {
namespace PowerMgr {
namespace {
constexpr int32_t LIM_CPU_BIG_ID = 1005;
}
ActionCpuBig::ActionCpuBig(const std::string& actionName)
{
    actionName_ = actionName;
}

void ActionCpuBig::InitParams(const std::string& params)
{
}

void ActionCpuBig::SetStrict(bool enable)
{
    isStrict_ = enable;
}

void ActionCpuBig::SetEnableEvent(bool enable)
{
    enableEvent_ = enable;
}

void ActionCpuBig::AddActionValue(std::string value)
{
    valueList_.push_back(atoi(value.c_str()));
}

void ActionCpuBig::Execute()
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
        SocLimitRequest(LIM_CPU_BIG_ID, value);
        WriteActionTriggeredHiSysEvent(enableEvent_, actionName_, value);
        lastValue_ = value;
        THERMAL_HILOGI(COMP_SVC, "action execute: {%{public}s = %{public}u}", actionName_.c_str(), lastValue_);
    }
}
} // namespace PowerMgr
} // namespace OHOS
