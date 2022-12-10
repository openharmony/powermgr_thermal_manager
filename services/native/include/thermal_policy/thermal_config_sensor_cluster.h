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
using SensorClusterPtr = std::shared_ptr<ThermalConfigSensorCluster>;
using SensorClusterMap = std::map<std::string, SensorClusterPtr>;

class ThermalConfigSensorCluster {
public:
    bool CheckStandard();
    void UpdateThermalLevel(TypeTempMap& typeTempInfo);

    uint32_t GetCurrentLevel()
    {
        return latestLevel_;
    }
    SensorInfoMap& GetSensorInfoList()
    {
        return sensorInfolist_;
    }
    AuxSensorInfoMap& GetAuxSensorInfoList()
    {
        return auxSensorInfolist_;
    }

    void SetSensorLevelInfo(SensorInfoMap& sensorInfolist)
    {
        sensorInfolist_ = sensorInfolist;
    }
    void SetAuxSensorLevelInfo(AuxSensorInfoMap& auxSensorInfolist)
    {
        auxSensorInfolist_ = auxSensorInfolist;
    }
    void SetDescFlag(bool descflag)
    {
        descFlag_ = descflag;
    }
    void SetAuxFlag(bool auxflag)
    {
        auxFlag_ = auxflag;
    }
    void SetRateFlag(bool rateFlag)
    {
        rateFlag_ =  rateFlag;
    }

private:
    void CalculateSensorLevel(TypeTempMap& typeTempInfo, std::vector<uint32_t>& levelList);
    void AscJudgment(std::vector<LevelItem>& levItems, int32_t curTemp, uint32_t& level);
    void DescJudgment(std::vector<LevelItem>& levItems, int32_t curTemp, uint32_t& level);
    void CheckExtraCondition(TypeTempMap& typeTempInfo, uint32_t& level);
    bool IsTempRateTrigger(uint32_t& level);
    bool IsAuxSensorTrigger(TypeTempMap& typeTempInfo, uint32_t& level);

    bool descFlag_ {false};
    bool auxFlag_ {false};
    bool rateFlag_ {false};
    uint32_t latestLevel_ {0};
    SensorInfoMap sensorInfolist_;
    AuxSensorInfoMap auxSensorInfolist_;
};
} // namespace PowerMgr
} // namesapce OHOS

#endif // THERMAL_CONFIG_SENSOR_CLUSTER_H