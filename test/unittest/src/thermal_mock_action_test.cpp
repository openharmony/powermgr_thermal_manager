/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifdef THERMAL_GTEST
#define private   public
#endif

#include "thermal_mock_action_test.h"

#include <cstdio>
#include <cstdlib>
#include <dirent.h>
#include <fcntl.h>
#include "securec.h"

#include "battery_srv_client.h"
#include "constants.h"
#include "mock_socperf_action.h"
#include "power_mgr_client.h"
#include "thermal_service.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

static sptr<ThermalService> g_thermalSvc = nullptr;

void ThermalMockActionTest::SetUpTestCase()
{
    g_thermalSvc = DelayedSpSingleton<ThermalService>::GetInstance();
    g_thermalSvc->OnStart();
    g_thermalSvc->InitStateMachine();
}

void ThermalMockActionTest::TearDownTestCase()
{
    g_thermalSvc->OnStop();
    DelayedSpSingleton<ThermalService>::DestroyInstance();
}

void ThermalMockActionTest::TearDown()
{
    InitNode();
    g_thermalSvc->SetScene("");
    g_thermalSvc->GetThermalInfo();
    MockSocPerfAction::ClearLimit();
    MockSocPerfAction::ClearBoost();
}

namespace {
/**
 * @tc.name: ThermalMockActionTest001
 * @tc.desc: test cpu boost action
 * @tc.type: FUNC
 * @tc.cond: Set Battery temp
 * @tc.result: level 1~3, socperf function execution three times
 * @tc.require: issueI6JSQD
 */
HWTEST_F (ThermalMockActionTest, ThermalMockActionTest001, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockActionTest001 start");
    ASSERT_NE(g_thermalSvc, nullptr);
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    ThermalLevel level = ThermalLevel::COOL;
    int32_t temp = 40100;
    int32_t expectLevel = 1;
    SetNodeValue(temp, BATTERY_PATH);
    g_thermalSvc->GetThermalInfo();
    g_thermalSvc->GetThermalLevel(level);
    EXPECT_EQ(expectLevel, static_cast<int32_t>(level));
    EXPECT_EQ(1, MockSocPerfAction::GetBoostRequestCounter());

    temp = 46100;
    expectLevel = 3;
    SetNodeValue(temp, BATTERY_PATH);
    g_thermalSvc->GetThermalInfo();
    g_thermalSvc->GetThermalLevel(level);
    EXPECT_EQ(expectLevel, static_cast<int32_t>(level));
    EXPECT_EQ(2, MockSocPerfAction::GetBoostRequestCounter());

    temp = 43100;
    expectLevel = 2;
    SetNodeValue(temp, BATTERY_PATH);
    g_thermalSvc->GetThermalInfo();
    g_thermalSvc->GetThermalLevel(level);
    EXPECT_EQ(expectLevel, static_cast<int32_t>(level));
    EXPECT_EQ(3, MockSocPerfAction::GetBoostRequestCounter());
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockActionTest001 end");
}

/**
 * @tc.name: ThermalMockActionTest002
 * @tc.desc: test cpu boost action
 * @tc.type: FUNC
 * @tc.cond: Set Battery temp
 * @tc.result: level 3, socperf function does not execute
 * @tc.require: issueI6JSQD
 */
HWTEST_F (ThermalMockActionTest, ThermalMockActionTest002, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockActionTest002 start");
    ASSERT_NE(g_thermalSvc, nullptr);
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    int32_t temp = 46100;
    ThermalLevel level = ThermalLevel::COOL;
    int32_t expectLevel = 3;
    SetNodeValue(temp, BATTERY_PATH);
    g_thermalSvc->GetThermalInfo();
    g_thermalSvc->GetThermalLevel(level);
    EXPECT_EQ(expectLevel, static_cast<int32_t>(level));
    EXPECT_EQ(0, MockSocPerfAction::GetBoostRequestCounter());
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockActionTest002 end");
}

/**
 * @tc.name: ThermalMockActionTest003
 * @tc.desc: test get cpu and gpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state not satisfied
 * @tc.result level 1
 * @tc.require: issueI6UI5Q
 */
HWTEST_F (ThermalMockActionTest, ThermalMockActionTest003, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockActionTest003: start");
    ASSERT_NE(g_thermalSvc, nullptr);
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    int32_t temp = 40100;
    ThermalLevel level = ThermalLevel::COOL;
    int32_t expectLevel = 1;
    SetNodeValue(temp, BATTERY_PATH);
    g_thermalSvc->GetThermalInfo();
    g_thermalSvc->GetThermalLevel(level);
    EXPECT_EQ(expectLevel, static_cast<int32_t>(level));
    EXPECT_EQ(1992000, MockSocPerfAction::GetLimitValue(LIM_CPU_BIG_ID));
    EXPECT_EQ(1991500, MockSocPerfAction::GetLimitValue(LIM_CPU_MED_ID));
    EXPECT_EQ(1991200, MockSocPerfAction::GetLimitValue(LIM_CPU_LIT_ID));
    int64_t gpuLimitValue = MockSocPerfAction::GetLimitValue(LIM_GPU_ID);
    if (PowerMgrClient::GetInstance().IsScreenOn()) {
        EXPECT_TRUE(gpuLimitValue == 512000);
    } else {
        EXPECT_TRUE(gpuLimitValue == 524288);
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockActionTest003: end");
}

/**
 * @tc.name: ThermalMockActionTest004
 * @tc.desc: test get cpu and gpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state not satisfied
 * @tc.result level 2
 * @tc.require: issueI6UI5Q
 */
HWTEST_F (ThermalMockActionTest, ThermalMockActionTest004, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockActionTest004: start");
    ASSERT_NE(g_thermalSvc, nullptr);
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    int32_t temp = 43100;
    ThermalLevel level = ThermalLevel::COOL;
    int32_t expectLevel = 2;
    SetNodeValue(temp, BATTERY_PATH);
    g_thermalSvc->GetThermalInfo();
    g_thermalSvc->GetThermalLevel(level);
    EXPECT_EQ(expectLevel, static_cast<int32_t>(level));
    EXPECT_EQ(1991000, MockSocPerfAction::GetLimitValue(LIM_CPU_BIG_ID));
    EXPECT_EQ(1990500, MockSocPerfAction::GetLimitValue(LIM_CPU_MED_ID));
    EXPECT_EQ(1990200, MockSocPerfAction::GetLimitValue(LIM_CPU_LIT_ID));
    int64_t gpuLimitValue = MockSocPerfAction::GetLimitValue(LIM_GPU_ID);
    if (PowerMgrClient::GetInstance().IsScreenOn()) {
        EXPECT_TRUE(gpuLimitValue == 487424);
    } else {
        EXPECT_TRUE(gpuLimitValue == 499712);
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockActionTest004: end");
}

/**
 * @tc.name: ThermalMockActionTest005
 * @tc.desc: test get cpu and gpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state not satisfied
 * @tc.result level 2
 * @tc.require: issueI6UI5Q
 */
HWTEST_F (ThermalMockActionTest, ThermalMockActionTest005, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockActionTest005: start");
    ASSERT_NE(g_thermalSvc, nullptr);
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    int32_t temp = 46100;
    ThermalLevel level = ThermalLevel::COOL;
    int32_t expectLevel = 3;
    SetNodeValue(temp, BATTERY_PATH);
    g_thermalSvc->GetThermalInfo();
    g_thermalSvc->GetThermalLevel(level);
    EXPECT_EQ(expectLevel, static_cast<int32_t>(level));
    EXPECT_EQ(1990000, MockSocPerfAction::GetLimitValue(LIM_CPU_BIG_ID));
    EXPECT_EQ(1989500, MockSocPerfAction::GetLimitValue(LIM_CPU_MED_ID));
    EXPECT_EQ(1989200, MockSocPerfAction::GetLimitValue(LIM_CPU_LIT_ID));
    int64_t gpuLimitValue = MockSocPerfAction::GetLimitValue(LIM_GPU_ID);
    if (PowerMgrClient::GetInstance().IsScreenOn()) {
        EXPECT_TRUE(gpuLimitValue == 462848);
    } else {
        EXPECT_TRUE(gpuLimitValue == 475136);
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockActionTest005: end");
}

/**
 * @tc.name: ThermalMockActionTest006
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, scene = "cam"
 * @tc.result level 1
 * @tc.require: issueI6UI5Q
 */
HWTEST_F (ThermalMockActionTest, ThermalMockActionTest006, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockActionTest006: start");
    ASSERT_NE(g_thermalSvc, nullptr);
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    int32_t temp = 40100;
    ThermalLevel level = ThermalLevel::COOL;
    int32_t expectLevel = 1;
    g_thermalSvc->SetScene("cam");
    SetNodeValue(temp, BATTERY_PATH);
    g_thermalSvc->GetThermalInfo();
    g_thermalSvc->GetThermalLevel(level);
    EXPECT_EQ(expectLevel, static_cast<int32_t>(level));
    int64_t cpuLimitValue = MockSocPerfAction::GetLimitValue(LIM_CPU_BIG_ID);
    auto state = BatterySrvClient::GetInstance().GetChargingStatus();
    if (state == BatteryChargeState::CHARGE_STATE_ENABLE) {
        EXPECT_TRUE(cpuLimitValue == 1991800);
    } else if (state == BatteryChargeState::CHARGE_STATE_NONE) {
        EXPECT_TRUE(cpuLimitValue == 1991600);
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockActionTest006: end");
}

/**
 * @tc.name: ThermalMockActionTest007
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, scene = "cam"
 * @tc.result level 2
 * @tc.require: issueI6UI5Q
 */
HWTEST_F (ThermalMockActionTest, ThermalMockActionTest007, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockActionTest007: start");
    ASSERT_NE(g_thermalSvc, nullptr);
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    int32_t temp = 43100;
    ThermalLevel level = ThermalLevel::COOL;
    int32_t expectLevel = 2;
    g_thermalSvc->SetScene("cam");
    SetNodeValue(temp, BATTERY_PATH);
    g_thermalSvc->GetThermalInfo();
    g_thermalSvc->GetThermalLevel(level);
    EXPECT_EQ(expectLevel, static_cast<int32_t>(level));
    int64_t cpuLimitValue = MockSocPerfAction::GetLimitValue(LIM_CPU_BIG_ID);
    auto state = BatterySrvClient::GetInstance().GetChargingStatus();
    if (state == BatteryChargeState::CHARGE_STATE_ENABLE) {
        EXPECT_TRUE(cpuLimitValue == 1990800);
    } else if (state == BatteryChargeState::CHARGE_STATE_NONE) {
        EXPECT_TRUE(cpuLimitValue == 1990600);
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockActionTest007: end");
}

/**
 * @tc.name: ThermalMockActionTest008
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, scene = "cam"
 * @tc.result level 3
 * @tc.require: issueI6UI5Q
 */
HWTEST_F (ThermalMockActionTest, ThermalMockActionTest008, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockActionTest008: start");
    ASSERT_NE(g_thermalSvc, nullptr);
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    int32_t temp = 46100;
    ThermalLevel level = ThermalLevel::COOL;
    int32_t expectLevel = 3;
    g_thermalSvc->SetScene("cam");
    SetNodeValue(temp, BATTERY_PATH);
    g_thermalSvc->GetThermalInfo();
    g_thermalSvc->GetThermalLevel(level);
    EXPECT_EQ(expectLevel, static_cast<int32_t>(level));
    int64_t cpuLimitValue = MockSocPerfAction::GetLimitValue(LIM_CPU_BIG_ID);
    auto state = BatterySrvClient::GetInstance().GetChargingStatus();
    if (state == BatteryChargeState::CHARGE_STATE_ENABLE) {
        EXPECT_TRUE(cpuLimitValue == 1989800);
    } else if (state == BatteryChargeState::CHARGE_STATE_NONE) {
        EXPECT_TRUE(cpuLimitValue == 1989600);
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockActionTest008: end");
}
}