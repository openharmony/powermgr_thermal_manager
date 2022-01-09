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
#include "cpu_action.h"

#include "thermal_common.h"

namespace OHOS {
namespace PowerMgr {
bool CpuAction::AddActionValue(uint32_t value)
{
    value_ = value;
    return true;
}

void CpuAction::Execute()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    if (CpuActionRequest(value_) != ERR_OK) {
        THERMAL_HILOGE(MODULE_THERMAL_PROTECTOR, "failed to set cpu freq");
    }
}

int32_t CpuAction::CpuActionRequest(uint32_t freq)
{
    THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}d", freq);
    uint32_t ret = -1;
    ret = this->CpuRequest(freq);
    if (ret != ERR_OK) {
        THERMAL_HILOGE(MODULE_THERMAL_PROTECTOR, "failed to set freq");
        return ret;
    }
    return ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS