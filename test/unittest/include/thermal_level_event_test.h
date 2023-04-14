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

#ifndef THERMAL_LEVEL_EVENT_TEST
#define THERMAL_LEVEL_EVENT_TEST

#include <gtest/gtest.h>
#include "thermal_service.h"

namespace OHOS {
namespace PowerMgr {
namespace {
const uint32_t MAX_PATH = 256;
const std::string BATTERY_PATH = "/data/service/el0/thermal/sensor/battery/temp";
const std::string SHELL_PATH = "/data/service/el0/thermal/sensor/shell/temp";
const std::string SOC_PATH = "/data/service/el0/thermal/sensor/soc/temp";
const std::string AMBIENT_PATH = "/data/service/el0/thermal/sensor/ambient/temp";
const std::string PA_PATH = "/data/service/el0/thermal/sensor/pa/temp";
const std::string AP_PATH = "/data/service/el0/thermal/sensor/ap/temp";
constexpr const char* SIMULATION_TEMP_DIR = "/data/service/el0/thermal/sensor/%s/temp";
}
class ThermalLevelEventTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    int32_t InitNode();
};
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_LEVEL_EVENT_TEST