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

#ifndef THERMAL_MGR_POLICY_TEST
#define THERMAL_MGR_POLICY_TEST

#include <gtest/gtest.h>
#include <string>

namespace OHOS {
namespace PowerMgr {
const uint32_t MAX_PATH = 256;
const std::string VOLUME_PATH = "/data/service/el0/thermal/config/volume";
const std::string BATTERY_PATH = "/data/service/el0/thermal/sensor/battery/temp";
const std::string SHELL_PATH = "/data/service/el0/thermal/sensor/shell/temp";
const std::string AMBIENT_PATH = "/data/service/el0/thermal/sensor/ambient/temp";
const std::string PA_PATH = "/data/service/el0/thermal/sensor/pa/temp";
const std::string AP_PATH = "/data/service/el0/thermal/sensor/ap/temp";
const std::string CONFIG_LEVEL_PATH = "/data/service/el0/thermal/config/configLevel";
const std::string LCD_PATH = "/data/service/el0/thermal/config/lcd";
const std::string PROCESS_PATH = "/data/service/el0/thermal/config/process_ctrl";
const std::string SHUTDOWN_PATH = "/data/service/el0/thermal/config/shut_down";
const std::string VENDOR_CONFIG = "/vendor/etc/thermal_config/thermal_service_config.xml";
constexpr const char* CPU_FREQ_PATH = "/data/service/el0/thermal/cooling/cpu/freq";
constexpr const char* GPU_FREQ_PATH = "/data/service/el0/thermal/cooling/gpu/freq";
constexpr const char* BATTERY_CHARGER_CURRENT_PATH = "/data/service/el0/thermal/cooling/battery/current";
constexpr const char* SIMULATION_TEMP_DIR = "/data/service/el0/thermal/sensor/%s/temp";
constexpr const char* SC_CURRENT_PATH = "/data/service/el0/thermal/config/sc_current";
constexpr const char* BUCK_CURRENT_PATH = "/data/service/el0/thermal/config/buck_current";
constexpr const char* SC_VOLTAGE_PATH = "/data/service/el0/thermal/config/sc_voltage";
constexpr const char* BUCK_VOLTAGE_PATH = "/data/service/el0/thermal/config/buck_voltage";
class ThermalMgrPolicyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static std::string GetActionValue(const std::string& path);
    static void SetSensorTemp(int32_t temperature, const std::string& path);
    static void Trim(char* str);
    static int32_t WriteFile(std::string path, std::string buf, size_t size);
    static int32_t ConvertInt(const std::string& value);
    static int32_t ReadFile(const char* path, char* buf, size_t size);
    static int32_t ReadSysfsFile(const char* path, char* buf, size_t size);
    static int32_t InitNode();
};
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_MGR_POLICY_TEST
