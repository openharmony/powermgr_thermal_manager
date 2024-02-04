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

#ifndef THERMAL_SRV_CONFIG_PARSER_H
#define THERMAL_SRV_CONFIG_PARSER_H

#include <string>
#include <vector>
#include <libxml/tree.h>
#include "fan_fault_detect.h"
#include "thermal_action_manager.h"
#include "thermal_config_sensor_cluster.h"
#include "thermal_policy.h"

namespace OHOS {
namespace PowerMgr {
class ThermalSrvConfigParser {
public:
    ThermalSrvConfigParser();
    ~ThermalSrvConfigParser() = default;
    ThermalSrvConfigParser(const ThermalSrvConfigParser&) = delete;
    ThermalSrvConfigParser& operator=(const ThermalSrvConfigParser&) = delete;
    bool ThermalSrvConfigInit(const std::string& path);
    bool ParseXmlFile(const std::string& path);

private:
    bool ParseRootNode(const xmlNodePtr& node);
    bool ParseBaseNode(const xmlNodePtr& node);
    bool ParseLevelNode(const xmlNodePtr& node);
    bool ParseStateNode(const xmlNodePtr& node);
    bool ParseActionNode(const xmlNodePtr& node);
    bool ParsePolicyNode(const xmlNodePtr& node);
    bool ParseIdleNode(const xmlNodePtr& node);
    bool ParseLevelState(const xmlNodePtr& cur, SensorClusterPtr& sc);
    bool ParseAuxSensorInfo(const xmlNodePtr& cur, SensorClusterPtr& sc);
    bool ParseSensorInfo(const xmlNodePtr& cur, SensorClusterPtr& sc);
    bool ParseAuxSensorLevInfo(const xmlNodePtr& cur, std::vector<std::string>& auxsensors,
        const uint32_t sensorIdx, std::vector<AuxLevelItem>& auxLevelItem);
    bool ParseAuxSensorTriggerRange(const xmlNodePtr& subNode, std::vector<std::string>& auxsensors,
        std::string& tempRangeStr, const uint32_t sensorIdx);
    bool ParseSensorLevelInfo(const xmlNodePtr& cur, std::vector<LevelItem>& levelItems,
        std::vector<std::string>& sensors, const uint32_t sensorIdx, SensorClusterPtr& sc);
    bool ParseLevelThreshold(const xmlNodePtr& subNode, LevelItem& levelItem,
        std::vector<std::string>& sensors, const uint32_t sensorIdx);
    bool ParseActionInfo(const xmlNodePtr& cur, ActionItem& ai);
    bool ParsePolicyActionInfo(const xmlNodePtr& cur, PolicyConfig& policyConfig);
    bool ParseFanNode(const xmlNodePtr& cur);
    bool ParseFanFaultInfo(const xmlNodePtr& cur,
        std::vector<std::string> &sensors, FanFaultInfoMap &fanFaultInfoMap);
};
} // namespace PowerMgr
} // namespace OHOS
#endif //  THERMAL_SRV_CONFIG_PARSER_H
