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

#ifndef THERMAL_MGR_ERRORS_H
#define THERMAL_MGR_ERRORS_H

#include <errors.h>

namespace OHOS {
namespace PowerMgr {
enum {
    /**
     *  Module type: THERMAL Manager Service
     */
    THERMAL_MODULE_TYPE_SERVICE = 0,
    /**
     *  Module type: THERMAL Manager Kit
     */
    THERMAL_MODULE_TYPE_KIT = 1
};

// offset of THERMALmgr error, only be used in this file.
constexpr ErrCode THERMALFWK_SERVICE_ERR_OFFSET = ErrCodeOffset(SUBSYS_POWERMNG, THERMAL_MODULE_TYPE_SERVICE);

enum {
    E_WRITE_PARCEL_ERROR_THERMAL = THERMALFWK_SERVICE_ERR_OFFSET,
    E_READ_PARCEL_ERROR_THERMAL,
    E_GET_SYSTEM_ABILITY_MANAGER_FAILED_THERMAL,
    E_GET_THERMAL_SERVICE_FAILED,
    E_ADD_DEATH_RECIPIENT_FAILED_THERMAL,
    E_INNER_ERR_THERMAL,
    E_EXCEED_PARAM_LIMIT,
};

enum class ThermalErrors : int32_t {
    ERR_OK = 0,
    ERR_PERMISSION_DENIED = 201,
    ERR_PARAM_INVALID = 401,
    ERR_CONNECTION_FAIL = 4800101
};
} // namespace ThermalMgr
} // namespace OHOS

#endif // THERMAL_MGR_ERRORS_H