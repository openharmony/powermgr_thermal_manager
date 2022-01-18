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

#ifndef THERMAL_MGR_POLICY_TEST
#define THERMAL_MGR_POLICY_TEST

#include <gtest/gtest.h>
#include <string>

namespace OHOS {
namespace PowerMgr {
const uint32_t MAX_PATH = 256;
const uint32_t WAIT_TIME = 1;
std::string batteryPath = "/data/sensor/battery/temp";
std::string shellPath = "/data/sensor/shell/temp";
std::string chargerPath = "/data/sensor/charger/temp";
std::string socPath = "/data/sensor/soc/temp";
std::string ambientPath = "/data/sensor/ambient/temp";
std::string cpuPath = "/data/sensor/cpu/temp";
std::string paPath = "/data/sensor/pa/temp";
std::string apPath = "/data/sensor/ap/temp";
std::string configLevelPath = "/data/thermal/config/configLevel";
std::string lcdPath = "/data/thermal/config/lcd";
std::string processPath = "/data/thermal/config/process_ctrl";
std::string shutdownPath = "/data/thermal/config/shut_down";
std::string stateScreenPath = "/data/thermal/state/screen";
std::string stateScenePath = "/data/thermal/state/scene";
std::string stateChargePath = "/data/thermal/state/charge";
const std::string CPU_FREQ_PATH = "/data/cooling/cpu/freq";
const std::string GPU_FREQ_PATH = "/data/cooling/gpu/freq";
const std::string BATTERY_CHARGER_CURRENT_PATH = "/data/cooling/charger/current";
std::string xmlServicePath = "/system/etc/thermal_config/thermal_service_config.xml";
std::string xmlKernelPath = "/system/etc/thermal_config/thermal_kernel_config.xml";
std::string xmlHdiPath = "/system/etc/thermal_config/thermal_hdi_config.xml";
class ThermalMgrPolicyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static int32_t WriteFile(std::string path, std::string buf, size_t size);
    static int32_t ReadFile(const char *path, char *buf, size_t size);
    static int32_t ConvertInt(const std::string &value);
};
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_MGR_POLICY_TEST