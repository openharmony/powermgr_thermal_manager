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

#include "action_display.h"

#include "display_power_mgr_client.h"
#include "file_operation.h"
#include "thermal_hisysevent.h"
#include "thermal_service.h"

using namespace OHOS::DisplayPowerMgr;
namespace OHOS {
namespace PowerMgr {
namespace {
auto g_service = DelayedSpSingleton<ThermalService>::GetInstance();
const std::string LCD_MOCK_PATH = "/data/service/el0/thermal/config/lcd";
constexpr int32_t DEFAULT_FALLBACK_VALUE = 100;
constexpr float PERCENT_UNIT = 100.0;
}
ActionDisplay::ActionDisplay(const std::string& actionName)
{
    actionName_ = actionName;
}

void ActionDisplay::InitParams(const std::string& params)
{
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
    valueList_.push_back(atoi(value.c_str()));
}

void ActionDisplay::Execute()
{
    THERMAL_RETURN_IF (g_service == nullptr);

    int32_t value = DEFAULT_FALLBACK_VALUE;
    if (!valueList_.empty()) {
        if (isStrict_) {
            value = *min_element(valueList_.begin(), valueList_.end());
        } else {
            value = *max_element(valueList_.begin(), valueList_.end());
        }
        valueList_.clear();
    }

    if (value != lastValue_) {
        if (!g_service->GetSimulationXml()) {
            RequestDisplay(static_cast<float>(value) / PERCENT_UNIT);
        } else {
            ExecuteMock(value);
        }
        WriteActionTriggeredHiSysEvent(enableEvent_, actionName_, value);
        lastValue_ = value;
    }
}

void ActionDisplay::RequestDisplay(float factor)
{
    if (!DisplayPowerMgrClient::GetInstance().DiscountBrightness(factor)) {
        THERMAL_HILOGE(COMP_SVC, "failed to discount brightness");
        return;
    }
    THERMAL_HILOGI(COMP_SVC, "action execute: {%{public}s = %{public}f}", actionName_.c_str(), factor);
}

void ActionDisplay::ExecuteMock(int32_t factor)
{
    std::string valueString = std::to_string(factor) + "\n";
    FileOperation::WriteFile(LCD_MOCK_PATH, valueString, valueString.length());
}
} // namespace PowerMgr
} // namespace OHOS
