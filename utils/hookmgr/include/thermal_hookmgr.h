/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef BATTERYMGR_UTILS_BATTERY_HOOKMGR_H
#define BATTERYMGR_UTILS_BATTERY_HOOKMGR_H

#include <cstdint>
#include <string>
#include "hookmgr.h"
#include "list.h"
#include "modulemgr.h"


namespace OHOS::PowerMgr {
enum class ThermalHookStage : int32_t {
    THERMAL_DECRYPT_CONFIG = 0,
    THERMAL_HOOK_STAGE_MAX = 1000,
};

struct DecryptConfigContext {
    int32_t configIndex;
    std::string result;
};

HOOK_MGR* GetThermalHookMgr();
} // namespace OHOS::PowerMgr

#endif