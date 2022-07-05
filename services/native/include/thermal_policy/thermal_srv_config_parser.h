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
#include <mutex>
#include <memory>
#include <libxml/tree.h>
#include "thermal_config_sensor_cluster.h"
#include "thermal_policy.h"

namespace OHOS {
namespace PowerMgr {
class ThermalSrvConfigParser {
public:
    ThermalSrvConfigParser();
    static ThermalSrvConfigParser &GetInstance();
    ~ThermalSrvConfigParser() = default;
    ThermalSrvConfigParser(const ThermalSrvConfigParser&) = delete;
    ThermalSrvConfigParser& operator=(const ThermalSrvConfigParser&) = delete;
    bool ThermalSrvConfigInit(std::string &path);
    bool ParseXMLFile(std::string &path);
private:
    void ParseBaseNode(xmlNodePtr node);
    void ParseLevelNode(xmlNodePtr node);
    void ParseStateNode(xmlNodePtr node);
    void ParseActionNode(xmlNodePtr node);
    void ParsePolicyNode(xmlNodePtr node);
    void ParseIdleNode(xmlNodePtr node);
    void ParseAuxSensorInfo(const xmlNode *cur, std::shared_ptr<ThermalConfigSensorCluster> &sc);
    void ParseSensorInfo(const xmlNode *cur, std::shared_ptr<ThermalConfigSensorCluster> &sc);
    std::vector<AuxLevelItem> ParseAuxSensorSubnodeInfo(const xmlNode *cur,
        std::vector<std::string>& auxsensors, const uint32_t i);
    bool ParseAuxSensorSubnodeInfoTrigerRange(const xmlNode *subNode, std::vector<std::string>& auxsensors,
        std::string& tempRanges, const uint32_t i);
    void ParseSensorSubnodeInfo(const xmlNode *cur, std::vector<LevelItem>& vItem, std::vector<std::string>& sensors,
        const uint32_t i, std::shared_ptr<ThermalConfigSensorCluster> &sc);
    void ParsePolicySubnode(const xmlNode *cur, PolicyConfig& policyConfig);
    std::mutex mutex_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif //  THERMAL_SRV_CONFIG_PARSER_H
