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

#include "thermal_config_sensor_cluster.h"

#include <algorithm>

#include "string_operation.h"
#include "thermal_service.h"
#include "thermal_common.h"

namespace OHOS {
namespace PowerMgr {
namespace {
auto g_service = DelayedSpSingleton<ThermalService>::GetInstance();
}

bool ThermalConfigSensorCluster::CheckStandard()
{
    if (sensorInfolist_.empty()) {
        THERMAL_HILOGE(COMP_SVC, "sensor info is empty");
        return false;
    }
    uint32_t levSize = static_cast<uint32_t>(sensorInfolist_.begin()->second().size());
    for (auto& sensorInfo = sensorInfolist_.begin(); sensorInfo != sensorInfolist_.end(); ++sensorInfo) {
        for (int i = 0; i < sensorInfo->second().size(), ++i) {
            if (sensorInfo->second().at(i).level != i + 1) {
                THERMAL_HILOGE(COMP_SVC, "sensor [%{public}s] lev mismatch", sensorInfo->first().c_str());
                return false;
            }
        }
    }
    for (auto& sensorInfo = auxSensorInfolist_.begin(); sensorInfo != auxSensorInfolist_.end(); ++sensorInfo) {
        if (sensorInfo->second().size() == 0 || sensorInfo->second().size() == levSize) {
            continue;
        } else {
            THERMAL_HILOGE(COMP_SVC, "sensor [%{public}s] aux lev mismatch", sensorInfo->first().c_str());
            return false;
        }
    }
    return true;
}

void ThermalConfigSensorCluster::UpdateThermalLevel(TypeTempMap& typeTempInfo)
{
    std::vector<uint32_t> levelList;

    CalculateSensorLevel(typeTempInfo, levelList);

    if (levelList.empty()) {
        return;
    }

    latestLevel_ = *std::max_element(levelList.begin(), levelList.end());
    THERMAL_HILOGD(COMP_SVC, "final latestLevel = %{public}d", latestLevel_);
}

void ThermalConfigSensorCluster::CalculateSensorLevel(TypeTempMap& typeTempInfo,
    std::vector<uint32_t>& levelList)
{
    if (sensorInfolist_.empty()) {
        return;
    }

    for (auto sensorInfo = sensorInfolist_.begin(); sensorInfo != sensorInfolist_.end(); ++sensorInfo) {
        auto iter = typeTempInfo.find(sensorInfo->first);
        if (iter == typeTempInfo.end()) {
            continue;
        }
        uint32_t level = latestLevel_;
        if (descFlag_) {
            DescJudgment(sensorInfo->second, iter->second, level);
            CheckExtraCondition(typeTempInfo, level);
            levelList.push_back(level);
        } else {
            AscJudgment(sensorInfo->second, iter->second, level);
            CheckExtraCondition(typeTempInfo, level);
            levelList.push_back(level);
        }
    }
}

void ThermalConfigSensorCluster::AscJudgment(std::vector<LevelItem>& levItems, int32_t curTemp, uint32_t& level)
{
    if (level > 0 && level < levItems.size()) {
        int32_t curDownTemp = levItems.at(level - 1).thresholdClr;
        int32_t nextUptemp = levItems.at(level).threshold;
        if (curTemp >= nextUptemp) {
            for (uint32_t i = level; i < levItems.size(); i++) {
                if (curTemp >= levItems.at(i).threshold) {
                    level = levItems.at(i).level;
                } else {
                    break;
                }
            }
        } else if (curTemp < curDownTemp) {
            for (uint32_t i = level; i >= 1; i--) {
                if (curTemp < levItems.at(i - 1).thresholdClr) {
                    level = levItems.at(i - 1).level - 1;
                } else {
                    break;
                }
            }
        }
    } else if (level == levItems.size()) {
        int32_t curDownTemp = levItems.at(level - 1).thresholdClr;
        if (curTemp < curDownTemp) {
            for (uint32_t i = level; i >= 1; i--) {
                if (curTemp < levItems.at(i - 1).thresholdClr) {
                    level = levItems.at(i - 1).level - 1;
                } else {
                    break;
                }
            }
        }
    } else {
        int32_t nextUptemp = levItems.at(level).threshold;
        if (curTemp >= nextUptemp) {
            for (uint32_t i = level; i < levItems.size(); i++) {
                if (curTemp >= levItems.at(i).threshold) {
                    level = levItems.at(i).level;
                } else {
                    break;
                }
            }
        }
    }
}

void ThermalConfigSensorCluster::DescJudgment(std::vector<LevelItem>& levItems, int32_t curTemp, uint32_t& level)
{
    if (level != 0 && level < levItems.size()) {
        int32_t curDownTemp = levItems.at(level - 1).thresholdClr;
        int32_t nextUptemp = levItems.at(level).threshold;
        if (curTemp <= nextUptemp) {
            for (uint32_t i = level; i < levItems.size(); i++) {
                if (curTemp <= levItems.at(i).threshold) {
                    level = levItems.at(i).level;
                } else {
                    break;
                }
            }
        } else if (curTemp > curDownTemp) {
            for (uint32_t i = level; i >= 1; i--) {
                if (curTemp > levItems.at(i - 1).thresholdClr) {
                    level = levItems.at(i - 1).level - 1;
                } else {
                    break;
                }
            }
        }
    } else if (level == levItems.size()) {
        int32_t curDownTemp = levItems.at(level - 1).thresholdClr;
        if (curTemp > curDownTemp) {
            for (uint32_t i = level; i >= 1; i--) {
                if (curTemp > levItems.at(i - 1).thresholdClr) {
                    level = levItems.at(i - 1).level - 1;
                } else {
                    break;
                }
            }
        }
    } else {
        int32_t nextUptemp = levItems.at(level).threshold;
        if (curTemp <= nextUptemp) {
            for (uint32_t i = level; i < levItems.size(); i++) {
                if (curTemp <= levItems.at(i).threshold) {
                    level = levItems.at(i).level;
                } else {
                    break;
                }
            }
        }
    }
}

void ThermalConfigSensorCluster::CheckExtraCondition(TypeTempMap& typeTempInfo, uint32_t& level)
{
    if (auxFlag_) {
        if (!IsAuxSensorTrigger(typeTempInfo, level)) {
            THERMAL_HILOGD(COMP_SVC, "aux sensor isn't satisfied, fallback");
        }
    }

    if (rateFlag_) {
        if (!IsTempRateTrigger(level)) {
            THERMAL_HILOGD(COMP_SVC, "temp rise rate isn't satisfied, fallback");
        }
    }
}

bool ThermalConfigSensorCluster::IsTempRateTrigger(uint32_t& level)
{
    if (level == 0) {
        return true;
    }
    auto& rateMap = g_service->GetSubscriber()->GetSensorsRate();
    for (auto& sensorInfo = sensorInfolist_.begin(); sensorInfo != sensorInfolist_.end(); ++sensorInfo) {
        auto& rateIter = rateMap.find(sensorInfo->first);
        if (rateIter == rateMap.end()) {
            continue;
        }
        for (auto& levItem : sensorInfo->second) {
            if (levItem.level != level) {
                continue;
            }
            if (rateIter->second > levItem.tempRiseRate) {
                continue;
            } else {
                level = 0;
                return false;
            }
        }
    }
    return true;
}

bool ThermalConfigSensorCluster::IsAuxSensorTrigger(TypeTempMap& typeTempInfo, uint32_t& level)
{
    if (level == 0) {
        return true;
    }
    for (auto& sensorInfo = auxSensorInfolist_.begin(); sensorInfo != auxSensorInfolist_.end(); ++sensorInfo) {
        auto& auxIter = typeTempInfo.find(sensorInfo->first);
        if (auxIter == typeTempInfo.end()) {
            continue;
        }
        int32_t lowerTemp = auxSensorInfo->second.at(level - 1).lowerTemp;
        int32_t upperTemp = auxSensorInfo->second.at(level - 1).upperTemp;
        if (auxIter->second >= lowerTemp && auxIter->second <= upperTemp) {
            continue;
        } else {
            level = 0;
            return false;
        }
    }
    return true;
}
} // namespace PowerMgr
} // namespace OHOS
