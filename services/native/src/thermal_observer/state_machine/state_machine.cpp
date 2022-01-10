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

#include "fcntl.h"
#include "unistd.h"
#include "parameters.h"

#include "string_operation.h"
#include "charger_state_collection.h"
#include "state_collection_factory.h"
#include "thermal_common.h"

namespace OHOS {
namespace PowerMgr {
namespace {
const int MAX_BUFF_SIZE = 128;
}
bool StateMachine::Init()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s StateMachine enter", __func__);
    if (receiver_ == nullptr) {
        receiver_ = std::make_shared<ThermalCommonEventReceiver>();
    }

    for (auto state = vState_.begin(); state != vState_.end(); state++) {
        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s StateMachine name = %{public}s",
            __func__, state->name.c_str());
        std::shared_ptr<IStateCollection> stateCollection = StateCollectionFactory::Create(state->name);
        if (state->isExistParam) {
            stateCollection->InitParam(state->params);
        }
        stateCollection->Init();
        stateCollectionMap_.emplace(std::pair(state->name, stateCollection));
    }
    return true;
}

/**
 * @brief Obtains a system property matching the specified <b>key</b>
 *
 * @param id value type of system property
 * @param key system property value
 * @param def Indicates the default value to return when no query result is found.
 * This parameter is specified by the caller.
 * @return T the current value of the system parameter `key`
 */
template<typename T> T StateMachine::QuerySystemProperty(uint32_t id, const std::string& key, const T& def)
{
    switch (id) {
        case STRING: {
            return OHOS::system::GetParameter(key, def);
        }
        case BOOL: {
            return OHOS::system::GetBoolParameter(key, def);
        }
        case UINT: {
            return OHOS::system::GetUintParameter(key, def);
        }
        case INT: {
            return OHOS::system::GetIntParameter(key, def);
        }
        default: {
            break;
        }
    }
    return def;
}

/**
 * @brief Provide file node read capability
 *
 * @tparam T template type
 * @param path current system file node path
 * @param value Indicates the default value to return when no query result is found.
 * This parameter is specified by the caller.
 * @return T current value of system file node.
 */
template<typename T> T StateMachine::ReadFileNode(const std::string& path, T& value)
{
    char bufNode[MAX_BUFF_SIZE];
    int fd, length;

    fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) {
        THERMAL_HILOGE(MODULE_THERMALHDI_SERVICE, "failed to open file");
        return value;
    }

    length = read(fd, bufNode, sizeof(bufNode));
    if (length < 0) {
        THERMAL_HILOGE(MODULE_THERMALHDI_SERVICE, "failed to read file");
        return value;
    }
    value = static_cast<T>(bufNode);
    close(fd);
    return value;
}
} // namespace PowerMgr
} // namespace OHOS