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

#ifndef THERMAL_LEVEL_INFO_H
#define THERMAL_LEVEL_INFO_H

#include <string>

namespace OHOS {
namespace PowerMgr {
enum class ThermalLevel : int32_t {
    /**
     * @brief cool level
     *
     */
        COOL,
    /**
     * @brief NORMAL level
     *
     */
        NORMAL,
    /**
     * @brief warm level
     *
     */
        WARM,
    /**
     * @brief hot level
     *
     */
        HOT,
    /**
     * @brief overheated level
     *
     */
        OVERHEATED,
    /**
     * @brief warning level
     *
     */
        WARNING,
    /**
     * @brief emergency level, it will make device shutdown
     *
     */
        EMERGENCY,
};

enum class ThermalCommonEventCode : uint32_t {
    /**
     * @brief The thermal level changed common event code.
     */
    CODE_THERMAL_LEVEL_CHANGED = 0
};
} // PowerMgr
} // OHOS

#endif // THERMAL_LEVEL_INFO_H