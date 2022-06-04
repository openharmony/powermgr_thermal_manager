/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef THERMAL_ACTION_HUB_TEST_H
#define THERMAL_ACTION_HUB_TEST_H

#include <gtest/gtest.h>
#include "ithermal_action_callback.h"
#include "thermal_action_callback_stub.h"

namespace OHOS {
namespace PowerMgr {
constexpr int SLEEP_WAIT_TIME_S = 1;
constexpr int SLEEP_WAIT_TIME_LOOP_S = 3;
const uint32_t MAX_PATH = 256;
const uint32_t WAIT_TIME = 10;
std::string batteryCurrentPath = "/data/mitigation/charger/current";
std::string cpuFreqPath = "/data/mitigation/cpu/freq";
std::string batteryPath = "/data/sensor/battery/temp";
std::string shellPath = "/data/sensor/shell/temp";
std::string chargerPath = "/data/sensor/charger/temp";
std::string socPath = "/data/sensor/soc/temp";
std::string ambientPath = "/data/sensor/ambient/temp";
std::string cpuPath = "/data/sensor/cpu/temp";
std::string paPath = "/data/sensor/pa/temp";
std::string apPath = "/data/sensor/ap/temp";

class ThermalActionHubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static int32_t WriteFile(std::string path, std::string buf, size_t size);
    static int32_t ReadFile(const char *path, char *buf, size_t size);
    static int32_t ConvertInt(const std::string &value);
    void InitData();

    class ThermalActionTest1Callback : public ThermalActionCallbackStub {
    public:
        ThermalActionTest1Callback() {};
        virtual ~ThermalActionTest1Callback() {};
        virtual void OnThermalActionChanged(ActionCallbackMap &actionCbMap) override;
    };

    class ThermalActionTest2Callback : public ThermalActionCallbackStub {
    public:
        ThermalActionTest2Callback() {};
        virtual ~ThermalActionTest2Callback() {};
        virtual void OnThermalActionChanged(ActionCallbackMap &actionCbMap) override;
    };

    class ThermalActionTest3Callback : public ThermalActionCallbackStub {
    public:
        ThermalActionTest3Callback() {};
        virtual ~ThermalActionTest3Callback() {};
        virtual void OnThermalActionChanged(ActionCallbackMap &actionCbMap) override;
    };
};
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_ACTION_HUB_TEST_H
