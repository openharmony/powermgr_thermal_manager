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

#include "action_display.h"

#include "display_power_mgr_client.h"
#include "thermal_service.h"

using namespace OHOS::DisplayPowerMgr;
namespace OHOS {
namespace PowerMgr {
namespace {
auto g_service = DelayedSpSingleton<ThermalService>::GetInstance();
}
bool ActionDisplay::InitParams(const std::string &params)
{
    params_ = params;
    return true;
}

void ActionDisplay::SetStrict(bool flag)
{
    flag_ = flag;
}

void ActionDisplay::AddActionValue(std::string value)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, " %{public}s value=%{public}s", __func__, value.c_str());
    if (value.empty()) return;
    valueList_.push_back(atoi(value.c_str()));
}

void ActionDisplay::Execute()
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
        if (!g_service->GetFlag()) {
            DisplayExecution(value);
        } else {
            DisplayRequest(value);
        }
        lastValue_ = value;
    }
}

uint32_t ActionDisplay::DisplayRequest(uint32_t brightness)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    uint32_t id = 0;
    if (!DisplayPowerMgrClient::GetInstance().SetBrightness(brightness, id)) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "%{public}s failed to set brightness", __func__);
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

uint32_t ActionDisplay::DisplayExecution(uint32_t brightness)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s start to set brightness", __func__);
    return ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS