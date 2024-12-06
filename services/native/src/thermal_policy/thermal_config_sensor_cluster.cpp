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
    uint32_t tempDiffLevSize = static_cast<uint32_t>(tempDiffInfoList_.size());
    if (tempDiffFlag_ && tempDiffLevSize != expectedLevSize) {
        THERMAL_HILOGE(COMP_SVC, "temp diff sensor lev size mismatch");
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
            DescJudgment(sensorInfo->second, iter->second, level, typeTempInfo);
            levelList.push_back(level);
        } else {
            AscJudgment(sensorInfo->second, iter->second, level, typeTempInfo);
            levelList.push_back(level);
        }
    }
}

void ThermalConfigSensorCluster::AscendLevelToThreshold(std::vector<LevelItem>& levItems, uint32_t& level,
    uint32_t end, int32_t curTemp, const TypeTempMap& typeTempInfo)
{
    for (uint32_t i = level; i < end; i++) {
        if (curTemp < levItems.at(i).threshold || !CheckExtraCondition(typeTempInfo, levItems.at(i).level)) {
            break;
        }
        level = levItems.at(i).level;
    }
}

void ThermalConfigSensorCluster::DescendLevelToThresholdClr(std::vector<LevelItem>& levItems,
    uint32_t& level, int32_t curTemp, const TypeTempMap& typeTempInfo)
{
    for (uint32_t i = level; i >= 1; i--) {
        if (curTemp >= levItems.at(i - 1).thresholdClr && CheckExtraCondition(typeTempInfo, levItems.at(i - 1).level)) {
            break;
        }
        level = (levItems.at(i - 1).level > 0) ? (levItems.at(i - 1).level - 1) : 0;
    }
}

void ThermalConfigSensorCluster::DescendLevelToThreshold(std::vector<LevelItem>& levItems,
    uint32_t& level, int32_t curTemp, const TypeTempMap& typeTempInfo)
{
    for (uint32_t i = level; i >= 1; i--) {
        if (curTemp < levItems.at(i - 1).thresholdClr || !CheckExtraCondition(typeTempInfo, levItems.at(i - 1).level)) {
            level = (levItems.at(i - 1).level > 0) ? (levItems.at(i - 1).level - 1) : 0;
        } else {
            break;
        }
    }
}

void ThermalConfigSensorCluster::AscendLevelToThresholdClr(std::vector<LevelItem>& levItems,
    uint32_t& level, uint32_t end, int32_t curTemp, const TypeTempMap& typeTempInfo)
{
    for (uint32_t i = level; i < end; i++) {
        if (curTemp >= levItems.at(i).threshold && CheckExtraCondition(typeTempInfo, levItems.at(i).level)) {
            level = levItems.at(i).level;
        } else {
            break;
        }
    }
}

void ThermalConfigSensorCluster::LevelUpwardsSearch(std::vector<LevelItem>& levItems,
    uint32_t& level, uint32_t end, int32_t curTemp, const TypeTempMap& typeTempInfo)
{
    for (uint32_t i = level; i < end; i++) {
        if (curTemp > levItems.at(i).threshold || !CheckExtraCondition(typeTempInfo, levItems.at(i).level)) {
            break;
        }
        level = levItems.at(i).level;
    }
}

void ThermalConfigSensorCluster::LevelDownwardsSearch(std::vector<LevelItem>& levItems,
    uint32_t& level, int32_t curTemp, const TypeTempMap& typeTempInfo)
{
    for (uint32_t i = level; i >= 1; i--) {
        if (curTemp <= levItems.at(i - 1).thresholdClr && CheckExtraCondition(typeTempInfo, levItems.at(i - 1).level)) {
            break;
        }
        level = (levItems.at(i - 1).level > 0) ? (levItems.at(i - 1).level - 1) : 0;
    }
}

void ThermalConfigSensorCluster::LevelDownwardsSearchWithThreshold(std::vector<LevelItem>& levItems,
    uint32_t& level, int32_t curTemp, const TypeTempMap& typeTempInfo)
{
    for (uint32_t i = level; i >= 1; i--) {
        if (curTemp > levItems.at(i - 1).thresholdClr || !CheckExtraCondition(typeTempInfo, levItems.at(i - 1).level)) {
            level = (levItems.at(i - 1).level > 0) ? (levItems.at(i - 1).level - 1) : 0;
        } else {
            break;
        }
    }
}

void ThermalConfigSensorCluster::LevelUpwardsSearchWithThreshold(std::vector<LevelItem>& levItems,
    uint32_t& level, uint32_t end, int32_t curTemp, const TypeTempMap& typeTempInfo)
{
    for (uint32_t i = level; i < end; i++) {
        if (curTemp <= levItems.at(i).threshold && CheckExtraCondition(typeTempInfo, levItems.at(i).level)) {
            level = levItems.at(i).level;
        } else {
            break;
        }
    }
}

void ThermalConfigSensorCluster::AscJudgment(std::vector<LevelItem>& levItems, int32_t curTemp, uint32_t& level,
    const TypeTempMap& typeTempInfo)
{
    if (level > 0 && level < levItems.size()) {
        int32_t curDownTemp = levItems.at(level - 1).thresholdClr;
        int32_t nextUptemp = levItems.at(level).threshold;
        if (curTemp >= nextUptemp) {
            AscendLevelToThreshold(levItems, level, levItems.size(), curTemp, typeTempInfo);
        } else if (curTemp < curDownTemp || !CheckExtraCondition(typeTempInfo, level)) {
            DescendLevelToThresholdClr(levItems, level, curTemp, typeTempInfo);
        }
    } else if (level == levItems.size()) {
        int32_t curDownTemp = levItems.at(level - 1).thresholdClr;
        if (curTemp < curDownTemp || !CheckExtraCondition(typeTempInfo, level)) {
            DescendLevelToThreshold(levItems, level, curTemp, typeTempInfo);
        }
    } else {
        int32_t nextUptemp = levItems.at(level).threshold;
        if (curTemp >= nextUptemp) {
            AscendLevelToThresholdClr(levItems, level, levItems.size(), curTemp, typeTempInfo);
        }
    }
}

void ThermalConfigSensorCluster::DescJudgment(std::vector<LevelItem>& levItems, int32_t curTemp, uint32_t& level,
    const TypeTempMap& typeTempInfo)
{
    if (level != 0 && level < levItems.size()) {
        int32_t curDownTemp = levItems.at(level - 1).thresholdClr;
        int32_t nextUptemp = levItems.at(level).threshold;
        if (curTemp <= nextUptemp) {
            LevelUpwardsSearch(levItems, level, levItems.size(), curTemp, typeTempInfo);
        } else if (curTemp > curDownTemp || !CheckExtraCondition(typeTempInfo, level)) {
            LevelDownwardsSearch(levItems, level, curTemp, typeTempInfo);
        }
    } else if (level == levItems.size()) {
        int32_t curDownTemp = levItems.at(level - 1).thresholdClr;
        if (curTemp > curDownTemp || !CheckExtraCondition(typeTempInfo, level)) {
            LevelDownwardsSearchWithThreshold(levItems, level, curTemp, typeTempInfo);
        }
    } else {
        int32_t nextUptemp = levItems.at(level).threshold;
        if (curTemp <= nextUptemp) {
            LevelUpwardsSearchWithThreshold(levItems, level, levItems.size(), curTemp, typeTempInfo);
        }
    }
}

bool ThermalConfigSensorCluster::CheckExtraCondition(const TypeTempMap& typeTempInfo, uint32_t& level)
{
    if (auxFlag_) {
        if (!IsAuxSensorTrigger(typeTempInfo, level)) {
            THERMAL_HILOGD(COMP_SVC, "aux sensor isn't satisfied, fallback");
            return false;
        }
    }

    if (rateFlag_) {
        if (!IsTempRateTrigger(level)) {
            THERMAL_HILOGD(COMP_SVC, "temp rise rate isn't satisfied, fallback");
            return false;
        }
    }

    if (tempDiffFlag_ && !IsTempDiffTrigger(typeTempInfo, level)) {
        THERMAL_HILOGD(COMP_SVC, "temp diff isn't satisfied, fallback");
        return false;
    }
    return true;
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
            return false;
        }
        int32_t lowerTemp = sensorInfo->second.at(level - 1).lowerTemp;
        int32_t upperTemp = sensorInfo->second.at(level - 1).upperTemp;
        if (auxIter->second >= lowerTemp && auxIter->second <= upperTemp) {
            continue;
        } else {
            return false;
        }
    }
    return true;
}

bool ThermalConfigSensorCluster::IsTempDiffTrigger(const TypeTempMap& typeTempInfo, uint32_t& level)
{
    if (level == 0) {
        THERMAL_HILOGD(COMP_SVC, "initial level is zero, temp diff not triggered");
        return true;
    }
    uint32_t levSize = static_cast<uint32_t>(tempDiffInfoList_.size());
    if (level <= 0 || level > levSize) {
        THERMAL_HILOGE(COMP_SVC, "target level or level size is illegal.");
        return false;
    }
    sort(tempDiffInfoList_.begin(), tempDiffInfoList_.end(),
        [](const TempDiffItem& t1, const TempDiffItem& t2) {
            return t1.level < t2.level;
    });
    auto& tempDiffInfo = tempDiffInfoList_.at(level - 1);
    auto& sensor1 = tempDiffInfo.sensor1;
    auto& sensor2 = tempDiffInfo.sensor2;
 
    std::map<std::string, int>::const_iterator ite1 = typeTempInfo.end(),
        ite2 = typeTempInfo.end();
    if ((ite1 = typeTempInfo.find(sensor1)) == typeTempInfo.end()
        || (ite2 = typeTempInfo.find(sensor2)) == typeTempInfo.end()) {
        THERMAL_HILOGE(COMP_SVC, "No temperature info about the target sensor: %{public}s and %{public}s.",
            sensor1.c_str(), sensor2.c_str());
        return false;
    }
 
    const auto& tempSensor1 = ite1->second;
    const auto& tempSensor2 = ite2->second;
    if ((tempSensor1 - tempSensor2) > tempDiffInfo.tempDiff) {
        THERMAL_HILOGE(COMP_SVC, "temp diff of target sensors do not meet the requirements.");
        return false;
    }
 
    THERMAL_HILOGD(COMP_SVC, "Temperature difference triggered successfully.");
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

void ThermalConfigSensorCluster::SetTempDiffFlag(bool tempDiffFlag)
{
    tempDiffFlag_ = tempDiffFlag;
}
 
void ThermalConfigSensorCluster::SetTempDiffInfo(TempDiffInfoList& tempDiffInfoList)
{
    tempDiffInfoList_ = tempDiffInfoList;
}
} // namespace PowerMgr
} // namespace OHOS
