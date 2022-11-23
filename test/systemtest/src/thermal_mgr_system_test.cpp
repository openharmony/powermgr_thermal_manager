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
#include "thermal_log.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
static sptr<ThermalService> service;
static std::mutex g_mtx;

using namespace OHOS::HiviewDFX;

static constexpr HiLogLabel LABEL = {LOG_CORE, 0, "ThermalMST"};

static bool StartThermalProtector()
{
    THERMAL_HILOGD(LABEL_TEST, "enter");
    FILE *fp = nullptr;
    fp = popen("/system/bin/thermal_protector&", "r");
    if (fp == nullptr) {
        HiLog::Error(LABEL, "popen function call failed.");
        return false;
    }

    pclose(fp);

    return true;
    THERMAL_HILOGD(LABEL_TEST, "return");
}

static bool StopThermalProtector()
{
    THERMAL_HILOGD(LABEL_TEST, "enter");
    FILE *fp = nullptr;
    fp = popen("kill -9 $(pidof thermal_protector)", "r");
    if (fp == nullptr) {
        HiLog::Error(LABEL, " popen function call failed.");
        return false;
    }

    pclose(fp);

    return true;
    THERMAL_HILOGD(LABEL_TEST, "return");
}

static bool CheckThermalProtectorPID()
{
    THERMAL_HILOGD(LABEL_TEST, "enter");
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
    THERMAL_HILOGD(LABEL_TEST, "return");
}
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

    int32_t fd = open(path, O_RDONLY, S_IRUSR | S_IRGRP | S_IROTH);
    if (fd < ERR_OK) {
        THERMAL_HILOGD(LABEL_TEST, "WriteFile: failed to open file fd: %{public}d", fd);
        return ERR_INVALID_VALUE;
    }

    ret = read(fd, buf, size);
    if (ret < ERR_OK) {
        THERMAL_HILOGD(LABEL_TEST, "WriteFile: failed to read file ret: %{public}d", ret);
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
        ret = snprintf_s(bufTemp, MAX_PATH, sizeof(bufTemp) - 1, SIMULATION_TEMP_DIR, iter.first.c_str());
        if (ret < EOK) {
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

void ThermalMgrSystemTest::SetUpTestCase()
{
}

void ThermalMgrSystemTest::TearDownTestCase()
{
}

void ThermalMgrSystemTest::SetUp()
{
    InitNode();
}

void ThermalMgrSystemTest::TearDown()
{
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest001: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(SLEEP_INTERVAL_SEC);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
    EXPECT_EQ(true, value == 1) << "ThermalMgrSystemTest001 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest001: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest002: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(SLEEP_INTERVAL_SEC);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
    EXPECT_EQ(true, value == 2) << "ThermalMgrSystemTest002 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest002: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest003: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(SLEEP_INTERVAL_SEC);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
    EXPECT_EQ(true, value == 3) << "ThermalMgrSystemTest003 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest003: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest003: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 48100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(SLEEP_INTERVAL_SEC);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 4) << "ThermalMgrSystemTest005 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest004: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest005: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(SLEEP_INTERVAL_SEC);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 1) << "ThermalMgrSystemTest005 failed";
 
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    batteryTemp = 48100;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(SLEEP_INTERVAL_SEC);

    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrSystemTest::ConvertInt(level);
    THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
    EXPECT_EQ(true, value == 4) << "ThermalMgrSystemTest005 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest005: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest006: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(SLEEP_INTERVAL_SEC);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 2) << "ThermalMgrSystemTest006 failed";
 
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    batteryTemp = 48100;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(SLEEP_INTERVAL_SEC);

    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrSystemTest::ConvertInt(level);
    THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
    EXPECT_EQ(true, value == 4) << "ThermalMgrSystemTest006 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest006: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest007: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 48200;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(SLEEP_INTERVAL_SEC);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 4) << "ThermalMgrSystemTest007 failed";
 
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    batteryTemp = 40900;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(SLEEP_INTERVAL_SEC);

    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrSystemTest::ConvertInt(level);
    THERMAL_HILOGD(LABEL_TEST, "value is: %{public}d", value);
    EXPECT_EQ(true, value == 1) << "ThermalMgrSystemTest007 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest007: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest008: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(SLEEP_INTERVAL_SEC);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 3) << "ThermalMgrSystemTest008 failed";
 
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    batteryTemp = 37000;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(SLEEP_INTERVAL_SEC);

    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrSystemTest::ConvertInt(level);
    THERMAL_HILOGD(LABEL_TEST, "value is: %{public}d", value);
    EXPECT_EQ(true, value == 0) << "ThermalMgrSystemTest008 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest008: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest009: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = -10000;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(SLEEP_INTERVAL_SEC);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 1) << "ThermalMgrSystemTest009 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest009: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest010: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = -15000;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(SLEEP_INTERVAL_SEC);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 2) << "ThermalMgrSystemTest010 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest010: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest011: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = -20100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(SLEEP_INTERVAL_SEC);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 3) << "ThermalMgrSystemTest011 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest011: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest012: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = -22000;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(SLEEP_INTERVAL_SEC);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 4) << "ThermalMgrSystemTest012 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest012: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest013: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = -10000;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(SLEEP_INTERVAL_SEC);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 1) << "ThermalMgrSystemTest013 failed";

    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    batteryTemp = -22000;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(SLEEP_INTERVAL_SEC);

    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrSystemTest::ConvertInt(level);
    THERMAL_HILOGD(LABEL_TEST, "value is: %{public}d", value);
    EXPECT_EQ(true, value == 4) << "ThermalMgrSystemTest013 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest013: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest014: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = -15000;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(SLEEP_INTERVAL_SEC);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 2) << "ThermalMgrSystemTest014 failed";

    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    batteryTemp = -22000;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(SLEEP_INTERVAL_SEC);

    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrSystemTest::ConvertInt(level);
    THERMAL_HILOGD(LABEL_TEST, "value is: %{public}d", value);
    EXPECT_EQ(true, value == 4) << "ThermalMgrSystemTest014 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest014: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest015: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = -22000;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(SLEEP_INTERVAL_SEC);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 4) << "ThermalMgrSystemTest015 failed";
 
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    batteryTemp = -10000;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(SLEEP_INTERVAL_SEC);

    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrSystemTest::ConvertInt(level);
    THERMAL_HILOGD(LABEL_TEST, "value is: %{public}d", value);
    EXPECT_EQ(true, value == 1) << "ThermalMgrSystemTest015 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest015: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest016: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = -19100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(SLEEP_INTERVAL_SEC);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 3) << "ThermalMgrSystemTest016 failed";

    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    batteryTemp = -1000;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    sleep(SLEEP_INTERVAL_SEC);

    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrSystemTest::ConvertInt(level);
    THERMAL_HILOGD(LABEL_TEST, "value is: %{public}d", value);
    EXPECT_EQ(true, value == 0) << "ThermalMgrSystemTest016 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest016: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest017: start.");
    int32_t ret = -1;
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(paTempBuf, MAX_PATH, sizeof(paTempBuf) - 1, paPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(amTempBuf, MAX_PATH, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= EOK);

    int32_t paTemp = 41000;
    std::string sTemp = to_string(paTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 10000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(SLEEP_INTERVAL_SEC);
    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    THERMAL_HILOGD(LABEL_TEST, "value is: %{public}d", value);
    EXPECT_EQ(true, value == 1) << "ThermalMgrSystemTest017 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest017: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest018: start.");
    int32_t ret = -1;
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(paTempBuf, MAX_PATH, sizeof(paTempBuf) - 1, paPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(amTempBuf, MAX_PATH, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= EOK);

    int32_t paTemp = 44000;
    std::string sTemp = to_string(paTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 10000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(SLEEP_INTERVAL_SEC);
    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    THERMAL_HILOGD(LABEL_TEST, "value is: %{public}d", value);
    EXPECT_EQ(true, value == 2) << "ThermalMgrSystemTest018 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest018: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest019: start.");
    int32_t ret = -1;
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(paTempBuf, MAX_PATH, sizeof(paTempBuf) - 1, paPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(amTempBuf, MAX_PATH, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= EOK);

    int32_t paTemp = 44000;
    std::string sTemp = to_string(paTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 1000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(SLEEP_INTERVAL_SEC);
    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    THERMAL_HILOGD(LABEL_TEST, "value is: %{public}d", value);
    EXPECT_EQ(true, value == 0) << "ThermalMgrSystemTest019 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest019: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest020: start.");
    int32_t ret = -1;
    char apTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    char shellTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(apTempBuf, MAX_PATH, sizeof(apTempBuf) - 1, apPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(amTempBuf, MAX_PATH, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(shellTempBuf, MAX_PATH, sizeof(shellTempBuf) - 1, shellPath.c_str());
    EXPECT_EQ(true, ret >= EOK);

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

    sleep(SLEEP_INTERVAL_SEC);
    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    THERMAL_HILOGD(LABEL_TEST, "value is: %{public}d", value);
    EXPECT_EQ(true, value == 1) << "ThermalMgrSystemTest020 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest020: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest021: start.");
    int32_t ret = -1;
    char apTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    char shellTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(apTempBuf, MAX_PATH, sizeof(apTempBuf) - 1, apPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(amTempBuf, MAX_PATH, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(shellTempBuf, MAX_PATH, sizeof(shellTempBuf) - 1, shellPath.c_str());
    EXPECT_EQ(true, ret >= EOK);

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

    sleep(SLEEP_INTERVAL_SEC);
    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(level);
    THERMAL_HILOGD(LABEL_TEST, "value is: %{public}d", value);
    EXPECT_EQ(true, value == 0) << "ThermalMgrSystemTest021 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest021: end.");
}

/**
 * @tc.name: ThermalMgrSystemTest022
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state not satisfied
 * @tc.result level 1, freq 1992000
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest022, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest022: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(SLEEP_INTERVAL_SEC);

    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    ret = snprintf_s(cpuBuf, MAX_PATH, sizeof(cpuBuf) - 1, CPU_FREQ_PATH);
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string freq = freqValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(freq);
    THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
    EXPECT_EQ(true, value == 1992000 || value == 1991500 || value == 1991200) << "ThermalMgrSystemTest022 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest022: end.");
}

/**
 * @tc.name: ThermalMgrSystemTest023
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state not satisfied
 * @tc.result level 2, freq 1992000
 */

HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest023, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest023: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(SLEEP_INTERVAL_SEC);

    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    ret = snprintf_s(cpuBuf, MAX_PATH, sizeof(cpuBuf) - 1, CPU_FREQ_PATH);
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string freq = freqValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(freq);
    THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
    EXPECT_EQ(true, value == 1991000 || value == 1990500 || value == 1990200) << "ThermalMgrSystemTest023 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest023: end.");
}

/**
 * @tc.name: ThermalMgrSystemTest024
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state not satisfied
 * @tc.result level 3, freq 1992000
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest024, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest024: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(SLEEP_INTERVAL_SEC);

    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    ret = snprintf_s(cpuBuf, MAX_PATH, sizeof(cpuBuf) - 1, CPU_FREQ_PATH);
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string freq = freqValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(freq);
    THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
    EXPECT_EQ(true, value == 1990000 || value == 1989500 || value == 1989200) << "ThermalMgrSystemTest024 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest024: end.");
}

/**
 * @tc.name: ThermalMgrSystemTest025
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state not satisfied
 * @tc.result level 4, freq 1992000
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest025, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest025: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 48100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(SLEEP_INTERVAL_SEC);

    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    ret = snprintf_s(cpuBuf, MAX_PATH, sizeof(cpuBuf) - 1, CPU_FREQ_PATH);
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string freq = freqValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(freq);
    THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
    EXPECT_EQ(true, value == 1990000 || value == 1989500 || value == 1989200) << "ThermalMgrSystemTest025 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest025: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest044: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp);
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(SLEEP_INTERVAL_SEC);

    char procsessBuf[MAX_PATH] = {0};
    char procsesValue[MAX_PATH] = {0};
    ret = snprintf_s(procsessBuf, MAX_PATH, sizeof(procsessBuf) - 1, processPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(procsessBuf, procsesValue, sizeof(procsesValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string process = procsesValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(process);
    THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
    EXPECT_EQ(true, value == 3) << "ThermalMgrSystemTest050 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest050: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest051: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp);
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(SLEEP_INTERVAL_SEC);

    char procsessBuf[MAX_PATH] = {0};
    char procsesValue[MAX_PATH] = {0};
    ret = snprintf_s(procsessBuf, MAX_PATH, sizeof(procsessBuf) - 1, processPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(procsessBuf, procsesValue, sizeof(procsesValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string process = procsesValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(process);
    THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
    EXPECT_EQ(true, value == 2) << "ThermalMgrSystemTest051 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest051: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest052: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp);
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(SLEEP_INTERVAL_SEC);

    char procsessBuf[MAX_PATH] = {0};
    char procsesValue[MAX_PATH] = {0};
    ret = snprintf_s(procsessBuf, MAX_PATH, sizeof(procsessBuf) - 1, processPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(procsessBuf, procsesValue, sizeof(procsesValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string process = procsesValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(process);
    THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
    EXPECT_EQ(true, value == 1) << "ThermalMgrSystemTest052 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest052: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest053: start.");
    int32_t ret = -1;
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(paTempBuf, MAX_PATH, sizeof(paTempBuf) - 1, paPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(amTempBuf, MAX_PATH, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= EOK);

    int32_t paTemp = 41000;
    std::string sTemp = to_string(paTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 10000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(SLEEP_INTERVAL_SEC);
    char procsessBuf[MAX_PATH] = {0};
    char procsesValue[MAX_PATH] = {0};
    ret = snprintf_s(procsessBuf, MAX_PATH, sizeof(procsessBuf) - 1, processPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(procsessBuf, procsesValue, sizeof(procsesValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string process = procsesValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(process);
    THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
    EXPECT_EQ(true, value == 2) << "ThermalMgrSystemTest053 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest053: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest054: start.");
    int32_t ret = -1;
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(paTempBuf, MAX_PATH, sizeof(paTempBuf) - 1, paPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(amTempBuf, MAX_PATH, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= EOK);

    int32_t paTemp = 44000;
    std::string sTemp = to_string(paTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 10000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(SLEEP_INTERVAL_SEC);
    char procsessBuf[MAX_PATH] = {0};
    char procsesValue[MAX_PATH] = {0};
    ret = snprintf_s(procsessBuf, MAX_PATH, sizeof(procsessBuf) - 1, processPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(procsessBuf, procsesValue, sizeof(procsesValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string process = procsesValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(process);
    THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
    EXPECT_EQ(true, value == 3) << "ThermalMgrSystemTest054 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest054: end.");
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
    sleep(SLEEP_INTERVAL_SEC);
    if (!CheckThermalProtectorPID()) {
        EXPECT_EQ(true, StopThermalProtector());
    }
    HiLog::Info(LABEL, "ThermalModuleServiceTest006 end");
}
}
