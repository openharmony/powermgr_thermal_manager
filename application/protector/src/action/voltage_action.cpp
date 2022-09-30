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
#include "voltage_action.h"

#include <iosfwd>
#include <climits>
#include <string>
#include "errors.h"
#include "securec.h"
#include "thermal_log.h"
#include "thermal_protector_util.h"

namespace OHOS {
namespace PowerMgr {
namespace {
constexpr const char* BATTERY_VOLTAGE_PATH = "/data/service/el0/thermal/cooling/battery/voltage";
constexpr int32_t MAX_PATH = 256;
}
bool VoltageAction::AddActionValue(uint32_t value)
{
    THERMAL_HILOGD(FEATURE_PROTECTOR, "Enter");
    latestvalue_ = value;
    return true;
}

void VoltageAction::Execute()
{
    static uint32_t value;
    if (value != latestvalue_) {
        if (BatteryVoltageActionRequest(latestvalue_) != ERR_OK) {
            THERMAL_HILOGW(FEATURE_PROTECTOR, "failed to set battery volatage");
        }
        value = latestvalue_;
    } else {
        latestvalue_ = 0;
    }
}

int32_t VoltageAction::BatteryVoltageActionRequest(uint32_t voltage)
{
    char voltageBuf[MAX_PATH] = {0};
    if (snprintf_s(voltageBuf, MAX_PATH, sizeof(voltageBuf) - 1, BATTERY_VOLTAGE_PATH) < EOK) {
        return ERR_INVALID_VALUE;
    }
    std::string voltageStr = std::to_string(voltage);
    return ThermalProtectorUtil::WriteFile(voltageBuf, voltageStr, voltageStr.length());
}
} // namespace PowerMgr
} // namespace OHOS
