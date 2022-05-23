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

#include "action_charger.h"

#include "thermal_service.h"

namespace OHOS {
namespace PowerMgr {
namespace {
auto g_service = DelayedSpSingleton<ThermalService>::GetInstance();
}
bool ActionCharger::InitParams(const std::string &params)
{
    params_ = params;
    return true;
}

void ActionCharger::SetStrict(bool flag)
{
    flag_ = flag;
}

void ActionCharger::AddActionValue(std::string value)
{
    THERMAL_HILOGD(COMP_SVC, "value=%{public}s", value.c_str());
    if (value.empty()) return;
    valueList_.push_back(atoi(value.c_str()));
}

void ActionCharger::Execute()
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    uint32_t value = lastValue_;
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
        ChargerRequest(value);
        lastValue_ = value;
    }
}

uint32_t ActionCharger::ChargerRequest(uint32_t current)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    auto thermalInterface = g_service->GetThermalInterface();
    if (thermalInterface != nullptr) {
        int32_t ret = thermalInterface->SetBatteryCurrent(current);
        if (ret != ERR_OK) {
            THERMAL_HILOGI(COMP_SVC, "failed to set charger current to thermal hdf");
            return ret;
        }
    }
    return ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS