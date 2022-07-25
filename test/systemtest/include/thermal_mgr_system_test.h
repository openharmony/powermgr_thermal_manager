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

#ifndef THERMAL_MGR_SYSTEM_TEST
#define THERMAL_MGR_SYSTEM_TEST

#include <gtest/gtest.h>
#include <string>

namespace OHOS {
namespace PowerMgr {
const int32_t BUFFER_SIZE = 512;
const uint32_t MAX_PATH = 256;
const uint32_t SLEEP_INTERVAL_SEC = 5;
std::string batteryPath = "/data/service/el0/thermal/sensor/battery/temp";
std::string shellPath = "/data/service/el0/thermal/sensor/shell/temp";
std::string chargerPath = "/data/service/el0/thermal/sensor/charger/temp";
std::string socPath = "/data/service/el0/thermal/sensor/soc/temp";
std::string ambientPath = "/data/service/el0/thermal/sensor/ambient/temp";
std::string cpuPath = "/data/service/el0/thermal/sensor/cpu/temp";
std::string paPath = "/data/service/el0/thermal/sensor/pa/temp";
std::string apPath = "/data/service/el0/thermal/sensor/ap/temp";
std::string configLevelPath = "/data/service/el0/thermal/config/configLevel";
std::string lcdPath = "/data/service/el0/thermal/config/lcd";
std::string processPath = "/data/service/el0/thermal/config/process_ctrl";
std::string shutdownPath = "/data/service/el0/thermal/config/shut_down";
std::string stateScreenPath = "/data/service/el0/thermal/state/screen";
std::string stateScenePath = "/data/service/el0/thermal/state/scene";
std::string stateChargePath = "/data/service/el0/thermal/state/charge";
constexpr const char* CPU_FREQ_PATH = "/data/service/el0/thermal/cooling/cpu/freq";
constexpr const char* GPU_FREQ_PATH = "/data/service/el0/thermal/cooling/gpu/freq";
constexpr const char* BATTERY_CHARGER_CURRENT_PATH = "/data/service/el0/thermal/cooling/battery/current";
constexpr const char* SIMULATION_TEMP_DIR = "/data/service/el0/thermal/sensor/%s/temp";
class ThermalMgrSystemTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static int32_t WriteFile(std::string path, std::string buf, size_t size);
    static int32_t ReadFile(const char *path, char *buf, size_t size);
    static int32_t ConvertInt(const std::string &value);
    static int32_t InitNode();
};
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_MGR_SYSTEM_TEST