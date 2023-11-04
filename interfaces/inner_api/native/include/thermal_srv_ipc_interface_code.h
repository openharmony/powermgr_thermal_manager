/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef THERMAL_MGR_IPC_INTERFACE_DODE_H
#define THERMAL_MGR_IPC_INTERFACE_DODE_H

/* SAID: 3303 */
namespace OHOS {
namespace PowerMgr {
enum class ThermalMgrInterfaceCode {
    REG_THERMAL_TEMP_CALLBACK = 0,
    UNREG_THERMAL_TEMP_CALLBACK,
    REG_THERMAL_LEVEL_CALLBACK,
    UNREG_THERMAL_LEVEL_CALLBACK,
    REG_THERMAL_ACTION_CALLBACK,
    UNREG_THERMAL_ACTION_CALLBACK,
    GET_SENSOR_INFO,
    GET_TEMP_LEVEL,
    GET_THERMAL_INFO,
    SET_SCENE,
    UPDATE_THERMAL_STATE,
    SHELL_DUMP
};
} // space PowerMgr
} // namespace OHOS

#endif // THERMAL_MGR_IPC_INTERFACE_DODE_H