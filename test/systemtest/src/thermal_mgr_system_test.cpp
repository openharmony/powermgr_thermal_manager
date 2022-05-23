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

using namespace OHOS::HiviewDFX;

static constexpr HiLogLabel LABEL = {LOG_CORE, 0, "ThermalMST"};


static bool StartThermalProtector()
{
    GTEST_LOG_(INFO) << "enter";
    FILE *fp = nullptr;
    fp = popen("/system/bin/thermal_protector&", "r");
    if (fp == nullptr) {
        HiLog::Error(LABEL, "popen function call failed.");
        return false;
    }

    pclose(fp);

    return true;
    GTEST_LOG_(INFO) << "return";
}

static bool StopThermalProtector()
{
    GTEST_LOG_(INFO) << "enter";
    FILE *fp = nullptr;
    fp = popen("kill -9 $(pidof thermal_protector)", "r");
    if (fp == nullptr) {
        HiLog::Error(LABEL, " popen function call failed.");
        return false;
    }

    pclose(fp);

    return true;
    GTEST_LOG_(INFO) << "return";
}

static bool CheckThermalProtectorPID()
{
    GTEST_LOG_(INFO) << "enter";
    FILE *fp = nullptr;
    fp = popen("pidof thermal_protector", "r");
    if (fp == nullptr) {
        HiLog::Error(LABEL, " popen function call failed.");
        return false;
    }
    char pid[BUFFER_SIZE];
    if (fgets(pid, sizeof(pid), fp) != nullptr) {
        pclose(fp);
        return true;
    }

    HiLog::Error(LABEL, "Getting Pid failed.");

    pclose(fp);

    return true;
    GTEST_LOG_(INFO) << "return";
}

int32_t ThermalMgrSystemTest::WriteFile(std::string path, std::string buf, size_t size)
{
    FILE *stream = fopen(path.c_str(), "w+");
    if (stream == nullptr) {
        return ERR_INVALID_VALUE;
    }
    size_t ret = fwrite(buf.c_str(), strlen(buf.c_str()), 1, stream);
    if (ret == ERR_OK) {
        THERMAL_HILOGE(COMP_SVC, "ret=%{public}zu", ret);
    }
    int32_t state = fseek(stream, 0, SEEK_SET);
    if (state != ERR_OK) {
        fclose(stream);
        return state;
    }
    state = fclose(stream);
    if (state != ERR_OK) {
        return state;
    }
    return ERR_OK;
}

int32_t ThermalMgrSystemTest::ReadFile(const char *path, char *buf, size_t size)
{
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

int32_t ThermalMgrSystemTest::InitNode()
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
        ret = snprintf_s(bufTemp, PATH_MAX, sizeof(bufTemp) - 1, SIMULATION_TEMP_DIR.c_str(), iter.first.c_str());
        if (ret < ERR_OK) {
            return ret;
        }
        std::string temp = std::to_string(iter.second) + "\n";
        WriteFile(bufTemp, temp, temp.length());
    }
    return ERR_OK;
}

int32_t ThermalMgrSystemTest::ConvertInt(const std::string &value)
{
    return std::stoi(value);
}

void ThermalMgrSystemTest::SetUpTestCase(void)
{
}

void ThermalMgrSystemTest::TearDownTestCase(void)
{
}

void ThermalMgrSystemTest::SetUp(void)
{
    InitNode();
}

void ThermalMgrSystemTest::TearDown(void)
{
    int32_t ret = -1;
    char stateChargeBuf[MAX_PATH] = {0};
    char stateSceneBuf[MAX_PATH] = {0};
    ret = snprintf_s(stateChargeBuf, PATH_MAX, sizeof(stateChargeBuf) - 1, stateChargePath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(stateSceneBuf, PATH_MAX, sizeof(stateSceneBuf) - 1, stateScenePath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    std::string chargeState = "0";
    ret = WriteFile(stateChargeBuf, chargeState, chargeState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string sceneState = "0";
    ret = WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
}

namespace {
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
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 1) << "ThermalMgrSystemTest001 failed";
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
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 2) << "ThermalMgrSystemTest002 failed";
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
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 3) << "ThermalMgrSystemTest003 failed";
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
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest003: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 48100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 4) << "ThermalMgrSystemTest005 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest004: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest005
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 1 ==> level 4
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest005, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest005: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 1) << "ThermalMgrSystemTest005 failed";
 
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    batteryTemp = 48100;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrSystemTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 4) << "ThermalMgrSystemTest005 failed";
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
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 2) << "ThermalMgrSystemTest006 failed";
 
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    batteryTemp = 48100;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrSystemTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 4) << "ThermalMgrSystemTest006 failed";
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
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 48200;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 4) << "ThermalMgrSystemTest007 failed";
 
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    batteryTemp = 40900;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrSystemTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value is:" << value;
    EXPECT_EQ(true, value == 1) << "ThermalMgrSystemTest007 failed";
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
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 3) << "ThermalMgrSystemTest008 failed";
 
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    batteryTemp = 37000;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrSystemTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value is:" << value;
    EXPECT_EQ(true, value == 0) << "ThermalMgrSystemTest008 failed";
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
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = -10000;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 1) << "ThermalMgrSystemTest009 failed";
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
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = -15000;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 2) << "ThermalMgrSystemTest010 failed";
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
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = -20100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 3) << "ThermalMgrSystemTest011 failed";
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
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = -22000;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 4) << "ThermalMgrSystemTest012 failed";
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
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = -10000;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 1) << "ThermalMgrSystemTest013 failed";

    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    batteryTemp = -22000;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrSystemTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value is:" << value;
    EXPECT_EQ(true, value == 4) << "ThermalMgrSystemTest013 failed";
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
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = -15000;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 2) << "ThermalMgrSystemTest014 failed";

    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    batteryTemp = -22000;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrSystemTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value is:" << value;
    EXPECT_EQ(true, value == 4) << "ThermalMgrSystemTest014 failed";
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
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = -22000;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 4) << "ThermalMgrSystemTest015 failed";
 
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    batteryTemp = -10000;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrSystemTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value is:" << value;
    EXPECT_EQ(true, value == 1) << "ThermalMgrSystemTest015 failed";
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
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = -19100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 3) << "ThermalMgrSystemTest016 failed";

    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    batteryTemp = -1000;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrSystemTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value is:" << value;
    EXPECT_EQ(true, value == 0) << "ThermalMgrSystemTest016 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest016: end.";
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
    int32_t ret = -1;
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(paTempBuf, PATH_MAX, sizeof(paTempBuf) - 1, paPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(amTempBuf, PATH_MAX, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    int32_t paTemp = 41000;
    std::string sTemp = to_string(paTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 10000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);
    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value is:" << value;
    EXPECT_EQ(true, value == 1) << "ThermalMgrSystemTest017 failed";
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
    int32_t ret = -1;
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(paTempBuf, PATH_MAX, sizeof(paTempBuf) - 1, paPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(amTempBuf, PATH_MAX, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    int32_t paTemp = 44000;
    std::string sTemp = to_string(paTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 10000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);
    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value is:" << value;
    EXPECT_EQ(true, value == 2) << "ThermalMgrSystemTest018 failed";
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
    int32_t ret = -1;
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(paTempBuf, PATH_MAX, sizeof(paTempBuf) - 1, paPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(amTempBuf, PATH_MAX, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    int32_t paTemp = 44000;
    std::string sTemp = to_string(paTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 1000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);
    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value is:" << value;
    EXPECT_EQ(true, value == 0) << "ThermalMgrSystemTest019 failed";
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
    ret = ThermalMgrSystemTest::WriteFile(apTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 1000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t shellTemp = 2000;
    sTemp = to_string(shellTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(shellTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);
    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value is:" << value;
    EXPECT_EQ(true, value == 1) << "ThermalMgrSystemTest020 failed";
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
    ret = ThermalMgrSystemTest::WriteFile(apTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 1000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t shellTemp = -100;
    sTemp = to_string(shellTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(shellTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);
    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value is:" << value;
    EXPECT_EQ(true, value == 0) << "ThermalMgrSystemTest021 failed";
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
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    ret = snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, CPU_FREQ_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string freq = freqValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "value:" << value;
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
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    ret = snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, CPU_FREQ_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string freq = freqValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "value:" << value;
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
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    ret = snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, CPU_FREQ_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string freq = freqValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "value:" << value;
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
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 48100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    ret = snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, CPU_FREQ_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string freq = freqValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "value:" << value;
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
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    char stateChargeBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(stateChargeBuf, PATH_MAX, sizeof(stateChargeBuf) - 1, stateChargePath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string chargeState = "1";
    ret = ThermalMgrSystemTest::WriteFile(stateChargeBuf, chargeState, chargeState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    ret = snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, CPU_FREQ_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string freq = freqValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 99000) << "ThermalMgrSystemTest026 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest026: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest027
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: charge = 1, scene = "cam"
 * @tc.result level 1, freq 80000
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest027, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest027: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    char stateChargeBuf[MAX_PATH] = {0};
    char stateSceneBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(stateChargeBuf, PATH_MAX, sizeof(stateChargeBuf) - 1, stateChargePath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(stateSceneBuf, PATH_MAX, sizeof(stateSceneBuf) - 1, stateScenePath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string chargeState = "1";
    ret = ThermalMgrSystemTest::WriteFile(stateChargeBuf, chargeState, chargeState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string sceneState = "cam";
    ret = ThermalMgrSystemTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    ret = snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, CPU_FREQ_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string freq = freqValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 99000) << "ThermalMgrSystemTest027 failed";
    sceneState = "null";
    ret = ThermalMgrSystemTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest027: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest028
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: charge = 0, scene = "cam"
 * @tc.result level 1, freq 90000
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest028, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest028: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    char stateChargeBuf[MAX_PATH] = {0};
    char stateSceneBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(stateChargeBuf, PATH_MAX, sizeof(stateChargeBuf) - 1, stateChargePath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(stateSceneBuf, PATH_MAX, sizeof(stateSceneBuf) - 1, stateScenePath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string chargeState = "0";
    ret = ThermalMgrSystemTest::WriteFile(stateChargeBuf, chargeState, chargeState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string sceneState = "cam";
    ret = ThermalMgrSystemTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    ret = snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, CPU_FREQ_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string freq = freqValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 99000) << "ThermalMgrSystemTest028 failed";
    sceneState = "null";
    ret = ThermalMgrSystemTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest028: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest029
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: charge = 1, no scene
 * @tc.result level 2, freq 90000
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest029, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest029: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    char stateChargeBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(stateChargeBuf, PATH_MAX, sizeof(stateChargeBuf) - 1, stateChargePath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string chargeState = "1";
    ret = ThermalMgrSystemTest::WriteFile(stateChargeBuf, chargeState, chargeState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    ret = snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, CPU_FREQ_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string freq = freqValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 90000) << "ThermalMgrSystemTest029 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest029: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest030
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: charge = 1, scene = "cam"
 * @tc.result level 2, freq 90000
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest030, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest030: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    char stateChargeBuf[MAX_PATH] = {0};
    char stateSceneBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(stateChargeBuf, PATH_MAX, sizeof(stateChargeBuf) - 1, stateChargePath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(stateSceneBuf, PATH_MAX, sizeof(stateSceneBuf) - 1, stateScenePath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string chargeState = "1";
    ret = ThermalMgrSystemTest::WriteFile(stateChargeBuf, chargeState, chargeState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string sceneState = "cam";
    ret = ThermalMgrSystemTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    ret = snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, CPU_FREQ_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string freq = freqValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 90000) << "ThermalMgrSystemTest030 failed";
    sceneState = "null";
    ret = ThermalMgrSystemTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest030: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest031
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: charge = 0, scene = "cam"
 * @tc.result level 2, freq 90000
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest031, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest031: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    char stateChargeBuf[MAX_PATH] = {0};
    char stateSceneBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(stateChargeBuf, PATH_MAX, sizeof(stateChargeBuf) - 1, stateChargePath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(stateSceneBuf, PATH_MAX, sizeof(stateSceneBuf) - 1, stateScenePath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string chargeState = "0";
    ret = ThermalMgrSystemTest::WriteFile(stateChargeBuf, chargeState, chargeState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string sceneState = "cam";
    ret = ThermalMgrSystemTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    ret = snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, CPU_FREQ_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string freq = freqValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 90000) << "ThermalMgrSystemTest031 failed";
    sceneState = "null";
    ret = ThermalMgrSystemTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest031: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest032
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: charge = 1, no scene
 * @tc.result level 3, freq 80000
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest032, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest032: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    char stateChargeBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(stateChargeBuf, PATH_MAX, sizeof(stateChargeBuf) - 1, stateChargePath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string chargeState = "1";
    ret = ThermalMgrSystemTest::WriteFile(stateChargeBuf, chargeState, chargeState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    ret = snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, CPU_FREQ_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string freq = freqValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 80000) << "ThermalMgrSystemTest032 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest032: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest033
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: charge = 1, scene = "cam"
 * @tc.result level 3, freq 80000
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest033, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest033: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    char stateChargeBuf[MAX_PATH] = {0};
    char stateSceneBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(stateChargeBuf, PATH_MAX, sizeof(stateChargeBuf) - 1, stateChargePath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(stateSceneBuf, PATH_MAX, sizeof(stateSceneBuf) - 1, stateScenePath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string chargeState = "1";
    ret = ThermalMgrSystemTest::WriteFile(stateChargeBuf, chargeState, chargeState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string sceneState = "cam";
    ret = ThermalMgrSystemTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    ret = snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, CPU_FREQ_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string freq = freqValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 80000) << "ThermalMgrSystemTest033 failed";
    sceneState = "null";
    ret = ThermalMgrSystemTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest033: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest034
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: charge = 0, scene = "cam"
 * @tc.result level 3, freq 80000
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest034, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest034: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    char stateChargeBuf[MAX_PATH] = {0};
    char stateSceneBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(stateChargeBuf, PATH_MAX, sizeof(stateChargeBuf) - 1, stateChargePath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(stateSceneBuf, PATH_MAX, sizeof(stateSceneBuf) - 1, stateScenePath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string chargeState = "0";
    ret = ThermalMgrSystemTest::WriteFile(stateChargeBuf, chargeState, chargeState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string sceneState = "cam";
    ret = ThermalMgrSystemTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    ret = snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, CPU_FREQ_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string freq = freqValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 80000) << "ThermalMgrSystemTest034 failed";
    sceneState = "null";
    ret = ThermalMgrSystemTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest034: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest035
 * @tc.desc: test get charge currentby setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state not satisfied
 * @tc.result level 1, current 1800
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest035, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest035: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char currentBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    ret = snprintf_s(currentBuf, PATH_MAX, sizeof(currentBuf) - 1, BATTERY_CHARGER_CURRENT_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(currentBuf, currentValue, sizeof(currentValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string current = currentValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(current);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 1800) << "ThermalMgrSystemTest035 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest035: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest036
 * @tc.desc: test get charge currentby setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: scene = "cam,call"
 * @tc.result level 1, current 1200
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest036, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest036: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    char stateSceneBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    ret = snprintf_s(stateSceneBuf, PATH_MAX, sizeof(stateSceneBuf) - 1, stateScenePath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    std::string sceneState = "cam,call";
    ret = ThermalMgrSystemTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    char currentBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    ret = snprintf_s(currentBuf, PATH_MAX, sizeof(currentBuf) - 1, BATTERY_CHARGER_CURRENT_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(currentBuf, currentValue, sizeof(currentValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string current = currentValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(current);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 1200) << "ThermalMgrSystemTest036 failed";

    sceneState = "null";
    ret = ThermalMgrSystemTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest036: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest037
 * @tc.desc: test get charge currentby setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state not satisfied
 * @tc.result level 2, current 1500
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest037, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest035: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char currentBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    ret = snprintf_s(currentBuf, PATH_MAX, sizeof(currentBuf) - 1, BATTERY_CHARGER_CURRENT_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(currentBuf, currentValue, sizeof(currentValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string current = currentValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(current);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 1500) << "ThermalMgrSystemTest037 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest037: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest038
 * @tc.desc: test get charge currentby setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: scene = "cam,call"
 * @tc.result level 2, current 1000
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest038, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest038: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    char stateSceneBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    ret = snprintf_s(stateSceneBuf, PATH_MAX, sizeof(stateSceneBuf) - 1, stateScenePath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    std::string sceneState = "cam,call";
    ret = ThermalMgrSystemTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 20);

    char currentBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    ret = snprintf_s(currentBuf, PATH_MAX, sizeof(currentBuf) - 1, BATTERY_CHARGER_CURRENT_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(currentBuf, currentValue, sizeof(currentValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string current = currentValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(current);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 1000) << "ThermalMgrSystemTest038 failed";

    sceneState = "null";
    ret = ThermalMgrSystemTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest038: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest039
 * @tc.desc: test get charge currentby setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state not satisfied
 * @tc.result level 3, current 1300
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest039, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest039: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char currentBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    ret = snprintf_s(currentBuf, PATH_MAX, sizeof(currentBuf) - 1, BATTERY_CHARGER_CURRENT_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(currentBuf, currentValue, sizeof(currentValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string current = currentValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(current);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 1300) << "ThermalMgrSystemTest039 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest039: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest040
 * @tc.desc: test get charge currentby setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: scene = "cam,call"
 * @tc.result level 3, current 800
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest040, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest040: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    char stateSceneBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    ret = snprintf_s(stateSceneBuf, PATH_MAX, sizeof(stateSceneBuf) - 1, stateScenePath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    std::string sceneState = "cam,call";
    ret = ThermalMgrSystemTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    char currentBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    ret = snprintf_s(currentBuf, PATH_MAX, sizeof(currentBuf) - 1, BATTERY_CHARGER_CURRENT_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(currentBuf, currentValue, sizeof(currentValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string current = currentValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(current);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 800) << "ThermalMgrSystemTest040 failed";

    sceneState = "null";
    ret = ThermalMgrSystemTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);

    GTEST_LOG_(INFO) << "ThermalMgrSystemTest040: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest041
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 1 current 1850
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest041, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest041: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = -10000;
    std::string sTemp = to_string(batteryTemp);
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char currentlBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    ret = snprintf_s(currentlBuf, PATH_MAX, sizeof(currentlBuf) - 1, BATTERY_CHARGER_CURRENT_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(currentlBuf, currentValue, sizeof(currentValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string current = currentValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(current);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 1850) << "ThermalMgrSystemTest041 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest041: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest042
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 2 current 1550
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest042, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest042: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = -14100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char currentlBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    ret = snprintf_s(currentlBuf, PATH_MAX, sizeof(currentlBuf) - 1, BATTERY_CHARGER_CURRENT_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(currentlBuf, currentValue, sizeof(currentValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string current = currentValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(current);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 1550) << "ThermalMgrSystemTest042 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest042: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest043
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 3 current 1150
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest043, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest043: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = -19100;
    std::string sTemp = to_string(batteryTemp);
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char currentlBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    ret = snprintf_s(currentlBuf, PATH_MAX, sizeof(currentlBuf) - 1, BATTERY_CHARGER_CURRENT_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(currentlBuf, currentValue, sizeof(currentValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string current = currentValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(current);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 1150) << "ThermalMgrSystemTest043 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest043: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest044
 * @tc.desc: test get brightness configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 1 brightness 120
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest044, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest044: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp);
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char lcdBuf[MAX_PATH] = {0};
    char lcdValue[MAX_PATH]= {0};
    ret = snprintf_s(lcdBuf, PATH_MAX, sizeof(lcdBuf) - 1, lcdPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string lcd = lcdValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(lcd);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 120) << "ThermalMgrSystemTest044 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest044: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest045
 * @tc.desc: test get brightness configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 2 brightness 120
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest045, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest045: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp);
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char lcdBuf[MAX_PATH] = {0};
    char lcdValue[MAX_PATH] = {0};
    ret = snprintf_s(lcdBuf, PATH_MAX, sizeof(lcdBuf) - 1, lcdPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string lcd = lcdValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(lcd);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 120) << "ThermalMgrSystemTest045 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest045: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest046
 * @tc.desc: test get brightness configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 3 brightness 120
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest046, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest046: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp);
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char lcdBuf[MAX_PATH] = {0};
    char lcdValue[MAX_PATH] = {0};
    ret = snprintf_s(lcdBuf, PATH_MAX, sizeof(lcdBuf) - 1, lcdPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string lcd = lcdValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(lcd);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 120) << "ThermalMgrSystemTest046 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest046: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest047
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set PA temp, High Temp With Aux sensor
 * @tc.result level 1 brightness 130
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest047, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest047: start.";
    int32_t ret = -1;
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(paTempBuf, PATH_MAX, sizeof(paTempBuf) - 1, paPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(amTempBuf, PATH_MAX, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    int32_t paTemp = 41000;
    std::string sTemp = to_string(paTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 10000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);
    char lcdBuf[MAX_PATH] = {0};
    char lcdValue[MAX_PATH] = {0};
    ret = snprintf_s(lcdBuf, PATH_MAX, sizeof(lcdBuf) - 1, lcdPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string lcd = lcdValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(lcd);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 130) << "ThermalMgrSystemTest047 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest047: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest048
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set PA temp, High Temp With Aux sensor
 * @tc.result level 2 brightness 100
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest048, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest047: start.";
    int32_t ret = -1;
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(paTempBuf, PATH_MAX, sizeof(paTempBuf) - 1, paPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(amTempBuf, PATH_MAX, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    int32_t paTemp = 44000;
    std::string sTemp = to_string(paTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 10000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);
    char lcdBuf[MAX_PATH] = {0};
    char lcdValue[MAX_PATH] = {0};
    ret = snprintf_s(lcdBuf, PATH_MAX, sizeof(lcdBuf) - 1, lcdPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string lcd = lcdValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(lcd);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 100) << "ThermalMgrSystemTest048 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest048: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest049
 * @tc.desc: get process and shutdown value
 * @tc.type: FEATURE
 * @tc.cond: Set AP temp, High Temp With Aux sensor
 * @tc.result level 1, process 0, shutdown 0
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest049, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest049: start.";
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
    ret = ThermalMgrSystemTest::WriteFile(apTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 1000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t shellTemp = 3000;
    sTemp = to_string(shellTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(shellTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);
    char procsessBuf[MAX_PATH] = {0};
    char procsesValue[MAX_PATH] = {0};
    ret = snprintf_s(procsessBuf, PATH_MAX, sizeof(procsessBuf) - 1, processPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(procsessBuf, procsesValue, sizeof(procsesValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string process = procsesValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(process);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 0) << "ThermalMgrSystemTest049 failed";

    char shutdownBuf[MAX_PATH] = {0};
    char shutdownValue[MAX_PATH] = {0};
    ret = snprintf_s(shutdownBuf, PATH_MAX, sizeof(shutdownBuf) - 1, shutdownPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(shutdownBuf, shutdownValue, sizeof(shutdownValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string shutdown = shutdownValue;
    value = ThermalMgrSystemTest::ConvertInt(shutdown);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 0) << "ThermalMgrSystemTest049 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest049: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest050
 * @tc.desc: test get process value by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 1 procss 1
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest050, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest044: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp);
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char procsessBuf[MAX_PATH] = {0};
    char procsesValue[MAX_PATH] = {0};
    ret = snprintf_s(procsessBuf, PATH_MAX, sizeof(procsessBuf) - 1, processPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(procsessBuf, procsesValue, sizeof(procsesValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string process = procsesValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(process);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 1) << "ThermalMgrSystemTest050 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest050: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest051
 * @tc.desc: test get process value by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 2 procss 1
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest051, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest051: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp);
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char procsessBuf[MAX_PATH] = {0};
    char procsesValue[MAX_PATH] = {0};
    ret = snprintf_s(procsessBuf, PATH_MAX, sizeof(procsessBuf) - 1, processPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(procsessBuf, procsesValue, sizeof(procsesValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string process = procsesValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(process);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 1) << "ThermalMgrSystemTest051 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest051: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest052
 * @tc.desc: test get process value by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 3 procss 1
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest052, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest052: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp);
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char procsessBuf[MAX_PATH] = {0};
    char procsesValue[MAX_PATH] = {0};
    ret = snprintf_s(procsessBuf, PATH_MAX, sizeof(procsessBuf) - 1, processPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(procsessBuf, procsesValue, sizeof(procsesValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string process = procsesValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(process);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 1) << "ThermalMgrSystemTest052 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest052: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest053
 * @tc.desc: test get process by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set PA temp, High Temp With Aux sensor
 * @tc.result level 1 process 2
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest053, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest053: start.";
    int32_t ret = -1;
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(paTempBuf, PATH_MAX, sizeof(paTempBuf) - 1, paPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(amTempBuf, PATH_MAX, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    int32_t paTemp = 41000;
    std::string sTemp = to_string(paTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 10000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);
    char procsessBuf[MAX_PATH] = {0};
    char procsesValue[MAX_PATH] = {0};
    ret = snprintf_s(procsessBuf, PATH_MAX, sizeof(procsessBuf) - 1, processPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(procsessBuf, procsesValue, sizeof(procsesValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string process = procsesValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(process);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 2) << "ThermalMgrSystemTest053 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest053: end.";
}

/**
 * @tc.name: ThermalMgrSystemTest054
 * @tc.desc: test get process by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set PA temp, High Temp With Aux sensor
 * @tc.result level 2 process 3
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest054, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest054: start.";
    int32_t ret = -1;
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(paTempBuf, PATH_MAX, sizeof(paTempBuf) - 1, paPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(amTempBuf, PATH_MAX, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    int32_t paTemp = 44000;
    std::string sTemp = to_string(paTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 10000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);
    char procsessBuf[MAX_PATH] = {0};
    char procsesValue[MAX_PATH] = {0};
    ret = snprintf_s(procsessBuf, PATH_MAX, sizeof(procsessBuf) - 1, processPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrSystemTest::ReadFile(procsessBuf, procsesValue, sizeof(procsesValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string process = procsesValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(process);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 3) << "ThermalMgrSystemTest054 failed";
    GTEST_LOG_(INFO) << "ThermalMgrSystemTest054: end.";
}

/*
 * Feature: Run thermal protect executable file.
 * Function: StartThermalProtector
 * CaseDescription:
 *
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest055, TestSize.Level0)
{
    HiLog::Info(LABEL, "ThermalModuleServiceTest006 start");
    if (!CheckThermalProtectorPID()) {
        EXPECT_EQ(true, StartThermalProtector());
    }
    sleep(WAIT_TIME);
    if (!CheckThermalProtectorPID()) {
        EXPECT_EQ(true, StopThermalProtector());
    }
    HiLog::Info(LABEL, "ThermalModuleServiceTest006 end");
}
}