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
    sensorsType_.clear();
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
        std::vector<std::string> countSizes;
        StringOperation::SplitString(countIter->second, countSizes, ",");
        const uint32_t INDEX0 = 0;
        const uint32_t INDEX1 = 1;
        StringOperation::StrToUint(countSizes[INDEX0], historyTempCount_);
        if (countSizes.size() > INDEX1) {
            StringOperation::StrToUint(countSizes[INDEX1], riseRateCount_);
        }
    }
}

void ThermalConfigBaseInfo::Dump()
{
    THERMAL_HILOGD(COMP_SVC, "type: %{public}s, count: %{public}d,%{public}d.",
        sensorType_.c_str(), historyTempCount_, riseRateCount_);
    uint32_t id = 0;
    for (auto type : sensorsType_) {
        THERMAL_HILOGI(COMP_SVC, "Dump: id=%{public}u, sensorType = %{public}s", id++, type.c_str());
    }
}
} // namespace PowerMgr
} // namespace OHOS
