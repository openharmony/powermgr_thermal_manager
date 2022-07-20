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
#ifndef ACTION_POPUP_H
#define ACTION_POPUP_H

#include "ithermal_action.h"
#include "window_manager.h"
#include "refbase.h"

namespace OHOS {
namespace PowerMgr {
class ActionPopup : public IThermalAction {
public:
    ActionPopup(const std::string& actionName);
    ~ActionPopup() = default;
    enum {
        LOWER_TEMP = 1,
        HIGHER_TEMP = 2
    };
    void InitParams(const std::string& params) override;
    virtual void SetStrict(bool flag) override;
    virtual void SetEnableEvent(bool enable) override;
    virtual void AddActionValue(std::string value) override;
    virtual void Execute() override;
    bool ShowDialog(const std::string &params);
    void HandlePopupEvent(const int32_t value);
    void GetDisplayPosition(int32_t& width, int32_t& height);
private:
    std::vector<uint32_t> valueList_;
    bool flag_;
    bool enableEvent_ = false;
    uint32_t lastValue_;
    int32_t dialogId_ {-1};
};
} // namespace PowerMgr
} // namespace OHOS
#endif // ACTION_POPUP_H
