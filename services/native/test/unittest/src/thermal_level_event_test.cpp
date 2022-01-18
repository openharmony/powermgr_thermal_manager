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
#include <fcntl.h>
#include <gtest/gtest.h>
#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <string_ex.h>
#include <unistd.h>
#include "action_thermal_level.h"
#include "file_operation.h"
#include "securec.h"
#include "thermal_common.h"
#include "thermal_service.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
using namespace OHOS::AAFwk;
using namespace OHOS::EventFwk;

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
    GTEST_LOG_(INFO) << " CommonEventThermalLevelTest: OnReceiveEvent Enter \n";
    std::string action = data.GetWant().GetAction();
    int min = 0;
    int max = 6;
    if (action == CommonEventSupport::COMMON_EVENT_THERMAL_LEVEL_CHANGED) {
        GTEST_LOG_(INFO) << "OnReceiveEvent: data=" << data.GetData();
        GTEST_LOG_(INFO) << "OnReceiveEvent: code=" << data.GetCode();
        EXPECT_EQ(true, (data.GetCode() >= min) && (data.GetCode() <= max));
    } else {
        GTEST_LOG_(INFO) << "failed to receive level changed event";
    }
}

shared_ptr<CommonEventThermalLevelTest> CommonEventThermalLevelTest::RegisterEvent()
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
}

void ThermalLevelEventTest::TearDownTestCase()
{}

void ThermalLevelEventTest::SetUp()
{}

void ThermalLevelEventTest::TearDown()
{
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
    char batteryTempBuf[MAX_PATH] = {0};
    int32_t temp = 41000;
    int32_t ret = -1;
    std::string sTemp = to_string(temp) + "\n";
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = FileOperation::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();
    sleep(WAIT_TIME * 10);
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
}

/*
 * @tc.number: ThermalLevelEventTest002
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ThermalLevelEventTest002:: Test Start!!";
    char batteryTempBuf[MAX_PATH] = {0};
    int32_t temp = 43100;
    int32_t ret = -1;
    std::string sTemp = to_string(temp) + "\n";
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = FileOperation::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();
    sleep(WAIT_TIME * 10);
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
}

/*
 * @tc.number: ThermalLevelEventTest003
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "CommonEventThermalLevelTest:: Test Start!!";
    char batteryTempBuf[MAX_PATH] = {0};
    int32_t temp = 46100;
    int32_t ret = -1;
    std::string sTemp = to_string(temp) + "\n";
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = FileOperation::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();
    sleep(WAIT_TIME * 10);
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
}

/*
 * @tc.number: ThermalLevelEventTest004
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest004, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "CommonEventThermalLevelTest:: Test Start!!";
    int32_t ret = -1;
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(paTempBuf, PATH_MAX, sizeof(paTempBuf) - 1, paPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(amTempBuf, PATH_MAX, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    int32_t paTemp = 41000;
    std::string sTemp = to_string(paTemp) + "\n";
    ret = FileOperation::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 10000;
    sTemp = to_string(amTemp) + "\n";
    ret = FileOperation::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();
    sleep(WAIT_TIME * 10);
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
    int32_t ret = -1;
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(paTempBuf, PATH_MAX, sizeof(paTempBuf) - 1, paPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(amTempBuf, PATH_MAX, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    int32_t paTemp = 44000;
    std::string sTemp = to_string(paTemp) + "\n";
    ret = FileOperation::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 10000;
    sTemp = to_string(amTemp) + "\n";
    ret = FileOperation::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();
    sleep(WAIT_TIME * 10);
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
}

/*
 * @tc.number: ThermalLevelEventTest006
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest006, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "CommonEventThermalLevelTest:: Test Start!!";
    int32_t ret = -1;
    char apTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    char shellTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(apTempBuf, PATH_MAX, sizeof(apTempBuf) - 1, apPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(amTempBuf, PATH_MAX, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(shellTempBuf, PATH_MAX, sizeof(shellTempBuf) - 1, shellPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    int32_t apTemp = 78000;
    std::string sTemp = to_string(apTemp) + "\n";
    ret = FileOperation::WriteFile(apTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 1000;
    sTemp = to_string(amTemp) + "\n";
    ret = FileOperation::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t shellTemp = 50000;
    sTemp = to_string(shellTemp) + "\n";
    ret = FileOperation::WriteFile(shellTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();
    sleep(WAIT_TIME * 10);
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
}

/*
 * @tc.number: ThermalLevelEventTest007
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest007, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "CommonEventThermalLevelTest:: Test Start!!";
    int32_t ret = -1;
    char socTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(socTempBuf, PATH_MAX, sizeof(socTempBuf) - 1, socPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t socTemp = -10000;
    std::string sTemp = to_string(socTemp);
    ret = FileOperation::WriteFile(socTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();
    sleep(WAIT_TIME * 10);
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
}
}