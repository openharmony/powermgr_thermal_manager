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

#include "thermal_config_base_info.h"
#include "string_operation.h"
#include "thermal_common.h"

namespace OHOS {
namespace PowerMgr {
bool ThermalConfigBaseInfo::Init()
{
    SetSensorsType();
    SetHistoryTempCount();
    Dump();
    return true;
}

void ThermalConfigBaseInfo::SetSensorsType()
{
    auto typesIter = baseItems_.find(SENSORS_TYPE_TAG);
    if (typesIter != baseItems_.end()) {
        sensorType_ = typesIter->second;
        StringOperation::SplitString(sensorType_, sensorsType_, ",");
    }
}

void ThermalConfigBaseInfo::SetHistoryTempCount()
{
    auto countIter = baseItems_.find(HISTORY_TEMP_COUNT_TAG);
    if (countIter != baseItems_.end()) {
        historyTempCount_ = static_cast<uint32_t>(atoi(countIter->second.c_str()));
    }
}

void ThermalConfigBaseInfo::Dump()
{
    THERMAL_HILOGD(COMP_SVC, "type: %{public}s, count: %{public}d.",
        sensorType_.c_str(), historyTempCount_);
    for (auto type : sensorsType_) {
        THERMAL_HILOGI(COMP_SVC, "sensorType = %{public}s", type.c_str());
    }
}
} // namespace PowerMgr
} // namespace OHOS
