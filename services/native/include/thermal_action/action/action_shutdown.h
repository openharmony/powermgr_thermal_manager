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

#ifndef ACTION_SHUTDOWN_H
#define ACTION_SHUTDOWN_H

#include "ithermal_action.h"

namespace OHOS {
namespace PowerMgr {
class ActionShutdown : public IThermalAction {
public:
    ActionShutdown(const std::string& actionName);
    ~ActionShutdown() = default;

    void InitParams(const std::string& params) override;
    virtual void SetStrict(bool flag) override;
    virtual void SetEnableEvent(bool enable) override;
    virtual void AddActionValue(std::string value) override;
    virtual void Execute() override;
    uint32_t ShutdownRequest(bool isShutdown);
    uint32_t DelayShutdown(bool isShutdown, int32_t temp, int32_t thresholdClr);
    /* the api is used to test */
    int32_t ShutdownExecution(bool isShutdown);
private:
    bool flag_;
    bool enableEvent_ = false;
    std::vector<int32_t> valuesList_;
    int32_t lastValue_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // ACTION_APPLICATION_PROCESS_H
