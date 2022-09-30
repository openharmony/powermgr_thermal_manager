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
#include "current_action.h"

#include <iosfwd>
#include <string>
#include <climits>
#include "errors.h"
#include "securec.h"
#include "thermal_log.h"
#include "thermal_protector_util.h"

namespace OHOS {
namespace PowerMgr {
namespace {
const std::string SIM_BATTERY_CURRENT_PATH = "/data/service/el0/thermal/cooling/battery/current";
constexpr int32_t MAX_PATH = 256;
}
bool CurrentAction::AddActionValue(uint32_t value)
{
    THERMAL_HILOGD(FEATURE_PROTECTOR, "Enter");
    latestvalue_ = value;
    return true;
}

void CurrentAction::Execute()
{
    static uint32_t value;
    if (value != latestvalue_) {
        if (BatteryCurrentActionRequest(latestvalue_) != ERR_OK) {
            THERMAL_HILOGE(FEATURE_PROTECTOR, "failed to set battery current");
        }
        value = latestvalue_;
    } else {
        latestvalue_ = 0;
    }
}

int32_t CurrentAction::BatteryCurrentActionRequest(uint32_t current)
{
    THERMAL_HILOGD(FEATURE_PROTECTOR, "%{public}d", current);
    char currentBuf[MAX_PATH] = {0};
    if (snprintf_s(currentBuf, MAX_PATH, sizeof(currentBuf) - 1, SIM_BATTERY_CURRENT_PATH.c_str()) < EOK) {
        return ERR_INVALID_VALUE;
    }
    std::string currentStr = std::to_string(current);
    return ThermalProtectorUtil::WriteFile(currentBuf, currentStr, currentStr.length());
}
} // namespace PowerMgr
} // namespace OHOS
