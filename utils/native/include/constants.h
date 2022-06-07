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

#ifndef OHOS_THERMAL_CONSTANTS_H
#define OHOS_THERMAL_CONSTANTS_H

#include <string>

namespace OHOS {
namespace PowerMgr {
    // action name
    const std::string CPU_ACTION_NAME = "cpu";
    const std::string GPU_ACTION_NAME = "gpu";
    const std::string LCD_ACTION_NAME = "lcd";
    const std::string CHARGER_ACTION_NAME = "current";
    const std::string SHUTDOWN_ACTION_NAME = "shut_down";
    const std::string PROCESS_ACTION_NAME = "process_ctrl";
    const std::string THERMAL_LEVEL_NAME = "thermallevel";
    const std::string POPUP_ACTION_NAME = "popup";
    // state name
    const std::string STATE_CHARGER = "charge";
    const std::string STATE_SCREEN = "screen";
    const std::string STATE_SCNEN = "scene";

    // scene name
    const std::string SCENE_CAMERA = "cam";
    const std::string SCENE_CALL = "call";
    const std::string SCENE_GAME = "game";
    const int MIN = 0;
    const int MAX = 3;
    const int INTERVAL = 5000;

    constexpr int TIME_TO_SLEEP = 1000;
    constexpr int32_t INVAILD_TEMP = -1000;
    constexpr int32_t INVALID_ACTION_VALUE = -2147483648;

    // sensor type
    const std::string BATTERY = "battery";
    const std::string SHELL = "shell";
    const std::string CPU = "cpu";
    const std::string SOC = "soc";
    const std::string AP = "ap";
    const std::string PA = "pa";
    const std::string CHARGER = "charger";
    const std::string AMBIENT = "ambient";

    constexpr int APP_FIRST_UID = 15000;

    // observer
    const int32_t INVALID_TEMP = -1000000;
    const int32_t TYPE_MAX_SIZE = 10;

    // fallback value
    const std::string FALLBACK_ACTION_VALUE = "0";
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
    // dialog
    const std::string THERMAL_LOWER_TEMP_PARAMS = "{\"cancelButton\":\"LowerTemp Cancel\"}";
    const std::string THERMAL_HIGH_TEMP_PARAMS = "{\"cancelButton\":\"HighTemp Cancel\"}";
} // namespace PowerMgr
} // namespace OHOS
#endif // OHOS_THERMAL_CONSTANTS_H
