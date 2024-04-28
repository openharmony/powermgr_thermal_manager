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

#ifndef ITHERMAL_ACTION_H
#define ITHERMAL_ACTION_H

#include <map>
#include <string>
#include <vector>

#include "thermal_common.h"

namespace OHOS {
namespace PowerMgr {
class IThermalAction {
public:
    IThermalAction() = default;
    virtual ~IThermalAction() = default;

    virtual void InitParams(const std::string& params) = 0;
    virtual void SetStrict(bool enable) = 0;
    virtual void SetEnableEvent(bool enable) = 0;
    virtual void AddActionValue(std::string value) = 0;
    virtual void Execute() = 0;

protected:
    bool isStrict_ {true};
    bool enableEvent_ {false};
    std::string actionName_ = "";
};
} // namespace PowerMgr
} // namespace OHOS
#endif // ITHERMAL_ACTION_H
