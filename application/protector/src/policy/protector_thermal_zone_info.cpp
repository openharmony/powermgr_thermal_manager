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

#include "thermal_log.h"

namespace OHOS {
namespace PowerMgr {
void ProtectorThermalZoneInfo::UpdateThermalLevel(int32_t curTemp)
{
    THERMAL_HILOGD(FEATURE_PROTECTOR, "Enter");
    uint32_t level = 0;
    if (desc_) {
        DescJudgment(curTemp, level);
    } else {
        AscJudgment(curTemp, level);
    }
    latestLevel_ = level;
}

void ProtectorThermalZoneInfo::AscJudgment(int32_t curTemp, uint32_t &level)
{
    THERMAL_HILOGD(FEATURE_PROTECTOR, "curTemp:%{public}d", curTemp);
    level = latestLevel_;
    if (level > 0 && level < tzItemList_.size()) {
        int32_t curDownTemp = tzItemList_.at(level - 1).thresholdClr;
        int32_t nextUptemp = tzItemList_.at(level).threshold;
        if (curTemp >= nextUptemp) {
            HandleAscNextUpTemp(level, curTemp);
        } else if (curTemp < curDownTemp) {
            HandleAscCurDownTemp(level, curTemp);
        } else {
            level = tzItemList_.at(level - 1).level;
            THERMAL_HILOGI(FEATURE_PROTECTOR, "third level = %{public}d", level);
        }
    } else if (level == tzItemList_.size()) {
        HandleAscMaxSizeTemp(level, curTemp);
    } else if (level == 0)  {
        HandleAscMinSizeTemp(level, curTemp);
    }
}

void ProtectorThermalZoneInfo::DescJudgment(int32_t curTemp, uint32_t &level)
{
    THERMAL_HILOGD(FEATURE_PROTECTOR, "curTemp:%{public}d", curTemp);
    level = latestLevel_;
    if (level > 0 && level < tzItemList_.size()) {
        int32_t curDownTemp = tzItemList_.at(level - 1).thresholdClr;
        int32_t nextUptemp = tzItemList_.at(level).threshold;
        if (curTemp <= nextUptemp) {
            HandleDescNextUpTemp(level, curTemp);
        } else if (curTemp > curDownTemp) {
            HandleDescCurDownTemp(level, curTemp);
        } else {
            level = tzItemList_.at(level - 1).level;
            THERMAL_HILOGI(FEATURE_PROTECTOR, "third level = %{public}d", level);
        }
    } else if (level == tzItemList_.size()) {
        HandleDescMaxSizeTemp(level, curTemp);
    } else if (level == 0) {
        HandleDescMinSizeTemp(level, curTemp);
    }
    THERMAL_HILOGD(FEATURE_PROTECTOR, "Exit");
}

void ProtectorThermalZoneInfo::HandleAscNextUpTemp(uint32_t &level, int32_t curTemp)
{
    for (uint32_t i = level; i < tzItemList_.size(); i++) {
        if (curTemp >= tzItemList_.at(i).threshold) {
            level = tzItemList_.at(i).level;
        } else {
            break;
        }
    }
    THERMAL_HILOGI(FEATURE_PROTECTOR, "first level = %{public}d", level);
}

void ProtectorThermalZoneInfo::HandleAscCurDownTemp(uint32_t &level, int32_t curTemp)
{
    for (uint32_t i = level; i >= 1; i--) {
        if (curTemp < tzItemList_.at(i - 1).thresholdClr) {
            level = tzItemList_.at(i - 1).level - 1;
        } else {
            break;
        }
    }
    THERMAL_HILOGI(FEATURE_PROTECTOR, "second level = %{public}d", level);
}

void ProtectorThermalZoneInfo::HandleAscMaxSizeTemp(uint32_t &level, int32_t curTemp)
{
    int32_t curDownTemp = tzItemList_.at(level - 1).thresholdClr;
    if (curTemp < curDownTemp) {
        for (uint32_t i = level; i >= 1; i--) {
            if (curTemp < tzItemList_.at(i - 1).thresholdClr) {
                level = tzItemList_.at(i - 1).level - 1;
            } else {
                break;
            }
        }
    }
    THERMAL_HILOGI(FEATURE_PROTECTOR, "fourth level = %{public}d", level);
}

void ProtectorThermalZoneInfo::HandleAscMinSizeTemp(uint32_t &level, int32_t curTemp)
{
    int32_t nextUptemp = tzItemList_.at(level).threshold;
    if (curTemp >= nextUptemp) {
        for (uint32_t i = level; i < tzItemList_.size(); i++) {
            if (curTemp >= tzItemList_.at(i).threshold) {
                level = tzItemList_.at(i).level;
            } else {
                break;
            }
        }
    } else {
        level = 0;
    }
    THERMAL_HILOGI(FEATURE_PROTECTOR, "fifth level = %{public}d", level);
}

void ProtectorThermalZoneInfo::HandleDescNextUpTemp(uint32_t &level, int32_t curTemp)
{
    for (uint32_t i = level; i < tzItemList_.size(); i++) {
        if (curTemp <= tzItemList_.at(i).threshold) {
            level = tzItemList_.at(i).level;
        } else {
            break;
        }
    }
    THERMAL_HILOGI(FEATURE_PROTECTOR, "first level = %{public}d", level);
}

void ProtectorThermalZoneInfo::HandleDescCurDownTemp(uint32_t &level, int32_t curTemp)
{
    for (uint32_t i = level; i >= 1; i--) {
        if (curTemp > tzItemList_.at(i - 1).thresholdClr) {
            level = tzItemList_.at(i - 1).level - 1;
        } else {
            break;
        }
    }
    THERMAL_HILOGI(FEATURE_PROTECTOR, "second level = %{public}d", level);
}

void ProtectorThermalZoneInfo::HandleDescMaxSizeTemp(uint32_t &level, int32_t curTemp)
{
    int32_t curDownTemp = tzItemList_.at(level - 1).thresholdClr;
    if (curTemp > curDownTemp) {
        for (uint32_t i = level; i >= 1; i--) {
            if (curTemp > tzItemList_.at(i - 1).thresholdClr) {
                level = tzItemList_.at(i - 1).level - 1;
            } else {
                break;
            }
        }
    }
    THERMAL_HILOGI(FEATURE_PROTECTOR, "fourth level = %{public}d", level);
}

void ProtectorThermalZoneInfo::HandleDescMinSizeTemp(uint32_t &level, int32_t curTemp)
{
    int32_t nextUptemp = tzItemList_.at(level).threshold;
    if (curTemp <= nextUptemp) {
        for (uint32_t i = level; i < tzItemList_.size(); i++) {
            if (curTemp <= tzItemList_.at(i).threshold) {
                level = tzItemList_.at(i).level;
            } else {
                break;
            }
        }
    } else {
        level = 0;
    }
    THERMAL_HILOGI(FEATURE_PROTECTOR, "fifth level = %{public}d", level);
}

void ProtectorThermalZoneInfo::SetThermalZoneItem(std::vector<ThermalZoneInfoItem> &tzItemList)
{
    tzItemList_ = tzItemList;
}

void ProtectorThermalZoneInfo::SetDesc(bool desc)
{
    desc_ = desc;
}

void ProtectorThermalZoneInfo::SetInterval(int32_t interval)
{
    interval_ = interval;
}

int32_t ProtectorThermalZoneInfo::GetInterval() const
{
    return interval_;
}

void ProtectorThermalZoneInfo::SetMultiple(int32_t multiple)
{
    multiple_ = multiple;
}

int32_t ProtectorThermalZoneInfo::GetMultiple() const
{
    return multiple_;
}

std::string ProtectorThermalZoneInfo::GetPath() const
{
    return path_;
}

void ProtectorThermalZoneInfo::SetPath(const std::string &path)
{
    path_ = path;
}

void ProtectorThermalZoneInfo::Dump()
{
    THERMAL_HILOGD(FEATURE_PROTECTOR, "interval:%{public}d, desc:%{public}d, multiple:%{public}d",
        interval_, desc_, multiple_);
    for (auto infoIter : tzItemList_) {
        THERMAL_HILOGI(FEATURE_PROTECTOR,
            "level: %{public}d, threshold:%{public}d, thresholdClr:%{public}d",
            infoIter.level, infoIter.threshold, infoIter.thresholdClr);
    }
}
} // namespace PowerMgr
} // namespace OHOS
