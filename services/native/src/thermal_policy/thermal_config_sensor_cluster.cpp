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

#include "thermal_config_sensor_cluster.h"

#include <algorithm>

#include "string_operation.h"
#include "thermal_service.h"
#include "thermal_common.h"

namespace OHOS {
namespace PowerMgr {
namespace {
}

bool ThermalConfigSensorCluster::CheckStandard()
{
    if (sensorInfolist_.empty()) {
        THERMAL_HILOGE(COMP_SVC, "sensor info is empty");
        return false;
    }
    uint32_t expectedLevSize = static_cast<uint32_t>(sensorInfolist_.begin()->second.size());
    for (auto sensorInfo = sensorInfolist_.begin(); sensorInfo != sensorInfolist_.end(); ++sensorInfo) {
        uint32_t actualLevSize = static_cast<uint32_t>(sensorInfo->second.size());
        if (actualLevSize != expectedLevSize) {
            THERMAL_HILOGE(COMP_SVC, "sensor [%{public}s] lev size mismatch", sensorInfo->first.c_str());
            return false;
        }
        for (uint32_t i = 0; i < sensorInfo->second.size(); ++i) {
            uint32_t expectedLev = i + 1;
            if (sensorInfo->second.at(i).level != expectedLev) {
                THERMAL_HILOGE(COMP_SVC, "sensor [%{public}s] lev mismatch", sensorInfo->first.c_str());
                return false;
            }
        }
    }
    for (auto sensorInfo = auxSensorInfolist_.begin(); sensorInfo != auxSensorInfolist_.end(); ++sensorInfo) {
        uint32_t actualLevSize = static_cast<uint32_t>(sensorInfo->second.size());
        if (actualLevSize == 0 || actualLevSize == expectedLevSize) {
            continue;
        }
        THERMAL_HILOGE(COMP_SVC, "sensor [%{public}s] aux lev size mismatch", sensorInfo->first.c_str());
        return false;
    }
    return true;
}

void ThermalConfigSensorCluster::UpdateThermalLevel(const TypeTempMap& typeTempInfo)
{
    std::vector<uint32_t> levelList;

    if (!CheckState()) {
        latestLevel_ = 0;
        return;
    }

    CalculateSensorLevel(typeTempInfo, levelList);

    if (levelList.empty()) {
        return;
    }

    latestLevel_ = *std::max_element(levelList.begin(), levelList.end());
    THERMAL_HILOGD(COMP_SVC, "final latestLevel = %{public}u", latestLevel_);
}

bool ThermalConfigSensorCluster::CheckState()
{
    if (stateMap_.empty()) {
        return true;
    }
    auto tms = ThermalService::GetInstance();
    for (auto prop = stateMap_.begin(); prop != stateMap_.end(); prop++) {
        StateMachine::StateMachineMap stateMachineMap = tms->GetStateMachineObj()->GetStateCollectionMap();
        auto stateIter = stateMachineMap.find(prop->first);
        if (stateIter == stateMachineMap.end() || stateIter->second == nullptr) {
            THERMAL_HILOGE(COMP_SVC, "can't find state machine [%{public}s]", prop->first.c_str());
            return false;
        }
        if (stateIter->second->DecideState(prop->second)) {
            continue;
        } else {
            return false;
        }
    }
    return true;
}

void ThermalConfigSensorCluster::CalculateSensorLevel(const TypeTempMap& typeTempInfo,
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

void ThermalConfigSensorCluster::AscendLevelToThreshold(std::vector<LevelItem>& levItems, uint32_t& level,
    uint32_t end, int32_t curTemp)
{
    for (uint32_t i = level; i < end; i++) {
        if (curTemp < levItems.at(i).threshold) {
            break;
        }
        level = levItems.at(i).level;
    }
}

void ThermalConfigSensorCluster::DescendLevelToThresholdClr(std::vector<LevelItem>& levItems,
    uint32_t& level, int32_t curTemp)
{
    for (uint32_t i = level; i >= 1; i--) {
        if (curTemp >= levItems.at(i - 1).thresholdClr) {
            break;
        }
        level = (levItems.at(i - 1).level > 0) ? (levItems.at(i - 1).level - 1) : 0;
    }
}

void ThermalConfigSensorCluster::DescendLevelToThreshold(std::vector<LevelItem>& levItems,
    uint32_t& level, int32_t curTemp)
{
    for (uint32_t i = level; i >= 1; i--) {
        if (curTemp < levItems.at(i - 1).thresholdClr) {
            level = (levItems.at(i - 1).level > 0) ? (levItems.at(i - 1).level - 1) : 0;
        } else {
            break;
        }
    }
}

void ThermalConfigSensorCluster::AscendLevelToThresholdClr(std::vector<LevelItem>& levItems,
    uint32_t& level, uint32_t end, int32_t curTemp)
{
    for (uint32_t i = level; i < end; i++) {
        if (curTemp >= levItems.at(i).threshold) {
            level = levItems.at(i).level;
        } else {
            break;
        }
    }
}

void ThermalConfigSensorCluster::LevelUpwardsSearch(std::vector<LevelItem>& levItems,
    uint32_t& level, uint32_t end, int32_t curTemp)
{
    for (uint32_t i = level; i < end; i++) {
        if (curTemp > levItems.at(i).threshold) {
            break;
        }
        level = levItems.at(i).level;
    }
}

void ThermalConfigSensorCluster::LevelDownwardsSearch(std::vector<LevelItem>& levItems,
    uint32_t& level, int32_t curTemp)
{
    for (uint32_t i = level; i >= 1; i--) {
        if (curTemp <= levItems.at(i - 1).thresholdClr) {
            break;
        }
        level = (levItems.at(i - 1).level > 0) ? (levItems.at(i - 1).level - 1) : 0;
    }
}

void ThermalConfigSensorCluster::LevelDownwardsSearchWithThreshold(std::vector<LevelItem>& levItems,
    uint32_t& level, int32_t curTemp)
{
    for (uint32_t i = level; i >= 1; i--) {
        if (curTemp > levItems.at(i - 1).thresholdClr) {
            level = (levItems.at(i - 1).level > 0) ? (levItems.at(i - 1).level - 1) : 0;
        } else {
            break;
        }
    }
}

void ThermalConfigSensorCluster::LevelUpwardsSearchWithThreshold(std::vector<LevelItem>& levItems,
    uint32_t& level, uint32_t end, int32_t curTemp)
{
    for (uint32_t i = level; i < end; i++) {
        if (curTemp <= levItems.at(i).threshold) {
            level = levItems.at(i).level;
        } else {
            break;
        }
    }
}

void ThermalConfigSensorCluster::AscJudgment(std::vector<LevelItem>& levItems, int32_t curTemp, uint32_t& level)
{
    if (level > 0 && level < levItems.size()) {
        int32_t curDownTemp = levItems.at(level - 1).thresholdClr;
        int32_t nextUptemp = levItems.at(level).threshold;
        if (curTemp >= nextUptemp) {
            AscendLevelToThreshold(levItems, level, levItems.size(), curTemp);
        } else if (curTemp < curDownTemp) {
            DescendLevelToThresholdClr(levItems, level, curTemp);
        }
    } else if (level == levItems.size()) {
        int32_t curDownTemp = levItems.at(level - 1).thresholdClr;
        if (curTemp < curDownTemp) {
            DescendLevelToThreshold(levItems, level, curTemp);
        }
    } else {
        int32_t nextUptemp = levItems.at(level).threshold;
        if (curTemp >= nextUptemp) {
            AscendLevelToThresholdClr(levItems, level, levItems.size(), curTemp);
        }
    }
}

void ThermalConfigSensorCluster::DescJudgment(std::vector<LevelItem>& levItems, int32_t curTemp, uint32_t& level)
{
    if (level != 0 && level < levItems.size()) {
        int32_t curDownTemp = levItems.at(level - 1).thresholdClr;
        int32_t nextUptemp = levItems.at(level).threshold;
        if (curTemp <= nextUptemp) {
            LevelUpwardsSearch(levItems, level, levItems.size(), curTemp);
        } else if (curTemp > curDownTemp) {
            LevelDownwardsSearch(levItems, level, curTemp);
        }
    } else if (level == levItems.size()) {
        int32_t curDownTemp = levItems.at(level - 1).thresholdClr;
        if (curTemp > curDownTemp) {
            LevelDownwardsSearchWithThreshold(levItems, level, curTemp);
        }
    } else {
        int32_t nextUptemp = levItems.at(level).threshold;
        if (curTemp <= nextUptemp) {
            LevelUpwardsSearchWithThreshold(levItems, level, levItems.size(), curTemp);
        }
    }
}

void ThermalConfigSensorCluster::CheckExtraCondition(const TypeTempMap& typeTempInfo, uint32_t& level)
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
    auto tms = ThermalService::GetInstance();
    const auto& rateMap = tms->GetSubscriber()->GetSensorsRate();
    for (auto sensorInfo = sensorInfolist_.begin(); sensorInfo != sensorInfolist_.end(); ++sensorInfo) {
        auto rateIter = rateMap.find(sensorInfo->first);
        if (rateIter == rateMap.end()) {
            continue;
        }
        for (const auto& levItem : sensorInfo->second) {
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

bool ThermalConfigSensorCluster::IsAuxSensorTrigger(const TypeTempMap& typeTempInfo, uint32_t& level)
{
    if (level == 0) {
        return true;
    }
    for (auto sensorInfo = auxSensorInfolist_.begin(); sensorInfo != auxSensorInfolist_.end(); ++sensorInfo) {
        auto auxIter = typeTempInfo.find(sensorInfo->first);
        if (auxIter == typeTempInfo.end()) {
            continue;
        }
        int32_t lowerTemp = sensorInfo->second.at(level - 1).lowerTemp;
        int32_t upperTemp = sensorInfo->second.at(level - 1).upperTemp;
        if (auxIter->second >= lowerTemp && auxIter->second <= upperTemp) {
            continue;
        } else {
            level = 0;
            return false;
        }
    }
    return true;
}

uint32_t ThermalConfigSensorCluster::GetCurrentLevel()
{
    return latestLevel_;
}

void ThermalConfigSensorCluster::SetSensorLevelInfo(SensorInfoMap& sensorInfolist)
{
    sensorInfolist_ = sensorInfolist;
}

void ThermalConfigSensorCluster::SetAuxSensorLevelInfo(AuxSensorInfoMap& auxSensorInfolist)
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
