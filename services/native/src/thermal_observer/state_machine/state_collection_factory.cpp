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

#include "state_collection_factory.h"

#include "charger_state_collection.h"
#include "screen_state_collection.h"
#include "scene_state_collection.h"
#include "constants.h"
#include "thermal_common.h"

namespace OHOS {
namespace PowerMgr {
std::shared_ptr<IStateCollection> StateCollectionFactory::Create(std::string &stateName)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    if (stateName == STATE_CHARGER) {
        return std::make_shared<ChargerStateCollection>();
    } else if (stateName == STATE_SCREEN) {
        return std::make_shared<ScreenStateCollection>();
    } else if (stateName == STATE_SCNEN) {
        return std::make_shared<SceneStateCollection>();
    } else {
        return nullptr;
    }
}
} // namespace PowerMgr
} // namespace OHOS