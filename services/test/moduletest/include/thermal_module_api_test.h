/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef THERMAL_MODULE_API_TEST
#define THERMAL_MODULE_API_TEST

#include "thermal_temp_callback_stub.h"
#include "thermal_level_callback_stub.h"
#include "thermal_common_file.h"

namespace OHOS {
namespace PowerMgr {
std::string batteryPath = "/data/thermal/battery/temp";
std::string paPath = "/data/thermal/pa0/temp";
std::string ambientPath = "/data/thermal/ambient/temp";
std::string apPath = "/data/thermal/ap/temp";
std::string chargerPath = "/data/thermal/charger/temp";
std::string socPath = "/data/thermal/soc/temp";

std::string batteryCurrentPath = "/data/mitigation/charger/current";
std::string cpuFreqPath = "/data/mitigation/cpu/freq";
const uint32_t MAX_PATH = 256;
const int32_t BUFFER_SIZE = 512;
constexpr uint32_t WAIT_TIME = 10;

class ThermalModuleApiTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp(void);
    void TearDown(void);
    class ThermalModuleApiTest1Callback : public ThermalTempCallbackStub {
    public:
        ThermalModuleApiTest1Callback() {};
        virtual ~ThermalModuleApiTest1Callback() {};
        virtual void OnThermalTempChanged(int32_t temp) override;
    };

    class ThermalModuleApiTest2Callback : public ThermalLevelCallbackStub {
    public:
        ThermalModuleApiTest2Callback() {};
        virtual ~ThermalModuleApiTest2Callback() {};
        virtual void GetThermalLevel(ThermalLevel level) override;
    };
};
}
}
#endif // THERMAL_MODULE_API_TEST