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

#ifndef OHOS_THERMAL_CONSTANTS_H
#define OHOS_THERMAL_CONSTANTS_H

#include <string>

namespace OHOS {
namespace PowerMgr {
    // action name
    constexpr const char* CPU_BIG_ACTION_NAME = "cpu_big";
    constexpr const char* CPU_MED_ACTION_NAME = "cpu_med";
    constexpr const char* CPU_LIT_ACTION_NAME = "cpu_lit";
    constexpr const char* GPU_ACTION_NAME = "gpu";
    constexpr const char* LCD_ACTION_NAME = "lcd";
    constexpr const char* VOLUME_ACTION_NAME = "volume";
    constexpr const char* SHUTDOWN_ACTION_NAME = "shut_down";
    constexpr const char* PROCESS_ACTION_NAME = "process_ctrl";
    constexpr const char* THERMAL_LEVEL_NAME = "thermallevel";
    constexpr const char* POPUP_ACTION_NAME = "popup";
    constexpr const char* CURRENT_SC_ACTION_NAME = "current_sc";
    constexpr const char* CURRENT_BUCK_ACTION_NAME = "current_buck";
    constexpr const char* VOLATAGE_SC_ACTION_NAME = "voltage_sc";
    constexpr const char* VOLATAGE_BUCK_ACTION_NAME = "voltage_buck";
    // state name
    constexpr const char* STATE_CHARGER = "charge";
    constexpr const char* STATE_SCREEN = "screen";
    constexpr const char* STATE_SCNEN = "scene";

    // scene name
    constexpr const char* SCENE_CAMERA = "cam";
    constexpr const char* SCENE_CALL = "call";
    constexpr const char* SCENE_GAME = "game";
    const int MIN = 0;
    const int MAX = 3;
    const int INTERVAL = 5000;
    constexpr const char* SC_PROTOCOL = "sc";
    constexpr const char* BUCK_PROTOCOL = "buck";
    constexpr int TIME_TO_SLEEP = 1000;
    constexpr int32_t INVAILD_TEMP = -1000;
    constexpr float INVALID_ACTION_VALUE = -3.1415926;

    // sensor type
    constexpr const char* BATTERY = "battery";
    constexpr const char* SHELL = "shell";
    constexpr const char* CPU = "cpu";
    constexpr const char* SOC = "soc";
    constexpr const char* AP = "ap";
    constexpr const char* PA = "pa";
    constexpr const char* CHARGER = "charger";
    constexpr const char* AMBIENT = "ambient";

    constexpr int APP_FIRST_UID = 15000;

    // observer
    const int32_t INVALID_TEMP = -1000000;
    const int32_t TYPE_MAX_SIZE = 10;

    // fallback value
    constexpr const char* FALLBACK_ACTION_VALUE = "0";
    const uint32_t ARG_0 = 0;
    const uint32_t ARG_1 = 1;
    const uint32_t ARG_2 = 2;
    const uint32_t ARG_3 = 3;
    const uint32_t ARG_4 = 4;
    const uint32_t ARG_5 = 5;
    const uint32_t ARG_6 = 6;
    const uint32_t ARG_7 = 7;
    const uint32_t ARG_8 = 8;
    const uint32_t ARG_9 = 9;
} // namespace PowerMgr
} // namespace OHOS
#endif // OHOS_THERMAL_CONSTANTS_H
