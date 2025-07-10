/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "thermal_level_callback.h"
#include "thermal_log.h"

namespace OHOS {
namespace PowerMgr {
ThermalLevelCallback::~ThermalLevelCallback() {}

void ThermalLevelCallback::UpdateCallback(callback_view<void(ohos::thermal::ThermalLevel)> thermalCb)
{
    std::lock_guard lock(mutex_);
    callback_ = optional<callback<void(ohos::thermal::ThermalLevel)>>{std::in_place, thermalCb};
}

bool ThermalLevelCallback::OnThermalLevelChanged(OHOS::PowerMgr::ThermalLevel level)
{
    std::lock_guard lock(mutex_);
    level_ = level;
    OnThermalLevel();
    return true;
}

void ThermalLevelCallback::OnThermalLevel()
{
    THERMAL_HILOGD(COMP_FWK, "level is: %{public}d", static_cast<int32_t>(level_));
    ohos::thermal::ThermalLevel levelValue = static_cast<ohos::thermal::ThermalLevel::key_t>(level_);
    if (callback_.has_value()) {
        callback_.value()(levelValue);
    }
}
} // namespace PowerMgr
} // namespace OHOS