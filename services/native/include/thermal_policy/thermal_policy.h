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

#ifndef THERMAL_POLICY_H
#define THERMAL_POLICY_H

#include <map>
#include <string>
#include <vector>

#include "thermal_srv_sensor_info.h"
#include "thermal_config_sensor_cluster.h"

namespace OHOS {
namespace PowerMgr {
struct PolicyAction {
    std::string actionName;
    std::string actionValue;
    std::map<std::string, std::string> actionPropMap;
    bool isProp;
};

struct PolicyConfig {
    uint32_t level;
    std::vector<PolicyAction> policyActionList;
};

class ThermalPolicy {
public:
    using PolicyConfigMap = std::map<std::string, std::vector<PolicyConfig>>;
    ThermalPolicy() = default;
    ~ThermalPolicy() = default;

    bool Init();
    void OnSensorInfoReported(const TypeTempMap& info);
    void ExecutePolicy();
    void SetPolicyMap(PolicyConfigMap& pcm);
    void SetSensorClusterMap(SensorClusterMap& scm);
    void FindSubscribeActionValue();
    /* Test */
    std::map<std::string, uint32_t> GetClusterLevelMap();
    void DumpLevel(std::string& result);
    void DumpPolicy(std::string& result);

private:
    void SortLevel();
    void RegisterObserver();
    void WriteLevel();
    void LevelDecision();
    void PolicyDecision();
    void ActionDecision(const std::vector<PolicyAction>& actionList);
    bool StateMachineDecision(const std::map<std::string, std::string>& stateMap);
    bool ActionExecution();
    void PrintPolicyState();
    void PrintPolicyAction(std::vector<PolicyAction> policyActionList, std::string& result);

    static bool LevelCompare(const PolicyConfig& r, const PolicyConfig& l)
    {
        return r.level < l.level;
    }

    TypeTempMap typeTempMap_;
    SensorClusterMap sensorClusterMap_;
    std::map<std::string, uint32_t> clusterLevelMap_;
    PolicyConfigMap clusterPolicyMap_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_POLICY_H
