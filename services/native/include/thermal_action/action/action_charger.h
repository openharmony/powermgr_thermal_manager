/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef ACTION_CHARGER_H
#define ACTION_CHARGER_H

#include "ithermal_action.h"
#ifdef DRIVERS_INTERFACE_BATTERY_ENABLE
#include "v2_0/types.h"
#endif

namespace OHOS {
namespace PowerMgr {
#ifdef DRIVERS_INTERFACE_BATTERY_ENABLE
using namespace OHOS::HDI::Battery::V2_0;
#endif
class ActionCharger : public IThermalAction {
public:
    ActionCharger(const std::string& actionName);
    ~ActionCharger() = default;

    void InitParams(const std::string& protocol) override;
    void SetStrict(bool enable) override;
    void SetEnableEvent(bool enable) override;
    void AddActionValue(uint32_t actionId, std::string value) override;
    void ExecuteInner() override;
    void ResetActionValue() override;
    int32_t ChargerRequest(int32_t current);
    int32_t WriteSimValue(int32_t simValue);
    static void ExecuteCurrentLimit();
private:
    uint32_t GetActionValue();
    std::vector<uint32_t> valueList_;
    uint32_t lastValue_ {0};
    std::string protocol_;
#ifdef DRIVERS_INTERFACE_BATTERY_ENABLE
    static std::vector<ChargingLimit> chargeLimitList_;
#endif
};
} // namespace PowerMgr
} // namespace OHOS
#endif // ACTION_CPU_H
