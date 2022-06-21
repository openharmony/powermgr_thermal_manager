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

#include <thread>
#include <condition_variable>
#include <mutex>
#include <list>
#include <vector>
#include <map>
#include <memory>

#include "delayed_sp_singleton.h"
#include "thermal_srv_sensor_info.h"
#include "thermal_config_sensor_cluster.h"
#include "thermalsrv_event_handler.h"

namespace OHOS {
namespace PowerMgr {
struct PolicyAction {
    std::string actionName;
    std::string actionValue;
    std::map<std::string, std::string> mActionProp;
    bool isProp;
};

struct PolicyConfig {
    uint32_t level;
    std::vector<PolicyAction> vPolicyAction;
};


class ThermalPolicy {
public:
    using PolicyConfigMap = std::map<std::string, std::vector<PolicyConfig>>;
    ThermalPolicy();
    ~ThermalPolicy() = default;
    bool Init();
    /* receive sensor temp related */
    void RegisterObserver();
    void GetSensorInfomation(TypeTempMap info);
    void SetPolicyMap(PolicyConfigMap &policyConfigMap);
    void SetSensorClusterMap(std::map<std::string, std::shared_ptr<ThermalConfigSensorCluster>> &msc);
    void DumpConfigInfo();
    void LevelDecision();
    void PolicyDecision(std::map<std::string, uint32_t> &clusterLevelMap);
    void ActionDecision(std::vector<PolicyAction> &vAction);
    bool StateMachineDecision(std::map<std::string, std::string> &stateMap);
    bool ActionFallbackDecision();
    bool ActionExecution();
    void SortLevel();
    void WriteLevel();
    void FindSubscribeActionValue();
public:
    /* Test */
    std::map<std::string, uint32_t> GetClusterLevelMap();
private:
    static bool LevelCompare(const PolicyConfig& r, const PolicyConfig& l)
    {
        return r.level < l.level;
    }
    struct classcomp {
        bool operator()(const std::shared_ptr<IThermalAction> &l, const std::shared_ptr<IThermalAction> &r) const
        {
            return l < r;
        }
    };
private:
    std::set<const std::shared_ptr<IThermalAction>, classcomp> actionFallbackSet_;
    std::set<const std::shared_ptr<IThermalAction>, classcomp> preExecuteList_;
    std::shared_ptr<ThermalsrvEventHandler> handler_;
    std::map<std::string, std::shared_ptr<ThermalConfigSensorCluster>> msc_;
    std::map<std::string, uint32_t> clusterLevelMap_;
    PolicyConfigMap clusterPolicyMap_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_POLICY_H
