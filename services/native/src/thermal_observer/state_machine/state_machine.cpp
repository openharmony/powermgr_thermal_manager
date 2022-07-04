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

#include "state_machine/state_machine.h"
#include "state_collection_factory.h"
#include "thermal_common.h"

namespace OHOS {
namespace PowerMgr {
bool StateMachine::Init()
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    if (receiver_ == nullptr) {
        receiver_ = std::make_shared<ThermalCommonEventReceiver>();
    }

    for (auto state = vState_.begin(); state != vState_.end(); state++) {
        THERMAL_HILOGI(COMP_SVC, "StateMachine name = %{public}s", state->name.c_str());
        std::shared_ptr<IStateCollection> stateCollection = StateCollectionFactory::Create(state->name);
        if (state->isExistParam) {
            stateCollection->InitParam(state->params);
        }
        stateCollection->Init();
        stateCollectionMap_.emplace(std::pair(state->name, stateCollection));
    }
    return true;
}
} // namespace PowerMgr
} // namespace OHOS
