/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "thermal_mgr_policy_test.h"

#include "battery_srv_client.h"
#include "power_mgr_client.h"
#include "thermal_service.h"
#include "thermal_mgr_client.h"
#include "mock_thermal_mgr_client.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

void ThermalMgrPolicyTest::TearDown()
{
    InitNode();
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("");
    MockThermalMgrClient::GetInstance().GetThermalInfo();
}

namespace {
/**
 * @tc.name: ThermalMgrPolicyTest001
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 1
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest001, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest001: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(40100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(CONFIG_LEVEL_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 1) << "ThermalMgrPolicyTest001 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest001: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest002
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 2
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest002, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest002: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(43100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(CONFIG_LEVEL_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 2) << "ThermalMgrPolicyTest002 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest002: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest003
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 3
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest003, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest003: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(46100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(CONFIG_LEVEL_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 3) << "ThermalMgrPolicyTest003 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest003: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest004
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 4
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest004, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest004: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(48100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(CONFIG_LEVEL_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 4) << "ThermalMgrPolicyTest004 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest004: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest005
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 1 ==> level 4
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest005, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest005: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(40100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(CONFIG_LEVEL_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 1) << "ThermalMgrPolicyTest005 failed";

    SetNodeValue(48100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    ret = GetNodeValue(CONFIG_LEVEL_PATH);
    value = ConvertInt(ret);
    EXPECT_TRUE(value == 4) << "ThermalMgrPolicyTest005 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest005: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest006
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 2 ==> level 4
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest006, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest006: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(43100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(CONFIG_LEVEL_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 2) << "ThermalMgrPolicyTest006 failed";

    SetNodeValue(48100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    ret = GetNodeValue(CONFIG_LEVEL_PATH);
    value = ConvertInt(ret);
    EXPECT_TRUE(value == 4) << "ThermalMgrPolicyTest006 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest006: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest007
 * @tc.desc: test level desc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 4 ===> level 1
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest007, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest007: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(48200, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(CONFIG_LEVEL_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 4) << "ThermalMgrPolicyTest007 failed";

    SetNodeValue(40900, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    ret = GetNodeValue(CONFIG_LEVEL_PATH);
    value = ConvertInt(ret);
    EXPECT_TRUE(value == 1) << "ThermalMgrPolicyTest007 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest007: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest008
 * @tc.desc: test level desc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 3 ===> level 0
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest008, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest008: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(46100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(CONFIG_LEVEL_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 3) << "ThermalMgrPolicyTest008 failed";

    SetNodeValue(37000, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    ret = GetNodeValue(CONFIG_LEVEL_PATH);
    value = ConvertInt(ret);
    EXPECT_TRUE(value == 0) << "ThermalMgrPolicyTest008 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest008: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest009
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 1
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest009, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest009: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(-10000, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(CONFIG_LEVEL_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 1) << "ThermalMgrPolicyTest009 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest009: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest010
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 2
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest010, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest010: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(-15000, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(CONFIG_LEVEL_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 2) << "ThermalMgrPolicyTest010 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest010: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest011
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 3
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest011, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest011: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(-20100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(CONFIG_LEVEL_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 3) << "ThermalMgrPolicyTest011 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest011: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest012
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 4
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest012, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest012: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(-22000, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(CONFIG_LEVEL_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 4) << "ThermalMgrPolicyTest012 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest012: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest013
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 1 ==> level 4
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest013, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest013: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(-10000, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(CONFIG_LEVEL_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 1) << "ThermalMgrPolicyTest013 failed";

    SetNodeValue(-22000, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    ret = GetNodeValue(CONFIG_LEVEL_PATH);
    value = ConvertInt(ret);
    EXPECT_TRUE(value == 4) << "ThermalMgrPolicyTest013 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest013: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest014
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 2 ==> level 4
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest014, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest014: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(-15000, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(CONFIG_LEVEL_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 2) << "ThermalMgrPolicyTest014 failed";

    SetNodeValue(-22000, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    ret = GetNodeValue(CONFIG_LEVEL_PATH);
    value = ConvertInt(ret);
    EXPECT_TRUE(value == 4) << "ThermalMgrPolicyTest014 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest014: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest015
 * @tc.desc: test level desc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 4 ===> level 1
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest015, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest015: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(-22000, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(CONFIG_LEVEL_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 4) << "ThermalMgrPolicyTest015 failed";

    SetNodeValue(-10000, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    ret = GetNodeValue(CONFIG_LEVEL_PATH);
    value = ConvertInt(ret);
    EXPECT_TRUE(value == 1) << "ThermalMgrPolicyTest015 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest015: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest016
 * @tc.desc: test level desc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 3 ===> level 0
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest016, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest016: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(-19100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(CONFIG_LEVEL_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 3) << "ThermalMgrPolicyTest016 failed";

    SetNodeValue(-1000, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    ret = GetNodeValue(CONFIG_LEVEL_PATH);
    value = ConvertInt(ret);
    EXPECT_TRUE(value == 0) << "ThermalMgrPolicyTest016 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest016: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest017
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set PA temp, High Temp With Aux sensor
 * @tc.result level 1
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest017, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest017: start");
    if (!IsMock(PA_PATH) || !IsMock(AMBIENT_PATH)) {
        THERMAL_HILOGD(LABEL_TEST, "Thermal is not mock, return");
        return;
    }
    SetNodeValue(41000, PA_PATH);
    SetNodeValue(10000, AMBIENT_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(CONFIG_LEVEL_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 1) << "ThermalMgrPolicyTest017 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest017: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest018
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set PA temp, High Temp With Aux sensor
 * @tc.result level 1
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest018, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest018: start");
    if (!IsMock(PA_PATH) || !IsMock(AMBIENT_PATH)) {
        THERMAL_HILOGD(LABEL_TEST, "Thermal is not mock, return");
        return;
    }
    SetNodeValue(44000, PA_PATH);
    SetNodeValue(10000, AMBIENT_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(CONFIG_LEVEL_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 2) << "ThermalMgrPolicyTest018 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest018: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest019
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set PA temp, High Temp With Aux sensor
 * @tc.result level 0
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest019, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest019: start");
    if (!IsMock(PA_PATH) || !IsMock(AMBIENT_PATH)) {
        THERMAL_HILOGD(LABEL_TEST, "Thermal is not mock, return");
        return;
    }
    SetNodeValue(44000, PA_PATH);
    SetNodeValue(1000, AMBIENT_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(CONFIG_LEVEL_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 0) << "ThermalMgrPolicyTest019 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest019: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest020
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set PA temp, High Temp With Aux sensor
 * @tc.result level 1
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest020, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest020: start");
    if (!IsMock(AP_PATH) || !IsMock(AMBIENT_PATH) || !IsMock(SHELL_PATH)) {
        THERMAL_HILOGD(LABEL_TEST, "Thermal is not mock, return");
        return;
    }
    SetNodeValue(78000, AP_PATH);
    SetNodeValue(1000, AMBIENT_PATH);
    SetNodeValue(2000, SHELL_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(CONFIG_LEVEL_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 1) << "ThermalMgrPolicyTest020 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest020: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest021
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set PA temp, High Temp With Aux sensor
 * @tc.result level 0
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest021, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest021: start");
    if (!IsMock(AP_PATH) || !IsMock(AMBIENT_PATH) || !IsMock(SHELL_PATH)) {
        THERMAL_HILOGD(LABEL_TEST, "Thermal is not mock, return");
        return;
    }
    SetNodeValue(78000, AP_PATH);
    SetNodeValue(1000, AMBIENT_PATH);
    SetNodeValue(-100, SHELL_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(CONFIG_LEVEL_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 0) << "ThermalMgrPolicyTest021 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest021: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest022
 * @tc.desc: test get charge currentby setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state not satisfied
 * @tc.result level 1, current 1800
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest022, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest022: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(40100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(BATTERY_CHARGER_CURRENT_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 1800) << "ThermalMgrPolicyTest022 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest022: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest023
 * @tc.desc: test get charge currentby setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: scene = "cam"
 * @tc.result level 1, current 1200
 * @tc.require: issueI5HWGZ
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest023, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest023: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("cam");
    SetNodeValue(40100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(BATTERY_CHARGER_CURRENT_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 1200) << "ThermalMgrPolicyTest023 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest023: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest024
 * @tc.desc: test get charge currentby setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state not satisfied
 * @tc.result level 2, current 1500
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest024, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest024: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(43100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(BATTERY_CHARGER_CURRENT_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 1500) << "ThermalMgrPolicyTest024 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest024: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest025
 * @tc.desc: test get charge currentby setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: scene = "cam"
 * @tc.result level 2, current 1000
 * @tc.require: issueI5HWGZ
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest025, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest025: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("cam");
    SetNodeValue(43100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(BATTERY_CHARGER_CURRENT_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 1000) << "ThermalMgrPolicyTest025 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest025: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest026
 * @tc.desc: test get charge currentby setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state not satisfied
 * @tc.result level 3, current 1300
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest026, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest026: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(46100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(BATTERY_CHARGER_CURRENT_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 1300) << "ThermalMgrPolicyTest026 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest026: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest027
 * @tc.desc: test get charge currentby setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: scene = "cam"
 * @tc.result level 3, current 800
 * @tc.require: issueI5HWGZ
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest027, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest027: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("cam");
    SetNodeValue(46100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(BATTERY_CHARGER_CURRENT_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 800) << "ThermalMgrPolicyTest027 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest027: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest028
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 1 current 1850
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest028, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest028: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(-10000, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(BATTERY_CHARGER_CURRENT_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 1850) << "ThermalMgrPolicyTest028 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest028: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest029
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 2 current 1550
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest029, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest029: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(-14100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(BATTERY_CHARGER_CURRENT_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 1550) << "ThermalMgrPolicyTest029 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest029: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest030
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 3 current 1150
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest030, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest030: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(-19100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(BATTERY_CHARGER_CURRENT_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 1150) << "ThermalMgrPolicyTest030 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest030: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest031
 * @tc.desc: test get brightness configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 1 brightness factor is 1.0
 * @tc.require: issueI5HWH6
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest031, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest031: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(40100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(LCD_PATH);
    EXPECT_TRUE(ret.substr(0, 3) == "1.0") << "ThermalMgrPolicyTest031 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest031: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest032
 * @tc.desc: test get brightness configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 2 brightness factor is 0.9
 * @tc.require: issueI5HWH6
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest032, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest032: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(43100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(LCD_PATH);
    EXPECT_TRUE(ret.substr(0, 3) == "0.9") << "ThermalMgrPolicyTest032 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest032: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest033
 * @tc.desc: test get brightness configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 3 brightness factor is 0.8
 * @tc.require: issueI5HWH6
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest033, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest033: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(46100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(LCD_PATH);
    EXPECT_TRUE(ret.substr(0, 3) == "0.8") << "ThermalMgrPolicyTest033 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest033: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest034
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set PA temp, High Temp With Aux sensor
 * @tc.result level 1 brightness factor is 0.7
 * @tc.require: issueI5HWH6
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest034, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest034: start");
    if (!IsMock(PA_PATH) || !IsMock(AMBIENT_PATH)) {
        THERMAL_HILOGD(LABEL_TEST, "Thermal is not mock, return");
        return;
    }
    SetNodeValue(41000, PA_PATH);
    SetNodeValue(10000, AMBIENT_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(LCD_PATH);
    EXPECT_TRUE(ret.substr(0, 3) == "0.7") << "ThermalMgrPolicyTest034 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest034: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest035
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set PA temp, High Temp With Aux sensor
 * @tc.result level 2 brightness factor is 0.6
 * @tc.require: issueI5HWH6
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest035, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest035: start");
    if (!IsMock(PA_PATH) || !IsMock(AMBIENT_PATH)) {
        THERMAL_HILOGD(LABEL_TEST, "Thermal is not mock, return");
        return;
    }
    SetNodeValue(44000, PA_PATH);
    SetNodeValue(10000, AMBIENT_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(LCD_PATH);
    EXPECT_TRUE(ret.substr(0, 3) == "0.6") << "ThermalMgrPolicyTest035 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest035: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest036
 * @tc.desc: get process and shutdown value
 * @tc.type: FEATURE
 * @tc.cond: Set AP temp, High Temp With Aux sensor
 * @tc.result level 1, process 3, shutdown 1
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest036, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest036: start");
    if (!IsMock(AP_PATH) || !IsMock(AMBIENT_PATH) || !IsMock(SHELL_PATH)) {
        THERMAL_HILOGD(LABEL_TEST, "Thermal is not mock, return");
        return;
    }
    SetNodeValue(78000, AP_PATH);
    SetNodeValue(1000, AMBIENT_PATH);
    SetNodeValue(3000, SHELL_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(PROCESS_PATH);
    int32_t value = ConvertInt(ret);
    if (PowerMgrClient::GetInstance().IsScreenOn()) {
        EXPECT_TRUE(value == 3) << "ThermalMgrPolicyTest036 failed";
    } else {
        EXPECT_TRUE(value == 0) << "ThermalMgrPolicyTest036 failed";
    }

    ret = GetNodeValue(SHUTDOWN_PATH);
    value = ConvertInt(ret);
    if (PowerMgrClient::GetInstance().IsScreenOn()) {
        EXPECT_EQ(true, value == 1) << "ThermalMgrPolicyTest036 failed";
    } else {
        EXPECT_EQ(true, value == 0) << "ThermalMgrPolicyTest036 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest036: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest037
 * @tc.desc: test get process value by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 1 procss 1
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest037, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest037: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(40100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(PROCESS_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 3) << "ThermalMgrPolicyTest037 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest037: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest038
 * @tc.desc: test get process value by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 2 procss 2
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest038, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest038: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(43100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(PROCESS_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 2) << "ThermalMgrPolicyTest038 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest038: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest039
 * @tc.desc: test get process value by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 3 procss 1
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest039, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest039: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(46100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(PROCESS_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 1) << "ThermalMgrPolicyTest039 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest039: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest040
 * @tc.desc: test get process by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set PA temp, High Temp With Aux sensor
 * @tc.result level 1 process 2
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest040, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest040: start");
    if (!IsMock(PA_PATH) || !IsMock(AMBIENT_PATH)) {
        THERMAL_HILOGD(LABEL_TEST, "Thermal is not mock, return");
        return;
    }
    SetNodeValue(41000, PA_PATH);
    SetNodeValue(10000, AMBIENT_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(PROCESS_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 2) << "ThermalMgrPolicyTest040 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest040: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest041
 * @tc.desc: test get process by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set PA temp, High Temp With Aux sensor
 * @tc.result level 2 process 3
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest041, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest041: start");
    if (!IsMock(PA_PATH) || !IsMock(AMBIENT_PATH)) {
        THERMAL_HILOGD(LABEL_TEST, "Thermal is not mock, return");
        return;
    }
    SetNodeValue(44000, PA_PATH);
    SetNodeValue(10000, AMBIENT_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(PROCESS_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 3) << "ThermalMgrPolicyTest041 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest041: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest042
 * @tc.desc: get the config current by setting battery temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp, charge_type: buck
 * @tc.result level 1, current 1200
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest042, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest042: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(40100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(BUCK_CURRENT_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 1200) << "ThermalMgrPolicyTest042 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest042: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest043
 * @tc.desc: get the config voltage by setting battery temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp, charge_type: sc
 * @tc.result level 1, voltage 4000
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest043, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest043: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(40100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(SC_VOLTAGE_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 4000) << "ThermalMgrPolicyTest043 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest043: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest044
 * @tc.desc: get the config voltage by setting battery temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp, charge_type: buck
 * @tc.result level 1, voltage 3000
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest044, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest044: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(40100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(BUCK_VOLTAGE_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 3000) << "ThermalMgrPolicyTest044 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest044: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest045
 * @tc.desc: get the config current by setting battery temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp, charge_type: buck
 * @tc.result level 2, current 1000
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest045, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest045: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(43100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(BUCK_CURRENT_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 1000) << "ThermalMgrPolicyTest045 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest045: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest046
 * @tc.desc: get the config voltage by setting battery temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp, charge_type: sc
 * @tc.result level 2， voltage 3000
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest046, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest046: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(43100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(SC_VOLTAGE_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 3000) << "ThermalMgrPolicyTest046 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest046: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest047
 * @tc.desc: get the config voltage by setting battery temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp, charge_type: buck
 * @tc.result level 2, voltage 2000
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest047, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest047: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(43100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(BUCK_VOLTAGE_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 2000) << "ThermalMgrPolicyTest047 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest047: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest048
 * @tc.desc: get the config current by setting battery temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp, charge_type: buck
 * @tc.result level 3, current 800
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest048, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest048: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(46100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(BUCK_CURRENT_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 800) << "ThermalMgrPolicyTest048 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest048: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest049
 * @tc.desc: get the config voltage by setting battery temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp, charge_type: sc
 * @tc.result level 3, voltage 2000
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest049, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest049: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(46100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(SC_VOLTAGE_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 2000) << "ThermalMgrPolicyTest049 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest049: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest050
 * @tc.desc: get the config voltage by setting battery temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp, charge_type: buck
 * @tc.result level 3, voltage 1000
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest050, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest050: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(46100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(BUCK_VOLTAGE_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == 1000) << "ThermalMgrPolicyTest050 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest050: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest053
 * @tc.desc: test set brightness according to the scene
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 1, scene cam, brightness factor is 0.99
 * @tc.require: issueI5HWH6
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest053, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest053: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("cam");
    SetNodeValue(40100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(LCD_PATH);
    EXPECT_TRUE(ret.substr(0, 4) == "0.99") << "ThermalMgrPolicyTest053 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest053: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest054
 * @tc.desc: test set brightness according to the scene
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 2, scene cam, brightness factor is 0.98
 * @tc.require: issueI5HWH6
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest054, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest054: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("cam");
    SetNodeValue(43100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(LCD_PATH);
    EXPECT_TRUE(ret.substr(0, 4) == "0.89") << "ThermalMgrPolicyTest054 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest054: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest055
 * @tc.desc: test set brightness according to the scene
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 3, scene cam, brightness factor is 0.97
 * @tc.require: issueI5HWH6
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest055, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest055: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("cam");
    SetNodeValue(46100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(LCD_PATH);
    EXPECT_TRUE(ret.substr(0, 4) == "0.79") << "ThermalMgrPolicyTest055 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest055: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest056
 * @tc.desc: test set brightness according to the scene
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 1, scene call, brightness factor is 0.98
 * @tc.require: issueI5HWH6
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest056, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest056: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("call");
    SetNodeValue(40100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(LCD_PATH);
    EXPECT_TRUE(ret.substr(0, 4) == "0.98") << "ThermalMgrPolicyTest056 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest056: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest057
 * @tc.desc: test set brightness according to the scene
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 2, scene call, brightness factor is 0.88
 * @tc.require: issueI5HWH6
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest057, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest057: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("call");
    SetNodeValue(43100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(LCD_PATH);
    EXPECT_TRUE(ret.substr(0, 4) == "0.88") << "ThermalMgrPolicyTest057 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest057: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest058
 * @tc.desc: test set brightness according to the scene
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 3, scene call, brightness factor is 0.78
 * @tc.require: issueI5HWH6
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest058, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest058: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("call");
    SetNodeValue(46100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(LCD_PATH);
    EXPECT_TRUE(ret.substr(0, 4) == "0.78") << "ThermalMgrPolicyTest058 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest058: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest059
 * @tc.desc: test set brightness according to the scene
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 1, scene game, brightness factor is 0.97
 * @tc.require: issueI5HWH6
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest059, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest059: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("game");
    SetNodeValue(40100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(LCD_PATH);
    EXPECT_TRUE(ret.substr(0, 4) == "0.97") << "ThermalMgrPolicyTest059 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest059: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest060
 * @tc.desc: test set brightness according to the scene
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 2, scene game, brightness factor is 0.87
 * @tc.require: issueI5HWH6
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest060, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest060: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("game");
    SetNodeValue(43100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(LCD_PATH);
    EXPECT_TRUE(ret.substr(0, 4) == "0.87") << "ThermalMgrPolicyTest060 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest060: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest061
 * @tc.desc: test set brightness according to the scene
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 3, scene game, brightness factor is 0.77
 * @tc.require: issueI5HWH6
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest061, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest061: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("game");
    SetNodeValue(46100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(LCD_PATH);
    EXPECT_TRUE(ret.substr(0, 4) == "0.77") << "ThermalMgrPolicyTest061 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest061: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest062
 * @tc.desc: test set brightness according to the scene
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 1, scene test, brightness factor is 0.91
 * @tc.require: issueI5HWH6
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest062, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest062: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("test");
    SetNodeValue(40100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(LCD_PATH);
    EXPECT_TRUE(ret.substr(0, 4) == "0.91") << "ThermalMgrPolicyTest062 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest062: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest063
 * @tc.desc: test set brightness according to the scene
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 1, brightness factor is 1.0; scene game, brightness factor is 0.97
 * @tc.require: issueI5HWH6
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest063, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest063: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(40100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(LCD_PATH);
    EXPECT_TRUE(ret.substr(0, 3) == "1.0") << "ThermalMgrPolicyTest063 failed";

    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("game");
    SetNodeValue(40100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    ret = GetNodeValue(LCD_PATH);
    EXPECT_TRUE(ret.substr(0, 4) == "0.97") << "ThermalMgrPolicyTest063 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest063: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest064
 * @tc.desc: test set brightness according to the scene
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 1, scene call, brightness factor is 0.98; scene empty, brightness factor is 1.0
 * @tc.require: issueI5HWH6
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest064, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest064: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("call");
    SetNodeValue(40100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(LCD_PATH);
    EXPECT_TRUE(ret.substr(0, 4) == "0.98") << "ThermalMgrPolicyTest064 failed";

    thermalMgrClient.SetScene("");
    SetNodeValue(40100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    ret = GetNodeValue(LCD_PATH);
    EXPECT_TRUE(ret.substr(0, 3) == "1.0") << "ThermalMgrPolicyTest064 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest064: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest065
 * @tc.desc: test set brightness according to the scene
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result scene cam, level 1, brightness factor is 0.99
 * @tc.require: issueI5HWH6
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest065, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest065: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("cam");
    SetNodeValue(40100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(LCD_PATH);
    EXPECT_TRUE(ret.substr(0, 4) == "0.99") << "ThermalMgrPolicyTest065 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest065: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest066
 * @tc.desc: test set brightness according to the scene
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result scene call, level 2, brightness factor is 0.88
 * @tc.require: issueI5HWH6
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest066, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest066: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("call");
    SetNodeValue(43100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(LCD_PATH);
    EXPECT_TRUE(ret.substr(0, 4) == "0.88") << "ThermalMgrPolicyTest066 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest066: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest067
 * @tc.desc: test set brightness according to the scene
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result scene game, level 3, brightness factor is 0.77
 * @tc.require: issueI5HWH6
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest067, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest067: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("game");
    SetNodeValue(46100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(LCD_PATH);
    EXPECT_TRUE(ret.substr(0, 4) == "0.77") << "ThermalMgrPolicyTest066 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest067: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest068
 * @tc.desc: get the config volume by setting battery temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp
 * @tc.result level 1, volume 1.0
 * @tc.require: issueI5HWH6
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest068, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest068: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(40100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(VOLUME_PATH);
    EXPECT_TRUE(ret.substr(0, 3) == "1.0") << "ThermalMgrPolicyTest068 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest068: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest069
 * @tc.desc: get the config current by setting battery temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp
 * @tc.result level 2, volume 0.8
 * @tc.require: issueI5HWH6
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest069, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest069: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(43100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(VOLUME_PATH);
    EXPECT_TRUE(ret.substr(0, 3) == "0.8") << "ThermalMgrPolicyTest069 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest069: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest070
 * @tc.desc: get the config current by setting battery temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp
 * @tc.result level 3, volume 0.7
 * @tc.require: issueI5HWH6
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest070, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest070: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    SetNodeValue(46100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    std::string ret = GetNodeValue(VOLUME_PATH);
    EXPECT_TRUE(ret.substr(0, 3) == "0.7") << "ThermalMgrPolicyTest070 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest070: end");
}

/**
 * @tc.name: ThermalMgrPolicyTest071
 * @tc.desc: test GetThermalSensorTemp function
 * @tc.type: FUNC
 * @tc.cond: Set Battery temp
 * @tc.result: Function return value is equal to the set value
 * @tc.require: issueI63SZ4
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest071, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest071: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    int32_t temp = 40100;
    SetNodeValue(temp, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    int32_t out = thermalMgrClient.GetThermalSensorTemp(SensorType::BATTERY);
    EXPECT_EQ(true, temp == out);
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest071: end");
}
}
