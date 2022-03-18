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
    void SetThermalZoneMap(ThermalZoneMap &tzInfoMap);
    ThermalZoneMap GetThermalZoneMap();
    void SetLevelAction(std::vector<LevelAction> &levelAction);
    std::vector<LevelAction> GetLevelAction();
    int32_t GetMaxCd();
private:
    void LevelDecision();
    void PolicyDecision();
    void ActionDecision(std::vector<ActionItem> &vAction);
    void ActionExecution();
    void ActionFallbackDecision();
    void SetCallback();
    void OnReceivedSensorsInfo(SensorsMap typeTempMap);
    void Dump();
    /* Gets the maximum common divisor of the time interval */
    int32_t GetMaxCommonDivisor(int32_t a, int32_t b);
    int32_t GetIntervalCommonDivisor(std::vector<int32_t> intervalList);
    void SetMultiples();
    void CalculateMaxCd();
    TypeLevelsMap typeLevelsMap_;
    SensorsMap typeTempMap_;
    ThermalZoneMap tzInfoMap_;
    LevelMap levelMap_;
    std::vector<LevelAction> levelAction_;
    std::vector<std::string> executeActionList_;
    int32_t maxCd_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif