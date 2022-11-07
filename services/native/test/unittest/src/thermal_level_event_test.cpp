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
#include "action_thermal_level.h"
#include "file_operation.h"
#include "mock_thermal_mgr_client.h"
#include "securec.h"
#include "thermal_common.h"
#include "thermal_service.h"
#include "thermal_mgr_client.h"
#include "thermal_level_info.h"

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
    THERMAL_HILOGD(LABEL_TEST, "CommonEventThermalLevelTest: OnReceiveEvent Enter \n");
    int invalidLevel = -1;
    std::string action = data.GetWant().GetAction();
    if (action == CommonEventSupport::COMMON_EVENT_THERMAL_LEVEL_CHANGED) {
        std::string key = ToString(static_cast<int32_t>(ThermalCommonEventCode::CODE_THERMAL_LEVEL_CHANGED));
        int32_t level = data.GetWant().GetIntParam(key, invalidLevel);
        GTEST_LOG_(INFO) << "thermal level: " << level;
    }
}

shared_ptr<CommonEventThermalLevelTest> CommonEventThermalLevelTest::RegisterEvent()
{
    THERMAL_HILOGD(LABEL_TEST, "RegisterEvent: Regist Subscriber Start!");
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

int32_t ThermalLevelEventTest::InitNode()
{
    char bufTemp[MAX_PATH] = {0};
    int32_t ret = -1;
    std::map<std::string, int32_t> sensor;
    sensor["battery"] = 0;
    sensor["charger"] = 0;
    sensor["pa"] = 0;
    sensor["ap"] = 0;
    sensor["ambient"] = 0;
    sensor["cpu"] = 0;
    sensor["soc"] = 0;
    sensor["shell"] = 0;
    for (auto iter : sensor) {
        ret = snprintf_s(bufTemp, MAX_PATH, sizeof(bufTemp) - 1, SIMULATION_TEMP_DIR, iter.first.c_str());
        if (ret < EOK) {
            return ret;
        }
        std::string temp = std::to_string(iter.second);
        FileOperation::WriteFile(bufTemp, temp, temp.length());
    }
    return ERR_OK;
}

void ThermalLevelEventTest::SetUpTestCase()
{
}

void ThermalLevelEventTest::TearDownTestCase()
{
}

void ThermalLevelEventTest::SetUp()
{
}

void ThermalLevelEventTest::TearDown()
{
    InitNode();
}

namespace {
/*
 * @tc.number: ThermalLevelEventTest001
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest001, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "CommonEventThermalLevelTest:: Test Start!!");
    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();
    char batteryTempBuf[MAX_PATH] = {0};
    int32_t temp = 41000;
    int32_t ret = -1;
    std::string sTemp = to_string(temp) + "\n";
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = FileOperation::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
}

/*
 * @tc.number: ThermalLevelEventTest002
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest002, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventTest002:: Test Start!!");
    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();
    char batteryTempBuf[MAX_PATH] = {0};
    int32_t temp = 43100;
    int32_t ret = -1;
    std::string sTemp = to_string(temp) + "\n";
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = FileOperation::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
}

/*
 * @tc.number: ThermalLevelEventTest003
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest003, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "CommonEventThermalLevelTest:: Test Start!!");
    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();
    char batteryTempBuf[MAX_PATH] = {0};
    int32_t temp = 46100;
    int32_t ret = -1;
    std::string sTemp = to_string(temp) + "\n";
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = FileOperation::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
}

/*
 * @tc.number: ThermalLevelEventTest004
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest004, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "CommonEventThermalLevelTest:: Test Start!!");
    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();
    int32_t ret = -1;
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(paTempBuf, MAX_PATH, sizeof(paTempBuf) - 1, PA_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(amTempBuf, MAX_PATH, sizeof(amTempBuf) - 1, AMBIENT_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);

    int32_t paTemp = 41000;
    std::string sTemp = to_string(paTemp) + "\n";
    ret = FileOperation::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 10000;
    sTemp = to_string(amTemp) + "\n";
    ret = FileOperation::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
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
    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();
    THERMAL_HILOGD(LABEL_TEST, "CommonEventThermalLevelTest:: Test Start!!");
    int32_t ret = -1;
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(paTempBuf, MAX_PATH, sizeof(paTempBuf) - 1, PA_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(amTempBuf, MAX_PATH, sizeof(amTempBuf) - 1, AMBIENT_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);

    int32_t paTemp = 44000;
    std::string sTemp = to_string(paTemp) + "\n";
    ret = FileOperation::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 10000;
    sTemp = to_string(amTemp) + "\n";
    ret = FileOperation::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
}

/*
 * @tc.number: ThermalLevelEventTest006
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest006, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "CommonEventThermalLevelTest:: Test Start!!");
    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();
    int32_t ret = -1;
    char apTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    char shellTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(apTempBuf, MAX_PATH, sizeof(apTempBuf) - 1, AP_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(amTempBuf, MAX_PATH, sizeof(amTempBuf) - 1, AMBIENT_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(shellTempBuf, MAX_PATH, sizeof(shellTempBuf) - 1, SHELL_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);

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
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
}

/*
 * @tc.number: ThermalLevelEventTest007
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest007, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "CommonEventThermalLevelTest:: Test Start!!");
    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();
    int32_t ret = -1;
    char socTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(socTempBuf, MAX_PATH, sizeof(socTempBuf) - 1, SOC_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t socTemp = -10000;
    std::string sTemp = to_string(socTemp);
    ret = FileOperation::WriteFile(socTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
}

/*
 * @tc.number: ThermalLevelEventTest008
 * @tc.name: ThermalLevelEventTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F(ThermalLevelEventTest, ThermalLevelEventTest008, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventTest008:: Test Start!!");
    int32_t ret = -1;
    bool result = false;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp);
    ret = FileOperation::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    shared_ptr<CommonEventThermalLevelTest> subscriber = CommonEventThermalLevelTest::RegisterEvent();
    MockThermalMgrClient::GetInstance().GetThermalInfo();

    CommonEventData stickyData;
    CommonEventManager::GetStickyCommonEvent(CommonEventSupport::COMMON_EVENT_THERMAL_LEVEL_CHANGED, stickyData);
    if (stickyData.GetWant().GetAction() != CommonEventSupport::COMMON_EVENT_THERMAL_LEVEL_CHANGED) {
        result = false;
    } else {
        THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventTest008:: sticky event successfully!!");
        result = true;
    }
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
}
}