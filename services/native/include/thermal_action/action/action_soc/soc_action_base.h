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

#ifndef SOC_ACTION_BASE_H
#define SOC_ACTION_BASE_H

#include "ithermal_action.h"

namespace OHOS {
namespace PowerMgr {
class SocActionBase {
public:
    SocActionBase() = default;
    ~SocActionBase() = default;

protected:
    virtual void SocLimitRequest(int32_t tag, int64_t value);
    virtual void SetSocPerfThermalLevel(uint32_t level);
    virtual void SocIsolateRequest(bool enable);
};
} // namespace PowerMgr
} // namespace OHOS
#endif // SOC_ACTION_BASE_H
