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

#include "thermal_mgr_interface_test.h"


#include <condition_variable>
#include <mutex>

#include "constants.h"
#include "mock_thermal_mgr_client.h"
#include "thermal_log.h"
#include "thermal_mgr_client.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
std::vector<std::string> typelist;
std::condition_variable g_callbackCV;
std::mutex g_mutex;
constexpr int64_t TIME_OUT = 1;
bool g_callbackTriggered = false;
bool g_levelCallBack4 = false;
bool g_levelCallBack5 = false;
} // namespace

void Notify()
{
    std::unique_lock<std::mutex> lock(g_mutex);
    g_callbackTriggered = true;
    lock.unlock();
    g_callbackCV.notify_one();
}

void Wait()
{
    std::unique_lock<std::mutex> lock(g_mutex);
    g_callbackCV.wait_for(lock, std::chrono::seconds(TIME_OUT), [] { return g_callbackTriggered; });
    EXPECT_TRUE(g_callbackTriggered);
    g_callbackTriggered = false;
}

void ThermalMgrInterfaceTest::TearDown()
{
    InitNode();
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("");
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    g_callbackTriggered = false;
}

void ThermalMgrInterfaceTest::InitData()
{
    typelist.push_back(BATTERY);
    typelist.push_back(SOC);
}

bool ThermalMgrInterfaceTest::ThermalTempTest1Callback::OnThermalTempChanged(TempCallbackMap& tempCbMap)
{
    int assertValue = 0;
    for (auto iter : tempCbMap) {
        THERMAL_HILOGD(LABEL_TEST, "type: %{public}s, temp: %{public}d", iter.first.c_str(), iter.second);
        EXPECT_EQ(true, iter.second >= assertValue) << "Test Failed";
    }
    Notify();
    return true;
}

bool ThermalMgrInterfaceTest::ThermalTempTest2Callback::OnThermalTempChanged(TempCallbackMap& tempCbMap)
{
    int assertValue = 0;
    for (auto iter : tempCbMap) {
        THERMAL_HILOGD(LABEL_TEST, "type: %{public}s, temp: %{public}d", iter.first.c_str(), iter.second);
        EXPECT_EQ(true, iter.second >= assertValue) << "Test Failed";
    }
    Notify();
    return true;
}

bool ThermalMgrInterfaceTest::ThermalLevelTest1Callback::GetThermalLevel(ThermalLevel level)
{
    int assertMin = -1;
    int assertMax = 6;
    int32_t levelValue = static_cast<int32_t>(level);
    THERMAL_HILOGD(LABEL_TEST, "level: %{public}d", levelValue);
    EXPECT_EQ(true, levelValue >= assertMin && levelValue <= assertMax) << "Test Failed";
    Notify();
    return true;
}

bool ThermalMgrInterfaceTest::ThermalLevelTest2Callback::GetThermalLevel(ThermalLevel level)
{
    int assertMin = -1;
    int assertMax = 6;
    int32_t levelValue = static_cast<int32_t>(level);
    THERMAL_HILOGD(LABEL_TEST, "level: %{public}d", levelValue);
    EXPECT_EQ(true, levelValue >= assertMin && levelValue <= assertMax) << "Test Failed";
    Notify();
    return true;
}

bool ThermalMgrInterfaceTest::ThermalLevelTest3Callback::GetThermalLevel(ThermalLevel level)
{
    int assertMin = -1;
    int assertMax = 6;
    int32_t levelValue = static_cast<int32_t>(level);
    THERMAL_HILOGD(LABEL_TEST, "level: %{public}d", levelValue);
    EXPECT_EQ(true, levelValue >= assertMin && levelValue <= assertMax) << "Test Failed";
    Notify();
    return true;
}

bool ThermalMgrInterfaceTest::ThermalLevelTest4Callback::GetThermalLevel(ThermalLevel level)
{
    int assertMin = -1;
    int assertMax = 6;
    int32_t levelValue = static_cast<int32_t>(level);
    THERMAL_HILOGD(LABEL_TEST, "level: %{public}d", levelValue);
    EXPECT_EQ(true, levelValue >= assertMin && levelValue <= assertMax) << "Test Failed";
    std::unique_lock<std::mutex> lock(g_mutex);
    g_levelCallBack4 = true;
    lock.unlock();
    g_callbackCV.notify_one();
    return true;
}

bool ThermalMgrInterfaceTest::ThermalLevelTest5Callback::GetThermalLevel(ThermalLevel level)
{
    int assertMin = -1;
    int assertMax = 6;
    int32_t levelValue = static_cast<int32_t>(level);
    THERMAL_HILOGD(LABEL_TEST, "level: %{public}d", levelValue);
    EXPECT_EQ(true, levelValue >= assertMin && levelValue <= assertMax) << "Test Failed";
    std::unique_lock<std::mutex> lock(g_mutex);
    g_levelCallBack5 = true;
    lock.unlock();
    g_callbackCV.notify_one();
    return true;
}

namespace {
/**
 * @tc.name: ThermalMgrInterfaceTest001
 * @tc.desc: test get sensor temp
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest001, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest001 start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    int32_t temp = 41000;
    int32_t ret = SetNodeValue(temp, BATTERY_PATH);
    EXPECT_EQ(true, ret == ERR_OK);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    int32_t out = thermalMgrClient.GetThermalSensorTemp(SensorType::BATTERY);
    EXPECT_EQ(true, temp == out) << "ThermalMgrInterfaceTest001 Failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest001 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest002
 * @tc.desc: test get sensor temp
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest002, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest002 start");
    if (!IsMock(SOC_PATH) || IsVendor()) {
        return;
    }
    int32_t temp = 10000;
    int32_t ret = SetNodeValue(temp, SOC_PATH);
    EXPECT_EQ(true, ret == ERR_OK);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    int32_t out = thermalMgrClient.GetThermalSensorTemp(SensorType::SOC);
    EXPECT_EQ(true, temp == out) << "ThermalMgrInterfaceTest002 Failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest002 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest003
 * @tc.desc: test get sensor temp
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest003, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest003 start");
    if (!IsMock(SHELL_PATH) || IsVendor()) {
        return;
    }
    int32_t temp = 11000;
    int32_t ret = SetNodeValue(temp, SHELL_PATH);
    EXPECT_EQ(true, ret == ERR_OK);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    int32_t out = thermalMgrClient.GetThermalSensorTemp(SensorType::SHELL);
    EXPECT_EQ(true, temp == out) << "ThermalMgrInterfaceTest003 Failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest003 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest004
 * @tc.desc: test get sensor temp
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest004, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest004 start");
    if (!IsMock(CPU_PATH) || IsVendor()) {
        return;
    }
    int32_t temp = 12000;
    int32_t ret = SetNodeValue(temp, CPU_PATH);
    EXPECT_EQ(true, ret == ERR_OK);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    int32_t out = thermalMgrClient.GetThermalSensorTemp(SensorType::SENSOR1);
    EXPECT_EQ(true, temp == out) << "ThermalMgrInterfaceTest004 Failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest004 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest005
 * @tc.desc: test get sensor temp
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest005, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest005 start");
    if (!IsMock(CHARGER_PATH) || IsVendor()) {
        return;
    }
    int32_t temp = 13000;
    int32_t ret = SetNodeValue(temp, CHARGER_PATH);
    EXPECT_EQ(true, ret == ERR_OK);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    int32_t out = thermalMgrClient.GetThermalSensorTemp(SensorType::SENSOR2);
    EXPECT_EQ(true, temp == out) << "ThermalMgrInterfaceTest005 Failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest005 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest006
 * @tc.desc: register callback and get temp list
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest006, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest006 start");
    if (!IsMock(BATTERY_PATH) || !IsMock(SOC_PATH) || IsVendor()) {
        return;
    }
    InitData();
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    const sptr<IThermalTempCallback> cb1 = new ThermalTempTest1Callback();

    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest006 start register");
    thermalMgrClient.SubscribeThermalTempCallback(typelist, cb1);
    int32_t temp = 10000;
    int32_t ret = -1;
    for (int i = 0; i < 10; i++) {
        temp += 100;
        ret = SetNodeValue(temp, SOC_PATH);
        EXPECT_EQ(true, ret == ERR_OK);
        ret = SetNodeValue(temp, BATTERY_PATH);
        EXPECT_EQ(true, ret == ERR_OK);
        MockThermalMgrClient::GetInstance().GetThermalInfo();
        Wait();
    }
    thermalMgrClient.UnSubscribeThermalTempCallback(cb1);
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest006 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest007
 * @tc.desc: register callback and get temp list
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest007, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest007 start");
    if (!IsMock(BATTERY_PATH) || !IsMock(SOC_PATH) || IsVendor()) {
        return;
    }
    int32_t ret = -1;
    InitData();
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    const sptr<IThermalTempCallback> cb1 = new ThermalTempTest1Callback();

    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest007 start register");
    thermalMgrClient.SubscribeThermalTempCallback(typelist, cb1);
    int32_t temp = 10000;
    for (int i = 0; i < 10; i++) {
        temp += 100;
        ret = SetNodeValue(temp, SOC_PATH);
        EXPECT_EQ(true, ret == ERR_OK);
        ret = SetNodeValue(temp, BATTERY_PATH);
        EXPECT_EQ(true, ret == ERR_OK);
        MockThermalMgrClient::GetInstance().GetThermalInfo();
        Wait();
    }
    thermalMgrClient.UnSubscribeThermalTempCallback(cb1);

    MockThermalMgrClient::GetInstance().GetThermalInfo();
    const sptr<IThermalTempCallback> cb2 = new ThermalTempTest2Callback();

    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest007 start register");
    thermalMgrClient.SubscribeThermalTempCallback(typelist, cb1);
    for (int i = 0; i < 10; i++) {
        temp += 100;
        ret = SetNodeValue(temp, SOC_PATH);
        EXPECT_EQ(true, ret == ERR_OK);
        ret = SetNodeValue(temp, BATTERY_PATH);
        EXPECT_EQ(true, ret == ERR_OK);
        MockThermalMgrClient::GetInstance().GetThermalInfo();
        Wait();
    }
    thermalMgrClient.UnSubscribeThermalTempCallback(cb1);
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest007 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest008
 * @tc.desc: test get level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest008, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest008 start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    int32_t temp = 40100;
    int32_t ret = SetNodeValue(temp, BATTERY_PATH);
    EXPECT_EQ(true, ret == ERR_OK);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    ThermalLevel level = thermalMgrClient.GetThermalLevel();
    EXPECT_EQ(true, level == ThermalLevel::NORMAL) << "ThermalMgrInterfaceTest008 Failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest008 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest009
 * @tc.desc: test get level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest009, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest009 start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    int32_t temp = 43100;
    int32_t ret = SetNodeValue(temp, BATTERY_PATH);
    EXPECT_EQ(true, ret == ERR_OK);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    ThermalLevel level = thermalMgrClient.GetThermalLevel();
    EXPECT_EQ(true, level == ThermalLevel::WARM) << "ThermalMgrInterfaceTest009 Failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest009 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest010
 * @tc.desc: test get level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest010, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest010 start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    int32_t temp = 46100;
    int32_t ret = SetNodeValue(temp, BATTERY_PATH);
    EXPECT_EQ(true, ret == ERR_OK);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    ThermalLevel level = thermalMgrClient.GetThermalLevel();
    EXPECT_EQ(true, level == ThermalLevel::HOT) << "ThermalMgrInterfaceTest010 Failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest010 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest011
 * @tc.desc: test get level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest011, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest011 start");
    if (!IsMock(PA_PATH) || !IsMock(AMBIENT_PATH) || IsVendor()) {
        return;
    }
    int32_t paTemp = 44100;
    int32_t amTemp = 20000;
    int32_t ret = SetNodeValue(paTemp, PA_PATH);
    EXPECT_EQ(true, ret == ERR_OK);
    ret = SetNodeValue(amTemp, AMBIENT_PATH);
    EXPECT_EQ(true, ret == ERR_OK);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    ThermalLevel level = thermalMgrClient.GetThermalLevel();
    EXPECT_EQ(true, level == ThermalLevel::WARNING) << "ThermalMgrInterfaceTest011 Failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest011 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest012
 * @tc.desc: Get Thermal Level
 * @tc.type: FUNC
 * @tc.result: level get min
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest012, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest012 start");
    if (!IsMock(AP_PATH) || !IsMock(AMBIENT_PATH) || !IsMock(SHELL_PATH) || IsVendor()) {
        return;
    }
    int32_t apTemp = 79000;
    int32_t amTemp = 60000;
    int32_t shellTemp = 30000;
    int32_t ret = SetNodeValue(apTemp, AP_PATH);
    EXPECT_EQ(true, ret == ERR_OK);
    ret = SetNodeValue(amTemp, AMBIENT_PATH);
    EXPECT_EQ(true, ret == ERR_OK);
    ret = SetNodeValue(shellTemp, SHELL_PATH);
    EXPECT_EQ(true, ret == ERR_OK);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    ThermalLevel level = thermalMgrClient.GetThermalLevel();
    EXPECT_EQ(true, level == ThermalLevel::EMERGENCY) << "ThermalMgrInterfaceTest012 Failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest012 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest0013
 * @tc.desc: test get level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest0013, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest0013 start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    int32_t temp = -19100;
    int32_t ret = SetNodeValue(temp, BATTERY_PATH);
    EXPECT_EQ(true, ret == ERR_OK);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    ThermalLevel level = thermalMgrClient.GetThermalLevel();
    EXPECT_EQ(true, level == ThermalLevel::COOL) << "ThermalMgrInterfaceTest0013 Failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest0013 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest014
 * @tc.desc: test get level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest014, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest014 start");
    if (!IsMock(PA_PATH) || !IsMock(AMBIENT_PATH) || IsVendor()) {
        return;
    }
    int32_t paTemp = 40100;
    int32_t amTemp = 20000;
    int32_t ret = SetNodeValue(paTemp, PA_PATH);
    EXPECT_EQ(true, ret == ERR_OK);
    ret = SetNodeValue(amTemp, AMBIENT_PATH);
    EXPECT_EQ(true, ret == ERR_OK);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    ThermalLevel level = thermalMgrClient.GetThermalLevel();
    EXPECT_EQ(true, level == ThermalLevel::OVERHEATED) << "ThermalMgrInterfaceTest014 Failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest014 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest015
 * @tc.desc: test register callback and get thermal level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest015, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest015 start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    const sptr<IThermalLevelCallback> cb1 = new ThermalLevelTest1Callback();
    thermalMgrClient.SubscribeThermalLevelCallback(cb1);
    Wait(); // thermal Level callback will be triggered when subscribed

    int32_t ret = -1;
    int32_t temp = -20000;
    ret = SetNodeValue(temp, BATTERY_PATH);
    EXPECT_EQ(true, ret == ERR_OK);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    EXPECT_FALSE(g_callbackTriggered);

    temp = 40100;
    ret = SetNodeValue(temp, BATTERY_PATH);
    EXPECT_EQ(true, ret == ERR_OK);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    Wait();

    temp = -10000;
    ret = SetNodeValue(temp, BATTERY_PATH);
    EXPECT_EQ(true, ret == ERR_OK);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    Wait();

    temp = 46000;
    ret = SetNodeValue(temp, BATTERY_PATH);
    EXPECT_EQ(true, ret == ERR_OK);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    Wait();

    thermalMgrClient.UnSubscribeThermalLevelCallback(cb1);
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest015 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest016
 * @tc.desc: test register callback and get thermal level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest016, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest016 start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    const sptr<IThermalLevelCallback> cb2 = new ThermalLevelTest2Callback();
    thermalMgrClient.SubscribeThermalLevelCallback(cb2);
    Wait(); // thermal Level callback will be triggered when subscribed

    int32_t ret = -1;
    int32_t temp = -25100;
    for (uint32_t i = 0; i < 5; i++) {
    THERMAL_HILOGD(LABEL_TEST, "temp: %{public}d", temp);
        ret = SetNodeValue(temp, BATTERY_PATH);
        EXPECT_EQ(true, ret == ERR_OK);
        MockThermalMgrClient::GetInstance().GetThermalInfo();
        temp += 5000;
        EXPECT_FALSE(g_callbackTriggered);
    }
    temp = 40100;
    for (uint32_t i = 0; i < 3; i++) {
        ret = SetNodeValue(temp, BATTERY_PATH);
        EXPECT_EQ(true, ret == ERR_OK);
        MockThermalMgrClient::GetInstance().GetThermalInfo();
        temp += 3000;
        Wait();
    }
    thermalMgrClient.UnSubscribeThermalLevelCallback(cb2);
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest016 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest017
 * @tc.desc: test register callback and get thermal level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest017, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest017 start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    const sptr<IThermalLevelCallback> cb4 = new ThermalLevelTest4Callback();
    const sptr<IThermalLevelCallback> cb5 = new ThermalLevelTest5Callback();
    thermalMgrClient.SubscribeThermalLevelCallback(cb4);
    thermalMgrClient.SubscribeThermalLevelCallback(cb5);
    // thermal Level callback will be triggered when subscribed
    std::unique_lock<std::mutex> lock(g_mutex);
    g_callbackCV.wait_for(lock, std::chrono::seconds(TIME_OUT), [] { return (g_levelCallBack4 && g_levelCallBack5); });
    EXPECT_TRUE(g_levelCallBack4);
    EXPECT_TRUE(g_levelCallBack5);
    g_levelCallBack4 = false;
    g_levelCallBack5 = false;

    int32_t temp = -20000;
    int32_t ret = SetNodeValue(temp, BATTERY_PATH);
    EXPECT_EQ(true, ret == ERR_OK);
    MockThermalMgrClient::GetInstance().GetThermalInfo();

    EXPECT_FALSE(g_levelCallBack4);
    EXPECT_FALSE(g_levelCallBack5);

    thermalMgrClient.UnSubscribeThermalLevelCallback(cb5);
    thermalMgrClient.UnSubscribeThermalLevelCallback(cb4);
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest017 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest018
 * @tc.desc: test register callback and get thermal level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest018, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest018 start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    const sptr<IThermalLevelCallback> cb1 = new ThermalLevelTest1Callback();
    thermalMgrClient.SubscribeThermalLevelCallback(cb1);
    Wait(); // thermal Level callback will be triggered when subscribed

    int32_t temp = -20000;
    int32_t ret = SetNodeValue(temp, BATTERY_PATH);
    EXPECT_EQ(true, ret == ERR_OK);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    EXPECT_FALSE(g_callbackTriggered);
    thermalMgrClient.UnSubscribeThermalLevelCallback(cb1);

    MockThermalMgrClient::GetInstance().GetThermalInfo();
    thermalMgrClient.SubscribeThermalLevelCallback(cb1);
    Wait(); // thermal Level callback will be triggered when subscribed

    temp = 48000;
    ret = SetNodeValue(temp, BATTERY_PATH);
    EXPECT_EQ(true, ret == ERR_OK);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    Wait();

    thermalMgrClient.UnSubscribeThermalLevelCallback(cb1);
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest020 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest021
 * @tc.desc: test get invaild temp
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest021, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest021 start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    int32_t temp = INVAILD_TEMP;
    int32_t ret = SetNodeValue(temp, BATTERY_PATH);
    EXPECT_EQ(true, ret == ERR_OK);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    int32_t out = thermalMgrClient.GetThermalSensorTemp(SensorType::BATTERY);
    EXPECT_EQ(true, temp == out) << "ThermalMgrInterfaceTest021 Failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest021 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest022
 * @tc.desc: test get invaild temp
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest022, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest022 start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    const sptr<IThermalTempCallback> cb = nullptr;
    InitData();
    thermalMgrClient.SubscribeThermalTempCallback(typelist, cb);

    int32_t temp = INVAILD_TEMP;
    int32_t ret = SetNodeValue(temp, BATTERY_PATH);
    EXPECT_EQ(true, ret == ERR_OK);

    MockThermalMgrClient::GetInstance().GetThermalInfo();
    thermalMgrClient.UnSubscribeThermalTempCallback(cb);
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest022 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest023
 * @tc.desc: test get level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest023, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest023 start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    int32_t temp = -1000;
    int32_t ret = SetNodeValue(temp, BATTERY_PATH);
    EXPECT_EQ(true, ret == ERR_OK);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    ThermalLevel level = thermalMgrClient.GetThermalLevel();
    EXPECT_EQ(true, level == ThermalLevel::COOL) << "ThermalMgrInterfaceTest023 Failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest023 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest024
 * @tc.desc: test register null callback
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest024, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest024 start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    const sptr<IThermalLevelCallback> cb = nullptr;
    thermalMgrClient.SubscribeThermalLevelCallback(cb);
    int32_t temp = INVAILD_TEMP;
    int32_t ret = SetNodeValue(temp, BATTERY_PATH);
    EXPECT_EQ(true, ret == ERR_OK);

    MockThermalMgrClient::GetInstance().GetThermalInfo();
    thermalMgrClient.UnSubscribeThermalLevelCallback(cb);
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest024 end");
}
} // namespace
