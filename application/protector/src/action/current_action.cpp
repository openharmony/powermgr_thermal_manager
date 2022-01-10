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

#include "thermal_common.h"

namespace OHOS {
namespace PowerMgr {
bool CurrentAction::AddActionValue(uint32_t value)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    value_ = value;
    return true;
}

void CurrentAction::Execute()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    if (BatteryCurrentActionRequest(value_) != ERR_OK) {
        THERMAL_HILOGE(MODULE_THERMAL_PROTECTOR, "failed to set current");
    }
}

int32_t CurrentAction::BatteryCurrentActionRequest(uint32_t current)
{
    THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}d", current);
    uint32_t ret = -1;
    ret = this->BatteryCurrentRequest(current);
    if (ret != ERR_OK) {
        THERMAL_HILOGE(MODULE_THERMAL_PROTECTOR, "failed to set current");
        return ret;
    }
    return ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS