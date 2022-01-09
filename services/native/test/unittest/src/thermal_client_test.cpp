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

#include "thermal_client_test.h"

#include <chrono>
#include <thread>
#include <iostream>
#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <string_ex.h>
#include <gtest/gtest.h>

#include "thermal_srv_sensor_info.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

void ThermalClientTest::SetUpTestCase()
{}

void ThermalClientTest::TearDownTestCase()
{}

void ThermalClientTest::SetUp()
{}

void ThermalClientTest::TearDown()
{}

/*
 * @tc.number: ThermalGetSensorTempTest001
 * @tc.name: ThermalClientTest
 * @tc.desc: Verify get the cpu temp according type.
 */
HWTEST_F(ThermalClientTest, ThermalGetSensorTempTest001, TestSize.Level0)
{
    int32_t value = 0, temp = 42000;
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    GTEST_LOG_(INFO) << "ThermalGetSensorTempTest001:: Test Start!!";
    thermalMgrClient.SetSensorTemp("cpu", temp);
    sleep(WAIR_TIME);
    value = thermalMgrClient.GetThermalSensorTemp(SensorType::CPU);
    GTEST_LOG_(INFO) << "CPU temp:" << value;
    EXPECT_EQ(true, value == temp) << "ThermalGetSensorTempTest001 Failed";
}

