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

#include "power_mgr_client.h"
#include "thermal_mgr_client.h"
#include "mock_thermal_mgr_client.h"

#define private   public
#define protected public
#include "thermal_service.h"
#include "thermal_srv_config_parser.h"
#include "thermal_srv_sensor_info.h"
#include "v1_1/ithermal_interface.h"
#include "v1_1/thermal_types.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
const std::string SYSTEM_THERMAL_SERVICE_CONFIG_PATH = "/system/etc/thermal_config/thermal_service_config.xml";
sptr<ThermalService> g_service = nullptr;
const int32_t LEVEL_0 = 0;
const int32_t LEVEL_1 = 1;
const int32_t LEVEL_2 = 2;
const int32_t LEVEL_3 = 3;
const int32_t LEVEL_4 = 4;
}

void ThermalMgrPolicyTest::TearDown()
{
    g_service->SetScene("");
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 0;
    event.info.push_back(info1);
    info1.type = "ap";
    event.info.push_back(info1);
    info1.type = "ambient";
    event.info.push_back(info1);
    info1.type = "shell";
    event.info.push_back(info1);
    info1.type = "pa";
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
}

void ThermalMgrPolicyTest::SetUpTestCase()
{
    g_service = ThermalService::GetInstance();
    g_service->InitSystemTestModules();
    g_service->OnStart();
    g_service->InitStateMachine();
    g_service->InitActionManager();
    g_service->GetBaseinfoObj()->Init();
    g_service->GetObserver()->InitSensorTypeMap();
}

void ThermalMgrPolicyTest::TearDownTestCase()
{
    g_service->OnStop();
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 40100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(CONFIG_LEVEL_PATH));
    EXPECT_TRUE(value == LEVEL_1) << "ThermalMgrPolicyTest001 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 43100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(CONFIG_LEVEL_PATH));
    EXPECT_TRUE(value == LEVEL_2) << "ThermalMgrPolicyTest002 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 46100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(CONFIG_LEVEL_PATH));
    EXPECT_TRUE(value == LEVEL_3) << "ThermalMgrPolicyTest003 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 48100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(CONFIG_LEVEL_PATH));
    EXPECT_TRUE(value == LEVEL_4) << "ThermalMgrPolicyTest004 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 40100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(CONFIG_LEVEL_PATH));
    EXPECT_TRUE(value == LEVEL_1) << "ThermalMgrPolicyTest005 failed";
    event.info.clear();

    info1.temp = 48100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    value = ConvertInt(GetNodeValue(CONFIG_LEVEL_PATH));
    EXPECT_TRUE(value == LEVEL_4) << "ThermalMgrPolicyTest005 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 43100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(CONFIG_LEVEL_PATH));
    EXPECT_TRUE(value == LEVEL_2) << "ThermalMgrPolicyTest006 failed";
    event.info.clear();

    info1.temp = 48100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    value = ConvertInt(GetNodeValue(CONFIG_LEVEL_PATH));
    EXPECT_TRUE(value == LEVEL_4) << "ThermalMgrPolicyTest006 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 48200;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(CONFIG_LEVEL_PATH));
    EXPECT_TRUE(value == LEVEL_4) << "ThermalMgrPolicyTest007 failed";
    event.info.clear();

    info1.temp = 40900;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    value = ConvertInt(GetNodeValue(CONFIG_LEVEL_PATH));
    EXPECT_TRUE(value == LEVEL_1) << "ThermalMgrPolicyTest007 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 46100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(CONFIG_LEVEL_PATH));
    EXPECT_TRUE(value == LEVEL_3) << "ThermalMgrPolicyTest008 failed";
    event.info.clear();

    info1.temp = 37000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    value = ConvertInt(GetNodeValue(CONFIG_LEVEL_PATH));
    EXPECT_TRUE(value == LEVEL_0) << "ThermalMgrPolicyTest008 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = -10000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(CONFIG_LEVEL_PATH));
    EXPECT_TRUE(value == LEVEL_1) << "ThermalMgrPolicyTest009 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = -15000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(CONFIG_LEVEL_PATH));
    EXPECT_TRUE(value == LEVEL_2) << "ThermalMgrPolicyTest010 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = -20100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(CONFIG_LEVEL_PATH));
    EXPECT_TRUE(value == LEVEL_3) << "ThermalMgrPolicyTest011 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = -22000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(CONFIG_LEVEL_PATH));
    EXPECT_TRUE(value == LEVEL_4) << "ThermalMgrPolicyTest012 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = -10000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(CONFIG_LEVEL_PATH));
    EXPECT_TRUE(value == LEVEL_1) << "ThermalMgrPolicyTest013 failed";
    event.info.clear();

    info1.temp = -22000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    value = ConvertInt(GetNodeValue(CONFIG_LEVEL_PATH));
    EXPECT_TRUE(value == LEVEL_4) << "ThermalMgrPolicyTest013 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = -15000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(CONFIG_LEVEL_PATH));
    EXPECT_TRUE(value == LEVEL_2) << "ThermalMgrPolicyTest014 failed";
    event.info.clear();

    info1.temp = -22000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    value = ConvertInt(GetNodeValue(CONFIG_LEVEL_PATH));
    EXPECT_TRUE(value == LEVEL_4) << "ThermalMgrPolicyTest014 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = -22000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(CONFIG_LEVEL_PATH));
    EXPECT_TRUE(value == LEVEL_4) << "ThermalMgrPolicyTest015 failed";
    event.info.clear();

    info1.temp = -10000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    value = ConvertInt(GetNodeValue(CONFIG_LEVEL_PATH));
    EXPECT_TRUE(value == LEVEL_1) << "ThermalMgrPolicyTest015 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = -19100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(CONFIG_LEVEL_PATH));
    EXPECT_TRUE(value == LEVEL_3) << "ThermalMgrPolicyTest016 failed";
    event.info.clear();

    info1.temp = -1000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    value = ConvertInt(GetNodeValue(CONFIG_LEVEL_PATH));
    EXPECT_TRUE(value == LEVEL_0) << "ThermalMgrPolicyTest016 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "pa";
    info1.temp = 41000;
    event.info.push_back(info1);
    info1.type = "ambient";
    info1.temp = 10000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(CONFIG_LEVEL_PATH));
    EXPECT_TRUE(value == LEVEL_1) << "ThermalMgrPolicyTest017 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "pa";
    info1.temp = 44000;
    event.info.push_back(info1);
    info1.type = "ambient";
    info1.temp = 10000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(CONFIG_LEVEL_PATH));
    EXPECT_TRUE(value == LEVEL_2) << "ThermalMgrPolicyTest018 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "pa";
    info1.temp = 44000;
    event.info.push_back(info1);
    info1.type = "ambient";
    info1.temp = 1000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(CONFIG_LEVEL_PATH));
    EXPECT_TRUE(value == LEVEL_0) << "ThermalMgrPolicyTest019 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "ap";
    info1.temp = 78000;
    event.info.push_back(info1);
    info1.type = "ambient";
    info1.temp = 1000;
    event.info.push_back(info1);
    info1.type = "shell";
    info1.temp = 2000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(CONFIG_LEVEL_PATH));
    EXPECT_TRUE(value == LEVEL_1) << "ThermalMgrPolicyTest020 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "ap";
    info1.temp = 78000;
    event.info.push_back(info1);
    info1.type = "ambient";
    info1.temp = 1000;
    event.info.push_back(info1);
    info1.type = "shell";
    info1.temp = -100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(CONFIG_LEVEL_PATH));
    EXPECT_TRUE(value == LEVEL_0) << "ThermalMgrPolicyTest021 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 40100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(BATTERY_CHARGER_CURRENT_PATH));
    int32_t currentNow = 1800;
    EXPECT_TRUE(value == currentNow) << "ThermalMgrPolicyTest022 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 40100;
    event.info.push_back(info1);
    g_service->SetScene("cam");
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(BATTERY_CHARGER_CURRENT_PATH));
    int32_t currentNow = 1200;
    EXPECT_TRUE(value == currentNow) << "ThermalMgrPolicyTest023 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 43100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(BATTERY_CHARGER_CURRENT_PATH));
    int32_t currentNow = 1500;
    EXPECT_TRUE(value == currentNow) << "ThermalMgrPolicyTest024 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 43100;
    event.info.push_back(info1);
    g_service->SetScene("cam");
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(BATTERY_CHARGER_CURRENT_PATH));
    int32_t currentNow = 1000;
    EXPECT_TRUE(value == currentNow) << "ThermalMgrPolicyTest025 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 46100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(BATTERY_CHARGER_CURRENT_PATH));
    int32_t currentNow = 1300;
    EXPECT_TRUE(value == currentNow) << "ThermalMgrPolicyTest026 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 46100;
    event.info.push_back(info1);
    g_service->SetScene("cam");
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(BATTERY_CHARGER_CURRENT_PATH));
    int32_t currentNow = 800;
    EXPECT_TRUE(value == currentNow) << "ThermalMgrPolicyTest027 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = -10000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(BATTERY_CHARGER_CURRENT_PATH));
    int32_t currentNow = 1850;
    EXPECT_TRUE(value == currentNow) << "ThermalMgrPolicyTest028 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = -14100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(BATTERY_CHARGER_CURRENT_PATH));
    int32_t currentNow = 1550;
    EXPECT_TRUE(value == currentNow) << "ThermalMgrPolicyTest029 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = -19100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(BATTERY_CHARGER_CURRENT_PATH));
    int32_t currentNow = 1150;
    EXPECT_TRUE(value == currentNow) << "ThermalMgrPolicyTest030 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 40100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 43100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 46100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "pa";
    info1.temp = 41000;
    event.info.push_back(info1);
    info1.type = "ambient";
    info1.temp = 10000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "pa";
    info1.temp = 44000;
    event.info.push_back(info1);
    info1.type = "ambient";
    info1.temp = 10000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "ap";
    info1.temp = 78000;
    event.info.push_back(info1);
    info1.type = "ambient";
    info1.temp = 10000;
    event.info.push_back(info1);
    info1.type = "shell";
    info1.temp = 3000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(PROCESS_PATH));
    if (PowerMgrClient::GetInstance().IsScreenOn()) {
        EXPECT_TRUE(value == 3) << "ThermalMgrPolicyTest036 failed";
    } else {
        EXPECT_TRUE(value == 0) << "ThermalMgrPolicyTest036 failed";
    }

    value = ConvertInt(GetNodeValue(SHUTDOWN_PATH));
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 40100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(PROCESS_PATH));
    EXPECT_TRUE(value == LEVEL_3) << "ThermalMgrPolicyTest037 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 43100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(PROCESS_PATH));
    EXPECT_TRUE(value == LEVEL_2) << "ThermalMgrPolicyTest038 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 46100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(PROCESS_PATH));
    EXPECT_TRUE(value == LEVEL_1) << "ThermalMgrPolicyTest039 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "pa";
    info1.temp = 41000;
    event.info.push_back(info1);
    info1.type = "ambient";
    info1.temp = 10000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    std::string ret = GetNodeValue(PROCESS_PATH);
    int32_t value = ConvertInt(ret);
    EXPECT_TRUE(value == LEVEL_2) << "ThermalMgrPolicyTest040 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "pa";
    info1.temp = 44000;
    event.info.push_back(info1);
    info1.type = "ambient";
    info1.temp = 10000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(PROCESS_PATH));
    EXPECT_TRUE(value == LEVEL_3) << "ThermalMgrPolicyTest041 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 40100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(BUCK_CURRENT_PATH));
    int32_t currentNow = 1200;
    EXPECT_TRUE(value == currentNow) << "ThermalMgrPolicyTest042 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 40100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(SC_VOLTAGE_PATH));
    int32_t voltage = 4000;
    EXPECT_TRUE(value == voltage) << "ThermalMgrPolicyTest043 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 40100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(BUCK_VOLTAGE_PATH));
    int32_t voltage = 3000;
    EXPECT_TRUE(value == voltage) << "ThermalMgrPolicyTest044 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 43100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(BUCK_CURRENT_PATH));
    int32_t currentNow = 1000;
    EXPECT_TRUE(value == currentNow) << "ThermalMgrPolicyTest045 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 43100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(SC_VOLTAGE_PATH));
    int32_t voltage = 3000;
    EXPECT_TRUE(value == voltage) << "ThermalMgrPolicyTest046 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 43100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(BUCK_VOLTAGE_PATH));
    int32_t voltage = 2000;
    EXPECT_TRUE(value == voltage) << "ThermalMgrPolicyTest047 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 46100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(BUCK_CURRENT_PATH));
    int32_t currentNow = 800;
    EXPECT_TRUE(value == currentNow) << "ThermalMgrPolicyTest048 failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 46100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 46100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 40100;
    event.info.push_back(info1);
    g_service->SetScene("cam");
    g_service->HandleThermalCallbackEvent(event);
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 43100;
    event.info.push_back(info1);
    g_service->SetScene("cam");
    g_service->HandleThermalCallbackEvent(event);
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 46100;
    event.info.push_back(info1);
    g_service->SetScene("cam");
    g_service->HandleThermalCallbackEvent(event);
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 40100;
    event.info.push_back(info1);
    g_service->SetScene("call");
    g_service->HandleThermalCallbackEvent(event);
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 43100;
    event.info.push_back(info1);
    g_service->SetScene("call");
    g_service->HandleThermalCallbackEvent(event);
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 46100;
    event.info.push_back(info1);
    g_service->SetScene("call");
    g_service->HandleThermalCallbackEvent(event);
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 40100;
    event.info.push_back(info1);
    g_service->SetScene("game");
    g_service->HandleThermalCallbackEvent(event);
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 43100;
    event.info.push_back(info1);
    g_service->SetScene("game");
    g_service->HandleThermalCallbackEvent(event);
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 46100;
    event.info.push_back(info1);
    g_service->SetScene("game");
    g_service->HandleThermalCallbackEvent(event);
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 40100;
    event.info.push_back(info1);
    g_service->SetScene("test");
    g_service->HandleThermalCallbackEvent(event);
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 40100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    std::string ret = GetNodeValue(LCD_PATH);
    EXPECT_TRUE(ret.substr(0, 3) == "1.0") << "ThermalMgrPolicyTest063 failed";
    event.info.clear();

    g_service->SetScene("game");
    info1.temp = 40100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 40100;
    event.info.push_back(info1);
    g_service->SetScene("call");
    g_service->HandleThermalCallbackEvent(event);
    std::string ret = GetNodeValue(LCD_PATH);
    EXPECT_TRUE(ret.substr(0, 4) == "0.98") << "ThermalMgrPolicyTest064 failed";
    event.info.clear();

    g_service->SetScene("");
    info1.temp = 40100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 40100;
    event.info.push_back(info1);
    g_service->SetScene("cam");
    g_service->HandleThermalCallbackEvent(event);
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 43100;
    event.info.push_back(info1);
    g_service->SetScene("call");
    g_service->HandleThermalCallbackEvent(event);
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 46100;
    event.info.push_back(info1);
    g_service->SetScene("game");
    g_service->HandleThermalCallbackEvent(event);
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 40100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 43100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 46100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 40100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    ThermalSrvSensorInfo info;
    g_service->GetThermalSrvSensorInfo(SensorType::BATTERY, info);
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest071: end");
}
}
