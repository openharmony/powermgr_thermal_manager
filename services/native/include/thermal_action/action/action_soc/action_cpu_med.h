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

#ifndef ACTION_CPU_MED_H
#define ACTION_CPU_MED_H

#include "ithermal_action.h"

namespace OHOS {
namespace PowerMgr {
class ActionCpuMed : public IThermalAction {
public:
    ActionCpuMed(const std::string& actionName);
    ~ActionCpuMed() = default;

    void InitParams(const std::string& params) override;
    virtual void SetStrict(bool flag) override;
    virtual void SetEnableEvent(bool enable) override;
    virtual void AddActionValue(std::string value) override;
    virtual void Execute() override;
    int32_t CpuRuquest(uint32_t freq);
private:
    std::vector<uint32_t> valueList_;
    bool flag_;
    bool enableEvent_ = false;
    uint32_t lastValue_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // ACTION_CPU_MED_H
