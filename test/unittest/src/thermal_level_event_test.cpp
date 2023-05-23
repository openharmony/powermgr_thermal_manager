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

#include "thermal_level_event_test.h"

#include <common_event_data.h>
#include <common_event_manager.h>
#include <common_event_publish_info.h>
#include <common_event_subscriber.h>
#include <common_event_support.h>
#include <condition_variable>
#include <mutex>

#include "mock_thermal_mgr_client.h"
#include "thermal_level_info.h"
#include "thermal_log.h"
#include "thermal_mgr_client.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
using namespace OHOS::AAFwk;
using namespace OHOS::EventFwk;

namespace {
std::condition_variable g_callbackCV;
std::mutex g_mutex;
constexpr int64_t TIME_OUT = 1;
bool g_callbackTriggered = false;
int32_t g_thermalLevel = -1;

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
} // namespace

class CommonEventThermalLevelTest : public CommonEventSubscriber {
public:
    CommonEventThermalLevelTest() = default;
    explicit CommonEventThermalLevelTest(const CommonEventSubscribeInfo &subscriberInfo);
    virtual ~CommonEventThermalLevelTest() {};
    virtual void OnReceiveEvent(const CommonEventData &data);
    static shared_ptr<CommonEventThermalLevelTest> RegisterEvent();
};

CommonEventThermalLevelTest::CommonEventThermalLevelTest
    (const CommonEventSubscribeInfo &subscriberInfo) : CommonEventSubscriber(subscriberInfo) {}


void CommonEventThermalLevelTest::OnReceiveEvent(const CommonEventData &data)
{
    THERMAL_HILOGD(LABEL_TEST, "CommonEventThermalLevelTest: OnReceiveEvent Enter");
    int invalidLevel = -1;
    std::string action = data.GetWant().GetAction();
    EXPECT_TRUE(action == CommonEventSupport::COMMON_EVENT_THERMAL_LEVEL_CHANGED);
    if (action == CommonEventSupport::COMMON_EVENT_THERMAL_LEVEL_CHANGED) {
        std::string key = ToString(static_cast<int32_t>(ThermalCommonEventCode::CODE_THERMAL_LEVEL_CHANGED));
        int32_t level = data.GetWant().GetIntParam(key, invalidLevel);
        GTEST_LOG_(INFO) << "thermal level: " << level;
        EXPECT_TRUE(g_thermalLevel == level);
    }
    Notify();
}

shared_ptr<CommonEventThermalLevelTest> CommonEventThermalLevelTest::RegisterEvent()
{
    THERMAL_HILOGD(LABEL_TEST, "RegisterEvent: Regist Subscriber Start");
    static const int32_t MAX_RETRY_TIMES = 2;
    auto succeed = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_THERMAL_LEVEL_CHANGED);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventThermalLevelTest>(subscribeInfo);
    for (int32_t tryTimes = 0; tryTimes < MAX_RETRY_TIMES; tryTimes++) {
        succeed = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    }
    if (!succeed) {
        THERMAL_HILOGD(COMP_SVC, "Failed to register subscriber");
        return nullptr;
    }
    return subscriberPtr;
}

void ThermalLevelEventTest::TearDown()
{
    InitNode();
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("");
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    g_thermalLevel = -1;
    g_callbackTriggered = false;
}

namespace {
/*
 * @tc.number: ThermalLevelEventTest001
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest001, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventTest001: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();
    int32_t batteryTemp = 41000;
    g_thermalLevel = 1;
    SetNodeValue(batteryTemp, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    Wait();
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventTest001: end");
}

/*
 * @tc.number: ThermalLevelEventTest002
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest002, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventTest002: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();
    int32_t batteryTemp = 43100;
    g_thermalLevel = 2;
    SetNodeValue(batteryTemp, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    Wait();
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventTest002: end");
}

/*
 * @tc.number: ThermalLevelEventTest003
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest003, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventTest003: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();
    int32_t batteryTemp = 46100;
    g_thermalLevel = 3;
    SetNodeValue(batteryTemp, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    Wait();
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventTest003: end");
}

/*
 * @tc.number: ThermalLevelEventTest004
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest004, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventTest004: start");
    if (!IsMock(PA_PATH) || !IsMock(AMBIENT_PATH) || IsVendor()) {
        return;
    }
    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();
    int32_t paTemp = 41000;
    int32_t amTemp = 10000;
    g_thermalLevel = 4;
    SetNodeValue(paTemp, PA_PATH);
    SetNodeValue(amTemp, AMBIENT_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    Wait();
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventTest004: end");
}

/*
 * @tc.number: ThermalLevelEventTest005
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest005, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventTest005: start");
    if (!IsMock(PA_PATH) || !IsMock(AMBIENT_PATH) || IsVendor()) {
        return;
    }
    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();
    int32_t paTemp = 44000;
    int32_t amTemp = 10000;
    g_thermalLevel = 5;
    SetNodeValue(paTemp, PA_PATH);
    SetNodeValue(amTemp, AMBIENT_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    Wait();
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventTest005: end");
}

/*
 * @tc.number: ThermalLevelEventTest006
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest006, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventTest006: start");
    if (!IsMock(AP_PATH) || !IsMock(AMBIENT_PATH) || !IsMock(SHELL_PATH) || IsVendor()) {
        return;
    }
    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();
    int32_t apTemp = 78000;
    int32_t amTemp = 1000;
    int32_t shellTemp = 30000;
    g_thermalLevel = 6;
    SetNodeValue(apTemp, AP_PATH);
    SetNodeValue(amTemp, AMBIENT_PATH);
    SetNodeValue(shellTemp, SHELL_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    Wait();
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventTest006: end");
}

/*
 * @tc.number: ThermalLevelEventTest007
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest007, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventTest007: start");
    if (!IsMock(BATTERY_PATH) || !IsMock(SOC_PATH) || IsVendor()) {
        return;
    }
    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();
    int32_t batteryTemp = 46100;
    g_thermalLevel = 3;
    SetNodeValue(batteryTemp, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();

    int32_t socTemp = -10000;
    g_thermalLevel = 0;
    SetNodeValue(socTemp, SOC_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    Wait();
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventTest007: end");
}

/*
 * @tc.number: ThermalLevelEventTest008
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest008, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventTest008: start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    bool result = false;
    int32_t batteryTemp = 40100;
    g_thermalLevel = 1;
    SetNodeValue(batteryTemp, BATTERY_PATH);
    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();
    EXPECT_TRUE(MockThermalMgrClient::GetInstance().GetThermalInfo());
    Wait();

    CommonEventData stickyData;
    CommonEventManager::GetStickyCommonEvent(CommonEventSupport::COMMON_EVENT_THERMAL_LEVEL_CHANGED, stickyData);
    if (stickyData.GetWant().GetAction() != CommonEventSupport::COMMON_EVENT_THERMAL_LEVEL_CHANGED) {
        result = false;
    } else {
        THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventTest008: sticky event successfully");
        result = true;
    }
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventTest008: end");
}
}
