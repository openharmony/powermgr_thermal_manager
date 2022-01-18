/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef THERMAL_KERNEL_POLICY_H
#define THERMAL_KERNEL_POLICY_H

#include <map>
#include <set>
#include "thermal_kernel_config_file.h"
#include "protector_thermal_zone_info.h"
#include "ithermal_action.h"

namespace OHOS {
namespace PowerMgr {
struct ActionItem {
    std::string name;
    uint32_t value;
};

struct LevelAction {
    std::string name;
    uint32_t level;
    std::vector<ActionItem> vAction;
};

class ThermalKernelPolicy {
public:
    using ThermalZoneMap = std::map<std::string, std::shared_ptr<ProtectorThermalZoneInfo>>;
    using LevelMap = std::map<std::string, uint32_t>;
    using TypeLevelsMap = std::map<std::string, std::vector<uint32_t>>;
    ThermalKernelPolicy() {};
    ~ThermalKernelPolicy() {};

    bool Init();
    void LevelDecision();
    void PolicyDecision();
    void ActionDecision(std::vector<ActionItem> &vAction);
    void ActionExecution();
    bool ActionFallbackDecision();
    void SetCallback();
    void OnReceivedSensorsInfo(SensorsMap typeTempMap);
    void SetThermalZoneMap(ThermalZoneMap &tzInfoMap);
    void SetLevelAction(std::vector<LevelAction> &levelAction);
    void Dump();
    std::vector<LevelAction> GetLevelAction();
private:
    struct classcomp {
        bool operator()(const std::shared_ptr<IThermalAction> &l, const std::shared_ptr<IThermalAction> &r) const
        {
            return l < r;
        }
    };
    TypeLevelsMap typeLevelsMap_;
    SensorsMap typeTempMap_;
    ThermalZoneMap tzInfoMap_;
    LevelMap levelMap_;
    std::vector<LevelAction> levelAction_;
    std::set<const std::shared_ptr<IThermalAction>, classcomp> actionFallbackSet_, preExecuteList_;
    std::vector<std::string> executeActionList_;
};
} // nameapce PowerMgr
} // namespace OHOS
#endif