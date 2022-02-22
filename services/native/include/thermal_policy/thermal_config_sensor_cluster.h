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
#include <mutex>

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
    bool Init();
    void UpdateThermalLevel(TypeTempMap &typeTempInfo);
    void CalculateSensorLevel(TypeTempMap &typeTempInfo, std::vector<uint32_t> &levelList, uint32_t &level);
    bool IsAuxSensorTrigger(TypeTempMap &typeTempInfo, uint32_t &level);
    bool IsTempRateTrigger(TypeTempMap &typeTempInfo, uint32_t &level);
    void AscJudgment(std::vector<LevelItem> &vlev, int32_t curTemp, uint32_t &level);
    void DescJudgment(std::vector<LevelItem> &vlev, int32_t curTemp, uint32_t &level);
    static bool CmpValue(const std::pair<std::string, uint32_t> left, const std::pair<std::string, uint32_t> right);

    void Dump();
    uint32_t GetCurrentLevel();
    bool GetDescFlag();
    bool GetAuxFlag();
    bool GetRateFlag();

    SensorInfoMap GetSensorInfoList();
    AuxSensorInfoMap GetAuxSensorInfoList();
    void SetSensorLevelInfo(SensorInfoMap &sensorInfolist);
    void SetAuxSensorLevelInfo(AuxSensorInfoMap &auxSensorInfolist);
    void SetDescFlag(bool descflag);
    void SetAuxFlag(bool auxflag);
    void SetRateFlag(bool rateFlag);
private:
    bool descFlag_ {false};
    bool auxFlag_ {false};
    bool rateFlag_ {false};
    uint32_t latestLevel_;
    SensorInfoMap sensorInfolist_;
    AuxSensorInfoMap auxSensorInfolist_;
    std::map<std::string, bool> isRateMap_;
    std::mutex levelMutex_;
};
} // namespace PowerMgr
} // namesapce OHOS

#endif // THERMAL_CONFIG_SENSOR_CLUSTER_H