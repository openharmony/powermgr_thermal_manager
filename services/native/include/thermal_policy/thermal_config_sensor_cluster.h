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

#ifndef THERMAL_CONFIG_SENSOR_CLUSTER_H
#define THERMAL_CONFIG_SENSOR_CLUSTER_H

#include <string>
#include <map>
#include <vector>
#include <memory>

namespace OHOS {
namespace PowerMgr {
struct LevelItem {
    int32_t threshold;
    int32_t thresholdClr;
    double tempRiseRate;
    uint32_t level;
};

struct AuxLevelItem {
    int32_t lowerTemp;
    int32_t upperTemp;
    int32_t level;
};

using TypeTempMap = std::map<std::string, int32_t>;
using SensorInfoMap = std::map<std::string, std::vector<LevelItem>>;
using AuxSensorInfoMap = std::map<std::string, std::vector<AuxLevelItem>>;

class ThermalConfigSensorCluster {
public:
    bool CheckStandard();
    void UpdateThermalLevel(const TypeTempMap& typeTempInfo);

    uint32_t GetCurrentLevel();
    void SetSensorLevelInfo(SensorInfoMap& sensorInfolist);
    void SetAuxSensorLevelInfo(AuxSensorInfoMap& auxSensorInfolist);
    void SetDescFlag(bool descflag);
    void SetAuxFlag(bool auxflag);
    void SetRateFlag(bool rateFlag);
    void AddState(std::string& state, std::string& val)
    {
        stateMap_.emplace(state, val);
    }

private:
    bool CheckState();
    void CalculateSensorLevel(const TypeTempMap& typeTempInfo, std::vector<uint32_t>& levelList);
    void AscendLevelToThreshold(std::vector<LevelItem>& levItems, uint32_t& level, uint32_t end, int32_t curTemp);
    void DescendLevelToThresholdClr(std::vector<LevelItem>& levItems, uint32_t& level, int32_t curTemp);
    void DescendLevelToThreshold(std::vector<LevelItem>& levItems, uint32_t& level, int32_t curTemp);
    void AscendLevelToThresholdClr(std::vector<LevelItem>& levItems, uint32_t& level, uint32_t end, int32_t curTemp);
    void LevelUpwardsSearch(std::vector<LevelItem>& levItems, uint32_t& level, uint32_t end, int32_t curTemp);
    void LevelDownwardsSearch(std::vector<LevelItem>& levItems, uint32_t& level, int32_t curTemp);
    void LevelDownwardsSearchWithThreshold(std::vector<LevelItem>& levItems, uint32_t& level, int32_t curTemp);
    void LevelUpwardsSearchWithThreshold(std::vector<LevelItem>& levItems, uint32_t& level,
        uint32_t end, int32_t curTemp);
    void AscJudgment(std::vector<LevelItem>& levItems, int32_t curTemp, uint32_t& level);
    void DescJudgment(std::vector<LevelItem>& levItems, int32_t curTemp, uint32_t& level);
    void CheckExtraCondition(const TypeTempMap& typeTempInfo, uint32_t& level);
    bool IsTempRateTrigger(uint32_t& level);
    bool IsAuxSensorTrigger(const TypeTempMap& typeTempInfo, uint32_t& level);

    bool descFlag_ {false};
    bool auxFlag_ {false};
    bool rateFlag_ {false};
    uint32_t latestLevel_ {0};
    SensorInfoMap sensorInfolist_;
    AuxSensorInfoMap auxSensorInfolist_;
    std::map<std::string, std::string> stateMap_;
};

using SensorClusterPtr = std::shared_ptr<ThermalConfigSensorCluster>;
using SensorClusterMap = std::map<std::string, SensorClusterPtr>;
} // namespace PowerMgr
} // namesapce OHOS
#endif // THERMAL_CONFIG_SENSOR_CLUSTER_H
