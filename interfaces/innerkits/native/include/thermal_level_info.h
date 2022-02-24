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

namespace OHOS {
namespace PowerMgr {
const std::string THERMAL_LEVEL_COOL = "thermal.level.COOL";
const std::string THERMAL_LEVEL_NORMAL = "thermal.level.NORMAL";
const std::string THERMAL_LEVEL_WARM = "thermal.level.WARM";
const std::string THERMAL_LEVEL_HOT = "thermal.level.HOT";
const std::string THERMAL_LEVEL_OVERHEATED = "thermal.level.OVERHEATED";
const std::string THERMAL_LEVEL_WARNING = "thermal.level.WARNING";
const std::string THERMAL_LEVEL_EMERGENCY = "thermal.level.EMERGENCY";

enum class ThermalLevel : int32_t {
    /**
     * @brief Invaild level
     *
     */
        INVALID_LEVEL = -1,
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
} // PowerMgr
} // OHOS

#endif // THERMAL_LEVEL_INFO_H