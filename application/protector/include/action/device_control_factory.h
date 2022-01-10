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
#ifndef DEVICE_CONTROL_FACTORY_H
#define DEVICE_CONTROL_FACTORY_H

#include <memory>
#include "ithermal_action.h"

namespace OHOS {
namespace PowerMgr {
class DeviceControlFactory {
public:
    DeviceControlFactory() = default;
    ~DeviceControlFactory() = default;
    enum {
        MODE_CPU = 0,
        MODE_CURRENT = 1,
        MODE_VOLTAGE = 2,
    };
    static std::shared_ptr<IThermalAction> Create(uint32_t mode);
};
} // namespace PowerMgr
} // namespace OHOS
#endif // DEVICE_CONTROL_FACTORY_H