/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "extend_state_collection.h"

#include "constants.h"
#include "securec.h"
#include "string_operation.h"
#include "thermal_service.h"
#include "thermal_common.h"

namespace OHOS {
namespace PowerMgr {
namespace {
auto g_service = ThermalService::GetInstance();
}
ExtendStateCollection::ExtendStateCollection(const std::string& stateName)
{
    stateName_ = stateName;
}
bool ExtendStateCollection::Init()
{
    return true;
}

bool ExtendStateCollection::InitParam(std::string& params)
{
    return true;
}

std::string ExtendStateCollection::GetState()
{
    return stateValue_;
}

void ExtendStateCollection::SetState(const std::string& stateValue)
{
    stateValue_ = stateValue;
}

bool ExtendStateCollection::DecideState(const std::string& value)
{
    if (stateValue_ == value) {
        THERMAL_HILOGD(COMP_SVC, "stateValue = %{public}s", stateValue_.c_str());
        return true;
    }
    return false;
}
} // namespace PowerMgr
} // namespace OHOS