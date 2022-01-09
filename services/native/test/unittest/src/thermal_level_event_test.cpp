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

#include "thermal_level_event_test.h"

#include <common_event_data.h>
#include <common_event_manager.h>
#include <common_event_publish_info.h>
#include <common_event_subscriber.h>
#include <common_event_support.h>
#include <datetime_ex.h>
#include <gtest/gtest.h>
#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <string_ex.h>
#include "thermal_common.h"
#include "thermal_level_action.h"
#include "thermal_service.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
using namespace OHOS::AAFwk;
using namespace OHOS::EventFwk;

static sptr<ThermalService> service;

class CommonEventThermalLevelTest : public CommonEventSubscriber {
public:
    CommonEventThermalLevelTest() = default;
    explicit CommonEventThermalLevelTest(const CommonEventSubscribeInfo &subscriberInfo);
    virtual ~CommonEventThermalLevelTest() {};
    virtual void OnReceiveEvent(const CommonEventData &data);
    static shared_ptr<CommonEventThermalLevelTest> RegisterEvent() const;
};

CommonEventThermalLevelTest::CommonEventThermalLevelTest
    (const CommonEventSubscribeInfo &subscriberInfo) : CommonEventSubscriber(subscriberInfo) {}


void CommonEventThermalLevelTest::OnReceiveEvent(const CommonEventData &data)
{
    GTEST_LOG_(INFO) << " CommonEventThermalLevelTest: OnReceiveEvent Enter \n";
    std::string action = data.GetWant().GetAction();
    if (action == CommonEventSupport::COMMON_EVENT_THERMAL_LEVEL_CHANGED) {
        GTEST_LOG_(INFO) << "OnReceiveEvent: data=" << data.GetData();
        GTEST_LOG_(INFO) << "OnReceiveEvent: code=" << data.GetCode();
    } else {
        GTEST_LOG_(INFO) << "failed to receive level changed event";
    }
}

shared_ptr<CommonEventThermalLevelTest> CommonEventThermalLevelTest::RegisterEvent() const
{
    GTEST_LOG_(INFO) << "RegisterEvent: Regist Subscriber Start!";
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
        THERMAL_HILOGD(MODULE_THERMALMGR_SERVICE, "Failed to register subscriber");
        return nullptr;
    }
    return subscriberPtr;
}

void ThermalLevelEventTest::SetUpTestCase()
{
    service = DelayedSpSingleton<ThermalService>::GetInstance();
    service->OnStart();
}

void ThermalLevelEventTest::TearDownTestCase()
{}

void ThermalLevelEventTest::SetUp()
{}

void ThermalLevelEventTest::TearDown()
{
    service->OnStop();
    DelayedSpSingleton<ThermalService>::DestroyInstance();
}

namespace {
/*
 * @tc.number: ThermalLevelEventTest001
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "CommonEventThermalLevelTest:: Test Start!!";
    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    tms->SetSensorTemp("battery", -18000);
    sleep(20);
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
}

/*
 * @tc.number: ThermalLevelEventTest002
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest002, TestSize.Level0)
{
    auto test = std::make_shared<CommonEventThermalLevelTest>();
    GTEST_LOG_(INFO) << "ThermalLevelEventTest002:: Test Start!!";

    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
}

/*
 * @tc.number: ThermalLevelEventTest003
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest003, TestSize.Level0)
{
    auto test = std::make_shared<CommonEventThermalLevelTest>();
    GTEST_LOG_(INFO) << "CommonEventThermalLevelTest:: Test Start!!";

    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();

    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    tms->SetSensorTemp("battery", 42000);
    sleep(20);
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
}

/*
 * @tc.number: ThermalLevelEventTest004
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest004, TestSize.Level0)
{
    auto test = std::make_shared<CommonEventThermalLevelTest>();
    GTEST_LOG_(INFO) << "CommonEventThermalLevelTest:: Test Start!!";

    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    tms->SetSensorTemp("battery", 44000);
    sleep(20);
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
}


/*
 * @tc.number: ThermalLevelEventTest005
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest005, TestSize.Level0)
{
    auto test = std::make_shared<CommonEventThermalLevelTest>();
    GTEST_LOG_(INFO) << "CommonEventThermalLevelTest:: Test Start!!";

    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    tms->SetSensorTemp("battery", 48000);
    sleep(20);
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
}

/*
 * @tc.number: ThermalLevelEventTest006
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest006, TestSize.Level0)
{
    auto test = std::make_shared<CommonEventThermalLevelTest>();
    GTEST_LOG_(INFO) << "CommonEventThermalLevelTest:: Test Start!!";

    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();

    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    tms->SetSensorTemp("pa", 42000);
    tms->SetSensorTemp("ambient", 32000);
    sleep(20);
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
}

/*
 * @tc.number: ThermalLevelEventTest007
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest007, TestSize.Level0)
{
    auto test = std::make_shared<CommonEventThermalLevelTest>();
    GTEST_LOG_(INFO) << "CommonEventThermalLevelTest:: Test Start!!";

    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    tms->SetSensorTemp("pa", 44000);
    tms->SetSensorTemp("ambient", 33000);
    sleep(20);
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
}
}