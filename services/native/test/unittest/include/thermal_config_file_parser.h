/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef THERMAL_CONFIG_PARSER_H
#define THERMAL_CONFIG_PARSER_H

#include <string>
#include <vector>
#include <libxml/tree.h>

#include "state_machine.h"
#include "thermal_action_manager.h"
#include "thermal_config_base_info.h"
#include "thermal_config_sensor_cluster.h"
#include "thermal_log.h"
#include "thermal_policy.h"

namespace OHOS {
namespace PowerMgr {
class ThermalConfigFileParser {
public:
    ThermalConfigFileParser()
    {
        THERMAL_HILOGI(LABEL_TEST, "Thermal config xml file parser instance is created");
    }
    ~ThermalConfigFileParser() = default;
    bool Init();
    bool GetActionEnableEvent(const std::string& actionName);
    bool GetActionStrict(const std::string& actionName);
    bool GetActionPolicy(const std::string& name, uint32_t level, std::vector<PolicyAction>& policy);
    std::vector<StateItem> GetStateItem();
private:
    bool LoadThermalSrvConfigXml(const std::string& path);
    bool ParseXmlFile(const std::string& path);
    void ParseBaseNode(xmlNodePtr node);
    void ParseLevelNode(xmlNodePtr node);
    void ParseStateNode(xmlNodePtr node);
    void ParseActionNode(xmlNodePtr node);
    void ParsePolicyNode(xmlNodePtr node);
    void ParseIdleNode(xmlNodePtr node);
    void ParseAuxSensorInfo(const xmlNode* cur, std::shared_ptr<ThermalConfigSensorCluster>& sc);
    void ParseSensorInfo(const xmlNode* cur, std::shared_ptr<ThermalConfigSensorCluster>& sc);
    std::vector<AuxLevelItem> ParseAuxSensorSubnodeInfo(const xmlNode* cur,
        std::vector<std::string>& auxSensorList, const uint32_t i);
    bool ParseAuxSensorSubnodeInfoTrigerRange(const xmlNode* subNode, std::vector<std::string>& auxSensorList,
        std::string& tempRanges, const uint32_t i);
    void ParseSensorSubnodeInfo(const xmlNode* cur, std::vector<LevelItem>& vItem, std::vector<std::string>& sensors,
        const uint32_t i, std::shared_ptr<ThermalConfigSensorCluster>& sc);
    void ParsePolicySubnode(const xmlNode* cur, PolicyConfig& policyConfig);
    BaseInfoMap baseInfoMap_;
    std::vector<StateItem> stateItem_;
    std::vector<ActionItem> actionItem_;
    ThermalPolicy::PolicyConfigMap policyConfigMap_;
    std::map<std::string, std::shared_ptr<ThermalConfigSensorCluster>> sensorClusterMap_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif //  THERMAL_CONFIG_PARSER_H