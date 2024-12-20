/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef ACTION_AIRPLANE_H
#define ACTION_AIRPLANE_H

#include "ithermal_action.h"

namespace OHOS {
namespace PowerMgr {
class ActionAirplane : public IThermalAction {
public:
    ActionAirplane(const std::string& actionName);
    ~ActionAirplane() = default;
    enum TempStatus {
        LOWER_TEMP = 0,
        HIGHER_TEMP = 1
    };
    void InitParams(const std::string& params) override;
    void SetStrict(bool enable) override;
    void SetEnableEvent(bool enable) override;
    void AddActionValue(uint32_t actionId, std::string value) override;
    void ExecuteInner() override;
    int32_t AirplaneRequest(const uint32_t& value);
    int32_t AirplaneExecution(const uint32_t& value);
private:
    uint32_t GetActionValue();
    std::vector<uint32_t> valueList_;
    uint32_t lastValue_ {0};
};
} // namespace PowerMgr
} // namespace OHOS
#endif // ACTION_AIRPLANE_H