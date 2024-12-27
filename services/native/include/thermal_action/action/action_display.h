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

#ifndef ACTION_DISPLAY_H
#define ACTION_DISPLAY_H

#include "ithermal_action.h"

namespace OHOS {
namespace PowerMgr {
class ActionDisplay : public IThermalAction {
public:
    ActionDisplay(const std::string& actionName);
    ~ActionDisplay() = default;

    void InitParams(const std::string& params) override;
    void SetStrict(bool enable) override;
    void SetEnableEvent(bool enable) override;
    void AddActionValue(uint32_t actionId, std::string value) override;
    void ExecuteInner() override;
    void ResetActionValue() override;
    void RequestDisplay(float factor);
    void ExecuteMock(float factor);

private:
    float GetActionValue();
    float lastValue_ {0.0f};
    std::vector<float> valueList_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // ACTION_DISPLAY_H
