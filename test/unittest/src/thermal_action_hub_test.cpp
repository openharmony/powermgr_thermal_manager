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

#include "thermal_action_hub_test.h"

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
std::vector<std::string> g_typeList;
std::condition_variable g_callbackCV;
std::mutex g_mutex;
constexpr int64_t TIME_OUT = 1;
bool g_callbackTriggered = false;
const std::string SYSTEM_THERMAL_SERVICE_CONFIG_PATH = "/system/etc/thermal_config/thermal_service_config.xml";
sptr<ThermalService> g_service = nullptr;
auto& g_thermalMgrClient = ThermalMgrClient::GetInstance();

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
    g_callbackCV.wait_for(lock, std::chrono::seconds(TIME_OUT), [] {
        return g_callbackTriggered;
    });
    EXPECT_TRUE(g_callbackTriggered);
    g_callbackTriggered = false;
}
} // namespace

void ThermalActionHubTest::TearDown()
{
    g_callbackTriggered = false;
}

void ThermalActionHubTest::SetUpTestCase()
{
    g_service = DelayedSpSingleton<ThermalService>::GetInstance();
    g_service->OnStart();
    g_service->GetConfigParser().ThermalSrvConfigInit(SYSTEM_THERMAL_SERVICE_CONFIG_PATH);
    g_service->InitStateMachine();
    g_service->InitActionManager();
}

void ThermalActionHubTest::TearDownTestCase()
{
    g_service->OnStop();
}

void ThermalActionHubTest::InitData()
{
    g_typeList.push_back(BATTERY);
    g_typeList.push_back(SOC);
}

bool ThermalActionHubTest::ThermalActionTest1Callback::OnThermalActionChanged(ActionCallbackMap& actionCbMap)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest1Callback::OnThermalActionChanged Enter");
    int32_t cpuBigFreq = 1992000;
    bool isFind = false;
    for (auto iter : actionCbMap) {
        if (iter.first == "cpu_big") {
            EXPECT_EQ(std::stoi(iter.second), cpuBigFreq);
            isFind = true;
        }
        GTEST_LOG_(INFO) << "actionName: " << iter.first << " actionValue: " << iter.second;
    }
    EXPECT_TRUE(isFind);
    Notify();
    return true;
}

bool ThermalActionHubTest::ThermalActionTest2Callback::OnThermalActionChanged(ActionCallbackMap& actionCbMap)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest2Callback::OnThermalActionChanged Enter");
    std::string lcd = "0.9";
    bool isFind = false;
    for (auto iter : actionCbMap) {
        if (iter.first == "lcd") {
            // 0： begin position; 3: end position
            EXPECT_EQ(iter.second.substr(0, 3), lcd);
            isFind = true;
        }
        GTEST_LOG_(INFO) << "actionName: " << iter.first << " actionValue: " << iter.second;
    }
    EXPECT_TRUE(isFind);
    Notify();
    return true;
}

bool ThermalActionHubTest::ThermalActionTest3Callback::OnThermalActionChanged(ActionCallbackMap& actionCbMap)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest3Callback::OnThermalActionChanged Enter");
    std::string cpuMedFreq = "1989500";
    std::string lcd = "0.8";
    bool isFindCpuMed = false;
    bool isFindLcd = false;
    for (auto iter : actionCbMap) {
        if (iter.first == "cpu_med") {
            EXPECT_EQ(iter.second, cpuMedFreq);
            isFindCpuMed = true;
        }
        if (iter.first == "lcd") {
            // 0： begin position; 3: end position
            EXPECT_EQ(iter.second.substr(0, 3), lcd);
            isFindLcd = true;
        }
        GTEST_LOG_(INFO) << "actionName: " << iter.first << " actionValue: " << iter.second;
    }
    EXPECT_TRUE(isFindCpuMed);
    EXPECT_TRUE(isFindLcd);
    Notify();
    return true;
}

bool ThermalActionHubTest::ThermalActionTest4Callback::OnThermalActionChanged(ActionCallbackMap& actionCbMap)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest4Callback::OnThermalActionChanged Enter");
    std::string lcd = "0.99";
    bool isFind = false;
    for (auto iter : actionCbMap) {
        if (iter.first == "lcd") {
            // 0： begin position; 4: end position
            EXPECT_EQ(iter.second.substr(0, 4), lcd);
            isFind = true;
        }
        GTEST_LOG_(INFO) << "actionName: " << iter.first << " actionValue: " << iter.second;
    }
    EXPECT_TRUE(isFind);
    Notify();
    return true;
}

bool ThermalActionHubTest::ThermalActionTest5Callback::OnThermalActionChanged(ActionCallbackMap& actionCbMap)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest5Callback::OnThermalActionChanged Enter");
    std::string lcd = "0.88";
    bool isFind = false;
    for (auto iter : actionCbMap) {
        if (iter.first == "lcd") {
            // 0： begin position; 4: end position
            EXPECT_EQ(iter.second.substr(0, 4), lcd);
            isFind = true;
        }
        GTEST_LOG_(INFO) << "actionName: " << iter.first << " actionValue: " << iter.second;
    }
    EXPECT_TRUE(isFind);
    Notify();
    return true;
}

bool ThermalActionHubTest::ThermalActionTest6Callback::OnThermalActionChanged(ActionCallbackMap& actionCbMap)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest6Callback::OnThermalActionChanged Enter");
    std::string lcd = "0.77";
    bool isFind = false;
    for (auto iter : actionCbMap) {
        if (iter.first == "lcd") {
            // 0： begin position; 4: end position
            EXPECT_EQ(iter.second.substr(0, 4), lcd);
            isFind = true;
        }
        GTEST_LOG_(INFO) << "actionName: " << iter.first << " actionValue: " << iter.second;
    }
    EXPECT_TRUE(isFind);
    Notify();
    return true;
}

bool ThermalActionHubTest::ThermalActionTest7Callback::OnThermalActionChanged(ActionCallbackMap& actionCbMap)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest7Callback::OnThermalActionChanged Enter");
    bool isFind = false;
    for (auto iter : actionCbMap) {
        if (iter.first == "boost") {
            EXPECT_TRUE(static_cast<bool>(std::stoi(iter.second)));
            isFind = true;
        }
        GTEST_LOG_(INFO) << "actionName: " << iter.first << " actionValue: " << iter.second;
    }
    EXPECT_TRUE(isFind);
    Notify();
    return true;
}

bool ThermalActionHubTest::ThermalActionTest8Callback::OnThermalActionChanged(ActionCallbackMap& actionCbMap)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest8Callback::OnThermalActionChanged Enter");
    bool isFind = false;
    for (auto iter : actionCbMap) {
        if (iter.first == "isolate") {
            EXPECT_TRUE(static_cast<bool>(std::stoi(iter.second)));
            isFind = true;
        }
        GTEST_LOG_(INFO) << "actionName: " << iter.first << " actionValue: " << iter.second;
    }
    EXPECT_TRUE(isFind);
    Notify();
    return true;
}

namespace {
/**
 * @tc.name: ThermalActionHubTest001
 * @tc.desc: register action is cpu_big test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalActionHubTest, ThermalActionHubTest001, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest001 start");
    std::vector<std::string> actionList;
    actionList.push_back("cpu_big");
    std::string desc = "";
    InitData();
    const sptr<IThermalActionCallback> cb1 = new ThermalActionTest1Callback();
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest001 start register");
    g_thermalMgrClient.SubscribeThermalActionCallback(actionList, desc, cb1);
    g_service->SubscribeThermalActionCallback(actionList, desc, cb1);
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 40100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    Wait();
    g_thermalMgrClient.UnSubscribeThermalActionCallback(cb1);
    g_service->UnSubscribeThermalActionCallback(cb1);
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest001 end");
}

/**
 * @tc.name: ThermalActionHubTest002
 * @tc.desc: register action is lcd test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalActionHubTest, ThermalActionHubTest002, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest002 start");
    std::vector<std::string> actionList;
    actionList.push_back("lcd");
    std::string desc = "";
    InitData();
    const sptr<IThermalActionCallback> cb2 = new ThermalActionTest2Callback();
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest002 start register");
    g_thermalMgrClient.SubscribeThermalActionCallback(actionList, desc, cb2);
    g_service->SubscribeThermalActionCallback(actionList, desc, cb2);
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 43100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    Wait();
    g_thermalMgrClient.UnSubscribeThermalActionCallback(cb2);
    g_service->UnSubscribeThermalActionCallback(cb2);
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest002 end");
}

/**
 * @tc.name: ThermalActionHubTest003
 * @tc.desc: register action is cpu_med and lcd test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalActionHubTest, ThermalActionHubTest003, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest003 start");
    std::vector<std::string> actionList;
    actionList.push_back("cpu_med");
    actionList.push_back("lcd");
    std::string desc = "";
    InitData();
    const sptr<IThermalActionCallback> cb3 = new ThermalActionTest3Callback();
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest003 start register");
    g_thermalMgrClient.SubscribeThermalActionCallback(actionList, desc, cb3);
    g_service->SubscribeThermalActionCallback(actionList, desc, cb3);
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 46100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    Wait();
    g_thermalMgrClient.UnSubscribeThermalActionCallback(cb3);
    g_service->UnSubscribeThermalActionCallback(cb3);
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest003 end");
}

/**
 * @tc.name: ThermalActionHubTest004
 * @tc.desc: register action is lcd test, scene cam, level 1
 * @tc.type: FUNC
 */
HWTEST_F(ThermalActionHubTest, ThermalActionHubTest004, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest004 start");
    std::vector<std::string> actionList;
    actionList.push_back("lcd");
    std::string desc = "";
    InitData();
    const sptr<IThermalActionCallback> cb4 = new ThermalActionTest4Callback();
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest004 start register");
    g_thermalMgrClient.SubscribeThermalActionCallback(actionList, desc, cb4);
    g_service->SubscribeThermalActionCallback(actionList, desc, cb4);
    g_thermalMgrClient.SetScene("cam");
    g_service->SetScene("cam");
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 40100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    Wait();
    g_thermalMgrClient.UnSubscribeThermalActionCallback(cb4);
    g_service->UnSubscribeThermalActionCallback(cb4);
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest004 end");
}

/**
 * @tc.name: ThermalActionHubTest005
 * @tc.desc: register action is lcd test, scene call, level 2
 * @tc.type: FUNC
 */
HWTEST_F(ThermalActionHubTest, ThermalActionHubTest005, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest005 start");
    std::vector<std::string> actionList;
    actionList.push_back("lcd");
    std::string desc = "";
    InitData();
    const sptr<IThermalActionCallback> cb5 = new ThermalActionTest5Callback();
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest005 start register");
    g_thermalMgrClient.SubscribeThermalActionCallback(actionList, desc, cb5);
    g_service->SubscribeThermalActionCallback(actionList, desc, cb5);
    g_thermalMgrClient.SetScene("call");
    g_service->SetScene("call");
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 43100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    Wait();
    g_thermalMgrClient.UnSubscribeThermalActionCallback(cb5);
    g_service->UnSubscribeThermalActionCallback(cb5);
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest005 end");
}

/**
 * @tc.name: ThermalActionHubTest006
 * @tc.desc: register action is lcd test, scene game, level 3
 * @tc.type: FUNC
 */
HWTEST_F(ThermalActionHubTest, ThermalActionHubTest006, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest006 start");
    std::vector<std::string> actionList;
    actionList.push_back("lcd");
    std::string desc = "";
    InitData();
    const sptr<IThermalActionCallback> cb6 = new ThermalActionTest6Callback();
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest006 start register");
    g_thermalMgrClient.SubscribeThermalActionCallback(actionList, desc, cb6);
    g_service->SubscribeThermalActionCallback(actionList, desc, cb6);
    g_thermalMgrClient.SetScene("game");
    g_service->SetScene("game");
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 46100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    Wait();
    g_thermalMgrClient.UnSubscribeThermalActionCallback(cb6);
    g_service->UnSubscribeThermalActionCallback(cb6);
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest006 end");
}

/**
 * @tc.name: ThermalActionHubTest007
 * @tc.desc: register action is boost test
 * @tc.type: FUNC
 * @tc.require: issueI6JSQD
 */
HWTEST_F(ThermalActionHubTest, ThermalActionHubTest007, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest007 start");
    std::vector<std::string> actionList;
    actionList.push_back("boost");
    std::string desc = "";
    InitData();
    const sptr<IThermalActionCallback> cbBoost = new ThermalActionTest7Callback();
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest001 start register");
    g_thermalMgrClient.SubscribeThermalActionCallback(actionList, desc, cbBoost);
    g_service->SubscribeThermalActionCallback(actionList, desc, cbBoost);
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 40100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    Wait();
    g_thermalMgrClient.UnSubscribeThermalActionCallback(cbBoost);
    g_service->UnSubscribeThermalActionCallback(cbBoost);
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest001 end");
}

/**
 * @tc.name: ThermalActionHubTest008
 * @tc.desc: register action is isolate cpu test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalActionHubTest, ThermalActionHubTest008, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest008 start");
    std::vector<std::string> actionList;
    actionList.push_back("isolate");
    std::string desc = "";
    InitData();
    const sptr<IThermalActionCallback> cbIsolateCpu = new ThermalActionTest8Callback();
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest008 start register");
    g_thermalMgrClient.SubscribeThermalActionCallback(actionList, desc, cbIsolateCpu);
    g_service->SubscribeThermalActionCallback(actionList, desc, cbIsolateCpu);
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 43100;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    Wait();
    g_thermalMgrClient.UnSubscribeThermalActionCallback(cbIsolateCpu);
    g_service->UnSubscribeThermalActionCallback(cbIsolateCpu);
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest008 end");
}
} // namespace
