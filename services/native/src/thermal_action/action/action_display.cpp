/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include <cmath>

#include "constants.h"
#ifdef HAS_THERMAL_DISPLAY_MANAGER_PART
#include "display_power_mgr_client.h"
#endif
#include "file_operation.h"
#include "thermal_hisysevent.h"
#include "thermal_service.h"

#ifdef HAS_THERMAL_DISPLAY_MANAGER_PART
using namespace OHOS::DisplayPowerMgr;
#endif
namespace OHOS {
namespace PowerMgr {
namespace {
const std::string LCD_MOCK_PATH = "/data/service/el0/thermal/config/lcd";
}

ActionDisplay::ActionDisplay(const std::string& actionName)
{
    actionName_ = actionName;
}

void ActionDisplay::InitParams(const std::string& params)
{
    (void)params;
}

void ActionDisplay::SetStrict(bool enable)
{
    isStrict_ = enable;
}

void ActionDisplay::SetEnableEvent(bool enable)
{
    enableEvent_ = enable;
}

void ActionDisplay::AddActionValue(std::string value)
{
    if (value.empty()) {
        return;
    }
    valueList_.push_back(static_cast<float>(strtof(value.c_str(), nullptr)));
}

void ActionDisplay::Execute()
{
    auto tms = ThermalService::GetInstance();
    THERMAL_RETURN_IF (tms == nullptr);
    float value = GetActionValue();
    if (fabs(value - lastValue_) > FLOAT_ACCURACY) {
        if (!tms->GetSimulationXml()) {
            RequestDisplay(value);
        } else {
            ExecuteMock(value);
        }
        WriteActionTriggeredHiSysEventWithRatio(enableEvent_, actionName_, value);
        tms->GetObserver()->SetDecisionValue(actionName_, std::to_string(value));
        lastValue_ = value;
        THERMAL_HILOGD(COMP_SVC, "action execute: {%{public}s = %{public}f}", actionName_.c_str(), lastValue_);
    }
    valueList_.clear();
}

float ActionDisplay::GetActionValue()
{
    float value = FALLBACK_VALUE_FLOAT;
    if (!valueList_.empty()) {
        if (isStrict_) {
            value = *min_element(valueList_.begin(), valueList_.end());
        } else {
            value = *max_element(valueList_.begin(), valueList_.end());
        }
    }
    return value;
}

void ActionDisplay::RequestDisplay(float factor)
{
#ifdef HAS_THERMAL_DISPLAY_MANAGER_PART
    if (!DisplayPowerMgrClient::GetInstance().DiscountBrightness(factor)) {
        THERMAL_HILOGE(COMP_SVC, "failed to discount brightness");
        return;
    }
#endif
}

void ActionDisplay::ExecuteMock(float factor)
{
    std::string valueString = std::to_string(factor) + "\n";
    FileOperation::WriteFile(LCD_MOCK_PATH, valueString, valueString.length());
}
} // namespace PowerMgr
} // namespace OHOS
