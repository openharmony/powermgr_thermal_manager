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
#include "thermal_mgr_client.h"
#include "thermal_srv_sensor_info.h"
#include "constants.h"
#include "thermal_common.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

static std::mutex g_mtx;
std::vector<std::string> typelist;

int32_t ThermalMgrInterfaceTest::WriteFile(std::string path, std::string buf, size_t size)
{
    FILE *stream = fopen(path.c_str(), "w+");
    if (stream == nullptr) {
        return ERR_INVALID_VALUE;
    }
    size_t ret = fwrite(buf.c_str(), strlen(buf.c_str()), 1, stream);
    if (ret == ERR_OK) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "ret=%{public}d", ret);
    }
    int32_t state = fseek(stream, 0, SEEK_SET);
    if (state != ERR_OK) {
        fclose(stream);
        return state;
    }
    state = fclose(stream);
    if (state != ERR_OK) {
        return state;
    }
    return ERR_OK;
}

int32_t ThermalMgrInterfaceTest::ReadFile(const char *path, char *buf, size_t size)
{
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
    typelist.push_back(SOC);
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

namespace {
/**
 * @tc.name: ThermalMgrInterfaceTest001
 * @tc.desc: test get sensor temp
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest001 start.";
    char batteryTempBuf[MAX_PATH] = {0};
    int32_t temp = 41000;
    int32_t ret = -1;
    std::string sTemp = to_string(temp) + "\n";
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrInterfaceTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(10 * SLEEP_WAIT_TIME_S);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    int32_t out = thermalMgrClient.GetThermalSensorTemp(SensorType::BATTERY);
    GTEST_LOG_(INFO) << "battry temp: " << out;
    EXPECT_EQ(true, temp == out) << "ThermalMgrInterfaceTest001 Failed";
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
    char socTempBuf[MAX_PATH] = {0};
    int32_t temp = 10000;
    int32_t ret = -1;
    std::string sTemp = to_string(temp) + "\n";
    ret = snprintf_s(socTempBuf, PATH_MAX, sizeof(socTempBuf) - 1, socPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrInterfaceTest::WriteFile(socTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(10 * SLEEP_WAIT_TIME_S);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    int32_t out = thermalMgrClient.GetThermalSensorTemp(SensorType::SOC);
    GTEST_LOG_(INFO) << "soc temp: " << out;
    EXPECT_EQ(true, temp == out) << "ThermalMgrInterfaceTest002 Failed";
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
    char shellTempBuf[MAX_PATH] = {0};
    int32_t temp = 11000;
    int32_t ret = -1;
    std::string sTemp = to_string(temp) + "\n";
    ret = snprintf_s(shellTempBuf, PATH_MAX, sizeof(shellTempBuf) - 1, shellPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrInterfaceTest::WriteFile(shellTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(10 * SLEEP_WAIT_TIME_S);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    int32_t out = thermalMgrClient.GetThermalSensorTemp(SensorType::SHELL);
    GTEST_LOG_(INFO) << "shell temp: " << out;
    EXPECT_EQ(true, temp == out) << "ThermalMgrInterfaceTest003 Failed";
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
    char cpuTempBuf[MAX_PATH] = {0};
    int32_t temp = 12000;
    int32_t ret = -1;
    std::string sTemp = to_string(temp) + "\n";
    ret = snprintf_s(cpuTempBuf, PATH_MAX, sizeof(cpuTempBuf) - 1, cpuPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrInterfaceTest::WriteFile(cpuTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(10 * SLEEP_WAIT_TIME_S);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    int32_t out = thermalMgrClient.GetThermalSensorTemp(SensorType::SENSOR1);
    GTEST_LOG_(INFO) << "shell temp: " << out;
    EXPECT_EQ(true, temp == out) << "ThermalMgrInterfaceTest004 Failed";
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
    char chargerTempBuf[MAX_PATH] = {0};
    int32_t temp = 13000;
    int32_t ret = -1;
    std::string sTemp = to_string(temp) + "\n";
    ret = snprintf_s(chargerTempBuf, PATH_MAX, sizeof(chargerTempBuf) - 1, chargerPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrInterfaceTest::WriteFile(chargerTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(10 * SLEEP_WAIT_TIME_S);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    int32_t out = thermalMgrClient.GetThermalSensorTemp(SensorType::SENSOR2);
    GTEST_LOG_(INFO) << "shell temp: " << out;
    EXPECT_EQ(true, temp == out) << "ThermalMgrInterfaceTest005 Failed";
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
    char batteryTempBuf[MAX_PATH] = {0};
    char socTempBuf[MAX_PATH] = {0};
    int32_t ret = -1;
    InitData();
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(socTempBuf, PATH_MAX, sizeof(socTempBuf) - 1, socPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    const sptr<IThermalTempCallback> cb1 = new ThermalTempTest1Callback();

    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest006 start register";
    thermalMgrClient.SubscribeThermalTempCallback(typelist, cb1);
    int32_t temp = 10000;
    for (int i = 0; i < 10; i++) {
        GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest006 start change temp";
        temp += 100;
        std::string sTemp = to_string(temp) + "\n";
        ret = ThermalMgrInterfaceTest::WriteFile(socTempBuf, sTemp, sTemp.length());
        EXPECT_EQ(true, ret == ERR_OK);
        ret = ThermalMgrInterfaceTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
        EXPECT_EQ(true, ret == ERR_OK);
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
    char batteryTempBuf[MAX_PATH] = {0};
    char socTempBuf[MAX_PATH] = {0};
    int32_t ret = -1;
    InitData();
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(socTempBuf, PATH_MAX, sizeof(socTempBuf) - 1, socPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    const sptr<IThermalTempCallback> cb1 = new ThermalTempTest1Callback();
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest007 start register";
    thermalMgrClient.SubscribeThermalTempCallback(typelist, cb1);
    int32_t temp = 10000;
    for (int i = 0; i < 10; i++) {
        GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest007 start change temp";
        temp += 100;
        std::string sTemp = to_string(temp) + "\n";
        ret = ThermalMgrInterfaceTest::WriteFile(socTempBuf, sTemp, sTemp.length());
        EXPECT_EQ(true, ret == ERR_OK);
        ret = ThermalMgrInterfaceTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
        EXPECT_EQ(true, ret == ERR_OK);
        sleep(SLEEP_WAIT_TIME_S * 10);
    }
    thermalMgrClient.UnSubscribeThermalTempCallback(cb1);
    sleep(SLEEP_WAIT_TIME_S * 10);
    const sptr<IThermalTempCallback> cb2 = new ThermalTempTest2Callback();
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest007 start register";
    thermalMgrClient.SubscribeThermalTempCallback(typelist, cb1);
    for (int i = 0; i < 10; i++) {
        GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest007 start change temp";
        temp += 100;
        std::string sTemp = to_string(temp) + "\n";
        ret = ThermalMgrInterfaceTest::WriteFile(socTempBuf, sTemp, sTemp.length());
        EXPECT_EQ(true, ret == ERR_OK);
        ret = ThermalMgrInterfaceTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
        EXPECT_EQ(true, ret == ERR_OK);
        sleep(SLEEP_WAIT_TIME_S * 10);
    }
    thermalMgrClient.UnSubscribeThermalTempCallback(cb1);
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest007 end.";
}


/**
 * @tc.name: ThermalMgrInterfaceTest008
 * @tc.desc: test get level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest008, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest008 start.";
    char batteryTempBuf[MAX_PATH] = {0};
    int32_t temp = 40100;
    int32_t ret = -1;
    std::string sTemp = to_string(temp) + "\n";
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrInterfaceTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(10 * SLEEP_WAIT_TIME_S);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    ThermalLevel level = thermalMgrClient.GetThermalLevel();
    int32_t levelValue = static_cast<int32_t>(level);
    GTEST_LOG_(INFO) << "levelValue: " << levelValue;
    EXPECT_EQ(true, level == ThermalLevel::NORMAL) << "ThermalMgrInterfaceTest008 Failed";
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
    char batteryTempBuf[MAX_PATH] = {0};
    int32_t temp = 43100;
    int32_t ret = -1;
    std::string sTemp = to_string(temp) + "\n";
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrInterfaceTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(10 * SLEEP_WAIT_TIME_S);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    ThermalLevel level = thermalMgrClient.GetThermalLevel();
    int32_t levelValue = static_cast<int32_t>(level);
    GTEST_LOG_(INFO) << "levelValue: " << levelValue;
    EXPECT_EQ(true, level == ThermalLevel::WARM) << "ThermalMgrInterfaceTest009 Failed";
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
    char batteryTempBuf[MAX_PATH] = {0};
    int32_t temp = 46100;
    int32_t ret = -1;
    std::string sTemp = to_string(temp) + "\n";
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrInterfaceTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(10 * SLEEP_WAIT_TIME_S);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    ThermalLevel level = thermalMgrClient.GetThermalLevel();
    int32_t levelValue = static_cast<int32_t>(level);
    GTEST_LOG_(INFO) << "levelValue: " << levelValue;
    EXPECT_EQ(true, level == ThermalLevel::HOT) << "ThermalMgrInterfaceTest010 Failed";
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
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    int32_t ret = -1;
    ret = snprintf_s(paTempBuf, PATH_MAX, sizeof(paTempBuf) - 1, paPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(amTempBuf, PATH_MAX, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t paTemp = 44100;
    int32_t amTemp = 20000;

    std::string sTemp = to_string(paTemp) + "\n";
    ret = ThermalMgrInterfaceTest::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrInterfaceTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(10 * SLEEP_WAIT_TIME_S);
    ThermalLevel level = thermalMgrClient.GetThermalLevel();
    int32_t levelValue = static_cast<int32_t>(level);
    GTEST_LOG_(INFO) << "levelValue: " << levelValue;
    EXPECT_EQ(true, level == ThermalLevel::HOT) << "ThermalMgrInterfaceTest011 Failed";
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest011 end.";
}

/**
 * @tc.name: ThermalMgrInterfaceTest012
 * @tc.desc: Get Thermal Level
 * @tc.type: FUNC
 * @tc.result: level get min
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest012, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest012 start.";
    char apTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    char shellTempBuf[MAX_PATH] = {0};
    int32_t ret = -1;
    ret = snprintf_s(apTempBuf, PATH_MAX, sizeof(apTempBuf) - 1, apPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(amTempBuf, PATH_MAX, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(shellTempBuf, PATH_MAX, sizeof(shellTempBuf) - 1, shellPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t apTemp = 79000;
    int32_t amTemp = 60000;
    int32_t shellTemp = 50000;
    std::string sTemp = to_string(apTemp) + "\n";
    ret = ThermalMgrInterfaceTest::WriteFile(apTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrInterfaceTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sTemp = to_string(shellTemp) + "\n";
    ret = ThermalMgrInterfaceTest::WriteFile(shellTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(10 * SLEEP_WAIT_TIME_S);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    ThermalLevel level = thermalMgrClient.GetThermalLevel();
    int32_t levelValue = static_cast<int32_t>(level);
    GTEST_LOG_(INFO) << "levelValue: " << levelValue;
    EXPECT_EQ(true, level == ThermalLevel::HOT) << "ThermalMgrInterfaceTest012 Failed";
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
    char batteryTempBuf[MAX_PATH] = {0};
    int32_t temp = -19100;
    int32_t ret = -1;
    std::string sTemp = to_string(temp) + "\n";
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrInterfaceTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    sleep(10 * SLEEP_WAIT_TIME_S);
    ThermalLevel level = thermalMgrClient.GetThermalLevel();
    int32_t levelValue = static_cast<int32_t>(level);
    GTEST_LOG_(INFO) << "levelValue: " << levelValue;
    EXPECT_EQ(true, level == ThermalLevel::COOL) << "ThermalMgrInterfaceTest0013 Failed";
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
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    int32_t ret = -1;
    ret = snprintf_s(paTempBuf, PATH_MAX, sizeof(paTempBuf) - 1, paPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(amTempBuf, PATH_MAX, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t paTemp = 40100;
    int32_t amTemp = 20000;

    std::string sTemp = to_string(paTemp) + "\n";
    ret = ThermalMgrInterfaceTest::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrInterfaceTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(10 * SLEEP_WAIT_TIME_S);
    ThermalLevel level = thermalMgrClient.GetThermalLevel();
    int32_t levelValue = static_cast<int32_t>(level);
    GTEST_LOG_(INFO) << "levelValue: " << levelValue;
    EXPECT_EQ(true, level == ThermalLevel::COOL) << "ThermalMgrInterfaceTest014 Failed";
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest014 end.";
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest014 start.";
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
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    const sptr<IThermalLevelCallback> cb1 = new ThermalLevelTest1Callback();
    thermalMgrClient.SubscribeThermalLevelCallback(cb1);

    int32_t temp = -20000;
    std::string sTemp = to_string(temp) + "\n";
    ret = ThermalMgrInterfaceTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(10 * SLEEP_WAIT_TIME_S);

    temp = 40100;
    sTemp = to_string(temp) + "\n";
    ret = ThermalMgrInterfaceTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(10 * SLEEP_WAIT_TIME_S);

    temp  = -10000;
    sTemp = to_string(temp) + "\n";
    ret = ThermalMgrInterfaceTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(10 * SLEEP_WAIT_TIME_S);

    temp  = 46000;
    sTemp = to_string(temp) + "\n";
    ret = ThermalMgrInterfaceTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
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

    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    int32_t temp = -20000;
    for (uint32_t i = 0; i < 10; i++) {
        GTEST_LOG_(INFO) << "change temp.";
        std::string sTemp = to_string(temp) + "\n";
        ret = ThermalMgrInterfaceTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
        EXPECT_EQ(true, ret == ERR_OK);
        sleep(10 * SLEEP_WAIT_TIME_S);
        temp += 10000;
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

    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t temp = -20000;
    std::string sTemp = to_string(temp) + "\n";
    ret = ThermalMgrInterfaceTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(10 * SLEEP_WAIT_TIME_S);

    thermalMgrClient.UnSubscribeThermalLevelCallback(cb2);
    thermalMgrClient.UnSubscribeThermalLevelCallback(cb1);
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest017 end.";
}

/**
 * @tc.name: ThermalMgrInterfaceTest018
 * @tc.desc: test register callback and get thermal level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest018, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest018 start.";
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    const sptr<IThermalLevelCallback> cb1 = new ThermalLevelTest1Callback();
    thermalMgrClient.SubscribeThermalLevelCallback(cb1);

    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t temp = -20000;
    std::string sTemp = to_string(temp) + "\n";
    ret = ThermalMgrInterfaceTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(10 * SLEEP_WAIT_TIME_S);
    thermalMgrClient.UnSubscribeThermalLevelCallback(cb1);
    sleep(SLEEP_WAIT_TIME_S);
    thermalMgrClient.SubscribeThermalLevelCallback(cb1);
    temp = 48000;
    sTemp = to_string(temp) + "\n";
    ret = ThermalMgrInterfaceTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(10 * SLEEP_WAIT_TIME_S);
    thermalMgrClient.UnSubscribeThermalLevelCallback(cb1);
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest020 end.";
}

/**
 * @tc.name: ThermalMgrInterfaceTest021
 * @tc.desc: test get invaild temp
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest021, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest021 start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t temp = INVAILD_TEMP;
    std::string sTemp = to_string(temp) + "\n";
    ret = ThermalMgrInterfaceTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(10 * SLEEP_WAIT_TIME_S);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    int32_t out = thermalMgrClient.GetThermalSensorTemp(SensorType::BATTERY);
    GTEST_LOG_(INFO) << "battry temp: " << out;
    EXPECT_EQ(true, temp == out) << "ThermalMgrInterfaceTest021 Failed";
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

    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t temp = INVAILD_TEMP;
    std::string sTemp = to_string(temp) + "\n";
    ret = ThermalMgrInterfaceTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(10 * SLEEP_WAIT_TIME_S);
    thermalMgrClient.UnSubscribeThermalTempCallback(cb);
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest022 end.";
}

/**
 * @tc.name: ThermalMgrInterfaceTest023
 * @tc.desc: test get level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest023, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest023 start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t temp = -1000;
    std::string sTemp = to_string(temp) + "\n";
    ret = ThermalMgrInterfaceTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(10 * SLEEP_WAIT_TIME_S);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    ThermalLevel level = thermalMgrClient.GetThermalLevel();
    int32_t levelValue = static_cast<int32_t>(level);
    GTEST_LOG_(INFO) << "levelValue: " << levelValue;
    EXPECT_EQ(true, level == ThermalLevel::OVERHEATED) << "ThermalMgrInterfaceTest023 Failed";
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
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t temp = INVAILD_TEMP;
    std::string sTemp = to_string(temp) + "\n";
    ret = ThermalMgrInterfaceTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(10 * SLEEP_WAIT_TIME_S);
    thermalMgrClient.UnSubscribeThermalLevelCallback(cb);
    GTEST_LOG_(INFO) << "ThermalMgrInterfaceTest024 end.";
}
}