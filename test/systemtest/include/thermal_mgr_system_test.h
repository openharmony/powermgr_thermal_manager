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
std::string batteryPath = "/data/thermal/battery/temp";
std::string paPath = "/data/thermal/pa/temp";
std::string ambientPath = "/data/thermal/ambient/temp";
std::string apPath = "/data/thermal/ap/temp";
std::string chargerPath = "/data/thermal/charger/temp";
std::string batteryCurrentPath = "/data/mitigation/charger/current";
std::string cpuFreqPath = "/data/mitigation/cpu/freq";
const uint32_t MAX_PATH = 256;
const uint32_t WAIT_TIME = 1;
std::string xmlServicePath = "/system/etc/thermal_config/thermal_service_config.xml";
std::string xmlKernelPath = "/system/etc/thermal_config/thermal_kernel_config.xml";
std::string xmlHdiPath = "/system/etc/thermal_config/thermal_hdi_config.xml";
class ThermalMgrSystemTest : public testing::Test {
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
#endif // THERMAL_MGR_SYSTEM_TEST