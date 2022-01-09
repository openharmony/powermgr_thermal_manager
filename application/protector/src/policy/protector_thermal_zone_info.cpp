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

#include "protector_thermal_zone_info.h"
#include "thermal_common.h"

namespace OHOS {
namespace PowerMgr {
bool ProtectorThermalZoneInfo::Init()
{
    return true;
}

void ProtectorThermalZoneInfo::UpdateThermalLevel(int32_t curTemp)
{
    THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s enter", __func__);
    uint32_t level = latestLevel_;
    if (desc_) {
        DescJudgment(curTemp, level);
    } else {
        AscJudgment(curTemp, level);
    }
}

void ProtectorThermalZoneInfo::AscJudgment(int32_t curTemp, uint32_t &level)
{
    THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s enter", __func__);
    if (level > 0 && level < vtzi_.size()) {
        int32_t curDownTemp = vtzi_.at(level - 1).thresholdClr;
        int32_t nextUptemp = vtzi_.at(level).threshold;
        if (curTemp >= nextUptemp) {
            for (uint32_t i = level; i < vtzi_.size(); i++) {
                if (curTemp >= vtzi_.at(i).threshold) {
                    level = vtzi_.at(i).level;
                } else {
                    break;
                }
            }
            THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s first level = %{public}d", __func__, level);
        } else if (curTemp < curDownTemp) {
            for (uint32_t i = level; i >= 1; i--) {
                if (curTemp < vtzi_.at(i - 1).thresholdClr) {
                    level = vtzi_.at(i - 1).level - 1;
                } else {
                    break;
                }
            }
            THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s second level = %{public}d", __func__, level);
        } else {
            level = vtzi_.at(level - 1).level;
            THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s third level = %{public}d", __func__, level);
        }
    } else if (level == vtzi_.size()) {
        int32_t curDownTemp = vtzi_.at(level - 1).thresholdClr;
        if (curTemp < curDownTemp) {
            for (uint32_t i = level; i >= 1; i--) {
                if (curTemp < vtzi_.at(i - 1).thresholdClr) {
                    level = vtzi_.at(i - 1).level - 1;
                } else {
                    break;
                }
            }
        }
        THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s fourth level = %{public}d", __func__, level);
    } else if (level == 0)  {
        int32_t nextUptemp = vtzi_.at(level).threshold;
        if (curTemp >= nextUptemp) {
            for (uint32_t i = level; i < vtzi_.size(); i++) {
                if (curTemp >= vtzi_.at(i).threshold) {
                    level = vtzi_.at(i).level;
                } else {
                    break;
                }
            }
        }
        THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s fifth level = %{public}d", __func__, level);
    }
}

void ProtectorThermalZoneInfo::DescJudgment(int32_t curTemp, uint32_t &level)
{
    THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s enter curTemp %{public}d, size = %{public}d",
        __func__, curTemp, vtzi_.size());
    level = latestLevel_;
    if (level != 0 && level < vtzi_.size()) {
        int32_t curDownTemp = vtzi_.at(level - 1).thresholdClr;
        int32_t nextUptemp = vtzi_.at(level).threshold;
        if (curTemp <= nextUptemp) {
            for (uint32_t i = level; i < vtzi_.size(); i++) {
                if (curTemp <= vtzi_.at(i).threshold) {
                    level = vtzi_.at(i).level;
                } else {
                    break;
                }
            }
            THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s first level = %{public}d", __func__, level);
        } else if (curTemp > curDownTemp) {
            for (uint32_t i = level; i >= 1; i--) {
                if (curTemp > vtzi_.at(i - 1).thresholdClr) {
                    level = vtzi_.at(i - 1).level - 1;
                } else {
                    break;
                }
            }
            THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s second level = %{public}d", __func__, level);
        } else {
            level = vtzi_.at(level - 1).level;
            THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s third level = %{public}d", __func__, level);
        }
    } else if (level == vtzi_.size()) {
        int32_t curDownTemp = vtzi_.at(level - 1).thresholdClr;
        if (curTemp > curDownTemp) {
            for (uint32_t i = level; i >= 1; i--) {
                if (curTemp > vtzi_.at(i - 1).thresholdClr) {
                    level = vtzi_.at(i - 1).level - 1;
                } else {
                    break;
                }
            }
        }
        THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s fourth level = %{public}d", __func__, level);
    } else if (level == 0) {
        int32_t nextUptemp = vtzi_.at(level).threshold;
        if (curTemp <= nextUptemp) {
            for (uint32_t i = level; i < vtzi_.size(); i++) {
                if (curTemp <= vtzi_.at(i).threshold) {
                    level = vtzi_.at(i).level;
                } else {
                    break;
                }
            }
        }
        THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s fifth level = %{public}d", __func__, level);
    }
    THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s return", __func__);
}

void ProtectorThermalZoneInfo::SetThermalZoneItem(std::vector<ThermalZoneInfoItem> &vtzi)
{
    vtzi_ = vtzi;
}

void ProtectorThermalZoneInfo::SetDesc(bool desc)
{
    desc_ = desc;
}

void ProtectorThermalZoneInfo::Dump()
{
    THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s enter", __func__);
    for (auto infoIter : vtzi_) {
        THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR,
            "%{public}s level = %{public}d, threshold = %{public}d, thresholdClr = %{public}d",
            __func__, infoIter.level, infoIter.threshold, infoIter.thresholdClr);
    }
}
} // namespace PowerMgr
} // namespace OHOS