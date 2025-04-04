/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef THERMAL_ACTION_FACTORY_H
#define THERMAL_ACTION_FACTORY_H

#include <functional>
#include <memory>

#include "ithermal_action.h"

namespace OHOS {
namespace PowerMgr {
using ActionFunc = std::function<std::shared_ptr<IThermalAction>(std::string)>;
class ThermalActionFactory {
public:
    static void InitFactory();
    static std::shared_ptr<IThermalAction> Create(const std::string& actionClass, const std::string& actionName);
};
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_ACTION_FACTORY_H
