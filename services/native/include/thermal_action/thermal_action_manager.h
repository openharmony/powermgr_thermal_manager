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

#ifndef THERMAL_ACTION_MANAGER_H
#define THERMAL_ACTION_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include <list>

#include "ithermal_action.h"
#include "action_thermal_level.h"

namespace OHOS {
namespace PowerMgr {
struct ActionItem {
    std::string name;
    std::string params;
    std::string uid;
    std::string protocol;
    bool strict = false;
    bool enableEvent = false;
};

class ThermalActionManager {
public:
    using ThermalActionMap = std::map<std::string, std::shared_ptr<IThermalAction>>;
    bool Init();
    void SubscribeThermalLevelCallback(const sptr<IThermalLevelCallback>& callback);
    void UnSubscribeThermalLevelCallback(const sptr<IThermalLevelCallback>& callback);
    uint32_t GetThermalLevel();
    void SetActionItem(std::vector<ActionItem> vActionItem)
    {
        vActionItem_ = vActionItem;
    }
    std::vector<ActionItem> GetActionItem()
    {
        return vActionItem_;
    }
    ThermalActionMap GetActionMap()
    {
        return actionMap_;
    }
    std::shared_ptr<ActionThermalLevel> GetActionThermalLevelObj()
    {
        return actionThermalLevel_;
    }

    int32_t CreateActionMockFile();
private:
    void InsertActionMap(const std::string& actionName, const std::string& protocol, bool strict, bool enableEvent);
    ThermalActionMap actionMap_;
    std::vector<std::string> actionNameList_;
    std::shared_ptr<ActionThermalLevel> actionThermalLevel_;
    std::vector<ActionItem> vActionItem_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_ACTION_MANAGER_H
