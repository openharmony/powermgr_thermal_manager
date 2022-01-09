/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
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


#include "thermal_mgr_system_test.h"

#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <fcntl.h>
#include <unistd.h>
#include "securec.h"

#include "thermal_service.h"
#include "thermal_mgr_client.h"
#include "constants.h"
#include "thermal_common.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

static sptr<ThermalService> service;
static std::mutex g_mtx;

void ThermalMgrSystemTest::SetUpTestCase(void)
{
    service = DelayedSpSingleton<ThermalService>::GetInstance();
    service->OnStart();
}

void ThermalMgrSystemTest::TearDownTestCase(void)
{
    service->OnStop();
    DelayedSpSingleton<ThermalService>::DestroyInstance();
}

void ThermalMgrSystemTest::SetUp(void)
{
}

void ThermalMgrSystemTest::TearDown(void)
{
}

int32_t ThermalMgrSystemTest::WriteFile(std::string path, std::string buf, size_t size)
{
    std::lock_guard<std::mutex> lck(g_mtx);
    int32_t fd = open(path.c_str(), O_RDWR);
    if (fd < ERR_OK) {
        GTEST_LOG_(INFO) << "WriteFile: failed to open file";
        return ERR_INVALID_VALUE;
    }
    write(fd, buf.c_str(), size);
    close(fd);
    return ERR_OK;
}

int32_t ThermalMgrSystemTest::ReadFile(const char *path, char *buf, size_t size)
{
    std::lock_guard<std::mutex> lck(g_mtx);
    int32_t ret;

    int32_t fd = open(path, O_RDONLY);
    if (fd < ERR_OK) {
        GTEST_LOG_(INFO) << "WriteFile: failed to open file" << fd;
        return ERR_INVALID_VALUE;
    }

    ret = read(fd, buf, size);
    if (ret < ERR_OK) {
        GTEST_LOG_(INFO) << "WriteFile: failed to read file" << ret;
        close(fd);
        return ERR_INVALID_VALUE;
    }

    close(fd);
    buf[size - 1] = '\0';
    return ERR_OK;
}

int32_t ThermalMgrSystemTest::ConvertInt(const std::string &value)
{
    return std::stoi(value);
}

/**
 * @tc.name: ThermalMgrSystemTest001
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 1
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest001, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest001: start.";
    int32_t batteryTemp = 40100;
    std::list<uint32_t> levelList;
    auto tms = DelayedSpSingleton<ThermalService>::GetInstance();
    tms->SetSensorTemp(BATTERY, batteryTemp);
    if (tms == nullptr)
    GTEST_LOG_(INFO) << "tms is nullptr";
    sleep(WAIT_TIME * 10);
    auto levelMap = tms->GetPolicy()->GetClusterLevelMap();
    for (auto level : levelMap) {
        GTEST_LOG_(INFO) << "level is:" << level.second;
        levelList.push_back(level.second);
    }
    auto level = *max_element(levelList.begin(), levelList.end());
    GTEST_LOG_(INFO) << "level is:" << level;
    EXPECT_EQ(true, level == 1) << "ThermalMgrSystemTest001 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest001: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest002
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 2
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest002, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest002: start.";
    int32_t batteryTemp = 43100;
    std::list<uint32_t> levelList;
    auto tms = DelayedSpSingleton<ThermalService>::GetInstance();
    tms->SetSensorTemp(BATTERY, batteryTemp);
    if (tms == nullptr)
    GTEST_LOG_(INFO) << "tms is nullptr";
    sleep(WAIT_TIME * 10);
    auto levelMap = tms->GetPolicy()->GetClusterLevelMap();
    for (auto level : levelMap) {
        GTEST_LOG_(INFO) << "level is:" << level.second;
        levelList.push_back(level.second);
    }
    auto level = *max_element(levelList.begin(), levelList.end());
    GTEST_LOG_(INFO) << "level is:" << level;
    EXPECT_EQ(true, level == 2) << "ThermalMgrSystemTest002 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest002: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest003
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 3
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest003, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest003: start.";
    int32_t batteryTemp = 46100;
    std::list<uint32_t> levelList;
    auto tms = DelayedSpSingleton<ThermalService>::GetInstance();
    tms->SetSensorTemp(BATTERY, batteryTemp);
    if (tms == nullptr)
    GTEST_LOG_(INFO) << "tms is nullptr";
    sleep(WAIT_TIME * 10);
    auto levelMap = tms->GetPolicy()->GetClusterLevelMap();
    for (auto level : levelMap) {
        GTEST_LOG_(INFO) << "level is:" << level.second;
        levelList.push_back(level.second);
    }
    auto level = *max_element(levelList.begin(), levelList.end());
    GTEST_LOG_(INFO) << "level is:" << level;
    EXPECT_EQ(true, level == 3) << "ThermalMgrSystemTest003 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest003: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest004
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 4
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest004, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest004: start.";
    int32_t batteryTemp = 48100 ;
    std::list<uint32_t> levelList;
    auto tms = DelayedSpSingleton<ThermalService>::GetInstance();
    tms->SetSensorTemp(BATTERY, batteryTemp);
    if (tms == nullptr)
    GTEST_LOG_(INFO) << "tms is nullptr";
    sleep(WAIT_TIME * 10);
    auto levelMap = tms->GetPolicy()->GetClusterLevelMap();
    for (auto level : levelMap) {
        GTEST_LOG_(INFO) << "level is:" << level.second;
        levelList.push_back(level.second);
    }
    auto level = *max_element(levelList.begin(), levelList.end());
    GTEST_LOG_(INFO) << "level is:" << level;
    EXPECT_EQ(true, level == 4) << "ThermalMgrSystemTest004 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest004: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest004
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 1 ==> level 4
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest005, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest005: start.";
    int32_t batteryTemp = 40100 ;
    std::list<uint32_t> levelList;
    auto tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr)
    GTEST_LOG_(INFO) << "tms is nullptr";

    tms->SetSensorTemp(BATTERY, batteryTemp);
    sleep(WAIT_TIME * 10);
    auto levelMap = tms->GetPolicy()->GetClusterLevelMap();
    for (auto level : levelMap) {
        GTEST_LOG_(INFO) << "level is:" << level.second;
        levelList.push_back(level.second);
    }

    auto level = *max_element(levelList.begin(), levelList.end());
    levelList.clear();
    sleep(WAIT_TIME * 5);
    batteryTemp = 48100;
    tms->SetSensorTemp(BATTERY, batteryTemp);
    sleep(WAIT_TIME * 10);
    levelMap = tms->GetPolicy()->GetClusterLevelMap();
    for (auto level : levelMap) {
        GTEST_LOG_(INFO) << "level is:" << level.second;
        levelList.push_back(level.second);
    }

    level = *max_element(levelList.begin(), levelList.end());
    GTEST_LOG_(INFO) << "level is:" << level;
    EXPECT_EQ(true, level == 4) << "ThermalMgrSystemTest005 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest005: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest006
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 2 ==> level 4
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest006, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest006: start.";
    int32_t batteryTemp = 43100 ;
    std::list<uint32_t> levelList;
    auto tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr)
    GTEST_LOG_(INFO) << "tms is nullptr";

    tms->SetSensorTemp(BATTERY, batteryTemp);
    sleep(WAIT_TIME * 10);
    auto levelMap = tms->GetPolicy()->GetClusterLevelMap();
    for (auto level : levelMap) {
        GTEST_LOG_(INFO) << "level is:" << level.second;
        levelList.push_back(level.second);
    }

    auto level = *max_element(levelList.begin(), levelList.end());
    levelList.clear();
    sleep(WAIT_TIME * 5);
    batteryTemp = 48100;
    tms->SetSensorTemp(BATTERY, batteryTemp);
    sleep(WAIT_TIME * 10);
    levelMap = tms->GetPolicy()->GetClusterLevelMap();
    for (auto level : levelMap) {
        GTEST_LOG_(INFO) << "level is:" << level.second;
        levelList.push_back(level.second);
    }

    level = *max_element(levelList.begin(), levelList.end());
    GTEST_LOG_(INFO) << "level is:" << level;
    EXPECT_EQ(true, level == 4) << "ThermalMgrSystemTest006 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest006: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest007
 * @tc.desc: test level desc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 4 ===> level 1
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest007, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest007: start.";
    int32_t batteryTemp = 48200 ;
    std::list<uint32_t> levelList;
    auto tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr)
    GTEST_LOG_(INFO) << "tms is nullptr";

    tms->SetSensorTemp(BATTERY, batteryTemp);
    sleep(WAIT_TIME * 10);
    auto levelMap = tms->GetPolicy()->GetClusterLevelMap();
    for (auto level : levelMap) {
        GTEST_LOG_(INFO) << "level is:" << level.second;
        levelList.push_back(level.second);
    }

    auto level = *max_element(levelList.begin(), levelList.end());
    levelList.clear();
    sleep(WAIT_TIME * 5);
    batteryTemp = 40900;
    tms->SetSensorTemp(BATTERY, batteryTemp);
    sleep(WAIT_TIME * 10);
    levelMap = tms->GetPolicy()->GetClusterLevelMap();
    for (auto level : levelMap) {
        GTEST_LOG_(INFO) << "level is:" << level.second;
        levelList.push_back(level.second);
    }

    level = *max_element(levelList.begin(), levelList.end());
    GTEST_LOG_(INFO) << "level is:" << level;
    EXPECT_EQ(true, level == 1) << "ThermalMgrSystemTest007 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest007: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest008
 * @tc.desc: test level desc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 3 ===> level 0
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest008, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest008: start.";
    int32_t batteryTemp = 46100 ;
    std::list<uint32_t> levelList;
    auto tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr)
    GTEST_LOG_(INFO) << "tms is nullptr";

    tms->SetSensorTemp(BATTERY, batteryTemp);
    sleep(WAIT_TIME * 10);
    auto levelMap = tms->GetPolicy()->GetClusterLevelMap();
    for (auto level : levelMap) {
        GTEST_LOG_(INFO) << "level is:" << level.second;
        levelList.push_back(level.second);
    }

    auto level = *max_element(levelList.begin(), levelList.end());
    levelList.clear();
    sleep(WAIT_TIME * 5);
    batteryTemp = 37000;
    tms->SetSensorTemp(BATTERY, batteryTemp);
    sleep(WAIT_TIME * 10);
    levelMap = tms->GetPolicy()->GetClusterLevelMap();
    for (auto level : levelMap) {
        GTEST_LOG_(INFO) << "level is:" << level.second;
        levelList.push_back(level.second);
    }

    level = *max_element(levelList.begin(), levelList.end());
    GTEST_LOG_(INFO) << "level is:" << level;
    EXPECT_EQ(true, level == 0) << "ThermalMgrSystemTest008 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest008: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest009
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 1
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest009, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest009: start.";
    int32_t batteryTemp = -10000;
    std::list<uint32_t> levelList;
    auto tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr)
    GTEST_LOG_(INFO) << "tms is nullptr";
    tms->SetSensorTemp(BATTERY, batteryTemp);
    sleep(WAIT_TIME * 10);
    auto levelMap = tms->GetPolicy()->GetClusterLevelMap();
    for (auto level : levelMap) {
        GTEST_LOG_(INFO) << "level is:" << level.second;
        levelList.push_back(level.second);
    }
    auto level = *max_element(levelList.begin(), levelList.end());
    GTEST_LOG_(INFO) << "level is:" << level;
    EXPECT_EQ(true, level == 1) << "ThermalMgrSystemTest009 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest009: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest010
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 2
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest010, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest010: start.";
    int32_t batteryTemp = -15000;
    std::list<uint32_t> levelList;
    auto tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr)
    GTEST_LOG_(INFO) << "tms is nullptr";
    tms->SetSensorTemp(BATTERY, batteryTemp);
    sleep(WAIT_TIME * 10);
    auto levelMap = tms->GetPolicy()->GetClusterLevelMap();
    for (auto level : levelMap) {
        GTEST_LOG_(INFO) << "level is:" << level.second;
        levelList.push_back(level.second);
    }
    auto level = *max_element(levelList.begin(), levelList.end());
    GTEST_LOG_(INFO) << "level is:" << level;
    EXPECT_EQ(true, level == 2) << "ThermalMgrSystemTest010 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest010: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest011
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 3
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest011, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest011: start.";
    int32_t batteryTemp = -20100;
    std::list<uint32_t> levelList;
    auto tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr)
    GTEST_LOG_(INFO) << "tms is nullptr";
    tms->SetSensorTemp(BATTERY, batteryTemp);
    sleep(WAIT_TIME * 10);
    auto levelMap = tms->GetPolicy()->GetClusterLevelMap();
    for (auto level : levelMap) {
        GTEST_LOG_(INFO) << "level is:" << level.second;
        levelList.push_back(level.second);
    }
    auto level = *max_element(levelList.begin(), levelList.end());
    GTEST_LOG_(INFO) << "level is:" << level;
    EXPECT_EQ(true, level == 3) << "ThermalMgrSystemTest011 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest011: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest012
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 4
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest012, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest012: start.";
    int32_t batteryTemp = -22000;
    std::list<uint32_t> levelList;
    auto tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr)
    GTEST_LOG_(INFO) << "tms is nullptr";
    tms->SetSensorTemp(BATTERY, batteryTemp);
    sleep(WAIT_TIME * 10);
    auto levelMap = tms->GetPolicy()->GetClusterLevelMap();
    for (auto level : levelMap) {
        GTEST_LOG_(INFO) << "level is:" << level.second;
        levelList.push_back(level.second);
    }
    auto level = *max_element(levelList.begin(), levelList.end());
    GTEST_LOG_(INFO) << "level is:" << level;
    EXPECT_EQ(true, level == 4) << "ThermalMgrSystemTest012 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest012: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest013
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 1 ==> level 4
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest013, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest013: start.";
    int32_t batteryTemp = -10000 ;
    std::list<uint32_t> levelList;
    auto tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr)
    GTEST_LOG_(INFO) << "tms is nullptr";

    tms->SetSensorTemp(BATTERY, batteryTemp);
    sleep(WAIT_TIME * 10);
    auto levelMap = tms->GetPolicy()->GetClusterLevelMap();
    for (auto level : levelMap) {
        GTEST_LOG_(INFO) << "level is:" << level.second;
        levelList.push_back(level.second);
    }

    auto level = *max_element(levelList.begin(), levelList.end());
    levelList.clear();
    sleep(WAIT_TIME * 5);
    batteryTemp = -22000;
    tms->SetSensorTemp(BATTERY, batteryTemp);
    sleep(WAIT_TIME * 10);
    levelMap = tms->GetPolicy()->GetClusterLevelMap();
    for (auto level : levelMap) {
        GTEST_LOG_(INFO) << "level is:" << level.second;
        levelList.push_back(level.second);
    }

    level = *max_element(levelList.begin(), levelList.end());
    GTEST_LOG_(INFO) << "level is:" << level;
    EXPECT_EQ(true, level == 4) << "ThermalMgrSystemTest013 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest013: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest014
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 2 ==> level 4
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest014, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest014: start.";
    int32_t batteryTemp = -15000 ;
    std::list<uint32_t> levelList;
    auto tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr)
    GTEST_LOG_(INFO) << "tms is nullptr";

    tms->SetSensorTemp(BATTERY, batteryTemp);
    sleep(WAIT_TIME * 10);
    auto levelMap = tms->GetPolicy()->GetClusterLevelMap();
    for (auto level : levelMap) {
        GTEST_LOG_(INFO) << "level is:" << level.second;
        levelList.push_back(level.second);
    }

    auto level = *max_element(levelList.begin(), levelList.end());
    levelList.clear();
    sleep(WAIT_TIME * 5);
    batteryTemp = -22000;
    tms->SetSensorTemp(BATTERY, batteryTemp);
    sleep(WAIT_TIME * 10);
    levelMap = tms->GetPolicy()->GetClusterLevelMap();
    for (auto level : levelMap) {
        GTEST_LOG_(INFO) << "level is:" << level.second;
        levelList.push_back(level.second);
    }

    level = *max_element(levelList.begin(), levelList.end());
    GTEST_LOG_(INFO) << "level is:" << level;
    EXPECT_EQ(true, level == 4) << "ThermalMgrSystemTest014 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest014: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest015
 * @tc.desc: test level desc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 4 ===> level 1
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest015, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest015: start.";
    int32_t batteryTemp = -22000 ;
    std::list<uint32_t> levelList;
    auto tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr)
    GTEST_LOG_(INFO) << "tms is nullptr";

    tms->SetSensorTemp(BATTERY, batteryTemp);
    sleep(WAIT_TIME * 10);
    auto levelMap = tms->GetPolicy()->GetClusterLevelMap();
    for (auto level : levelMap) {
        GTEST_LOG_(INFO) << "level is:" << level.second;
        levelList.push_back(level.second);
    }

    auto level = *max_element(levelList.begin(), levelList.end());
    levelList.clear();
    sleep(WAIT_TIME * 5);
    batteryTemp = -10000;
    tms->SetSensorTemp(BATTERY, batteryTemp);
    sleep(WAIT_TIME * 10);
    levelMap = tms->GetPolicy()->GetClusterLevelMap();
    for (auto level : levelMap) {
        GTEST_LOG_(INFO) << "level is:" << level.second;
        levelList.push_back(level.second);
    }

    level = *max_element(levelList.begin(), levelList.end());
    GTEST_LOG_(INFO) << "level is:" << level;
    EXPECT_EQ(true, level == 1) << "ThermalMgrSystemTest015 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest015: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest016
 * @tc.desc: test level desc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 3 ===> level 0
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest016, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest016: start.";
    int32_t batteryTemp = -19100 ;
    std::list<uint32_t> levelList;
    auto tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr)
    GTEST_LOG_(INFO) << "tms is nullptr";

    tms->SetSensorTemp(BATTERY, batteryTemp);
    sleep(WAIT_TIME * 10);
    auto levelMap = tms->GetPolicy()->GetClusterLevelMap();
    for (auto level : levelMap) {
        GTEST_LOG_(INFO) << "level is:" << level.second;
        levelList.push_back(level.second);
    }

    auto level = *max_element(levelList.begin(), levelList.end());
    levelList.clear();
    sleep(WAIT_TIME * 5);
    batteryTemp = -4000;
    tms->SetSensorTemp(BATTERY, batteryTemp);
    sleep(WAIT_TIME * 10);
    levelMap = tms->GetPolicy()->GetClusterLevelMap();
    for (auto level : levelMap) {
        GTEST_LOG_(INFO) << "level is:" << level.second;
        levelList.push_back(level.second);
    }

    level = *max_element(levelList.begin(), levelList.end());
    GTEST_LOG_(INFO) << "level is:" << level;
    EXPECT_EQ(true, level == 0) << "ThermalMgrSystemTest008 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest008: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest017
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set PA temp, High Temp With Aux sensor
 * @tc.result level 1
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest017, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest017: start.";
    int32_t paTemp = 41000 ;
    int32_t ambientTemp = 10000;
    std::list<uint32_t> levelList;
    auto tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr)
    GTEST_LOG_(INFO) << "tms is nullptr";

    tms->SetSensorTemp(PA, paTemp);
    tms->SetSensorTemp(AMBIENT, ambientTemp);

    sleep(WAIT_TIME * 10);
    auto levelMap = tms->GetPolicy()->GetClusterLevelMap();
    for (auto level : levelMap) {
        GTEST_LOG_(INFO) << "level is:" << level.second;
        levelList.push_back(level.second);
    }
    auto level = *max_element(levelList.begin(), levelList.end());
    GTEST_LOG_(INFO) << "level is:" << level;
    EXPECT_EQ(true, level == 1) << "ThermalMgrSystemTest017 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest017: end.";
}


/**
 * @tc.name: ThermalMgrSystemTest018
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set PA temp, High Temp With Aux sensor
 * @tc.result level 1
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest018, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest018: start.";
    int32_t paTemp = 44000;
    int32_t ambientTemp = 10000;
    std::list<uint32_t> levelList;
    auto tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr)
    GTEST_LOG_(INFO) << "tms is nullptr";

    tms->SetSensorTemp(PA, paTemp);
    tms->SetSensorTemp(AMBIENT, ambientTemp);

    sleep(WAIT_TIME * 10);
    auto levelMap = tms->GetPolicy()->GetClusterLevelMap();
    for (auto level : levelMap) {
        GTEST_LOG_(INFO) << "level is:" << level.second;
        levelList.push_back(level.second);
    }
    auto level = *max_element(levelList.begin(), levelList.end());
    GTEST_LOG_(INFO) << "level is:" << level;
    EXPECT_EQ(true, level == 2) << "ThermalMgrSystemTest018 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest018: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest019
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set PA temp, High Temp With Aux sensor
 * @tc.result level 0
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest019, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest019: start.";
    int32_t paTemp = 44000;
    int32_t ambientTemp = 1000;
    std::list<uint32_t> levelList;
    auto tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr)
    GTEST_LOG_(INFO) << "tms is nullptr";

    tms->SetSensorTemp(PA, paTemp);
    tms->SetSensorTemp(AMBIENT, ambientTemp);

    sleep(WAIT_TIME * 10);
    auto levelMap = tms->GetPolicy()->GetClusterLevelMap();
    for (auto level : levelMap) {
        GTEST_LOG_(INFO) << "level is:" << level.second;
        levelList.push_back(level.second);
    }
    auto level = *max_element(levelList.begin(), levelList.end());
    GTEST_LOG_(INFO) << "level is:" << level;
    EXPECT_EQ(true, level == 0) << "ThermalMgrSystemTest019 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest019: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest020
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set PA temp, High Temp With Aux sensor
 * @tc.result level 1
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest020, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest020: start.";
    int32_t apTemp = 78000;
    int32_t ambientTemp = 1000;
    int32_t shellTemp = 50000;
    std::list<uint32_t> levelList;
    auto tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr)
    GTEST_LOG_(INFO) << "tms is nullptr";

    tms->SetSensorTemp(AP, apTemp);
    tms->SetSensorTemp(AMBIENT, ambientTemp);
    tms->SetSensorTemp(SHELL, shellTemp);

    sleep(WAIT_TIME * 10);
    auto levelMap = tms->GetPolicy()->GetClusterLevelMap();
    for (auto level : levelMap) {
        GTEST_LOG_(INFO) << "level is:" << level.second;
        levelList.push_back(level.second);
    }
    auto level = *max_element(levelList.begin(), levelList.end());
    GTEST_LOG_(INFO) << "level is:" << level;
    EXPECT_EQ(true, level == 1) << "ThermalMgrSystemTest020 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest020: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest021
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set PA temp, High Temp With Aux sensor
 * @tc.result level 0
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest021, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest021: start.";
    int32_t apTemp = 78000;
    int32_t ambientTemp = 1000;
    int32_t shellTemp = 0;
    std::list<uint32_t> levelList;
    auto tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr)
    GTEST_LOG_(INFO) << "tms is nullptr";

    tms->SetSensorTemp(AP, apTemp);
    tms->SetSensorTemp(AMBIENT, ambientTemp);
    tms->SetSensorTemp(SHELL, shellTemp);

    sleep(WAIT_TIME * 10);
    auto levelMap = tms->GetPolicy()->GetClusterLevelMap();
    for (auto level : levelMap) {
        GTEST_LOG_(INFO) << "level is:" << level.second;
        levelList.push_back(level.second);
    }
    auto level = *max_element(levelList.begin(), levelList.end());
    GTEST_LOG_(INFO) << "level is:" << level;
    EXPECT_EQ(true, level == 0) << "ThermalMgrSystemTest021 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest021: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest022
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state not satisfied
 * @tc.result level 1, freq 99000
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest022, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest022: start.";
    int32_t batteryTemp = 40100;
    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};

    snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, cpuFreqPath.c_str());
    auto tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr)
    GTEST_LOG_(INFO) << "tms is nullptr";

    tms->SetSensorTemp(BATTERY, batteryTemp);
    sleep(WAIT_TIME * 10);
    int32_t ret = ThermalMgrSystemTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrSystemTest022: Failed to read file ";
    }

    std::string freq = freqValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "freq is:" << value;
    EXPECT_EQ(true, value == 99000) << "ThermalMgrSystemTest022 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest022: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest023
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state not satisfied
 * @tc.result level 2, freq 90000
 */

HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest023, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest023: start.";
    int32_t batteryTemp = 43100;
    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};

    snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, cpuFreqPath.c_str());
    auto tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr)
    GTEST_LOG_(INFO) << "tms is nullptr";

    tms->SetSensorTemp(BATTERY, batteryTemp);
    sleep(WAIT_TIME * 10);
    int32_t ret = ThermalMgrSystemTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrSystemTest023: Failed to read file ";
    }

    std::string freq = freqValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "freq is:" << value;
    EXPECT_EQ(true, value == 90000) << "ThermalMgrSystemTest023 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest023: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest024
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state not satisfied
 * @tc.result level 3, freq 80000
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest024, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest024: start.";
    int32_t batteryTemp = 46100;
    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};

    snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, cpuFreqPath.c_str());
    auto tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr)
    GTEST_LOG_(INFO) << "tms is nullptr";

    tms->SetSensorTemp(BATTERY, batteryTemp);
    sleep(WAIT_TIME * 10);
    int32_t ret = ThermalMgrSystemTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrSystemTest024: Failed to read file ";
    }

    std::string freq = freqValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "freq is:" << value;
    EXPECT_EQ(true, value == 80000) << "ThermalMgrSystemTest024 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest024: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest025
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state not satisfied
 * @tc.result level 4, freq 80000
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest025, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest025: start.";
    int32_t batteryTemp = 48100;
    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};

    snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, cpuFreqPath.c_str());
    auto tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr)
    GTEST_LOG_(INFO) << "tms is nullptr";

    tms->SetSensorTemp(BATTERY, batteryTemp);
    sleep(WAIT_TIME * 10);
    int32_t ret = ThermalMgrSystemTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrSystemTest025: Failed to read file ";
    }

    std::string freq = freqValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "freq is:" << value;
    EXPECT_EQ(true, value == 80000) << "ThermalMgrSystemTest025 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest025: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest026
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: charge = 1, no scene
 * @tc.result level 1, freq 99000
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest026, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest026: start.";
    int32_t batteryTemp = 40100;
    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    std::string charge = "1";
    snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, cpuFreqPath.c_str());
    auto tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr)
    GTEST_LOG_(INFO) << "tms is nullptr";

    tms->SetSensorTemp(BATTERY, batteryTemp);
    auto stateMap = tms->GetStateMachineObj()->GetStateCollectionMap();
    auto chargerIter = stateMap.find(STATE_CHARGER);
    if (chargerIter != stateMap.end()) {
        chargerIter->second->SetState(charge);
    }

    sleep(WAIT_TIME * 10);
    int32_t ret = ThermalMgrSystemTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrSystemTest026: Failed to read file ";
    }

    std::string freq = freqValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "freq is:" << value;
    EXPECT_EQ(true, value == 99000) << "ThermalMgrSystemTest026 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest026: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest027
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: cam = 1, no scene
 * @tc.result level 1, freq 90000
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest027, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest027: start.";
    int32_t batteryTemp = 40100;
    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    std::string charge = "0";
    snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, cpuFreqPath.c_str());
    auto tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr)
    GTEST_LOG_(INFO) << "tms is nullptr";

    tms->SetSensorTemp(BATTERY, batteryTemp);
    auto stateMap = tms->GetStateMachineObj()->GetStateCollectionMap();
    auto iter = stateMap.find(STATE_CHARGER);
    if (iter != stateMap.end()) {
        iter->second->SetState(charge);
    }

    sleep(WAIT_TIME * 10);
    int32_t ret = ThermalMgrSystemTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrSystemTest027: Failed to read file ";
    }

    std::string freq = freqValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "freq is:" << value;
    EXPECT_EQ(true, value == 99000) << "ThermalMgrSystemTest027 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest027: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest028
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: cam = 1, scene = cam
 * @tc.result level 1, freq 80000
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest028, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest028: start.";
    int32_t batteryTemp = 40100;
    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    std::string charge = "0";
    snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, cpuFreqPath.c_str());
    auto tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr)
    GTEST_LOG_(INFO) << "tms is nullptr";

    tms->SetSensorTemp(BATTERY, batteryTemp);
    auto stateMap = tms->GetStateMachineObj()->GetStateCollectionMap();
    auto iter = stateMap.find(STATE_CHARGER);
    if (iter != stateMap.end()) {
        iter->second->SetState(charge);
    }
    auto scene = stateMap.find(STATE_SCNEN);
    if (scene != stateMap.end()) {
        scene->second->SetState(SCENE_CAMERA);
    }

    sleep(WAIT_TIME * 10);
    int32_t ret = ThermalMgrSystemTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrSystemTest028: Failed to read file ";
    }

    std::string freq = freqValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "freq is:" << value;
    EXPECT_EQ(true, value == 80000) << "ThermalMgrSystemTest028 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest028: end.";
}