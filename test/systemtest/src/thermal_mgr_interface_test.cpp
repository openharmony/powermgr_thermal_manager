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

#include <atomic>
#include <condition_variable>
#include <mutex>

#include "constants.h"
#include "mock_thermal_mgr_client.h"
#include "thermal_log.h"
#include "thermal_mgr_client.h"

#define private   public
#define protected public
#include "thermal_service.h"
#include "thermal_srv_config_parser.h"
#include "v1_1/ithermal_interface.h"
#include "v1_1/thermal_types.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
using namespace OHOS::HDI::Thermal::V1_1;

namespace {
std::vector<std::string> typelist;
std::condition_variable g_callbackCV;
std::mutex g_mutex;
constexpr int64_t TIME_OUT = 3;
std::atomic_bool g_callbackTriggered = false;
std::atomic_bool g_levelCallBack4 = false;
std::atomic_bool g_levelCallBack5 = false;
const std::string SYSTEM_THERMAL_SERVICE_CONFIG_PATH = "/system/etc/thermal_config/thermal_service_config.xml";
sptr<ThermalService> g_service = nullptr;
auto& g_thermalMgrClient = ThermalMgrClient::GetInstance();

void Notify()
{
    g_callbackTriggered = true;
    g_callbackCV.notify_one();
}

void Wait()
{
    std::unique_lock<std::mutex> lock(g_mutex);
    g_callbackCV.wait_for(lock, std::chrono::seconds(TIME_OUT), [] {
        return g_callbackTriggered.load();
    });
    EXPECT_TRUE(g_callbackTriggered);
    g_callbackTriggered = false;
}
} // namespace

void ThermalMgrInterfaceTest::TearDown()
{
    g_service->SetScene("");
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 0;
    event.info.push_back(info1);
    info1.type = "ap";
    event.info.push_back(info1);
    info1.type = "pa";
    event.info.push_back(info1);
    info1.type = "shell";
    event.info.push_back(info1);
    info1.type = "ambient";
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    g_callbackTriggered = false;
}

void ThermalMgrInterfaceTest::SetUpTestCase()
{
    g_service = ThermalService::GetInstance();
    g_service->InitSystemTestModules();
    g_service->OnStart();
    g_service->GetBaseinfoObj()->Init();
    g_service->GetObserver()->InitSensorTypeMap();
}

void ThermalMgrInterfaceTest::TearDownTestCase()
{
    g_service->OnStop();
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

bool ThermalMgrInterfaceTest::ThermalLevelTest1Callback::OnThermalLevelChanged(ThermalLevel level)
{
    int assertMin = -1;
    int assertMax = 7;
    int32_t levelValue = static_cast<int32_t>(level);
    THERMAL_HILOGD(LABEL_TEST, "level: %{public}d", levelValue);
    EXPECT_EQ(true, levelValue >= assertMin && levelValue <= assertMax) << "Test Failed";
    Notify();
    return true;
}

bool ThermalMgrInterfaceTest::ThermalLevelTest2Callback::OnThermalLevelChanged(ThermalLevel level)
{
    int assertMin = -1;
    int assertMax = 7;
    int32_t levelValue = static_cast<int32_t>(level);
    THERMAL_HILOGD(LABEL_TEST, "level: %{public}d", levelValue);
    EXPECT_EQ(true, levelValue >= assertMin && levelValue <= assertMax) << "Test Failed";
    Notify();
    return true;
}

bool ThermalMgrInterfaceTest::ThermalLevelTest3Callback::OnThermalLevelChanged(ThermalLevel level)
{
    int assertMin = -1;
    int assertMax = 7;
    int32_t levelValue = static_cast<int32_t>(level);
    THERMAL_HILOGD(LABEL_TEST, "level: %{public}d", levelValue);
    EXPECT_EQ(true, levelValue >= assertMin && levelValue <= assertMax) << "Test Failed";
    Notify();
    return true;
}

bool ThermalMgrInterfaceTest::ThermalLevelTest4Callback::OnThermalLevelChanged(ThermalLevel level)
{
    int assertMin = -1;
    int assertMax = 7;
    int32_t levelValue = static_cast<int32_t>(level);
    THERMAL_HILOGD(LABEL_TEST, "level: %{public}d", levelValue);
    EXPECT_EQ(true, levelValue >= assertMin && levelValue <= assertMax) << "Test Failed";
    g_levelCallBack4 = true;
    g_callbackCV.notify_one();
    return true;
}

bool ThermalMgrInterfaceTest::ThermalLevelTest5Callback::OnThermalLevelChanged(ThermalLevel level)
{
    int assertMin = -1;
    int assertMax = 7;
    int32_t levelValue = static_cast<int32_t>(level);
    THERMAL_HILOGD(LABEL_TEST, "level: %{public}d", levelValue);
    EXPECT_EQ(true, levelValue >= assertMin && levelValue <= assertMax) << "Test Failed";
    g_levelCallBack5 = true;
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
    sleep(1);
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 41000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    ThermalSrvSensorInfo info;
    g_service->GetThermalSrvSensorInfo(SensorType::BATTERY, info);
    g_thermalMgrClient.GetThermalSensorTemp(SensorType::BATTERY);
    EXPECT_EQ(info1.temp, info.GetTemp()) << "ThermalMgrInterfaceTest001 Failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "soc";
    info1.temp = 10000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    ThermalSrvSensorInfo info;
    g_service->GetThermalSrvSensorInfo(SensorType::SOC, info);
    g_thermalMgrClient.GetThermalSensorTemp(SensorType::SOC);
    EXPECT_EQ(info1.temp, info.GetTemp()) << "ThermalMgrInterfaceTest002 Failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "shell";
    info1.temp = 11000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    ThermalSrvSensorInfo info;
    g_service->GetThermalSrvSensorInfo(SensorType::SHELL, info);
    g_thermalMgrClient.GetThermalSensorTemp(SensorType::SHELL);
    EXPECT_EQ(info1.temp, info.GetTemp()) << "ThermalMgrInterfaceTest003 Failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "cpu";
    info1.temp = 12000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    ThermalSrvSensorInfo info;
    g_service->GetThermalSrvSensorInfo(SensorType::SENSOR1, info);
    g_thermalMgrClient.GetThermalSensorTemp(SensorType::SENSOR1);
    EXPECT_EQ(info1.temp, info.GetTemp()) << "ThermalMgrInterfaceTest004 Failed";
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
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "charger";
    info1.temp = 13000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    ThermalSrvSensorInfo info;
    g_service->GetThermalSrvSensorInfo(SensorType::SENSOR2, info);
    g_thermalMgrClient.GetThermalSensorTemp(SensorType::SENSOR2);
    EXPECT_EQ(info1.temp, info.GetTemp()) << "ThermalMgrInterfaceTest005 Failed";
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
    InitData();
    const sptr<IThermalTempCallback> cb1 = new ThermalTempTest1Callback();
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest006 start register");
    g_service->SubscribeThermalTempCallback(typelist, cb1);
    g_thermalMgrClient.SubscribeThermalTempCallback(typelist, cb1);
    int32_t temp = 10000;
    for (int i = 0; i < 10; i++) {
        HdfThermalCallbackInfo event;
        ThermalZoneInfo info1;
        info1.type = "soc";
        info1.temp = temp;
        event.info.push_back(info1);

        ThermalZoneInfo info2;
        info2.type = "battery";
        info2.temp = temp;
        event.info.push_back(info2);
        g_service->HandleThermalCallbackEvent(event);
        Wait();
    }
    g_service->UnSubscribeThermalTempCallback(cb1);
    g_thermalMgrClient.UnSubscribeThermalTempCallback(cb1);
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest006 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest007
 * @tc.desc: test register callback and get thermal level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest007, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest007 start");
    const sptr<IThermalLevelCallback> cb4 = new ThermalLevelTest4Callback();
    const sptr<IThermalLevelCallback> cb5 = new ThermalLevelTest5Callback();
    g_thermalMgrClient.SubscribeThermalLevelCallback(cb4);
    g_service->SubscribeThermalLevelCallback(cb4);
    g_thermalMgrClient.SubscribeThermalLevelCallback(cb5);
    g_service->SubscribeThermalLevelCallback(cb5);
    // thermal Level callback will be triggered when subscribed
    std::unique_lock<std::mutex> lock(g_mutex);
    g_callbackCV.wait_for(lock, std::chrono::seconds(TIME_OUT), [] {
        return (g_levelCallBack4.load() && g_levelCallBack5.load());
    });
    EXPECT_TRUE(g_levelCallBack4);
    EXPECT_TRUE(g_levelCallBack5);
    g_levelCallBack4 = false;
    g_levelCallBack5 = false;

    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = -20000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    EXPECT_FALSE(g_levelCallBack4);
    EXPECT_FALSE(g_levelCallBack5);

    g_thermalMgrClient.UnSubscribeThermalLevelCallback(cb5);
    g_service->UnSubscribeThermalLevelCallback(cb5);
    g_thermalMgrClient.UnSubscribeThermalLevelCallback(cb4);
    g_service->UnSubscribeThermalLevelCallback(cb4);
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest007 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest008
 * @tc.desc: test register callback and get thermal level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest008, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest008 start");
    const sptr<IThermalLevelCallback> cb2 = new ThermalLevelTest2Callback();
    g_thermalMgrClient.SubscribeThermalLevelCallback(cb2);
    g_service->SubscribeThermalLevelCallback(cb2);
    Wait(); // thermal Level callback will be triggered when subscribed

    int32_t temp = -25100;
    for (uint32_t i = 0; i < 5; i++) {
        THERMAL_HILOGD(LABEL_TEST, "temp: %{public}d", temp);
        HdfThermalCallbackInfo event;
        ThermalZoneInfo info1;
        info1.type = "battery";
        info1.temp = temp;
        event.info.push_back(info1);
        g_service->HandleThermalCallbackEvent(event);
        temp += 5000;
        EXPECT_FALSE(g_callbackTriggered);
    }
    temp = 40100;
    for (uint32_t i = 0; i < 3; i++) {
        HdfThermalCallbackInfo event;
        ThermalZoneInfo info1;
        info1.type = "battery";
        info1.temp = temp;
        event.info.push_back(info1);
        g_service->HandleThermalCallbackEvent(event);
        temp += 3000;
        Wait();
    }
    g_thermalMgrClient.UnSubscribeThermalLevelCallback(cb2);
    g_service->SubscribeThermalLevelCallback(cb2);
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest008 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest009
 * @tc.desc: test register callback and get thermal level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest009, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest009 start");
    const sptr<IThermalLevelCallback> cb1 = new ThermalLevelTest1Callback();
    g_thermalMgrClient.SubscribeThermalLevelCallback(cb1);
    g_service->SubscribeThermalLevelCallback(cb1);
    Wait(); // thermal Level callback will be triggered when subscribed

    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = -20000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    EXPECT_FALSE(g_callbackTriggered);
    event.info.clear();

    info1.temp = 40100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    Wait();
    event.info.clear();

    info1.temp = -10000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    Wait();
    event.info.clear();

    info1.temp = 46000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    Wait();

    g_thermalMgrClient.UnSubscribeThermalLevelCallback(cb1);
    g_service->UnSubscribeThermalLevelCallback(cb1);
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest009 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest010
 * @tc.desc: test register callback and get thermal level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest010, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest010 start");
    const sptr<IThermalLevelCallback> cb1 = new ThermalLevelTest1Callback();
    g_thermalMgrClient.SubscribeThermalLevelCallback(cb1);
    g_service->SubscribeThermalLevelCallback(cb1);
    Wait(); // thermal Level callback will be triggered when subscribed

    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = -20000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    EXPECT_FALSE(g_callbackTriggered);
    g_thermalMgrClient.UnSubscribeThermalLevelCallback(cb1);
    g_service->UnSubscribeThermalLevelCallback(cb1);

    g_service->HandleThermalCallbackEvent(event);
    g_thermalMgrClient.SubscribeThermalLevelCallback(cb1);
    g_service->SubscribeThermalLevelCallback(cb1);
    Wait(); // thermal Level callback will be triggered when subscribed

    event.info.clear();
    info1.temp = 48000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    Wait();

    g_thermalMgrClient.UnSubscribeThermalLevelCallback(cb1);
    g_service->UnSubscribeThermalLevelCallback(cb1);
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest020 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest011
 * @tc.desc: register callback and get temp list
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest011, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest011 start");
    InitData();
    const sptr<IThermalTempCallback> cb1 = new ThermalTempTest1Callback();
    g_service->SubscribeThermalTempCallback(typelist, cb1);
    g_thermalMgrClient.SubscribeThermalTempCallback(typelist, cb1);
    int32_t temp = 10000;
    for (int i = 0; i < 10; i++) {
        HdfThermalCallbackInfo event;
        ThermalZoneInfo info1;
        info1.type = "soc";
        info1.temp = temp;
        event.info.push_back(info1);

        ThermalZoneInfo info2;
        info2.type = "battery";
        info2.temp = temp;
        event.info.push_back(info2);
        g_service->HandleThermalCallbackEvent(event);
        temp += 100;
        Wait();
    }
    g_service->UnSubscribeThermalTempCallback(cb1);
    g_thermalMgrClient.UnSubscribeThermalTempCallback(cb1);
    const sptr<IThermalTempCallback> cb2 = new ThermalTempTest2Callback();
    g_service->SubscribeThermalTempCallback(typelist, cb1);
    g_thermalMgrClient.SubscribeThermalTempCallback(typelist, cb1);
    for (int i = 0; i < 10; i++) {
        HdfThermalCallbackInfo event;
        ThermalZoneInfo info1;
        info1.type = "soc";
        info1.temp = temp;
        event.info.push_back(info1);

        ThermalZoneInfo info2;
        info2.type = "battery";
        info2.temp = temp;
        event.info.push_back(info2);
        g_service->HandleThermalCallbackEvent(event);
        temp += 100;
        Wait();
    }
    g_service->UnSubscribeThermalTempCallback(cb1);
    g_thermalMgrClient.UnSubscribeThermalTempCallback(cb1);
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest011 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest012
 * @tc.desc: test get invaild temp
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest012, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest012 start");
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = INVAILD_TEMP;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    g_thermalMgrClient.GetThermalSensorTemp(SensorType::BATTERY);
    ThermalSrvSensorInfo info;
    g_service->GetThermalSrvSensorInfo(SensorType::BATTERY, info);
    EXPECT_EQ(INVAILD_TEMP, info.GetTemp()) << "ThermalMgrInterfaceTest012 Failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest012 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest013
 * @tc.desc: test get invaild temp
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest013, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest013 start");
    const sptr<IThermalTempCallback> cb = nullptr;
    InitData();
    g_thermalMgrClient.SubscribeThermalTempCallback(typelist, cb);
    g_service->SubscribeThermalTempCallback(typelist, cb);
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = INVAILD_TEMP;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    g_thermalMgrClient.UnSubscribeThermalTempCallback(cb);
    EXPECT_TRUE(g_service->UnSubscribeThermalTempCallback(cb));
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest013 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest014
 * @tc.desc: test register null callback
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest014, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest014 start");
    const sptr<IThermalLevelCallback> cb = nullptr;
    g_thermalMgrClient.SubscribeThermalLevelCallback(cb);
    g_service->SubscribeThermalLevelCallback(cb);
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = INVAILD_TEMP;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    g_thermalMgrClient.UnSubscribeThermalLevelCallback(cb);
    EXPECT_TRUE(g_service->UnSubscribeThermalLevelCallback(cb));
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest014 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest015
 * @tc.desc: test get level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest015, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest015 start");
    std::vector<int32_t> temps {-1000, 40100, 43100, 46100};
    std::vector<ThermalLevel> levels {ThermalLevel::COOL, ThermalLevel::NORMAL, ThermalLevel::WARM, ThermalLevel::HOT};
    for (uint32_t i = 0; i < temps.size(); ++i) {
        HdfThermalCallbackInfo event;
        ThermalZoneInfo info1;
        info1.type = "battery";
        info1.temp = temps[i];
        event.info.push_back(info1);
        g_service->HandleThermalCallbackEvent(event);
        g_thermalMgrClient.GetThermalLevel();
        ThermalLevel level;
        g_service->GetThermalLevel(level);
        GTEST_LOG_(INFO) << "test thermal temp: " << temps[i];
        EXPECT_EQ(level, levels[i]) << "ThermalMgrInterfaceTest015 Failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest015 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest016
 * @tc.desc: test get level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest016, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest016 start");
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "pa";
    info1.temp = 40100;
    event.info.push_back(info1);
    info1.type = "ambient";
    info1.temp = 20000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    g_thermalMgrClient.GetThermalLevel();
    ThermalLevel level;
    g_service->GetThermalLevel(level);
    EXPECT_EQ(level, ThermalLevel::OVERHEATED) << "ThermalMgrInterfaceTest016 Failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest016 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest017
 * @tc.desc: test get level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest017, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest017 start");
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "pa";
    info1.temp = 44100;
    event.info.push_back(info1);
    info1.type = "ambient";
    info1.temp = 20000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    g_thermalMgrClient.GetThermalLevel();
    ThermalLevel level;
    g_service->GetThermalLevel(level);
    EXPECT_EQ(level, ThermalLevel::WARNING) << "ThermalMgrInterfaceTest017 Failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest017 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest018
 * @tc.desc: Get Thermal Level
 * @tc.type: FUNC
 * @tc.result: level get min
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest018, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest018 start");
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "ap";
    info1.temp = 79000;
    event.info.push_back(info1);
    info1.type = "ambient";
    info1.temp = 60000;
    event.info.push_back(info1);
    info1.type = "shell";
    info1.temp = 30000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    g_thermalMgrClient.GetThermalLevel();
    ThermalLevel level;
    g_service->GetThermalLevel(level);
    EXPECT_EQ(level, ThermalLevel::EMERGENCY) << "ThermalMgrInterfaceTest018 Failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest018 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest019
 * @tc.desc: test get ESCAPE level
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest019, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest019 start");
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "pa";
    info1.temp = 46100;
    event.info.push_back(info1);
    info1.type = "ambient";
    info1.temp = 20000;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    g_thermalMgrClient.GetThermalLevel();
    ThermalLevel level;
    g_service->GetThermalLevel(level);
    EXPECT_EQ(level, ThermalLevel::ESCAPE) << "ThermalMgrInterfaceTest019 Failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest019 end");
}

/**
 * @tc.name: ThermalMgrInterfaceTest020
 * @tc.desc: Update Thermal State
 * @tc.type: FUNC
 * @tc.result: state changed
 */
HWTEST_F(ThermalMgrInterfaceTest, ThermalMgrInterfaceTest020, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest020 start");
    std::string tag1 = "modeid";
    std::string val1 = "100";
    bool ret = g_thermalMgrClient.UpdateThermalState(tag1, val1, false);
    EXPECT_TRUE(ret == true);
    g_service->UpdateThermalState(tag1, val1, false);
    std::map<std::string, std::string> stateMap {{tag1, val1}};
    bool result = g_service->GetPolicy()->StateMachineDecision(stateMap);
    EXPECT_TRUE(result == true);
    
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 40100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    int32_t value = ConvertInt(GetNodeValue(CONFIG_LEVEL_PATH));
    EXPECT_TRUE(value == 1);
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrInterfaceTest020 end");
}

} // namespace
