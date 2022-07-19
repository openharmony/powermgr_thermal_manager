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

#ifndef ACTION_VOLATEG_H
#define ACTION_VOLATEG_H

#include "ithermal_action.h"
#include "v1_1/types.h"

namespace OHOS {
namespace PowerMgr {
using namespace OHOS::HDI::Battery::V1_1;
class ActionVoltage : public IThermalAction {
public:
    ActionVoltage(const std::string& actionName);
    ~ActionVoltage() = default;

    void InitParams(const std::string& protocol) override;
    virtual void SetStrict(bool flag) override;
    virtual void SetEnableEvent(bool enable) override;
    virtual void AddActionValue(std::string value) override;
    virtual void Execute() override;
    int32_t SetVoltage(int32_t voltage);
    int32_t WriteMockNode(int32_t mockValue);
    static void ExecuteVoltageLimit();
private:
    std::vector<uint32_t> valueList_;
    bool flag_;
    bool enableEvent_ = false;
    uint32_t lastValue_;
    std::string protocol_;
    static std::vector<ChargingLimit> chargeLimitList_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // ACTION_CPU_H
