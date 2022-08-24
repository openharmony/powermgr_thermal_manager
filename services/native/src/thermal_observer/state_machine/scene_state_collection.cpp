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

#include "scene_state_collection.h"

#include <algorithm>

#include "constants.h"
#include "file_operation.h"
#include "securec.h"
#include "string_operation.h"
#include "thermal_service.h"
#include "thermal_common.h"

namespace OHOS {
namespace PowerMgr {
namespace {
auto g_service = DelayedSpSingleton<ThermalService>::GetInstance();
}
bool SceneStateCollection::Init()
{
    return true;
}

bool SceneStateCollection::InitParam(std::string& params)
{
    return true;
}

std::string SceneStateCollection::GetState()
{
    return "";
}

void SceneStateCollection::SetState()
{
}

bool SceneStateCollection::DecideState(const std::string& value)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    if ((!g_service->GetScene().empty()) && (g_service->GetScene() == value)) {
        std::string scene = g_service->GetScene();
        THERMAL_HILOGD(COMP_SVC, "scene = %{public}s", scene.c_str());
        return true;
    }

    return false;
}
} // namespace PowerMgr
} // namespace OHOS