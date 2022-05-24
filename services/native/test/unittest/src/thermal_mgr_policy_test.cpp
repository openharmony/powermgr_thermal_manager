/*
 * Copyright (c) 2020-2022 Huawei Device Co., Ltd.
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


#include "thermal_mgr_policy_test.h"

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

const int NUM_ZERO = 0;
static sptr<ThermalService> service;
static std::mutex g_mtx;

int32_t ThermalMgrPolicyTest::WriteFile(std::string path, std::string buf, size_t size)
{
    FILE *stream = fopen(path.c_str(), "w+");
    if (stream == nullptr) {
        return ERR_INVALID_VALUE;
    }
    size_t ret = fwrite(buf.c_str(), strlen(buf.c_str()), 1, stream);
    if (ret == ERR_OK) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "ret=%{public}zu", ret);
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

int32_t ThermalMgrPolicyTest::ReadSysfsFile(const char *path, char *buf, size_t size)
{
    int32_t readSize;
    int fd = open(path, O_RDONLY);
    if (fd < NUM_ZERO) {
        GTEST_LOG_(INFO) << "failed to open file node";
        return ERR_INVALID_VALUE;
    }

    readSize = read(fd, buf, size - 1);
    if (readSize < NUM_ZERO) {
        GTEST_LOG_(INFO) << "failed to read file";
        close(fd);
        return ERR_INVALID_VALUE;
    }

    buf[readSize] = '\0';
    Trim(buf);
    close(fd);

    return ERR_OK;
}

void ThermalMgrPolicyTest::Trim(char* str)
{
    if (str == nullptr) {
        return;
    }

    str[strcspn(str, "\n")] = 0;
}

int32_t ThermalMgrPolicyTest::ReadFile(const char* path, char* buf, size_t size)
{
    int32_t ret = ReadSysfsFile(path, buf, size);
    if (ret != NUM_ZERO) {
        GTEST_LOG_(INFO) << "failed to read file";
        return ret;
    }
    return ERR_OK;
}

int32_t ThermalMgrPolicyTest::ConvertInt(const std::string &value)
{
    if (IsNumericStr(value)) {
        return std::stoi(value);
    }
    return -1;
}

int32_t ThermalMgrPolicyTest::InitNode()
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
        std::string temp = std::to_string(iter.second);
        WriteFile(bufTemp, temp, temp.length());
    }
    return ERR_OK;
}

void ThermalMgrPolicyTest::SetUpTestCase(void)
{
}

void ThermalMgrPolicyTest::TearDownTestCase(void)
{
}

void ThermalMgrPolicyTest::SetUp(void)
{
    InitNode();
}

void ThermalMgrPolicyTest::TearDown(void)
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
 * @tc.name: ThermalMgrPolicyTest001
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 1
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest001, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest001: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 1) << "ThermalMgrPolicyTest001 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest001: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest002
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 2
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest002, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest002: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 2) << "ThermalMgrPolicyTest002 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest002: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest003
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 3
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest003, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest003: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 3) << "ThermalMgrPolicyTest003 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest003: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest004
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 4
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest004, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest003: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 48100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
    EXPECT_EQ(true, value == 4) << "ThermalMgrPolicyTest005 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest004: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest005
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 1 ==> level 4
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest005, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest005: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
    EXPECT_EQ(true, value == 1) << "ThermalMgrPolicyTest005 failed";
 
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    batteryTemp = 48100;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrPolicyTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 4) << "ThermalMgrPolicyTest005 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest005: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest006
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 2 ==> level 4
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest006, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest006: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
    EXPECT_EQ(true, value == 2) << "ThermalMgrPolicyTest006 failed";
 
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    batteryTemp = 48100;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrPolicyTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 4) << "ThermalMgrPolicyTest006 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest006: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest007
 * @tc.desc: test level desc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 4 ===> level 1
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest007, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest007: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 48200;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
    EXPECT_EQ(true, value == 4) << "ThermalMgrPolicyTest007 failed";
 
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    batteryTemp = 40900;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrPolicyTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value is:" << value;
    EXPECT_EQ(true, value == 1) << "ThermalMgrPolicyTest007 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest007: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest008
 * @tc.desc: test level desc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 3 ===> level 0
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest008, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest008: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
    EXPECT_EQ(true, value == 3) << "ThermalMgrPolicyTest008 failed";
 
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    batteryTemp = 37000;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrPolicyTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value is:" << value;
    EXPECT_EQ(true, value == 0) << "ThermalMgrPolicyTest008 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest008: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest009
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 1
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest009, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest009: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = -10000;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
    EXPECT_EQ(true, value == 1) << "ThermalMgrPolicyTest009 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest009: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest010
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 2
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest010, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest010: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = -15000;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
    EXPECT_EQ(true, value == 2) << "ThermalMgrPolicyTest010 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest010: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest011
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 3
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest011, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest011: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = -20100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
    EXPECT_EQ(true, value == 3) << "ThermalMgrPolicyTest011 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest011: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest012
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 4
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest012, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest012: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = -22000;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
    EXPECT_EQ(true, value == 4) << "ThermalMgrPolicyTest012 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest012: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest013
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 1 ==> level 4
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest013, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest013: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = -10000;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
    EXPECT_EQ(true, value == 1) << "ThermalMgrPolicyTest013 failed";

    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    batteryTemp = -22000;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrPolicyTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value is:" << value;
    EXPECT_EQ(true, value == 4) << "ThermalMgrPolicyTest013 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest013: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest014
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 2 ==> level 4
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest014, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest014: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = -15000;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
    EXPECT_EQ(true, value == 2) << "ThermalMgrPolicyTest014 failed";

    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    batteryTemp = -22000;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrPolicyTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value is:" << value;
    EXPECT_EQ(true, value == 4) << "ThermalMgrPolicyTest014 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest014: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest015
 * @tc.desc: test level desc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 4 ===> level 1
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest015, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest015: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = -22000;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
    EXPECT_EQ(true, value == 4) << "ThermalMgrPolicyTest015 failed";
 
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    batteryTemp = -10000;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrPolicyTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value is:" << value;
    EXPECT_EQ(true, value == 1) << "ThermalMgrPolicyTest015 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest015: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest016
 * @tc.desc: test level desc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp
 * @tc.result level 3 ===> level 0
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest016, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest016: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = -19100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
    EXPECT_EQ(true, value == 3) << "ThermalMgrPolicyTest016 failed";

    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    batteryTemp = -1000;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrPolicyTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value is:" << value;
    EXPECT_EQ(true, value == 0) << "ThermalMgrPolicyTest016 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest016: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest017
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set PA temp, High Temp With Aux sensor
 * @tc.result level 1
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest017, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest017: start.";
    int32_t ret = -1;
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(paTempBuf, PATH_MAX, sizeof(paTempBuf) - 1, paPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(amTempBuf, PATH_MAX, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    int32_t paTemp = 41000;
    std::string sTemp = to_string(paTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 10000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);
    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value is:" << value;
    EXPECT_EQ(true, value == 1) << "ThermalMgrPolicyTest017 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest017: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest018
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set PA temp, High Temp With Aux sensor
 * @tc.result level 1
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest018, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest018: start.";
    int32_t ret = -1;
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(paTempBuf, PATH_MAX, sizeof(paTempBuf) - 1, paPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(amTempBuf, PATH_MAX, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    int32_t paTemp = 44000;
    std::string sTemp = to_string(paTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 10000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);
    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value is:" << value;
    EXPECT_EQ(true, value == 2) << "ThermalMgrPolicyTest018 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest018: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest019
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set PA temp, High Temp With Aux sensor
 * @tc.result level 0
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest019, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest019: start.";
    int32_t ret = -1;
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(paTempBuf, PATH_MAX, sizeof(paTempBuf) - 1, paPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(amTempBuf, PATH_MAX, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    int32_t paTemp = 44000;
    std::string sTemp = to_string(paTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 1000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);
    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value is:" << value;
    EXPECT_EQ(true, value == 0) << "ThermalMgrPolicyTest019 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest019: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest020
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set PA temp, High Temp With Aux sensor
 * @tc.result level 1
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest020, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest020: start.";
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
    ret = ThermalMgrPolicyTest::WriteFile(apTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 1000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t shellTemp = 2000;
    sTemp = to_string(shellTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(shellTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);
    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value is:" << value;
    EXPECT_EQ(true, value == 1) << "ThermalMgrPolicyTest020 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest020: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest021
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set PA temp, High Temp With Aux sensor
 * @tc.result level 0
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest021, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest021: start.";
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
    ret = ThermalMgrPolicyTest::WriteFile(apTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 1000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t shellTemp = -100;
    sTemp = to_string(shellTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(shellTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);
    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, PATH_MAX, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
    GTEST_LOG_(INFO) << "value is:" << value;
    EXPECT_EQ(true, value == 0) << "ThermalMgrPolicyTest021 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest021: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest022
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state not satisfied
 * @tc.result level 1, freq 99000
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest022, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest022: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    ret = snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, CPU_FREQ_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string freq = freqValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 99000) << "ThermalMgrPolicyTest022 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest022: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest023
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state not satisfied
 * @tc.result level 2, freq 90000
 */

HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest023, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest023: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    ret = snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, CPU_FREQ_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string freq = freqValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 90000) << "ThermalMgrPolicyTest023 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest023: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest024
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state not satisfied
 * @tc.result level 3, freq 80000
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest024, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest024: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    ret = snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, CPU_FREQ_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string freq = freqValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 80000) << "ThermalMgrPolicyTest024 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest024: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest025
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state not satisfied
 * @tc.result level 4, freq 80000
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest025, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest025: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 48100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    ret = snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, CPU_FREQ_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string freq = freqValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 80000) << "ThermalMgrPolicyTest025 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest025: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest026
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: charge = 1, no scene
 * @tc.result level 1, freq 99000
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest026, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest026: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    char stateChargeBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(stateChargeBuf, PATH_MAX, sizeof(stateChargeBuf) - 1, stateChargePath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string chargeState = "1";
    ret = ThermalMgrPolicyTest::WriteFile(stateChargeBuf, chargeState, chargeState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    ret = snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, CPU_FREQ_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string freq = freqValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 99000) << "ThermalMgrPolicyTest026 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest026: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest027
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: charge = 1, scene = "cam"
 * @tc.result level 1, freq 80000
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest027, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest027: start.";
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
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string chargeState = "1";
    ret = ThermalMgrPolicyTest::WriteFile(stateChargeBuf, chargeState, chargeState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string sceneState = "cam";
    ret = ThermalMgrPolicyTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    ret = snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, CPU_FREQ_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string freq = freqValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 80000) << "ThermalMgrPolicyTest027 failed";
    sceneState = "null";
    ret = ThermalMgrPolicyTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest027: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest028
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: charge = 0, scene = "cam"
 * @tc.result level 1, freq 90000
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest028, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest028: start.";
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
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string chargeState = "0";
    ret = ThermalMgrPolicyTest::WriteFile(stateChargeBuf, chargeState, chargeState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string sceneState = "cam";
    ret = ThermalMgrPolicyTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    ret = snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, CPU_FREQ_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string freq = freqValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 90000) << "ThermalMgrPolicyTest028 failed";
    sceneState = "null";
    ret = ThermalMgrPolicyTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest028: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest029
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: charge = 1, no scene
 * @tc.result level 2, freq 90000
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest029, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest029: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    char stateChargeBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(stateChargeBuf, PATH_MAX, sizeof(stateChargeBuf) - 1, stateChargePath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string chargeState = "1";
    ret = ThermalMgrPolicyTest::WriteFile(stateChargeBuf, chargeState, chargeState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    ret = snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, CPU_FREQ_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string freq = freqValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 90000) << "ThermalMgrPolicyTest029 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest029: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest030
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: charge = 1, scene = "cam"
 * @tc.result level 2, freq 70000
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest030, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest030: start.";
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
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string chargeState = "1";
    ret = ThermalMgrPolicyTest::WriteFile(stateChargeBuf, chargeState, chargeState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string sceneState = "cam";
    ret = ThermalMgrPolicyTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    ret = snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, CPU_FREQ_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string freq = freqValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 70000) << "ThermalMgrPolicyTest030 failed";
    sceneState = "null";
    ret = ThermalMgrPolicyTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest030: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest031
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: charge = 0, scene = "cam"
 * @tc.result level 2, freq 80000
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest031, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest031: start.";
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
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string chargeState = "0";
    ret = ThermalMgrPolicyTest::WriteFile(stateChargeBuf, chargeState, chargeState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string sceneState = "cam";
    ret = ThermalMgrPolicyTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    ret = snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, CPU_FREQ_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string freq = freqValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 80000) << "ThermalMgrPolicyTest031 failed";
    sceneState = "null";
    ret = ThermalMgrPolicyTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest031: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest032
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: charge = 1, no scene
 * @tc.result level 3, freq 80000
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest032, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest032: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    char stateChargeBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(stateChargeBuf, PATH_MAX, sizeof(stateChargeBuf) - 1, stateChargePath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string chargeState = "1";
    ret = ThermalMgrPolicyTest::WriteFile(stateChargeBuf, chargeState, chargeState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    ret = snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, CPU_FREQ_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string freq = freqValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 80000) << "ThermalMgrPolicyTest032 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest032: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest033
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: charge = 1, scene = "cam"
 * @tc.result level 3, freq 60000
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest033, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest033: start.";
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
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string chargeState = "1";
    ret = ThermalMgrPolicyTest::WriteFile(stateChargeBuf, chargeState, chargeState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string sceneState = "cam";
    ret = ThermalMgrPolicyTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    ret = snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, CPU_FREQ_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string freq = freqValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 60000) << "ThermalMgrPolicyTest033 failed";
    sceneState = "null";
    ret = ThermalMgrPolicyTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest033: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest034
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: charge = 0, scene = "cam"
 * @tc.result level 3, freq 80000
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest034, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest034: start.";
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
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string chargeState = "0";
    ret = ThermalMgrPolicyTest::WriteFile(stateChargeBuf, chargeState, chargeState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string sceneState = "cam";
    ret = ThermalMgrPolicyTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    ret = snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, CPU_FREQ_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string freq = freqValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(freq);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 70000) << "ThermalMgrPolicyTest034 failed";
    sceneState = "null";
    ret = ThermalMgrPolicyTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest034: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest035
 * @tc.desc: test get charge currentby setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state not satisfied
 * @tc.result level 1, current 1800
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest035, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest035: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char currentBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    ret = snprintf_s(currentBuf, PATH_MAX, sizeof(currentBuf) - 1, BATTERY_CHARGER_CURRENT_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(currentBuf, currentValue, sizeof(currentValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string current = currentValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(current);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 1800) << "ThermalMgrPolicyTest035 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest035: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest036
 * @tc.desc: test get charge currentby setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: scene = "cam,call"
 * @tc.result level 1, current 1200
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest036, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest036: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    char stateSceneBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    ret = snprintf_s(stateSceneBuf, PATH_MAX, sizeof(stateSceneBuf) - 1, stateScenePath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    std::string sceneState = "cam,call";
    ret = ThermalMgrPolicyTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    char currentBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    ret = snprintf_s(currentBuf, PATH_MAX, sizeof(currentBuf) - 1, BATTERY_CHARGER_CURRENT_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(currentBuf, currentValue, sizeof(currentValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string current = currentValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(current);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 1200) << "ThermalMgrPolicyTest036 failed";

    sceneState = "null";
    ret = ThermalMgrPolicyTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest036: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest037
 * @tc.desc: test get charge currentby setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state not satisfied
 * @tc.result level 2, current 1500
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest037, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest035: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char currentBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    ret = snprintf_s(currentBuf, PATH_MAX, sizeof(currentBuf) - 1, BATTERY_CHARGER_CURRENT_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(currentBuf, currentValue, sizeof(currentValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string current = currentValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(current);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 1500) << "ThermalMgrPolicyTest037 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest037: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest038
 * @tc.desc: test get charge currentby setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: scene = "cam,call"
 * @tc.result level 2, current 1000
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest038, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest038: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    char stateSceneBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    ret = snprintf_s(stateSceneBuf, PATH_MAX, sizeof(stateSceneBuf) - 1, stateScenePath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    std::string sceneState = "cam,call";
    ret = ThermalMgrPolicyTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    char currentBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    ret = snprintf_s(currentBuf, PATH_MAX, sizeof(currentBuf) - 1, BATTERY_CHARGER_CURRENT_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(currentBuf, currentValue, sizeof(currentValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string current = currentValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(current);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 1000) << "ThermalMgrPolicyTest038 failed";

    sceneState = "null";
    ret = ThermalMgrPolicyTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest038: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest039
 * @tc.desc: test get charge currentby setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state not satisfied
 * @tc.result level 3, current 1300
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest039, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest039: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char currentBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    ret = snprintf_s(currentBuf, PATH_MAX, sizeof(currentBuf) - 1, BATTERY_CHARGER_CURRENT_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(currentBuf, currentValue, sizeof(currentValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string current = currentValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(current);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 1300) << "ThermalMgrPolicyTest039 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest039: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest040
 * @tc.desc: test get charge currentby setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: scene = "cam,call"
 * @tc.result level 3, current 800
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest040, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest040: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    char stateSceneBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    ret = snprintf_s(stateSceneBuf, PATH_MAX, sizeof(stateSceneBuf) - 1, stateScenePath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    std::string sceneState = "cam,call";
    ret = ThermalMgrPolicyTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(WAIT_TIME * 10);

    char currentBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    ret = snprintf_s(currentBuf, PATH_MAX, sizeof(currentBuf) - 1, BATTERY_CHARGER_CURRENT_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(currentBuf, currentValue, sizeof(currentValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string current = currentValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(current);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 800) << "ThermalMgrPolicyTest040 failed";

    sceneState = "null";
    ret = ThermalMgrPolicyTest::WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);

    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest040: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest041
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 1 current 1850
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest041, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest041: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = -10000;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char currentlBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    ret = snprintf_s(currentlBuf, PATH_MAX, sizeof(currentlBuf) - 1, BATTERY_CHARGER_CURRENT_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(currentlBuf, currentValue, sizeof(currentValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string current = currentValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(current);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 1850) << "ThermalMgrPolicyTest041 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest041: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest042
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 2 current 1550
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest042, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest042: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = -14100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char currentlBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    ret = snprintf_s(currentlBuf, PATH_MAX, sizeof(currentlBuf) - 1, BATTERY_CHARGER_CURRENT_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(currentlBuf, currentValue, sizeof(currentValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string current = currentValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(current);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 1550) << "ThermalMgrPolicyTest042 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest042: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest043
 * @tc.desc: test get current configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 3 current 1150
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest043, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest043: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = -19100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char currentlBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    ret = snprintf_s(currentlBuf, PATH_MAX, sizeof(currentlBuf) - 1, BATTERY_CHARGER_CURRENT_PATH.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(currentlBuf, currentValue, sizeof(currentValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string current = currentValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(current);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 1150) << "ThermalMgrPolicyTest043 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest043: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest044
 * @tc.desc: test get brightness configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 1 brightness 188
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest044, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest044: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char lcdBuf[MAX_PATH] = {0};
    char lcdValue[MAX_PATH]= {0};
    ret = snprintf_s(lcdBuf, PATH_MAX, sizeof(lcdBuf) - 1, lcdPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string lcd = lcdValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(lcd);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 188) << "ThermalMgrPolicyTest044 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest044: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest045
 * @tc.desc: test get brightness configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 2 brightness 155
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest045, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest045: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char lcdBuf[MAX_PATH] = {0};
    char lcdValue[MAX_PATH] = {0};
    ret = snprintf_s(lcdBuf, PATH_MAX, sizeof(lcdBuf) - 1, lcdPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string lcd = lcdValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(lcd);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 155) << "ThermalMgrPolicyTest045 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest045: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest046
 * @tc.desc: test get brightness configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 3 brightness 120
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest046, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest046: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char lcdBuf[MAX_PATH] = {0};
    char lcdValue[MAX_PATH] = {0};
    ret = snprintf_s(lcdBuf, PATH_MAX, sizeof(lcdBuf) - 1, lcdPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string lcd = lcdValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(lcd);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 120) << "ThermalMgrPolicyTest046 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest046: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest047
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set PA temp, High Temp With Aux sensor
 * @tc.result level 1 brightness 130
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest047, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest047: start.";
    int32_t ret = -1;
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(paTempBuf, PATH_MAX, sizeof(paTempBuf) - 1, paPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(amTempBuf, PATH_MAX, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    int32_t paTemp = 41000;
    std::string sTemp = to_string(paTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 10000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);
    char lcdBuf[MAX_PATH] = {0};
    char lcdValue[MAX_PATH] = {0};
    ret = snprintf_s(lcdBuf, PATH_MAX, sizeof(lcdBuf) - 1, lcdPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string lcd = lcdValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(lcd);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 130) << "ThermalMgrPolicyTest047 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest047: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest048
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set PA temp, High Temp With Aux sensor
 * @tc.result level 2 brightness 100
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest048, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest047: start.";
    int32_t ret = -1;
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(paTempBuf, PATH_MAX, sizeof(paTempBuf) - 1, paPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(amTempBuf, PATH_MAX, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    int32_t paTemp = 44000;
    std::string sTemp = to_string(paTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 10000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);
    char lcdBuf[MAX_PATH] = {0};
    char lcdValue[MAX_PATH] = {0};
    ret = snprintf_s(lcdBuf, PATH_MAX, sizeof(lcdBuf) - 1, lcdPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string lcd = lcdValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(lcd);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 100) << "ThermalMgrPolicyTest048 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest048: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest049
 * @tc.desc: get process and shutdown value
 * @tc.type: FEATURE
 * @tc.cond: Set AP temp, High Temp With Aux sensor
 * @tc.result level 1, process 3, shutdown 1
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest049, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest049: start.";
    int32_t ret = -1;
    char apTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    char shellTempBuf[MAX_PATH] = {0};
    char stateScreenBuf[MAX_PATH] = {0};
    ret = snprintf_s(apTempBuf, PATH_MAX, sizeof(apTempBuf) - 1, apPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(amTempBuf, PATH_MAX, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(shellTempBuf, PATH_MAX, sizeof(shellTempBuf) - 1, shellPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(stateScreenBuf, PATH_MAX, sizeof(stateScreenBuf) - 1, stateScreenPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    int32_t apTemp = 78000;
    std::string sTemp = to_string(apTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(apTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 1000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t shellTemp = 3000;
    sTemp = to_string(shellTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(shellTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t stateScreen = 1;
    sTemp = to_string(stateScreen);
    ret = ThermalMgrPolicyTest::WriteFile(stateScreenBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 3);
    char procsessBuf[MAX_PATH] = {0};
    char procsesValue[MAX_PATH] = {0};
    ret = snprintf_s(procsessBuf, PATH_MAX, sizeof(procsessBuf) - 1, processPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(procsessBuf, procsesValue, sizeof(procsesValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string process = procsesValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(process);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 3) << "ThermalMgrPolicyTest049 failed";

    char shutdownBuf[MAX_PATH] = {0};
    char shutdownValue[MAX_PATH] = {0};
    ret = snprintf_s(shutdownBuf, PATH_MAX, sizeof(shutdownBuf) - 1, shutdownPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(shutdownBuf, shutdownValue, sizeof(shutdownValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string shutdown = shutdownValue;
    value = ThermalMgrPolicyTest::ConvertInt(shutdown);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 0) << "ThermalMgrPolicyTest049 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest049: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest050
 * @tc.desc: test get process value by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 1 procss 1
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest050, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest050: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char procsessBuf[MAX_PATH] = {0};
    char procsesValue[MAX_PATH] = {0};
    ret = snprintf_s(procsessBuf, PATH_MAX, sizeof(procsessBuf) - 1, processPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(procsessBuf, procsesValue, sizeof(procsesValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string process = procsesValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(process);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 3) << "ThermalMgrPolicyTest050 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest050: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest051
 * @tc.desc: test get process value by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 2 procss 2
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest051, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest051: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char procsessBuf[MAX_PATH] = {0};
    char procsesValue[MAX_PATH] = {0};
    ret = snprintf_s(procsessBuf, PATH_MAX, sizeof(procsessBuf) - 1, processPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(procsessBuf, procsesValue, sizeof(procsesValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string process = procsesValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(process);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 2) << "ThermalMgrPolicyTest051 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest051: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest052
 * @tc.desc: test get process value by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 3 procss 1
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest052, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest052: start.";
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, PATH_MAX, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);

    char procsessBuf[MAX_PATH] = {0};
    char procsesValue[MAX_PATH] = {0};
    ret = snprintf_s(procsessBuf, PATH_MAX, sizeof(procsessBuf) - 1, processPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(procsessBuf, procsesValue, sizeof(procsesValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string process = procsesValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(process);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 1) << "ThermalMgrPolicyTest052 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest052: end.";
}

/**
 * @tc.name: ThermalMgrPolicyTest053
 * @tc.desc: test get process by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set PA temp, High Temp With Aux sensor
 * @tc.result level 1 process 2
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest053, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest053: start.";
    int32_t ret = -1;
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(paTempBuf, PATH_MAX, sizeof(paTempBuf) - 1, paPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(amTempBuf, PATH_MAX, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    int32_t paTemp = 41000;
    std::string sTemp = to_string(paTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 10000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);
    char procsessBuf[MAX_PATH] = {0};
    char procsesValue[MAX_PATH] = {0};
    ret = snprintf_s(procsessBuf, PATH_MAX, sizeof(procsessBuf) - 1, processPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(procsessBuf, procsesValue, sizeof(procsesValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string process = procsesValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(process);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 2) << "ThermalMgrPolicyTest053 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest053: end.";
}
/**
 * @tc.name: ThermalMgrPolicyTest054
 * @tc.desc: test get process by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set PA temp, High Temp With Aux sensor
 * @tc.result level 2 process 3
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest054, Function|MediumTest|Level2)
{
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest054: start.";
    int32_t ret = -1;
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(paTempBuf, PATH_MAX, sizeof(paTempBuf) - 1, paPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = snprintf_s(amTempBuf, PATH_MAX, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);

    int32_t paTemp = 44000;
    std::string sTemp = to_string(paTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 10000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME * 10);
    char procsessBuf[MAX_PATH] = {0};
    char procsesValue[MAX_PATH] = {0};
    ret = snprintf_s(procsessBuf, PATH_MAX, sizeof(procsessBuf) - 1, processPath.c_str());
    EXPECT_EQ(true, ret >= ERR_OK);
    ret = ThermalMgrPolicyTest::ReadFile(procsessBuf, procsesValue, sizeof(procsesValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string process = procsesValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(process);
    GTEST_LOG_(INFO) << "value:" << value;
    EXPECT_EQ(true, value == 3) << "ThermalMgrPolicyTest054 failed";
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest054: end.";
}
}