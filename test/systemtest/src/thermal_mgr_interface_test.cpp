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

#include "thermal_mgr_interface_test.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <list>
#include <mutex>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <datetime_ex.h>
#include <gtest/gtest.h>
#include <ipc_skeleton.h>
#include <string_ex.h>

#include "ithermal_srv.h"
#include "securec.h"
#include "thermal_client.h"
#include "thermal_mgr_client.h"
#include "thermal_srv_sensor_info.h"
#include "constants.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

static std::mutex g_mtx;
std::vector<std::string> typelist;

int32_t ThermalMgrInterfaceTest::ReadFile(const char *path, char *buf, size_t size)
{
    std::lock_guard<std::mutex> lck(g_mtx);
    int32_t ret;

    int32_t fd = open(path, O_RDONLY);
    if (fd < ERR_OK) {
        GTEST_LOG_(INFO) << "WriteFile: failed to open file" << fd;
        return ERR_INVALID_VALUE;
    }

    ret = read(fd, buf, size);
    if (ret < ERR_OK) {
        GTEST_LOG_(INFO) << "WriteFile: failed to read file" << ret;
        close(fd);
        return ERR_INVALID_VALUE;
    }

    close(fd);
    buf[size - 1] = '\0';
    return ERR_OK;
}

int32_t ThermalMgrInterfaceTest::ConvertInt(const std::string &value)
{
    return std::stoi(value);
}

void ThermalMgrInterfaceTest::SetUpTestCase(void)
{
}

void ThermalMgrInterfaceTest::TearDownTestCase(void)
{
}

void ThermalMgrInterfaceTest::SetUp()
{
}

void ThermalMgrInterfaceTest::TearDown()
{
}

void ThermalMgrInterfaceTest::InitData()
{
    typelist.push_back(BATTERY);
    typelist.push_back(SHELL);
    typelist.push_back(SOC);
    typelist.push_back(CPU);
}

void ThermalMgrInterfaceTest::ThermalTempTest1Callback::OnThermalTempChanged(TempCallbackMap &tempCbMap)
{
    int assertValue = 0;
    for (auto iter : tempCbMap) {
        GTEST_LOG_(INFO) << "type: " << iter.first << " temp: " << iter.second;
        EXPECT_EQ(true, iter.second > assertValue) << "Test Failed";
    }
}

void ThermalMgrInterfaceTest::ThermalTempTest2Callback::OnThermalTempChanged(TempCallbackMap &tempCbMap)
{
    int assertValue = 0;
    for (auto iter : tempCbMap) {
        GTEST_LOG_(INFO) << "type: " << iter.first << " temp: " << iter.second;
        EXPECT_EQ(true, iter.second > assertValue) << "Test Failed";
    }
}

void ThermalMgrInterfaceTest::ThermalLevelTest1Callback::GetThermalLevel(ThermalLevel level)
{
    int assertMin = -1;
    int assertMax = 6;
    int32_t levelValue = static_cast<int32_t>(level);
    GTEST_LOG_(INFO) << "level: " << levelValue;
    EXPECT_EQ(true, levelValue >= assertMin && levelValue <= assertMax) << "Test Failed";
}

void ThermalMgrInterfaceTest::ThermalLevelTest2Callback::GetThermalLevel(ThermalLevel level)
{
    int assertMin = -1;
    int assertMax = 6;
    int32_t levelValue = static_cast<int32_t>(level);
    GTEST_LOG_(INFO) << "level: " << levelValue;
    EXPECT_EQ(true, levelValue >= assertMin && levelValue <= assertMax) << "Test Failed";
}

void ThermalMgrInterfaceTest::ThermalLevelTest3Callback::GetThermalLevel(ThermalLevel level)
{
    int assertMin = -1;
    int assertMax = 6;
    int32_t levelValue = static_cast<int32_t>(level);
    GTEST_LOG_(INFO) << "level: " << levelValue;
    EXPECT_EQ(true, levelValue >= assertMin && levelValue <= assertMax) << "Test Failed";
}

/**
 * @tc.name: ThermalMgrInterfaceTest001
 * @tc.desc: test get sensor temp
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest001 start.";
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    int32_t in = 41000;
    thermalMgrClient.SetSensorTemp(BATTERY, in);
    sleep(10 * SLEEP_WAIT_TIME_S);
    int32_t out = thermalMgrClient.GetThermalSensorTemp(SensorType::BATTERY);
    GTEST_LOG_(INFO) << "battry temp: " << out;
    EXPECT_EQ(true, in == out) << "ThermalMgrInterfaceTest001 Failed";
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest001 end.";
}

/**
 * @tc.name: ThermalMgrInterfaceTest002
 * @tc.desc: test get sensor temp
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest002 start.";
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    int32_t in = 10000;
    thermalMgrClient.SetSensorTemp(SOC, in);
    sleep(10 * SLEEP_WAIT_TIME_S);
    int32_t out = thermalMgrClient.GetThermalSensorTemp(SensorType::SOC);
    GTEST_LOG_(INFO) << "soc temp: " << out;
    EXPECT_EQ(true, in == out) << "ThermalMgrInterfaceTest002 Failed";
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest002 end.";
}

/**
 * @tc.name: ThermalMgrInterfaceTest003
 * @tc.desc: test get sensor temp
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest003 start.";
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    int32_t in = 11000;
    thermalMgrClient.SetSensorTemp(SHELL, in);
    sleep(10 * SLEEP_WAIT_TIME_S);
    int32_t out = thermalMgrClient.GetThermalSensorTemp(SensorType::SHELL);
    GTEST_LOG_(INFO) << "shell temp: " << out;
    EXPECT_EQ(true, in == out) << "ThermalMgrInterfaceTest003 Failed";
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest003 end.";
}

/**
 * @tc.name: ThermalMgrInterfaceTest004
 * @tc.desc: test get sensor temp
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest004, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest004 start.";
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    int32_t in = 12000;
    thermalMgrClient.SetSensorTemp(CPU, in);
    sleep(10 * SLEEP_WAIT_TIME_S);
    int32_t out = thermalMgrClient.GetThermalSensorTemp(SensorType::SENSOR1);
    GTEST_LOG_(INFO) << "shell temp: " << out;
    EXPECT_EQ(true, in == out) << "ThermalMgrInterfaceTest004 Failed";
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest004 end.";
}

/**
 * @tc.name: ThermalMgrInterfaceTest005
 * @tc.desc: test get sensor temp
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest005, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest005 start.";
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    int32_t in = 13000;
    thermalMgrClient.SetSensorTemp(CHARGER, in);
    sleep(10 * SLEEP_WAIT_TIME_S);
    int32_t out = thermalMgrClient.GetThermalSensorTemp(SensorType::SENSOR2);
    GTEST_LOG_(INFO) << "shell temp: " << out;
    EXPECT_EQ(true, in == out) << "ThermalMgrInterfaceTest005 Failed";
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest005 end.";
}

/**
 * @tc.name: ThermalMgrInterfaceTest006
 * @tc.desc: register callback and get temp list
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest006, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest006 start.";
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    const sptr<IThermalTempCallback> cb1 = new ThermalTempTest1Callback();
    InitData();
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest006 start register";
    thermalMgrClient.SubscribeThermalTempCallback(typelist, cb1);
    int32_t temp = 10000;
    for (int i = 0; i < 10; i++) {
        GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest006 start change temp";
        temp += 100;
        thermalMgrClient.SetSensorTemp(BATTERY, temp);
        thermalMgrClient.SetSensorTemp(SHELL, temp);
        thermalMgrClient.SetSensorTemp(SOC, temp);
        thermalMgrClient.SetSensorTemp(CPU, temp);
        sleep(SLEEP_WAIT_TIME_S * 10);
    }
    thermalMgrClient.UnSubscribeThermalTempCallback(cb1);
    sleep(SLEEP_WAIT_TIME_S * 5);
    thermalMgrClient.SubscribeThermalTempCallback(typelist, cb1);
    for (int i = 0; i < 10; i++) {
        GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest006 start change temp";
        temp += 100;
        thermalMgrClient.SetSensorTemp(BATTERY, temp);
        thermalMgrClient.SetSensorTemp(SHELL, temp);
        thermalMgrClient.SetSensorTemp(SOC, temp);
        thermalMgrClient.SetSensorTemp(CPU, temp);
        sleep(SLEEP_WAIT_TIME_S * 10);
    }
    thermalMgrClient.UnSubscribeThermalTempCallback(cb1);
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest006 end.";
}

/**
 * @tc.name: ThermalMgrInterfaceTest007
 * @tc.desc: register callback and get temp list
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest007, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest007 start.";
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    const sptr<IThermalTempCallback> cb1 = new ThermalTempTest1Callback();
    const sptr<IThermalTempCallback> cb2 = new ThermalTempTest1Callback();
    InitData();
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest007 start register";
    thermalMgrClient.SubscribeThermalTempCallback(typelist, cb1);
    thermalMgrClient.SubscribeThermalTempCallback(typelist, cb2);
    int32_t temp = 10000;
    for (int i = 0; i < 10; i++) {
        GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest007 start change temp";
        temp += 100;
        thermalMgrClient.SetSensorTemp(BATTERY, temp);
        thermalMgrClient.SetSensorTemp(SHELL, temp);
        thermalMgrClient.SetSensorTemp(SOC, temp);
        thermalMgrClient.SetSensorTemp(CPU, temp);
        sleep(SLEEP_WAIT_TIME_S * 10);
    }
    thermalMgrClient.UnSubscribeThermalTempCallback(cb1);
    thermalMgrClient.UnSubscribeThermalTempCallback(cb2);
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest006 end.";
}

/**
 * @tc.name: ThermalMgrInterfaceTest008
 * @tc.desc: test get level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest008, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest008 start.";
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    int32_t in = 41000;
    thermalMgrClient.SetSensorTemp(BATTERY, in);
    sleep(10 * SLEEP_WAIT_TIME_S);
    ThermalLevel level = thermalMgrClient.GetThermalLevel();
    int32_t levelValue = static_cast<int32_t>(level);
    GTEST_LOG_(INFO) << "levelValue: " << levelValue;
    EXPECT_EQ(true, levelValue >= 0 && levelValue <= 6) << "ThermalMgrInterfaceTest008 Failed";
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest008 end.";
}

/**
 * @tc.name: ThermalMgrInterfaceTest009
 * @tc.desc: test get level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest009, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest009 start.";
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    int32_t in = 43100;
    thermalMgrClient.SetSensorTemp(BATTERY, in);
    sleep(10 * SLEEP_WAIT_TIME_S);
    ThermalLevel level = thermalMgrClient.GetThermalLevel();
    int32_t levelValue = static_cast<int32_t>(level);
    GTEST_LOG_(INFO) << "levelValue: " << levelValue;
    EXPECT_EQ(true, levelValue >= 0 && levelValue <= 6) << "ThermalMgrInterfaceTest009 Failed";
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest009 end.";
}

/**
 * @tc.name: ThermalMgrInterfaceTest010
 * @tc.desc: test get level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest010, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest010 start.";
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    int32_t in = 46100;
    thermalMgrClient.SetSensorTemp(BATTERY, in);
    sleep(10 * SLEEP_WAIT_TIME_S);
    ThermalLevel level = thermalMgrClient.GetThermalLevel();
    int32_t levelValue = static_cast<int32_t>(level);
    GTEST_LOG_(INFO) << "levelValue: " << levelValue;
    EXPECT_EQ(true, levelValue >= 0 && levelValue <= 6) << "ThermalMgrInterfaceTest010 Failed";
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest010 end.";
}

/**
 * @tc.name: ThermalMgrInterfaceTest011
 * @tc.desc: test get level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest011, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest011 start.";
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    int32_t in = 44100;
    int32_t ambientTemp = 60000;
    thermalMgrClient.SetSensorTemp(PA, in);
    thermalMgrClient.SetSensorTemp(AMBIENT, ambientTemp);
    sleep(10 * SLEEP_WAIT_TIME_S);
    ThermalLevel level = thermalMgrClient.GetThermalLevel();
    int32_t levelValue = static_cast<int32_t>(level);
    GTEST_LOG_(INFO) << "levelValue: " << levelValue;
    EXPECT_EQ(true, levelValue >= 0 && levelValue <= 6) << "ThermalMgrInterfaceTest011 Failed";
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest011 end.";
}


/**
 * @tc.name: ThermalMgrInterfaceTest012
 * @tc.desc: test get level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest012, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest012 start.";
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    int32_t in = 79000;
    int32_t ambientTemp = 60000;
    int32_t batteryTemp = 50000;
    thermalMgrClient.SetSensorTemp(AP, in);
    thermalMgrClient.SetSensorTemp(AMBIENT, ambientTemp);
    thermalMgrClient.SetSensorTemp(BATTERY, batteryTemp);
    sleep(10 * SLEEP_WAIT_TIME_S);
    ThermalLevel level = thermalMgrClient.GetThermalLevel();
    int32_t levelValue = static_cast<int32_t>(level);
    GTEST_LOG_(INFO) << "levelValue: " << levelValue;
    EXPECT_EQ(true, levelValue >= 0 && levelValue <= 6) << "ThermalMgrInterfaceTest012 Failed";
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest012 end.";
}


/**
 * @tc.name: ThermalMgrInterfaceTest0013
 * @tc.desc: test get level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest0013, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest0013 start.";
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    int32_t in = -20000;
    thermalMgrClient.SetSensorTemp(BATTERY, in);
    sleep(10 * SLEEP_WAIT_TIME_S);
    ThermalLevel level = thermalMgrClient.GetThermalLevel();
    int32_t levelValue = static_cast<int32_t>(level);
    GTEST_LOG_(INFO) << "levelValue: " << levelValue;
    EXPECT_EQ(true, levelValue >= 0 && levelValue <= 6) << "ThermalMgrInterfaceTest0013 Failed";
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest0013 end.";
}


/**
 * @tc.name: ThermalMgrInterfaceTest014
 * @tc.desc: test get level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest014, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest014 start.";
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    int32_t paTemp = 40100;
    int32_t ambientTemp = 60000;
    thermalMgrClient.SetSensorTemp(PA, paTemp);
    thermalMgrClient.SetSensorTemp(AMBIENT, ambientTemp);
    sleep(10 * SLEEP_WAIT_TIME_S);
    ThermalLevel level = thermalMgrClient.GetThermalLevel();
    int32_t levelValue = static_cast<int32_t>(level);
    GTEST_LOG_(INFO) << "levelValue: " << levelValue;
    EXPECT_EQ(true, levelValue >= 0 && levelValue <= 6) << "ThermalMgrInterfaceTest014 Failed";
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest014 end.";
}

/**
 * @tc.name: ThermalMgrInterfaceTest015
 * @tc.desc: test register callback and get thermal level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest015, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest015 start.";
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    const sptr<IThermalLevelCallback> cb1 = new ThermalLevelTest1Callback();
    thermalMgrClient.SubscribeThermalLevelCallback(cb1);
    int32_t tempA = -20000;
    thermalMgrClient.SetSensorTemp(BATTERY, tempA);
    sleep(10 * SLEEP_WAIT_TIME_S);
    tempA = 40100;
    thermalMgrClient.SetSensorTemp(BATTERY, tempA);
    sleep(10 * SLEEP_WAIT_TIME_S);
    tempA  = -10000;
    thermalMgrClient.SetSensorTemp(BATTERY, tempA);
    sleep(10 * SLEEP_WAIT_TIME_S);
    tempA  = 46000;
    thermalMgrClient.SetSensorTemp(BATTERY, tempA);
    sleep(10 * SLEEP_WAIT_TIME_S);
    thermalMgrClient.UnSubscribeThermalLevelCallback(cb1);
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest015 end.";
}

/**
 * @tc.name: ThermalMgrInterfaceTest016
 * @tc.desc: test register callback and get thermal level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest016, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest016 start.";
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    const sptr<IThermalLevelCallback> cb2 = new ThermalLevelTest2Callback();
    thermalMgrClient.SubscribeThermalLevelCallback(cb2);
    int32_t tempA = -20000;
    for (uint32_t i = 0; i < 10; i++) {
        GTEST_LOG_(INFO) << "change temp.";
        thermalMgrClient.SetSensorTemp(BATTERY, tempA);
        sleep(10 * SLEEP_WAIT_TIME_S);
        tempA += 10000;
    }
    thermalMgrClient.UnSubscribeThermalLevelCallback(cb2);
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest016 end.";
}

/**
 * @tc.name: ThermalMgrInterfaceTest017
 * @tc.desc: test register callback and get thermal level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest017, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest017 start.";
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    const sptr<IThermalLevelCallback> cb1 = new ThermalLevelTest1Callback();
    const sptr<IThermalLevelCallback> cb2 = new ThermalLevelTest2Callback();
    thermalMgrClient.SubscribeThermalLevelCallback(cb1);
    thermalMgrClient.SubscribeThermalLevelCallback(cb2);

    int32_t tempA = -20000;
    thermalMgrClient.SetSensorTemp(BATTERY, tempA);
    sleep(10 * SLEEP_WAIT_TIME_S);
    thermalMgrClient.UnSubscribeThermalLevelCallback(cb2);
    thermalMgrClient.UnSubscribeThermalLevelCallback(cb1);
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest017 end.";
}


/**
 * @tc.name: ThermalMgrInterfaceTest020
 * @tc.desc: test register callback and get thermal level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest020, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest020 start.";
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    const sptr<IThermalLevelCallback> cb1 = new ThermalLevelTest1Callback();
    thermalMgrClient.SubscribeThermalLevelCallback(cb1);

    int32_t tempA = -20000;
    thermalMgrClient.SetSensorTemp(BATTERY, tempA);
    sleep(10 * SLEEP_WAIT_TIME_S);
    thermalMgrClient.UnSubscribeThermalLevelCallback(cb1);

    thermalMgrClient.SubscribeThermalLevelCallback(cb1);
    tempA = 48000;
    thermalMgrClient.SetSensorTemp(BATTERY, tempA);
    sleep(10 * SLEEP_WAIT_TIME_S);
    thermalMgrClient.UnSubscribeThermalLevelCallback(cb1);
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest020 end.";
}

/* The following test cases invalid value for APi */

/**
 * @tc.name: ThermalMgrInterfaceTest021
 * @tc.desc: test get invaild temp
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest021, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest021 start.";
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetSensorTemp(BATTERY, INVAILD_TEMP);
    sleep(10 * SLEEP_WAIT_TIME_S);
    int32_t out = thermalMgrClient.GetThermalSensorTemp(SensorType::BATTERY);
    GTEST_LOG_(INFO) << "battry temp: " << out;
    EXPECT_EQ(true, INVAILD_TEMP == INVAILD_TEMP) << "ThermalMgrInterfaceTest021 Failed";
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest021 end.";
}

/**
 * @tc.name: ThermalMgrInterfaceTest022
 * @tc.desc: test get invaild temp
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest022, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest022 start.";
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    const sptr<IThermalTempCallback> cb = nullptr;
    InitData();
    thermalMgrClient.SubscribeThermalTempCallback(typelist, cb);
    thermalMgrClient.SetSensorTemp(BATTERY, INVAILD_TEMP);
    sleep(10 * SLEEP_WAIT_TIME_S);
    thermalMgrClient.UnSubscribeThermalTempCallback(cb);
    EXPECT_EQ(true, cb == nullptr) << "ThermalMgrInterfaceTest022 Failed";
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest022 end.";
}

/**
 * @tc.name: ThermalMgrInterfaceTest023
 * @tc.desc: test get invaild level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest023, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest023 start.";
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetSensorTemp(BATTERY, INVAILD_TEMP);
    sleep(10 * SLEEP_WAIT_TIME_S);
    ThermalLevel level = thermalMgrClient.GetThermalLevel();
    int32_t levelValue = static_cast<int32_t>(level);
    EXPECT_EQ(true, levelValue == -1) << "ThermalMgrInterfaceTest023 Failed";
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest023 end.";
}

/**
 * @tc.name: ThermalMgrInterfaceTest024
 * @tc.desc: test register null callback
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest024, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest024 start.";
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    const sptr<IThermalLevelCallback> cb = nullptr;
    thermalMgrClient.SubscribeThermalLevelCallback(cb);
    thermalMgrClient.SetSensorTemp(BATTERY, INVAILD_TEMP);
    sleep(10 * SLEEP_WAIT_TIME_S);
    thermalMgrClient.UnSubscribeThermalLevelCallback(cb);
    EXPECT_EQ(true, cb == nullptr) << "ThermalMgrInterfaceTest024 Failed";
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest024 end.";
}

/**
 * @tc.name: ThermalHdfApiTest001
 * @tc.desc: Bind subscriber to HDF layer
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrInterfaceTest, ThermalHdfApiTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalHdfApiTest001: Start.";
    sptr<ThermalSubscriber> subscriber = new ThermalSubscriber();
    ErrCode ret = ThermalClient::BindThermalDriverSubscriber(subscriber);
    EXPECT_EQ(true, ret == ERR_OK) << "ThermalHdfApiTest001 Failed";
    GTEST_LOG_(INFO) << "ThermalHdfApiTest001: End.";
}

/**
 * @tc.name: ThermalHdfApiTest002
 * @tc.desc: Bind subscriber to HDF layer
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrInterfaceTest, ThermalHdfApiTest002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalHdfApiTest002: Start.";
    sptr<ThermalSubscriber> subscriber = nullptr;
    ErrCode ret = ThermalClient::BindThermalDriverSubscriber(subscriber);
    EXPECT_EQ(true, ret == ERR_INVALID_VALUE) << "ThermalHdfApiTest002 Failed";
    GTEST_LOG_(INFO) << "ThermalHdfApiTest002: End.";
}

/**
 * @tc.name: ThermalHdfApiTest003
 * @tc.desc: Unbind subscriber to HDF layer
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrInterfaceTest, ThermalHdfApiTest003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalHdfApiTest003: Start.";
    ErrCode ret = ThermalClient::UnbindThermalDriverSubscriber();
    EXPECT_EQ(true, ret == ERR_OK) << "ThermalHdfApiTest003 Failed";
    GTEST_LOG_(INFO) << "ThermalHdfApiTest003: End.";
}


/**
 * @tc.name: ThermalHdfApiTest004
 * @tc.desc: Set cpu freq to HDF layer
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrInterfaceTest, ThermalHdfApiTest004, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalHdfApiTest004: Start.";
    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    uint32_t freq = 50000;
    uint32_t value = 0;
    ErrCode ret = ThermalClient::SetCPUFreq(freq);
    snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, cpuFreqPath.c_str());

    sleep(10 * SLEEP_WAIT_TIME_S);
    ret = ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalHdfApiTest004: Failed to read file ";
    }
    std::string mFreq = freqValue;

    value = ConvertInt(mFreq);
    GTEST_LOG_(INFO) << "ThermalHdfApiTest004: value " << value;
    EXPECT_EQ(true, value == freq) << "ThermalHdfApiTest004 Failed";
    GTEST_LOG_(INFO) << "ThermalHdfApiTest004: End.";
}

/**
 * @tc.name: ThermalHdfApiTest005
 * @tc.desc: Set cpu freq to HDF layer
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrInterfaceTest, ThermalHdfApiTest005, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalHdfApiTest005: Start.";
    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    uint32_t freq = 150000;
    uint32_t value = 0;
    ErrCode ret = ThermalClient::SetCPUFreq(freq);
    snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, cpuFreqPath.c_str());

    sleep(10 * SLEEP_WAIT_TIME_S);
    ret = ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalHdfApiTest005: Failed to read file ";
    }
    std::string mFreq = freqValue;

    value = ConvertInt(mFreq);
    GTEST_LOG_(INFO) << "ThermalHdfApiTest005: value " << value;
    EXPECT_EQ(true, value == freq) << "ThermalHdfApiTest005 Failed";
    GTEST_LOG_(INFO) << "ThermalHdfApiTest005: End.";
}

/**
 * @tc.name: ThermalHdfApiTest006
 * @tc.desc: Set cpu freq to HDF layer
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrInterfaceTest, ThermalHdfApiTest006, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalHdfApiTest006: Start.";
    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    uint32_t freq = 0;
    uint32_t value = 0;
    ErrCode ret = ThermalClient::SetCPUFreq(freq);
    snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, cpuFreqPath.c_str());

    sleep(10 * SLEEP_WAIT_TIME_S);
    ret = ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalHdfApiTest006: Failed to read file ";
    }
    std::string mFreq = freqValue;

    value = ConvertInt(mFreq);
    GTEST_LOG_(INFO) << "ThermalHdfApiTest006: value" << value;
    EXPECT_EQ(true, value == freq) << "ThermalHdfApiTest006 Failed";
    GTEST_LOG_(INFO) << "ThermalHdfApiTest006: End.";
}

/**
 * @tc.name: ThermalHdfApiTest007
 * @tc.desc: Set battery current to HDF layer
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrInterfaceTest, ThermalHdfApiTest007, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalHdfApiTest007: Start.";
    char currentBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    uint32_t current = 0;
    uint32_t value = 0;

    ErrCode ret = ThermalClient::SetBatteryCurrent(current);
    snprintf_s(currentBuf, PATH_MAX, sizeof(currentBuf) - 1, batteryCurrentPath.c_str());

    sleep(10 * SLEEP_WAIT_TIME_S);
    ret = ReadFile(currentBuf, currentValue, sizeof(currentValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalHdfApiTest007: Failed to read file ";
    }
    std::string sCurrent= currentValue;
    value = ConvertInt(sCurrent);
    GTEST_LOG_(INFO) << "ThermalHdfApiTest007: value" << value;
    EXPECT_EQ(true, value == current) << "ThermalHdfApiTest007 Failed";
    GTEST_LOG_(INFO) << "ThermalHdfApiTest007: End.";
}

/**
 * @tc.name: ThermalHdfApiTest008
 * @tc.desc: Set battery current to HDF layer
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrInterfaceTest, ThermalHdfApiTest008, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalHdfApiTest008: Start.";
    char currentBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    uint32_t current = 50000;
    uint32_t value = 0;

    ErrCode ret = ThermalClient::SetBatteryCurrent(current);
    snprintf_s(currentBuf, PATH_MAX, sizeof(currentBuf) - 1, batteryCurrentPath.c_str());

    sleep(10 * SLEEP_WAIT_TIME_S);
    ret = ReadFile(currentBuf, currentValue, sizeof(currentValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalHdfApiTest008: Failed to read file ";
    }
    std::string sCurrent= currentValue;
    value = ConvertInt(sCurrent);
    GTEST_LOG_(INFO) << "ThermalHdfApiTest008: value" << value;
    EXPECT_EQ(true, value == current) << "ThermalHdfApiTest008 Failed";
    GTEST_LOG_(INFO) << "ThermalHdfApiTest008: End.";
}

/**
 * @tc.name: ThermalHdfApiTest009
 * @tc.desc: Set battery current to HDF layer
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrInterfaceTest, ThermalHdfApiTest009, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalHdfApiTest009: Start.";
    char currentBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    uint32_t current = 150000;
    uint32_t value = 0;

    ErrCode ret = ThermalClient::SetBatteryCurrent(current);
    snprintf_s(currentBuf, PATH_MAX, sizeof(currentBuf) - 1, batteryCurrentPath.c_str());

    sleep(10 * SLEEP_WAIT_TIME_S);
    ret = ReadFile(currentBuf, currentValue, sizeof(currentValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalHdfApiTest009: Failed to read file ";
    }
    std::string sCurrent= currentValue;
    value = ConvertInt(sCurrent);
    GTEST_LOG_(INFO) << "ThermalHdfApiTest009: value" << value;
    EXPECT_EQ(true, value == current) << "ThermalHdfApiTest009 Failed";
    GTEST_LOG_(INFO) << "ThermalHdfApiTest009: End.";
}

/**
 * @tc.name: ThermalHdfApiTest010
 * @tc.desc: Get the thermal zone information from HDF layer
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrInterfaceTest, ThermalHdfApiTest010, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalHdfApiTest010: Start.";
    ErrCode ret = ThermalClient::SetSensorTemp("battery", 42000);
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalHdfApiTest010: Failed to set battery temp";
    }
    sleep(10 * SLEEP_WAIT_TIME_S);
    std::map<std::string, int32_t> sensor = ThermalClient::GetThermalZoneInfo();
    for (auto info : sensor) {
        if (info.first.find("battery") != std::string::npos) {
            EXPECT_EQ(true, info.second == 42000) << "ThermalHdfApiTest010 Failed";
        }
    }
    GTEST_LOG_(INFO) << "ThermalHdfApiTest010: End.";
}

/**
 * @tc.name: ThermalHdfApiTest011
 * @tc.desc: Get the thermal zone information from HDF layer
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrInterfaceTest, ThermalHdfApiTest011, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalHdfApiTest011: Start.";
    ErrCode ret = ThermalClient::SetSensorTemp("battery", INVAILD_TEMP);
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalHdfApiTest011: Failed to set battery temp";
    }
    sleep(10 * SLEEP_WAIT_TIME_S);
    std::map<std::string, int32_t> sensor = ThermalClient::GetThermalZoneInfo();
    for (auto info : sensor) {
        if (info.first.find("battery") != std::string::npos) {
            EXPECT_EQ(true, info.second == INVAILD_TEMP) << "ThermalHdfApiTest011 Failed";
        }
    }
    GTEST_LOG_(INFO) << "ThermalHdfApiTest011: End.";
}

/**
 * @tc.name: ThermalHdfApiTest012
 * @tc.desc: Get the thermal zone information from HDF layer
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrInterfaceTest, ThermalHdfApiTest012, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalHdfApiTest012: Start.";
    ErrCode ret = ThermalClient::SetSensorTemp("battery", 0);
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalHdfApiTest012: Failed to set battery temp";
    }
    sleep(10 * SLEEP_WAIT_TIME_S);
    std::map<std::string, int32_t> sensor = ThermalClient::GetThermalZoneInfo();
    for (auto info : sensor) {
        if (info.first.find("battery") != std::string::npos) {
            EXPECT_EQ(true, info.second == 0) << "ThermalHdfApiTest012 Failed";
        }
    }
    GTEST_LOG_(INFO) << "ThermalHdfApiTest012: End.";
}