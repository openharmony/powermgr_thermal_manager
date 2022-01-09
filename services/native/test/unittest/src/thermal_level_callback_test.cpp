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

#include "thermal_level_callback_test.h"

#include <iostream>
#include <chrono>
#include <thread>

#include <ipc_skeleton.h>
#include <string_ex.h>
#include <if_system_ability_manager.h>
#include <gtest/gtest.h>

#include "thermal_mgr_client.h"
#include "thermal_common.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

void ThermalLevelCallbackTest::ThermalLevelTest1Callback::GetThermalLevel(ThermalLevel level)
{
    int min = -1;
    int max = 6;
    uint32_t value = static_cast<uint32_t>(level);
    GTEST_LOG_(INFO) << "ThermalLevelTest1Callback level: " << value;
    EXPECT_EQ(true, value >= min || value <= max) << "ThermalLevelCallback001 falied";
}

void ThermalLevelCallbackTest::ThermalLevelTest2Callback::GetThermalLevel(ThermalLevel level)
{
    int min = -1;
    int max = 6;
    uint32_t value = static_cast<uint32_t>(level);
    GTEST_LOG_(INFO) << "ThermalLevelTest2Callback level: " << value;
    EXPECT_EQ(true, value >= min || value <= max) << "ThermalLevelCallback001 falied";
}

void ThermalLevelCallbackTest::ThermalLevelTest3Callback::GetThermalLevel(ThermalLevel level)
{
    int min = -1;
    int max = 6;
    uint32_t value = static_cast<uint32_t>(level);
    GTEST_LOG_(INFO) << "ThermalLevelTest3Callback level: " << value;
    EXPECT_EQ(true, value >= min || value <= max) << "ThermalLevelCallback001 falied";
}

/**
 * @tc.name: ThermalLevelCallback001
 * @tc.desc: test thermal level callback in proxy
 * @tc.type: FUNC
 */
HWTEST_F (ThermalLevelCallbackTest, ThermalLevelCallback001, TestSize.Level0)
{
    int32_t temp = 40000;
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    sptr<IThermalLevelCallback> cb1 = new ThermalLevelTest1Callback();
    GTEST_LOG_(INFO) << "first Start register";
    thermalMgrClient.SubscribeThermalLevelCallback(cb1);
    sptr<IThermalLevelCallback> cb2 = new ThermalLevelTest2Callback();
    GTEST_LOG_(INFO) << "second Start register";
    thermalMgrClient.SubscribeThermalLevelCallback(cb2);
    sptr<IThermalLevelCallback> cb3 = new ThermalLevelTest3Callback();
    GTEST_LOG_(INFO) << "third Start register";
    thermalMgrClient.SubscribeThermalLevelCallback(cb3);
    for (uint32_t i = 0; i < 10; i ++) {
        temp += 10;
        thermalMgrClient.SetSensorTemp("battery", temp);
        sleep(10);
    }
    GTEST_LOG_(INFO) << "third Cancell register";
    thermalMgrClient.UnSubscribeThermalLevelCallback(cb3);
    GTEST_LOG_(INFO) << "second Cancell register";
    thermalMgrClient.UnSubscribeThermalLevelCallback(cb2);
    GTEST_LOG_(INFO) << "first Cancell register";
    thermalMgrClient.UnSubscribeThermalLevelCallback(cb1);
}

/**
 * @tc.name: ThermalLevelCallback001
 * @tc.desc: test thermal level callback in proxy
 * @tc.type: FUNC
 */
HWTEST_F (ThermalLevelCallbackTest, ThermalLevelTest1, TestSize.Level0)
{
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    ThermalLevel level;
    GTEST_LOG_(INFO) << "Enter";
    thermalMgrClient.GetThermalLevel(level);
    GTEST_LOG_(INFO) << "level is: " << static_cast<uint32_t>(level);
}