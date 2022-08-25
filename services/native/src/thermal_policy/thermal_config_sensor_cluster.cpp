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
bool ThermalConfigSensorCluster::Init()
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    return true;
}

void ThermalConfigSensorCluster::Dump()
{
}

void ThermalConfigSensorCluster::UpdateThermalLevel(TypeTempMap &typeTempInfo)
{
    uint32_t level = 0;
    std::vector<uint32_t> levelList;
    if (auxFlag_) {
        if (!IsAuxSensorTrigger(typeTempInfo, level)) {
            THERMAL_HILOGD(COMP_SVC, "all auxiliary sensor not satisfied");
        }
    }

    if (rateFlag_) {
        if (!IsTempRateTrigger(typeTempInfo, level)) {
            THERMAL_HILOGD(COMP_SVC, "all sensors not satisfied");
        }
    }

    CalculateSensorLevel(typeTempInfo, levelList, level);

    if (levelList.empty()) return;

    auto max = std::max_element(levelList.begin(), levelList.end());
    latestLevel_ = *max;
    THERMAL_HILOGD(COMP_SVC, "final latestLevel =  %{public}d", latestLevel_);
    isRateMap_.clear();
}

void ThermalConfigSensorCluster::CalculateSensorLevel(TypeTempMap &typeTempInfo,
    std::vector<uint32_t> &levelList, uint32_t &level)
{
    if (sensorInfolist_.empty()) {
        return;
    }

    for (auto sensorInfo = sensorInfolist_.begin(); sensorInfo != sensorInfolist_.end(); ++sensorInfo) {
        auto iter = typeTempInfo.find(sensorInfo->first);
        if (iter != typeTempInfo.end()) {
            level = latestLevel_;
            if (descFlag_) {
                DescJudgment(sensorInfo->second, iter->second, level);
                auto rateItem = isRateMap_.find(sensorInfo->first);
                if (rateItem != isRateMap_.end()) {
                    if (!rateItem->second) {
                        level = 0;
                    }
                }
                if (auxFlag_) {
                    if (IsAuxSensorTrigger(typeTempInfo, level)) {
                        THERMAL_HILOGD(COMP_SVC, "all auxiliary sensor is satisfied");
                    }
                }
                levelList.push_back(level);
            } else {
                AscJudgment(sensorInfo->second, iter->second, level);
                auto rateItem = isRateMap_.find(sensorInfo->first);
                if (rateItem != isRateMap_.end()) {
                    if (!rateItem->second) {
                        level = 0;
                    }
                }
                if (auxFlag_) {
                    if (IsAuxSensorTrigger(typeTempInfo, level)) {
                        THERMAL_HILOGD(COMP_SVC, "all auxiliary sensor is satisfied");
                    }
                }
                levelList.push_back(level);
            }
        } else {
            continue;
        }
    }
}

bool ThermalConfigSensorCluster::CmpValue(const std::pair<std::string, uint32_t> left,
    const std::pair<std::string, uint32_t> right)
{
    return left.second < right.second;
}

void ThermalConfigSensorCluster::AscJudgment(std::vector<LevelItem> &vlev, int32_t curTemp, uint32_t &level)
{
    if (level > 0 && level < vlev.size()) {
        int32_t curDownTemp = vlev.at(level - 1).thresholdClr;
        int32_t nextUptemp = vlev.at(level).threshold;
        if (curTemp >= nextUptemp) {
            for (uint32_t i = level; i < vlev.size(); i++) {
                if (curTemp >= vlev.at(i).threshold) {
                    level = vlev.at(i).level;
                } else {
                    break;
                }
            }
            THERMAL_HILOGD(COMP_SVC, "first level = %{public}d", level);
        } else if (curTemp < curDownTemp) {
            for (uint32_t i = level; i >= 1; i--) {
                if (curTemp < vlev.at(i - 1).thresholdClr) {
                    level = vlev.at(i - 1).level - 1;
                } else {
                    break;
                }
            }
            THERMAL_HILOGD(COMP_SVC, "second level = %{public}d", level);
        } else {
            level = vlev.at(level - 1).level;
            THERMAL_HILOGD(COMP_SVC, "third level = %{public}d", level);
        }
    } else if (level == vlev.size()) {
        int32_t curDownTemp = vlev.at(level - 1).thresholdClr;
        if (curTemp < curDownTemp) {
            for (uint32_t i = level; i >= 1; i--) {
                if (curTemp < vlev.at(i - 1).thresholdClr) {
                    level = vlev.at(i - 1).level - 1;
                } else {
                    break;
                }
            }
        } else {
            level = vlev.at(level - 1).level;
        }
        THERMAL_HILOGD(COMP_SVC, "fourth level = %{public}d", level);
    } else if (level == 0)  {
        int32_t nextUptemp = vlev.at(level).threshold;
        if (curTemp >= nextUptemp) {
            for (uint32_t i = level; i < vlev.size(); i++) {
                if (curTemp >= vlev.at(i).threshold) {
                    level = vlev.at(i).level;
                } else {
                    break;
                }
            }
        } else {
            level = 0;
        }
        THERMAL_HILOGD(COMP_SVC, "fifth level = %{public}d", level);
    }
}

void ThermalConfigSensorCluster::DescJudgment(std::vector<LevelItem> &vlev, int32_t curTemp, uint32_t &level)
{
    level = latestLevel_;
    if (level != 0 && level < vlev.size()) {
        int32_t curDownTemp = vlev.at(level - 1).thresholdClr;
        int32_t nextUptemp = vlev.at(level).threshold;
        if (curTemp <= nextUptemp) {
            for (uint32_t i = level; i < vlev.size(); i++) {
                if (curTemp <= vlev.at(i).threshold) {
                    level = vlev.at(i).level;
                } else {
                    break;
                }
            }
            THERMAL_HILOGD(COMP_SVC, "first level = %{public}d", level);
        } else if (curTemp > curDownTemp) {
            for (uint32_t i = level; i >= 1; i--) {
                if (curTemp > vlev.at(i - 1).thresholdClr) {
                    level = vlev.at(i - 1).level - 1;
                } else {
                    break;
                }
            }
            THERMAL_HILOGD(COMP_SVC, "second level = %{public}d", level);
        } else {
            level = vlev.at(level - 1).level;
            THERMAL_HILOGD(COMP_SVC, "third level = %{public}d", level);
        }
    } else if (level == vlev.size()) {
        int32_t curDownTemp = vlev.at(level - 1).thresholdClr;
        if (curTemp > curDownTemp) {
            for (uint32_t i = level; i >= 1; i--) {
                if (curTemp > vlev.at(i - 1).thresholdClr) {
                    level = vlev.at(i - 1).level - 1;
                } else {
                    break;
                }
            }
        } else {
            level = vlev.at(level - 1).level;
        }
        THERMAL_HILOGD(COMP_SVC, "fourth level = %{public}d", level);
    } else if (level == 0) {
        int32_t nextUptemp = vlev.at(level).threshold;
        if (curTemp <= nextUptemp) {
            for (uint32_t i = level; i < vlev.size(); i++) {
                if (curTemp <= vlev.at(i).threshold) {
                    level = vlev.at(i).level;
                } else {
                    break;
                }
            }
        } else {
            level = 0;
        }
        THERMAL_HILOGD(COMP_SVC, "fifth level = %{public}d", level);
    }
}

bool ThermalConfigSensorCluster::IsTempRateTrigger(TypeTempMap &typeTempInfo, uint32_t &level)
{
    bool ret  = false;
    bool allRate = false;
    auto rateMap = g_service->GetSubscriber()->GetSensorsRate();
    if (level == 0) return false;
    for (auto sensorInfo = sensorInfolist_.begin(); sensorInfo != sensorInfolist_.end(); ++sensorInfo) {
        auto iter = typeTempInfo.find(sensorInfo->first);
        THERMAL_HILOGD(COMP_SVC, "type:%{public}s temp:%{public}d", iter->first.c_str(), iter->second);
        if (iter != typeTempInfo.end()) {
            auto rateIter = rateMap.find(sensorInfo->first);
            if (rateIter != rateMap.end()) {
                double configRate = sensorInfo->second.at(level - 1).tempRiseRate;
                THERMAL_HILOGD(COMP_SVC, "configRate = %{public}f", configRate);
                if (rateIter->second > configRate) {
                    ret = true;
                } else {
                    level = 0;
                    ret = false;
                }
            }
        }
        allRate |= ret;
        isRateMap_.insert(std::make_pair(sensorInfo->first, ret));
    }
    return allRate;
}

bool ThermalConfigSensorCluster::IsAuxSensorTrigger(TypeTempMap &typeTempInfo, uint32_t &level)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    bool ret = false;
    bool allAux = false;

    if (level == 0) return false;

    for (auto auxSensorInfo = auxSensorInfolist_.begin(); auxSensorInfo != auxSensorInfolist_.end(); ++auxSensorInfo) {
        auto auxIter = typeTempInfo.find(auxSensorInfo->first);
        if (auxIter != typeTempInfo.end()) {
            int32_t lowerTemp = auxSensorInfo->second.at(level - 1).lowerTemp;
            int32_t upperTemp = auxSensorInfo->second.at(level - 1).upperTemp;
            if (auxIter->second >= lowerTemp && auxIter->second <= upperTemp) {
                ret = true;
            } else {
                level = 0;
                ret = false;
            }
        }
        allAux |= ret;
    }
    return allAux;
}

uint32_t ThermalConfigSensorCluster::GetCurrentLevel()
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    std::unique_lock<std::mutex> lock(levelMutex_);
    return latestLevel_;
}

bool ThermalConfigSensorCluster::GetDescFlag()
{
    return descFlag_;
}

bool ThermalConfigSensorCluster::GetAuxFlag()
{
    return auxFlag_;
}

bool ThermalConfigSensorCluster::GetRateFlag()
{
    return rateFlag_;
}

SensorInfoMap ThermalConfigSensorCluster::GetSensorInfoList()
{
    return sensorInfolist_;
}

AuxSensorInfoMap ThermalConfigSensorCluster::GetAuxSensorInfoList()
{
    return auxSensorInfolist_;
}
void ThermalConfigSensorCluster::SetSensorLevelInfo(SensorInfoMap &sensorInfolist)
{
    sensorInfolist_ = sensorInfolist;
}

void ThermalConfigSensorCluster::SetAuxSensorLevelInfo(AuxSensorInfoMap &auxSensorInfolist)
{
    auxSensorInfolist_ = auxSensorInfolist;
}

void ThermalConfigSensorCluster::SetDescFlag(bool descflag)
{
    descFlag_ = descflag;
}

void ThermalConfigSensorCluster::SetAuxFlag(bool auxflag)
{
    auxFlag_ = auxflag;
}

void ThermalConfigSensorCluster::SetRateFlag(bool rateFlag)
{
    rateFlag_ =  rateFlag;
}
} // namespace PowerMgr
} // namespace OHOS
