/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "action_thermal_level.h"
#include "file_operation.h"
#include "securec.h"
#include "thermal_common.h"
#include "thermal_level_info.h"
#include "thermal_service.h"
#include <common_event_data.h>
#include <common_event_manager.h>
#include <common_event_publish_info.h>
#include <common_event_subscriber.h>
#include <common_event_support.h>
#include <condition_variable>
#include <datetime_ex.h>
#include <dirent.h>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <mutex>
#include <string_ex.h>
#include <unistd.h>

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
using namespace OHOS::AAFwk;
using namespace OHOS::EventFwk;

namespace {
std::condition_variable g_cv;
std::mutex g_mtx;
std::string g_action = "";
constexpr int64_t TIME_OUT = 1;
} // namespace

class CommonEventThermalLevel1Test : public CommonEventSubscriber {
public:
    CommonEventThermalLevel1Test() = default;
    explicit CommonEventThermalLevel1Test(const CommonEventSubscribeInfo& subscriberInfo);
    virtual ~CommonEventThermalLevel1Test() {};
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
    virtual ~CommonEventThermalLevel2Test() {};
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
    virtual ~CommonEventThermalLevel3Test() {};
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
    virtual ~CommonEventThermalIdleTrueTest() {};
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
    virtual ~CommonEventThermalIdleFalseTest() {};
    virtual void OnReceiveEvent(const CommonEventData& data);
    static shared_ptr<CommonEventThermalIdleFalseTest> RegisterEvent();
};

CommonEventThermalIdleFalseTest::CommonEventThermalIdleFalseTest(const CommonEventSubscribeInfo& subscriberInfo)
    : CommonEventSubscriber(subscriberInfo)
{
}

void CommonEventThermalLevel1Test::OnReceiveEvent(const CommonEventData& data)
{
    THERMAL_HILOGD(LABEL_TEST, "CommonEventThermalLevel1Test: OnReceiveEvent Enter \n");
    int32_t invalidLevel = -1;
    std::string key = ToString(static_cast<int32_t>(ThermalCommonEventCode::CODE_THERMAL_LEVEL_CHANGED));
    int32_t level = data.GetWant().GetIntParam(key, invalidLevel);
    GTEST_LOG_(INFO) << "thermal level: " << level;
    g_cv.notify_one();
    EXPECT_EQ(level, static_cast<int32_t>(ThermalLevel::NORMAL)) << "get thermal level failed";
}

void CommonEventThermalLevel2Test::OnReceiveEvent(const CommonEventData& data)
{
    THERMAL_HILOGD(LABEL_TEST, "CommonEventThermalLevel2Test: OnReceiveEvent Enter \n");
    int32_t invalidLevel = -1;
    std::string key = ToString(static_cast<int32_t>(ThermalCommonEventCode::CODE_THERMAL_LEVEL_CHANGED));
    int32_t level = data.GetWant().GetIntParam(key, invalidLevel);
    GTEST_LOG_(INFO) << "thermal level: " << level;
    g_cv.notify_one();
    EXPECT_EQ(level, static_cast<int32_t>(ThermalLevel::WARM)) << "get thermal level failed";
}

void CommonEventThermalLevel3Test::OnReceiveEvent(const CommonEventData& data)
{
    THERMAL_HILOGD(LABEL_TEST, "CommonEventThermalLevel3Test: OnReceiveEvent Enter \n");
    int32_t invalidLevel = -1;
    std::string key = ToString(static_cast<int32_t>(ThermalCommonEventCode::CODE_THERMAL_LEVEL_CHANGED));
    int32_t level = data.GetWant().GetIntParam(key, invalidLevel);
    GTEST_LOG_(INFO) << "thermal level: " << level;
    g_cv.notify_one();
    EXPECT_EQ(level, static_cast<int32_t>(ThermalLevel::HOT)) << "get thermal level failed";
}

void CommonEventThermalIdleTrueTest::OnReceiveEvent(const CommonEventData& data)
{
    THERMAL_HILOGD(LABEL_TEST, "CommonEventThermalIdleTrueTest: OnReceiveEvent Enter \n");
    bool invalidState = true;
    std::string key = ToString(static_cast<uint32_t>(ChargeIdleEventCode::EVENT_CODE_CHARGE_IDLE_STATE));
    bool getState = data.GetWant().GetIntParam(key, invalidState);
    GTEST_LOG_(INFO) << "charger state: " << getState;
    g_cv.notify_one();
    EXPECT_EQ(getState, true) << "get charger state failed";
}

void CommonEventThermalIdleFalseTest::OnReceiveEvent(const CommonEventData& data)
{
    THERMAL_HILOGD(LABEL_TEST, "CommonEventThermalIdleFalseTest: OnReceiveEvent Enter \n");
    bool invalidState = true;
    std::string key = ToString(static_cast<uint32_t>(ChargeIdleEventCode::EVENT_CODE_CHARGE_IDLE_STATE));
    bool getState = data.GetWant().GetIntParam(key, invalidState);
    GTEST_LOG_(INFO) << "charger state: " << getState;
    g_cv.notify_one();
    EXPECT_EQ(getState, false) << "get charger state failed";
}

shared_ptr<CommonEventThermalLevel1Test> CommonEventThermalLevel1Test::RegisterEvent()
{
    THERMAL_HILOGD(LABEL_TEST, "RegisterEvent: Regist Subscriber Start!");
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
    THERMAL_HILOGD(LABEL_TEST, "RegisterEvent: Regist Subscriber Start!");
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
    THERMAL_HILOGD(LABEL_TEST, "RegisterEvent: Regist Subscriber Start!");
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
    THERMAL_HILOGD(LABEL_TEST, "RegisterEvent: Regist Subscriber Start!");
    int32_t retryTimes = 2;
    bool succeed = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_CHARGE_IDLE);
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
    THERMAL_HILOGD(LABEL_TEST, "RegisterEvent: Regist Subscriber Start!");
    int32_t retryTimes = 2;
    bool succeed = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_CHARGE_IDLE);
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

int32_t ThermalLevelEventSystemTest::InitNode()
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
        ret = snprintf_s(bufTemp, MAX_PATH, sizeof(bufTemp) - 1, SIMULATION_TEMP_DIR.c_str(), iter.first.c_str());
        if (ret < EOK) {
            return ret;
        }
        std::string temp = std::to_string(iter.second);
        FileOperation::WriteFile(bufTemp, temp, temp.length());
    }
    return ERR_OK;
}

void ThermalLevelEventSystemTest::TearDown()
{
    InitNode();
}

bool ThermalLevelEventSystemTest::IsMock(const std::string& path)
{
    DIR* dir = opendir(path.c_str());
    if (dir == nullptr) {
        return false;
    }
    struct dirent* ptr = nullptr;
    while ((ptr = readdir(dir)) != nullptr) {
        if (strcmp(".", ptr->d_name) != 0 && strcmp("..", ptr->d_name) != 0) {
            closedir(dir);
            return true;
        }
    }
    closedir(dir);
    return false;
}

namespace {
/*
 * @tc.number: ThermalLevelEventSystemTest001
 * @tc.name: ThermalLevelEventSystemTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F (ThermalLevelEventSystemTest, ThermalLevelEventSystemTest001, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventSystemTest001:: Test Start!!");
    char batteryTempBuf[MAX_PATH] = {0};
    shared_ptr<CommonEventThermalLevel1Test> subscriber = CommonEventThermalLevel1Test::RegisterEvent();
    int32_t ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);

    int32_t batteryTemp = 40600;
    std::string sTemp = to_string(batteryTemp);
    ret = FileOperation::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::unique_lock<std::mutex> lck(g_mtx);
    if (g_cv.wait_for(lck, std::chrono::seconds(TIME_OUT)) == std::cv_status::timeout) {
        g_cv.notify_one();
    }
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
}

/*
 * @tc.number: ThermalLevelEventSystemTest002
 * @tc.name: ThermalLevelEventSystemTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F (ThermalLevelEventSystemTest, ThermalLevelEventSystemTest002, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventSystemTest002:: Test Start!!");
    char batteryTempBuf[MAX_PATH] = {0};
    shared_ptr<CommonEventThermalLevel2Test> subscriber = CommonEventThermalLevel2Test::RegisterEvent();
    int32_t ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);

    int32_t batteryTemp = 43600;
    std::string sTemp = to_string(batteryTemp);
    ret = FileOperation::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::unique_lock<std::mutex> lck(g_mtx);
    if (g_cv.wait_for(lck, std::chrono::seconds(TIME_OUT)) == std::cv_status::timeout) {
        g_cv.notify_one();
    }
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
}

/*
 * @tc.number: ThermalLevelEventSystemTest003
 * @tc.name: ThermalLevelEventSystemTest
 * @tc.desc: Verify the receive the level common event
 */
HWTEST_F (ThermalLevelEventSystemTest, ThermalLevelEventSystemTest003, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventSystemTest003:: Test Start!!");
    char batteryTempBuf[MAX_PATH] = {0};
    shared_ptr<CommonEventThermalLevel3Test> subscriber = CommonEventThermalLevel3Test::RegisterEvent();
    int32_t ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);

    int32_t batteryTemp = 46600;
    std::string sTemp = to_string(batteryTemp);
    ret = FileOperation::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::unique_lock<std::mutex> lck(g_mtx);
    if (g_cv.wait_for(lck, std::chrono::seconds(TIME_OUT)) == std::cv_status::timeout) {
        g_cv.notify_one();
    }
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
}

/*
 * @tc.number: ThermalLevelEventSystemTest004
 * @tc.name: ThermalLevelEventSystemTest
 * @tc.desc: Verify the receive the idle common event
 */
HWTEST_F (ThermalLevelEventSystemTest, ThermalLevelEventSystemTest004, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventSystemTest004:: Test Start!!");

    char batteryTempBuf[MAX_PATH] = {0};
    char batteryCapacityBuf[MAX_PATH] = {0};
    char batteryChargerBuf[MAX_PATH] = {0};
    char chargerCurrentBuf[MAX_PATH] = {0};
    shared_ptr<CommonEventThermalIdleTrueTest> subscriber = CommonEventThermalIdleTrueTest::RegisterEvent();
    int32_t ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 40600;
    std::string sTemp = to_string(batteryTemp);
    ret = FileOperation::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (IsMock(BATTERY_CAPACITY_PATH) && IsMock(CHARGER_STATUS_PATH) && IsMock(CHARGER_CURRENT_PATH)) {
        ret = snprintf_s(batteryCapacityBuf, MAX_PATH, sizeof(batteryCapacityBuf) - 1, BATTERY_CAPACITY_PATH.c_str());
        EXPECT_EQ(true, ret >= EOK);
        int32_t batteryCapacity = 90;
        std::string cTemp = to_string(batteryCapacity);
        ret = FileOperation::WriteFile(batteryCapacityBuf, cTemp, cTemp.length());
        EXPECT_EQ(true, ret == ERR_OK);

        ret = snprintf_s(batteryChargerBuf, MAX_PATH, sizeof(batteryChargerBuf) - 1, CHARGER_STATUS_PATH.c_str());
        EXPECT_EQ(true, ret >= EOK);
        std::string batteryChargerState = "Charging";
        ret = FileOperation::WriteFile(batteryChargerBuf, batteryChargerState, batteryChargerState.length());
        EXPECT_EQ(true, ret == ERR_OK);

        ret = snprintf_s(chargerCurrentBuf, MAX_PATH, sizeof(chargerCurrentBuf) - 1, CHARGER_CURRENT_PATH.c_str());
        EXPECT_EQ(true, ret >= EOK);
        int32_t chargerCurrent = 1100;
        std::string cCurrent = to_string(chargerCurrent);
        ret = FileOperation::WriteFile(chargerCurrentBuf, cCurrent, cCurrent.length());
        EXPECT_EQ(true, ret == ERR_OK);
    }

    system("hidumper -s 3302 -a -r");
    std::unique_lock<std::mutex> lck(g_mtx);
    if (g_cv.wait_for(lck, std::chrono::seconds(TIME_OUT)) == std::cv_status::timeout) {
        g_cv.notify_one();
    }
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
}

/*
 * @tc.number: ThermalLevelEventSystemTest005
 * @tc.name: ThermalLevelEventSystemTest
 * @tc.desc: Verify the receive the idle common event
 */
HWTEST_F (ThermalLevelEventSystemTest, ThermalLevelEventSystemTest005, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalLevelEventSystemTest005:: Test Start!!");

    char batteryTempBuf[MAX_PATH] = {0};
    char batteryCapacityBuf[MAX_PATH] = {0};
    char batteryChargerBuf[MAX_PATH] = {0};
    char chargerCurrentBuf[MAX_PATH] = {0};

    shared_ptr<CommonEventThermalIdleFalseTest> subscriber = CommonEventThermalIdleFalseTest::RegisterEvent();
    int32_t ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 40000;
    std::string sTemp = to_string(batteryTemp);
    ret = FileOperation::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (IsMock(BATTERY_CAPACITY_PATH) && IsMock(CHARGER_STATUS_PATH) && IsMock(CHARGER_CURRENT_PATH)) {
        ret = snprintf_s(batteryCapacityBuf, MAX_PATH, sizeof(batteryCapacityBuf) - 1, BATTERY_CAPACITY_PATH.c_str());
        EXPECT_EQ(true, ret >= EOK);
        int32_t batteryCapacity = 70;
        std::string capacity = to_string(batteryCapacity);
        ret = FileOperation::WriteFile(batteryCapacityBuf, capacity, capacity.length());
        EXPECT_EQ(true, ret == ERR_OK);

        ret = snprintf_s(batteryChargerBuf, MAX_PATH, sizeof(batteryChargerBuf) - 1, CHARGER_STATUS_PATH.c_str());
        EXPECT_EQ(true, ret >= EOK);
        std::string batteryChargerState = "DisCharging";
        ret = FileOperation::WriteFile(batteryChargerBuf, batteryChargerState, batteryChargerState.length());
        EXPECT_EQ(true, ret == ERR_OK);

        ret = snprintf_s(chargerCurrentBuf, MAX_PATH, sizeof(chargerCurrentBuf) - 1, CHARGER_CURRENT_PATH.c_str());
        EXPECT_EQ(true, ret >= EOK);
        int32_t chargerCurrent = 900;
        std::string current = to_string(chargerCurrent);
        ret = FileOperation::WriteFile(chargerCurrentBuf, current, current.length());
        EXPECT_EQ(true, ret == ERR_OK);
    }

    system("hidumper -s 3302 -a -r");
    std::unique_lock<std::mutex> lck(g_mtx);
    if (g_cv.wait_for(lck, std::chrono::seconds(TIME_OUT)) == std::cv_status::timeout) {
        g_cv.notify_one();
    }
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
}
} // namespace