/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "thermal_level_event_system_test.h"

#include <common_event_data.h>
#include <common_event_manager.h>
#include <common_event_publish_info.h>
#include <common_event_subscriber.h>
#include <common_event_support.h>
#include <condition_variable>
#include <mutex>
#ifdef BATTERY_MANAGER_ENABLE
#include "battery_info.h"
#endif
#include "thermal_level_info.h"
#include "thermal_log.h"
#include "thermal_mgr_client.h"
#include "thermal_mgr_listener.h"

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
using namespace OHOS::AAFwk;
using namespace OHOS::EventFwk;
using namespace OHOS::HDI::Thermal::V1_1;

namespace {
std::condition_variable g_callbackCV;
std::mutex g_mutex;
constexpr int64_t TIME_OUT = 1;
bool g_callbackTriggered = false;
const std::string SYSTEM_THERMAL_SERVICE_CONFIG_PATH = "/system/etc/thermal_config/thermal_service_config.xml";
sptr<ThermalService> g_service = nullptr;
} // namespace

static void Notify()
{
    std::unique_lock<std::mutex> lock(g_mutex);
    g_callbackTriggered = true;
    lock.unlock();
    g_callbackCV.notify_one();
}

static void Wait()
{
    std::unique_lock<std::mutex> lock(g_mutex);
    g_callbackCV.wait_for(lock, std::chrono::seconds(TIME_OUT), [] { return g_callbackTriggered; });
    EXPECT_TRUE(g_callbackTriggered);
    g_callbackTriggered = false;
}

static bool PublishChangedEvent(int32_t capacity, int32_t chargerCurrent)
{
#ifdef BATTERY_MANAGER_ENABLE
    Want want;
    want.SetParam(BatteryInfo::COMMON_EVENT_KEY_CAPACITY, capacity);
    want.SetParam(
        BatteryInfo::COMMON_EVENT_KEY_CHARGE_STATE, static_cast<int32_t>(BatteryChargeState::CHARGE_STATE_ENABLE));
    want.SetAction(CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
    CommonEventData data;
    data.SetWant(want);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(false);
    bool isSuccess = CommonEventManager::PublishCommonEvent(data, publishInfo);

    Want wantInner;
    wantInner.SetParam(BatteryInfo::COMMON_EVENT_KEY_PLUGGED_NOW_CURRENT, chargerCurrent);
    wantInner.SetAction(BatteryInfo::COMMON_EVENT_BATTERY_CHANGED_INNER);
    data.SetWant(wantInner);
    bool isSuccessInner = CommonEventManager::PublishCommonEvent(data, publishInfo);

    return isSuccess && isSuccessInner;
#endif
    return true;
}

class CommonEventThermalLevel1Test : public CommonEventSubscriber {
public:
    CommonEventThermalLevel1Test() = default;
    explicit CommonEventThermalLevel1Test(const CommonEventSubscribeInfo& subscriberInfo);
    virtual ~CommonEventThermalLevel1Test() = default;
    virtual void OnReceiveEvent(const CommonEventData& data);
    static shared_ptr<CommonEventThermalLevel1Test> RegisterEvent();
};

CommonEventThermalLevel1Test::CommonEventThermalLevel1Test(const CommonEventSubscribeInfo& subscriberInfo)
    : CommonEventSubscriber(subscriberInfo)
{
}

class CommonEventThermalLevel2Test : public CommonEventSubscriber {
public:
    CommonEventThermalLevel2Test() = default;
    explicit CommonEventThermalLevel2Test(const CommonEventSubscribeInfo& subscriberInfo);
    virtual ~CommonEventThermalLevel2Test() = default;
    virtual void OnReceiveEvent(const CommonEventData& data);
    static shared_ptr<CommonEventThermalLevel2Test> RegisterEvent();
};

CommonEventThermalLevel2Test::CommonEventThermalLevel2Test(const CommonEventSubscribeInfo& subscriberInfo)
    : CommonEventSubscriber(subscriberInfo)
{
}

class CommonEventThermalLevel3Test : public CommonEventSubscriber {
public:
    CommonEventThermalLevel3Test() = default;
    explicit CommonEventThermalLevel3Test(const CommonEventSubscribeInfo& subscriberInfo);
    virtual ~CommonEventThermalLevel3Test() = default;
    virtual void OnReceiveEvent(const CommonEventData& data);
    static shared_ptr<CommonEventThermalLevel3Test> RegisterEvent();
};

CommonEventThermalLevel3Test::CommonEventThermalLevel3Test(const CommonEventSubscribeInfo& subscriberInfo)
    : CommonEventSubscriber(subscriberInfo)
{
}

class CommonEventThermalIdleTrueTest : public CommonEventSubscriber {
public:
    CommonEventThermalIdleTrueTest() = default;
    explicit CommonEventThermalIdleTrueTest(const CommonEventSubscribeInfo& subscriberInfo);
    virtual ~CommonEventThermalIdleTrueTest() = default;
    virtual void OnReceiveEvent(const CommonEventData& data);
    static shared_ptr<CommonEventThermalIdleTrueTest> RegisterEvent();
};

CommonEventThermalIdleTrueTest::CommonEventThermalIdleTrueTest(const CommonEventSubscribeInfo& subscriberInfo)
    : CommonEventSubscriber(subscriberInfo)
{
}

class CommonEventThermalIdleFalseTest : public CommonEventSubscriber {
public:
    CommonEventThermalIdleFalseTest() = default;
    explicit CommonEventThermalIdleFalseTest(const CommonEventSubscribeInfo& subscriberInfo);
    virtual ~CommonEventThermalIdleFalseTest() = default;
    virtual void OnReceiveEvent(const CommonEventData& data);
    static shared_ptr<CommonEventThermalIdleFalseTest> RegisterEvent();
};

CommonEventThermalIdleFalseTest::CommonEventThermalIdleFalseTest(const CommonEventSubscribeInfo& subscriberInfo)
    : CommonEventSubscriber(subscriberInfo)
{
}

void CommonEventThermalLevel1Test::OnReceiveEvent(const CommonEventData& data)
{
    THERMAL_HILOGD(LABEL_TEST, "CommonEventThermalLevel1Test: OnReceiveEvent Enter");
    int32_t invalidLevel = -1;
    std::string key = ToString(static_cast<int32_t>(ThermalCommonEventCode::CODE_THERMAL_LEVEL_CHANGED));
    int32_t level = data.GetWant().GetIntParam(key, invalidLevel);
    GTEST_LOG_(INFO) << "thermal level: " << level;
    Notify();
    EXPECT_EQ(level, static_cast<int32_t>(ThermalLevel::NORMAL)) << "get thermal level failed";
}

void CommonEventThermalLevel2Test::OnReceiveEvent(const CommonEventData& data)
{
    THERMAL_HILOGD(LABEL_TEST, "CommonEventThermalLevel2Test: OnReceiveEvent Enter");
    int32_t invalidLevel = -1;
    std::string key = ToString(static_cast<int32_t>(ThermalCommonEventCode::CODE_THERMAL_LEVEL_CHANGED));
    int32_t level = data.GetWant().GetIntParam(key, invalidLevel);
    GTEST_LOG_(INFO) << "thermal level: " << level;
    Notify();
    EXPECT_EQ(level, static_cast<int32_t>(ThermalLevel::WARM)) << "get thermal level failed";
}

void CommonEventThermalLevel3Test::OnReceiveEvent(const CommonEventData& data)
{
    THERMAL_HILOGD(LABEL_TEST, "CommonEventThermalLevel3Test: OnReceiveEvent Enter");
    int32_t invalidLevel = -1;
    std::string key = ToString(static_cast<int32_t>(ThermalCommonEventCode::CODE_THERMAL_LEVEL_CHANGED));
    int32_t level = data.GetWant().GetIntParam(key, invalidLevel);
    GTEST_LOG_(INFO) << "thermal level: " << level;
    Notify();
    EXPECT_EQ(level, static_cast<int32_t>(ThermalLevel::HOT)) << "get thermal level failed";
}

void CommonEventThermalIdleTrueTest::OnReceiveEvent(const CommonEventData& data)
{
    THERMAL_HILOGD(LABEL_TEST, "CommonEventThermalIdleTrueTest: OnReceiveEvent Enter");
    bool invalidState = false;
    std::string key = ToString(static_cast<uint32_t>(ChargeIdleEventCode::EVENT_CODE_CHARGE_IDLE_STATE));
    bool getState = data.GetWant().GetBoolParam(key, invalidState);
    GTEST_LOG_(INFO) << "charger state: " << getState;
    Notify();
    EXPECT_EQ(getState, true) << "get charger state failed";
}

void CommonEventThermalIdleFalseTest::OnReceiveEvent(const CommonEventData& data)
{
    THERMAL_HILOGD(LABEL_TEST, "CommonEventThermalIdleFalseTest: OnReceiveEvent Enter");
    bool invalidState = true;
    std::string key = ToString(static_cast<uint32_t>(ChargeIdleEventCode::EVENT_CODE_CHARGE_IDLE_STATE));
    bool getState = data.GetWant().GetBoolParam(key, invalidState);
    GTEST_LOG_(INFO) << "charger state: " << getState;
    Notify();
    EXPECT_EQ(getState, false) << "get charger state failed";
}

shared_ptr<CommonEventThermalLevel1Test> CommonEventThermalLevel1Test::RegisterEvent()
{
    THERMAL_HILOGD(LABEL_TEST, "RegisterEvent: Regist Subscriber Start");
    int32_t retryTimes = 2;
    bool succeed = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_THERMAL_LEVEL_CHANGED);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventThermalLevel1Test>(subscribeInfo);
    for (int32_t tryTimes = 0; tryTimes < retryTimes; tryTimes++) {
        succeed = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    }
    if (!succeed) {
        THERMAL_HILOGD(COMP_SVC, "Failed to register subscriber");
        return nullptr;
    }
    return subscriberPtr;
}

shared_ptr<CommonEventThermalLevel2Test> CommonEventThermalLevel2Test::RegisterEvent()
{
    THERMAL_HILOGD(LABEL_TEST, "RegisterEvent: Regist Subscriber Start");
    int32_t retryTimes = 2;
    bool succeed = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_THERMAL_LEVEL_CHANGED);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventThermalLevel2Test>(subscribeInfo);
    for (int32_t tryTimes = 0; tryTimes < retryTimes; tryTimes++) {
        succeed = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    }
    if (!succeed) {
        THERMAL_HILOGD(COMP_SVC, "Failed to register subscriber");
        return nullptr;
    }
    return subscriberPtr;
}

shared_ptr<CommonEventThermalLevel3Test> CommonEventThermalLevel3Test::RegisterEvent()
{
    THERMAL_HILOGD(LABEL_TEST, "RegisterEvent: Regist Subscriber Start");
    int32_t retryTimes = 2;
    bool succeed = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_THERMAL_LEVEL_CHANGED);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventThermalLevel3Test>(subscribeInfo);
    for (int32_t tryTimes = 0; tryTimes < retryTimes; tryTimes++) {
        succeed = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    }
    if (!succeed) {
        THERMAL_HILOGD(COMP_SVC, "Failed to register subscriber");
        return nullptr;
    }
    return subscriberPtr;
}

shared_ptr<CommonEventThermalIdleTrueTest> CommonEventThermalIdleTrueTest::RegisterEvent()
{
    THERMAL_HILOGD(LABEL_TEST, "RegisterEvent: Regist Subscriber Start");
    int32_t retryTimes = 2;
    bool succeed = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_CHARGE_IDLE_MODE_CHANGED);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventThermalIdleTrueTest>(subscribeInfo);
    for (int32_t tryTimes = 0; tryTimes < retryTimes; tryTimes++) {
        succeed = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    }
    if (!succeed) {
        THERMAL_HILOGD(COMP_SVC, "Failed to register subscriber");
        return nullptr;
    }
    return subscriberPtr;
}

shared_ptr<CommonEventThermalIdleFalseTest> CommonEventThermalIdleFalseTest::RegisterEvent()
{
    THERMAL_HILOGD(LABEL_TEST, "RegisterEvent: Regist Subscriber Start");
    int32_t retryTimes = 2;
    bool succeed = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_CHARGE_IDLE_MODE_CHANGED);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventThermalIdleFalseTest>(subscribeInfo);
    for (int32_t tryTimes = 0; tryTimes < retryTimes; tryTimes++) {
        succeed = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    }
    if (!succeed) {
        THERMAL_HILOGD(COMP_SVC, "Failed to register subscriber");
        return nullptr;
    }
    return subscriberPtr;
}

void ThermalLevelEventSystemTest::SetUpTestCase()
{
    g_service = ThermalService::GetInstance();
    g_service->InitSystemTestModules();
    g_service->OnStart();
    g_service->InitStateMachine();
}

void ThermalLevelEventSystemTest::TearDownTestCase()
{
    g_service->OnStop();
}

void ThermalLevelEventSystemTest::TearDown()
{
    g_service->SetScene("");
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 0;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    g_callbackTriggered = false;
}


namespace {
/*
 * @tc.number: ThermalLevelEventSystemTest001
 * @tc.name: ThermalLevelEventSystemTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F (ThermalLevelEventSystemTest, ThermalLevelEventSystemTest001, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventSystemTest001: Start");
    shared_ptr<CommonEventThermalLevel1Test> subscriber = CommonEventThermalLevel1Test::RegisterEvent();
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 40600;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    Wait();
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventSystemTest001: End");
}

/*
 * @tc.number: ThermalLevelEventSystemTest002
 * @tc.name: ThermalLevelEventSystemTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F (ThermalLevelEventSystemTest, ThermalLevelEventSystemTest002, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventSystemTest002: Start");
    shared_ptr<CommonEventThermalLevel2Test> subscriber = CommonEventThermalLevel2Test::RegisterEvent();
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 43600;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    Wait();
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventSystemTest002: End");
}

/*
 * @tc.number: ThermalLevelEventSystemTest003
 * @tc.name: ThermalLevelEventSystemTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F (ThermalLevelEventSystemTest, ThermalLevelEventSystemTest003, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventSystemTest003: Start");
    shared_ptr<CommonEventThermalLevel3Test> subscriber = CommonEventThermalLevel3Test::RegisterEvent();
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 46600;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
    Wait();
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventSystemTest003: End");
}

/*
 * @tc.number: ThermalLevelEventSystemTest004
 * @tc.name: ThermalLevelEventSystemTest
 * @tc.desc: Verify the receive the idle common event
 */
HWTEST_F (ThermalLevelEventSystemTest, ThermalLevelEventSystemTest004, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventSystemTest004: Start");
    int32_t batteryCapacity = 90;
    int32_t chargerCurrent = 1100;
    int32_t invalid = 0;
    system("hidumper -s 3302 -a -u");
    EXPECT_TRUE(PublishChangedEvent(batteryCapacity, invalid));
    sleep(TIME_OUT);
    shared_ptr<CommonEventThermalIdleTrueTest> subscriber = CommonEventThermalIdleTrueTest::RegisterEvent();
    EXPECT_TRUE(PublishChangedEvent(batteryCapacity, chargerCurrent));
    Wait();
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
    system("hidumper -s 3302 -a -r");
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventSystemTest004: End");
}

/*
 * @tc.number: ThermalLevelEventSystemTest005
 * @tc.name: ThermalLevelEventSystemTest
 * @tc.desc: Verify the receive the idle common event
 */
HWTEST_F (ThermalLevelEventSystemTest, ThermalLevelEventSystemTest005, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventSystemTest005: Start");
    int32_t lastBatteryCapacity = 90;
    int32_t lastChargerCurrent = 1100;
    int32_t batteryCapacity = 70;
    int32_t chargerCurrent = 900;
    system("hidumper -s 3302 -a -u");
    EXPECT_TRUE(PublishChangedEvent(lastBatteryCapacity, lastChargerCurrent));
    sleep(TIME_OUT);
    shared_ptr<CommonEventThermalIdleFalseTest> subscriber = CommonEventThermalIdleFalseTest::RegisterEvent();
    EXPECT_TRUE(PublishChangedEvent(batteryCapacity, chargerCurrent));
    Wait();
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
    system("hidumper -s 3302 -a -r");
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventSystemTest005: End");
}
} // namespace
