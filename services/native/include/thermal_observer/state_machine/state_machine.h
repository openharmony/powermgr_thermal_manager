/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <string>
#include <vector>
#include <memory>
#include <map>

#include <istate_collection.h>
#include "thermal_common_event_receiver.h"

namespace OHOS {
namespace PowerMgr {
struct StateItem {
    std::string name;
    std::string params;
    bool isExistParam = false;
};

class StateMachine {
public:
    enum {
        STRING = 0,
        BOOL = 1,
        UINT = 2,
        INT = 3,
    };
    using StateMachineMap = std::map<std::string, std::shared_ptr<IStateCollection>>;
    bool Init();
    bool CreateInstance();
    /* Provide system attribute query capability */
    template<typename T> T QuerySystemProperty(uint32_t id, const std::string& key, const T& def);
    /* Provide file node read capability */
    template<typename T> T ReadFileNode(const std::string& path, T& value);
    void CommonEventReceiverInit();
    void SetStateItem(std::vector<StateItem>& vstateItem)
    {
        vState_ = vstateItem;
    }

    StateMachineMap GetStateCollectionMap()
    {
        return stateCollectionMap_;
    }

    std::shared_ptr<ThermalCommonEventReceiver> GetCommonEventReceiver()
    {
        return receiver_;
    }
private:
    StateMachineMap stateCollectionMap_;
    std::vector<StateItem> vState_;
    std::shared_ptr<ThermalCommonEventReceiver> receiver_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // STATE_MACHINE_H