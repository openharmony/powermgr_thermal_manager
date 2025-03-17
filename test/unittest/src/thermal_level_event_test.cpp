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

#include "thermal_level_info.h"
#include "thermal_log.h"

#define private   public
#define protected public
#include "thermal_config_file_parser.h"
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
using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;

AccessTokenID ThermalLevelEventTest::tokenID_ = 0;
static constexpr int32_t DEFAULT_API_VERSION = 8;
namespace {
std::condition_variable g_callbackCV;
std::mutex g_mutex;
constexpr int64_t TIME_OUT = 3;
bool g_callbackTriggered = false;
int32_t g_thermalLevel = -1;
const std::string SYSTEM_THERMAL_SERVICE_CONFIG_PATH = "/system/etc/thermal_config/thermal_service_config.xml";
sptr<ThermalService> g_service = nullptr;
ThermalConfigFileParser g_parser;

uint64_t g_token;

PermissionStateFull g_infoManagerTestState = {
    .grantFlags = {1},
    .grantStatus = {PermissionState::PERMISSION_GRANTED},
    .isGeneral = true,
    .permissionName = "ohos.permission.PUBLISH_SYSTEM_COMMON_EVENT",
    .resDeviceID = {"local"}
};

HapPolicyParams g_infoManagerTestPolicyPrams = {
    .apl = APL_NORMAL,
    .domain = "test.domain.ThermalLevelEventTest",
    .permList = {},
    .permStateList = {g_infoManagerTestState}
};

HapInfoParams g_infoManagerTestInfoParms = {
    .bundleName = "com.ohos.thermal_level_event_test",
    .userID = 100,
    .instIndex = 0,
    .appIDDesc = "ThermalLevelEventTest",
    .apiVersion = DEFAULT_API_VERSION,
    .isSystemApp = true
};

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

class CommonEventThermalLevelTest : public CommonEventSubscriber {
public:
    CommonEventThermalLevelTest() = default;
    explicit CommonEventThermalLevelTest(const CommonEventSubscribeInfo& subscriberInfo);
    virtual ~CommonEventThermalLevelTest() {};
    virtual void OnReceiveEvent(const CommonEventData& data);
    static shared_ptr<CommonEventThermalLevelTest> RegisterEvent();
};

CommonEventThermalLevelTest::CommonEventThermalLevelTest(const CommonEventSubscribeInfo& subscriberInfo)
    : CommonEventSubscriber(subscriberInfo)
{
}

void CommonEventThermalLevelTest::OnReceiveEvent(const CommonEventData& data)
{
    THERMAL_HILOGI(LABEL_TEST, "CommonEventThermalLevelTest: OnReceiveEvent Enter");
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
    THERMAL_HILOGI(LABEL_TEST, "RegisterEvent: Regist Subscriber Start");
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
        THERMAL_HILOGI(COMP_SVC, "Failed to register subscriber");
        return nullptr;
    }
    return subscriberPtr;
}

void ThermalLevelEventTest::TearDown()
{
    g_thermalLevel = -1;
    g_callbackTriggered = false;
}

void ThermalLevelEventTest::SetUpTestCase()
{
    AccessTokenIDEx tokenIdEx = {0};
    tokenIdEx = AccessTokenKit::AllocHapToken(g_infoManagerTestInfoParms, g_infoManagerTestPolicyPrams);
    tokenID_ = tokenIdEx.tokenIdExStruct.tokenID;
    ASSERT_NE(0, tokenID_);
    g_token = GetSelfTokenID();
    ASSERT_EQ(0, SetSelfTokenID(tokenID_));
    g_service = ThermalService::GetInstance();
    g_service->InitSystemTestModules();
    g_service->OnStart();
    g_parser.Init();
}

void ThermalLevelEventTest::TearDownTestCase()
{
    if (g_service != nullptr) {
        g_service->OnStop();
    }
    AccessTokenKit::DeleteToken(tokenID_);
    SetSelfTokenID(g_token);
}

namespace {
/*
 * @tc.number: ThermalLevelEventTest001
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest001, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalLevelEventTest001 function start!");
    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 46100;
    event.info.push_back(info1);
    g_thermalLevel = 3;
    GTEST_LOG_(INFO) << "test thermal level: " << g_thermalLevel;
    g_service->HandleThermalCallbackEvent(event);
    event.info.clear();
    Wait();

    info1.type = "soc";
    info1.temp = -10000;
    event.info.push_back(info1);
    g_thermalLevel = 0;
    GTEST_LOG_(INFO) << "test thermal level: " << g_thermalLevel;
    g_service->HandleThermalCallbackEvent(event);
    Wait();
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
    THERMAL_HILOGI(LABEL_TEST, "ThermalLevelEventTest001 function end!");
}

/*
 * @tc.number: ThermalLevelEventTest002
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest002, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalLevelEventTest002 function start!");
    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();

    auto levelItems = g_parser.GetLevelItems("base_safe", "battery");
    for (auto item : levelItems) {
        std::vector<PolicyAction> policy;
        if (!g_parser.GetActionPolicy("base_safe", item.level, policy)) {
            continue;
        }
        HdfThermalCallbackInfo event;
        ThermalZoneInfo info;
        info.type = "battery";
        info.temp = item.threshold;
        g_thermalLevel = item.level;
        event.info.push_back(info);
        GTEST_LOG_(INFO) << "test thermal level: " << g_thermalLevel << ", threshold: " << item.threshold;
        g_service->HandleThermalCallbackEvent(event);
        Wait();
    }
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
    THERMAL_HILOGI(LABEL_TEST, "ThermalLevelEventTest002 function end!");
}

/*
 * @tc.number: ThermalLevelEventTest003
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest003, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalLevelEventTest003 function start!");
    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();

    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "pa";
    info1.temp = 41000;
    event.info.push_back(info1);

    ThermalZoneInfo info2;
    info2.type = "ambient";
    info2.temp = 10000;
    event.info.push_back(info2);

    g_thermalLevel = 4;
    GTEST_LOG_(INFO) << "test thermal level: " << g_thermalLevel;
    g_service->HandleThermalCallbackEvent(event);
    Wait();
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
    THERMAL_HILOGI(LABEL_TEST, "ThermalLevelEventTest003 function end!");
}

/*
 * @tc.number: ThermalLevelEventTest004
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest004, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalLevelEventTest004 function start!");
    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();

    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "pa";
    info1.temp = 44000;
    event.info.push_back(info1);

    ThermalZoneInfo info2;
    info2.type = "ambient";
    info2.temp = 10000;
    event.info.push_back(info2);

    g_thermalLevel = 5;
    GTEST_LOG_(INFO) << "test thermal level: " << g_thermalLevel;
    g_service->HandleThermalCallbackEvent(event);
    Wait();
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
    THERMAL_HILOGI(LABEL_TEST, "ThermalLevelEventTest004 function end!");
}

/*
 * @tc.number: ThermalLevelEventTest005
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest005, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalLevelEventTest005 function start!");
    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "ap";
    info1.temp = 78000;
    event.info.push_back(info1);

    ThermalZoneInfo info2;
    info2.type = "ambient";
    info2.temp = 1000;
    event.info.push_back(info2);

    ThermalZoneInfo info3;
    info3.type = "shell";
    info3.temp = 30000;
    event.info.push_back(info3);

    g_thermalLevel = 6;
    GTEST_LOG_(INFO) << "test thermal level: " << g_thermalLevel;
    g_service->HandleThermalCallbackEvent(event);
    Wait();
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
    THERMAL_HILOGI(LABEL_TEST, "ThermalLevelEventTest005 function end!");
}
} // namespace
