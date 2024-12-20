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

    std::lock_guard<std::mutex> lock(stateMutex_);
    if (initFlag_) {
        THERMAL_HILOGI(COMP_SVC, "state machine already init");
        return true;
    }
    initFlag_ = true;

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
    if (!receiver_->Register()) {
        THERMAL_HILOGE(COMP_SVC, "ThermalCommonEventReceiver register failed!");
    }
    return true;
}

void StateMachine::UpdateState(std::string stateName, std::string stateValue)
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    auto iter = stateCollectionMap_.find(stateName);
    if (iter != stateCollectionMap_.end()) {
        iter->second->SetState(stateValue);
        return;
    }
    vState_.push_back({stateName, "", false});
    std::shared_ptr<IStateCollection> stateCollection = StateCollectionFactory::Create(stateName);
    stateCollection->SetState(stateValue);
    stateCollectionMap_.emplace(std::pair(stateName, stateCollection));
    THERMAL_HILOGI(COMP_SVC, "StateMachine add state success");
}

void StateMachine::DumpState(std::string& result)
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    for (auto iter = vState_.begin(); iter != vState_.end(); ++iter) {
        result.append("name: ");
        result.append(iter->name);
        if (!iter->params.empty()) {
            result.append("\t");
            result.append("params: ");
            result.append(iter->params);
        }
        result.append("\n");
    }
}

void StateMachine::DumpIdle(std::string& result)
{
    result.append("thermallevel: ");
    result.append(std::to_string(idleStateConfig_.level));
    result.append("\n");
    result.append("soc: ");
    result.append(std::to_string(idleStateConfig_.soc));
    result.append("\n");
    result.append("charging: ");
    result.append(std::to_string(idleStateConfig_.charging));
    result.append("\n");
    result.append("current: ");
    result.append(std::to_string(idleStateConfig_.current));
    result.append("\n");
}
} // namespace PowerMgr
} // namespace OHOS
