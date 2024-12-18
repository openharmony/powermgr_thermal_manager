/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef ACTION_NODE_H
#define ACTION_NODE_H

#include <string>

#include "ithermal_action.h"

namespace OHOS {
namespace PowerMgr {
class ActionNode : public IThermalAction {
public:
    ActionNode(const std::string& actionName);
    ~ActionNode() = default;

    void InitParams(const std::string& params) override;
    void SetStrict(bool enable) override;
    void SetEnableEvent(bool enable) override;
    void AddActionValue(std::string value) override;
    void Execute() override;

private:
    int32_t lastValue_ {INT_MAX};
    int32_t fallbackValue_ {0};
    std::string nodePath_;
    std::vector<int32_t> valueList_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // ACTION_NODE_H
