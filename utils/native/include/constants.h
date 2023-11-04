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

#ifndef OHOS_THERMAL_CONSTANTS_H
#define OHOS_THERMAL_CONSTANTS_H

#include <string>

namespace OHOS {
namespace PowerMgr {
    // action name
    const std::string CPU_BIG_ACTION_NAME = "cpu_big";
    const std::string CPU_MED_ACTION_NAME = "cpu_med";
    const std::string CPU_LIT_ACTION_NAME = "cpu_lit";
    const std::string CPU_BOOST_ACTION_NAME = "boost";
    const std::string CPU_ISOLATE_ACTION_NAME = "isolate";
    const std::string GPU_ACTION_NAME = "gpu";
    const std::string LCD_ACTION_NAME = "lcd";
    const std::string VOLUME_ACTION_NAME = "volume";
    const std::string SHUTDOWN_ACTION_NAME = "shut_down";
    const std::string PROCESS_ACTION_NAME = "process_ctrl";
    const std::string THERMAL_LEVEL_NAME = "thermallevel";
    const std::string POPUP_ACTION_NAME = "popup";
    const std::string CURRENT_ACTION_NAME = "current";
    const std::string VOLATAGE_ACTION_NAME = "voltage";
    const std::string CURRENT_SC_ACTION_NAME = "current_sc";
    const std::string CURRENT_BUCK_ACTION_NAME = "current_buck";
    const std::string VOLATAGE_SC_ACTION_NAME = "voltage_sc";
    const std::string VOLATAGE_BUCK_ACTION_NAME = "voltage_buck";
    const std::string NODE_ACTION_NAME = "node";
    // state name
    const std::string STATE_CHARGER = "charge";
    const std::string STATE_SCREEN = "screen";
    const std::string STATE_SCNEN = "scene";

    // scene sensor name
    const std::string BATTERY = "battery";
    const std::string BUCK_PROTOCOL = "buck";
    const std::string SC_PROTOCOL = "sc";
    const std::string SOC = "soc";
    constexpr int32_t INVAILD_TEMP = -1000;
    constexpr int32_t MIN = 0;
    constexpr int32_t MAX = 3;
    constexpr int32_t INTERVAL = 5000;

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

    //fallback value
    constexpr float FALLBACK_VALUE_FLOAT = 1.0f;
    constexpr uint32_t FALLBACK_VALUE_UINT_ZERO = 0;
    constexpr uint32_t FALLBACK_VALUE_UINT_SOC = static_cast<uint32_t>(INT_MAX);

    // floating-point precision
    constexpr float FLOAT_ACCURACY = 0.001f;

    // strtol function base parameter format
    constexpr uint32_t STRTOL_FORMART_DEC = 10;

    //socperf resource id
    constexpr int32_t LIM_CPU_BIG_ID = 1005;
    constexpr int32_t LIM_CPU_MED_ID = 1003;
    constexpr int32_t LIM_CPU_LIT_ID = 1001;
    constexpr int32_t LIM_GPU_ID = 1021;
} // namespace PowerMgr
} // namespace OHOS
#endif // OHOS_THERMAL_CONSTANTS_H
