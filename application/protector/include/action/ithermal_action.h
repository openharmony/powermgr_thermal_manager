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
#ifndef ITHERMAL_ACTION_H
#define ITHERMAL_ACTION_H

#include <stdint.h>

namespace OHOS {
namespace PowerMgr {
class IThermalAction {
public:
    IThermalAction() = default;
    virtual ~IThermalAction() = default;
    virtual bool AddActionValue(uint32_t value) = 0;
    virtual void Execute() = 0;
};
} // namespace PowerMgr
} // namespace OHOS
#endif