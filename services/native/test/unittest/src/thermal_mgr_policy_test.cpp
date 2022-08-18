/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

int32_t ThermalMgrPolicyTest::ReadSysfsFile(const char *path, char *buf, size_t size)
{
    int32_t readSize;
    int fd = open(path, O_RDONLY);
    if (fd < NUM_ZERO) {
        THERMAL_HILOGE(LABEL_TEST, "failed to open file node");
        return ERR_INVALID_VALUE;
    }

    readSize = read(fd, buf, size - 1);
    if (readSize < NUM_ZERO) {
        THERMAL_HILOGE(LABEL_TEST, "failed to read file");
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
        THERMAL_HILOGD(LABEL_TEST, "failed to read file");
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
        ret = snprintf_s(bufTemp, MAX_PATH, sizeof(bufTemp) - 1, SIMULATION_TEMP_DIR, iter.first.c_str());
        if (ret < EOK) {
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
    ret = snprintf_s(stateChargeBuf, MAX_PATH, sizeof(stateChargeBuf) - 1, stateChargePath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(stateSceneBuf, MAX_PATH, sizeof(stateSceneBuf) - 1, stateScenePath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    std::string chargeState = "0";
    ret = WriteFile(stateChargeBuf, chargeState, chargeState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string sceneState = "0";
    ret = WriteFile(stateSceneBuf, sceneState, sceneState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest001: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char levelBuf[MAX_PATH] = {0};
        char levelValue[MAX_PATH] = {0};
        ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string level = levelValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 1) << "ThermalMgrPolicyTest001 failed";
    }

    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest001: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest002: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char levelBuf[MAX_PATH] = {0};
        char levelValue[MAX_PATH] = {0};
        ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string level = levelValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 2) << "ThermalMgrPolicyTest002 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest002: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest003: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char levelBuf[MAX_PATH] = {0};
        char levelValue[MAX_PATH] = {0};
        ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string level = levelValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 3) << "ThermalMgrPolicyTest003 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest003: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest003: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 48100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char levelBuf[MAX_PATH] = {0};
        char levelValue[MAX_PATH] = {0};
        ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string level = levelValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
        EXPECT_EQ(true, value == 4) << "ThermalMgrPolicyTest005 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest004: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest005: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    std::string level;
    int32_t value;
    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
        EXPECT_EQ(true, ret == ERR_OK);
        level = levelValue;
        value = ThermalMgrPolicyTest::ConvertInt(level);
        EXPECT_EQ(true, value == 1) << "ThermalMgrPolicyTest005 failed";
    }

    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    batteryTemp = 48100;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
        EXPECT_EQ(true, ret == ERR_OK);
        level = levelValue;
        value = ThermalMgrPolicyTest::ConvertInt(level);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 4) << "ThermalMgrPolicyTest005 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest005: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest006: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    std::string level;
    int32_t value;
    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
        EXPECT_EQ(true, ret == ERR_OK);
        level = levelValue;
        value = ThermalMgrPolicyTest::ConvertInt(level);
        EXPECT_EQ(true, value == 2) << "ThermalMgrPolicyTest006 failed";
    }

    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    batteryTemp = 48100;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
        EXPECT_EQ(true, ret == ERR_OK);
        level = levelValue;
        value = ThermalMgrPolicyTest::ConvertInt(level);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 4) << "ThermalMgrPolicyTest006 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest006: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest007: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 48200;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    std::string level;
    int32_t value;
    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char levelBuf[MAX_PATH] = {0};
        char levelValue[MAX_PATH] = {0};
        ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
        EXPECT_EQ(true, ret == ERR_OK);
        level = levelValue;
        value = ThermalMgrPolicyTest::ConvertInt(level);
        EXPECT_EQ(true, value == 4) << "ThermalMgrPolicyTest007 failed";
    }

    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    batteryTemp = 40900;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
        EXPECT_EQ(true, ret == ERR_OK);
        level = levelValue;
        value = ThermalMgrPolicyTest::ConvertInt(level);
        THERMAL_HILOGD(LABEL_TEST, "value is: %{public}d", value);
        EXPECT_EQ(true, value == 1) << "ThermalMgrPolicyTest007 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest007: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest008: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    std::string level;
    int32_t value;
    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char levelBuf[MAX_PATH] = {0};
        char levelValue[MAX_PATH] = {0};
        ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
        EXPECT_EQ(true, ret == ERR_OK);
        level = levelValue;
        value = ThermalMgrPolicyTest::ConvertInt(level);
        EXPECT_EQ(true, value == 3) << "ThermalMgrPolicyTest008 failed";
    }

    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    batteryTemp = 37000;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
        EXPECT_EQ(true, ret == ERR_OK);
        level = levelValue;
        value = ThermalMgrPolicyTest::ConvertInt(level);
        THERMAL_HILOGD(LABEL_TEST, "value is: %{public}d", value);
        EXPECT_EQ(true, value == 0) << "ThermalMgrPolicyTest008 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest008: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest009: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = -10000;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char levelBuf[MAX_PATH] = {0};
        char levelValue[MAX_PATH] = {0};
        ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string level = levelValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
        EXPECT_EQ(true, value == 1) << "ThermalMgrPolicyTest009 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest009: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest010: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = -15000;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char levelBuf[MAX_PATH] = {0};
        char levelValue[MAX_PATH] = {0};
        ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string level = levelValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
        EXPECT_EQ(true, value == 2) << "ThermalMgrPolicyTest010 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest010: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest011: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = -20100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char levelBuf[MAX_PATH] = {0};
        char levelValue[MAX_PATH] = {0};
        ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string level = levelValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
        EXPECT_EQ(true, value == 3) << "ThermalMgrPolicyTest011 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest011: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest012: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = -22000;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char levelBuf[MAX_PATH] = {0};
        char levelValue[MAX_PATH] = {0};
        ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string level = levelValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
        EXPECT_EQ(true, value == 4) << "ThermalMgrPolicyTest012 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest012: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest013: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = -10000;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    std::string level;
    int32_t value;
    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
        EXPECT_EQ(true, ret == ERR_OK);
        level = levelValue;
        value = ThermalMgrPolicyTest::ConvertInt(level);
        EXPECT_EQ(true, value == 1) << "ThermalMgrPolicyTest013 failed";
    }

    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    batteryTemp = -22000;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
        EXPECT_EQ(true, ret == ERR_OK);
        level = levelValue;
        value = ThermalMgrPolicyTest::ConvertInt(level);
        THERMAL_HILOGD(LABEL_TEST, "value is: %{public}d", value);
        EXPECT_EQ(true, value == 4) << "ThermalMgrPolicyTest013 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest013: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest014: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = -15000;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    std::string level;
    int32_t value;
    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
        EXPECT_EQ(true, ret == ERR_OK);
        level = levelValue;
        value = ThermalMgrPolicyTest::ConvertInt(level);
        EXPECT_EQ(true, value == 2) << "ThermalMgrPolicyTest014 failed";
    }

    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    batteryTemp = -22000;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
        EXPECT_EQ(true, ret == ERR_OK);
        level = levelValue;
        value = ThermalMgrPolicyTest::ConvertInt(level);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 4) << "ThermalMgrPolicyTest014 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest014: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest015: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = -22000;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    std::string level;
    int32_t value;
    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
        EXPECT_EQ(true, ret == ERR_OK);
        level = levelValue;
        value = ThermalMgrPolicyTest::ConvertInt(level);
        EXPECT_EQ(true, value == 4) << "ThermalMgrPolicyTest015 failed";
    }

    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    batteryTemp = -10000;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
        EXPECT_EQ(true, ret == ERR_OK);
        level = levelValue;
        value = ThermalMgrPolicyTest::ConvertInt(level);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 1) << "ThermalMgrPolicyTest015 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest015: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest016: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = -19100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    std::string level;
    int32_t value;
    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
        EXPECT_EQ(true, ret == ERR_OK);
        level = levelValue;
        value = ThermalMgrPolicyTest::ConvertInt(level);
        EXPECT_EQ(true, value == 3) << "ThermalMgrPolicyTest016 failed";
    }

    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    batteryTemp = -1000;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
        EXPECT_EQ(true, ret == ERR_OK);
        level = levelValue;
        value = ThermalMgrPolicyTest::ConvertInt(level);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 0) << "ThermalMgrPolicyTest016 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest016: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest017: start.");
    int32_t ret = -1;
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(paTempBuf, MAX_PATH, sizeof(paTempBuf) - 1, paPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(amTempBuf, MAX_PATH, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= EOK);

    int32_t paTemp = 41000;
    std::string sTemp = to_string(paTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 10000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);
        char levelBuf[MAX_PATH] = {0};
        char levelValue[MAX_PATH] = {0};
        ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string level = levelValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 1) << "ThermalMgrPolicyTest017 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest017: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest018: start.");
    int32_t ret = -1;
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(paTempBuf, MAX_PATH, sizeof(paTempBuf) - 1, paPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(amTempBuf, MAX_PATH, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= EOK);

    int32_t paTemp = 44000;
    std::string sTemp = to_string(paTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 10000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);
        char levelBuf[MAX_PATH] = {0};
        char levelValue[MAX_PATH] = {0};
        ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string level = levelValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 2) << "ThermalMgrPolicyTest018 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest018: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest019: start.");
    int32_t ret = -1;
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(paTempBuf, MAX_PATH, sizeof(paTempBuf) - 1, paPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(amTempBuf, MAX_PATH, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= EOK);

    int32_t paTemp = 44000;
    std::string sTemp = to_string(paTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 1000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);
        char levelBuf[MAX_PATH] = {0};
        char levelValue[MAX_PATH] = {0};
        ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string level = levelValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 0) << "ThermalMgrPolicyTest019 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest019: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest020: start.");
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

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);
        char levelBuf[MAX_PATH] = {0};
        char levelValue[MAX_PATH] = {0};
        ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string level = levelValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 1) << "ThermalMgrPolicyTest020 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest020: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest021: start.");
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

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);
        char levelBuf[MAX_PATH] = {0};
        char levelValue[MAX_PATH] = {0};
        ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, configLevelPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string level = levelValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(level);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 0) << "ThermalMgrPolicyTest021 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest021: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest022
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state not satisfied
 * @tc.result level 1, freq 1991500
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest022, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest022: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char cpuBuf[MAX_PATH] = {0};
        char freqValue[MAX_PATH] = {0};
        ret = snprintf_s(cpuBuf, MAX_PATH, sizeof(cpuBuf) - 1, CPU_FREQ_PATH);
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string freq = freqValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(freq);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 1992000 || value == 1991500 || value == 1991200) << "ThermalMgrPolicyTest022 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest022: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest023
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state not satisfied
 * @tc.result level 2, freq 1990500
 */

HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest023, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest023: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char cpuBuf[MAX_PATH] = {0};
        char freqValue[MAX_PATH] = {0};
        ret = snprintf_s(cpuBuf, MAX_PATH, sizeof(cpuBuf) - 1, CPU_FREQ_PATH);
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string freq = freqValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(freq);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 1991000 || value == 1990500 || value == 1990200) << "ThermalMgrPolicyTest023 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest023: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest024
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state not satisfied
 * @tc.result level 3, freq 1989500
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest024, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest024: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char cpuBuf[MAX_PATH] = {0};
        char freqValue[MAX_PATH] = {0};
        ret = snprintf_s(cpuBuf, MAX_PATH, sizeof(cpuBuf) - 1, CPU_FREQ_PATH);
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string freq = freqValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(freq);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 1990000 || value == 1989500 || value == 1989200) << "ThermalMgrPolicyTest024 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest024: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest025
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state not satisfied
 * @tc.result level 4, freq 1989500
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest025, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest025: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 48100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char cpuBuf[MAX_PATH] = {0};
        char freqValue[MAX_PATH] = {0};
        ret = snprintf_s(cpuBuf, MAX_PATH, sizeof(cpuBuf) - 1, CPU_FREQ_PATH);
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string freq = freqValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(freq);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 1990000 || value == 1989500 || value == 1989200) << "ThermalMgrPolicyTest025 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest025: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest026
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: charge = 1, no scene
 * @tc.result level 1, freq 1991500
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest026, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest026: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    char stateChargeBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(stateChargeBuf, MAX_PATH, sizeof(stateChargeBuf) - 1, stateChargePath.c_str());
    EXPECT_EQ(true, ret >= EOK);

    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string chargeState = "1";
    ret = ThermalMgrPolicyTest::WriteFile(stateChargeBuf, chargeState, chargeState.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char cpuBuf[MAX_PATH] = {0};
        char freqValue[MAX_PATH] = {0};
        ret = snprintf_s(cpuBuf, MAX_PATH, sizeof(cpuBuf) - 1, CPU_FREQ_PATH);
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string freq = freqValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(freq);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 1992000 || value == 1991500 || value == 1991200) << "ThermalMgrPolicyTest026 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest026: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest027
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: charge = 1, scene = "cam"
 * @tc.result level 1, freq 1991800
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest027, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest027: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    char stateChargeBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(stateChargeBuf, MAX_PATH, sizeof(stateChargeBuf) - 1, stateChargePath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("cam");

    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string chargeState = "1";
    ret = ThermalMgrPolicyTest::WriteFile(stateChargeBuf, chargeState, chargeState.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char cpuBuf[MAX_PATH] = {0};
        char freqValue[MAX_PATH] = {0};
        ret = snprintf_s(cpuBuf, MAX_PATH, sizeof(cpuBuf) - 1, CPU_FREQ_PATH);
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string freq = freqValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(freq);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 1991800) << "ThermalMgrPolicyTest027 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest027: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest028
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: charge = 0, scene = "cam"
 * @tc.result level 1, freq 1991600
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest028, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest028: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    char stateChargeBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(stateChargeBuf, MAX_PATH, sizeof(stateChargeBuf) - 1, stateChargePath.c_str());
    EXPECT_EQ(true, ret >= EOK);

    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string chargeState = "0";
    ret = ThermalMgrPolicyTest::WriteFile(stateChargeBuf, chargeState, chargeState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("cam");

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char cpuBuf[MAX_PATH] = {0};
        char freqValue[MAX_PATH] = {0};
        ret = snprintf_s(cpuBuf, MAX_PATH, sizeof(cpuBuf) - 1, CPU_FREQ_PATH);
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string freq = freqValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(freq);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 1991600) << "ThermalMgrPolicyTest028 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest028: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest029
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: charge = 1, no scene
 * @tc.result level 2, freq 1990500
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest029, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest029: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    char stateChargeBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(stateChargeBuf, MAX_PATH, sizeof(stateChargeBuf) - 1, stateChargePath.c_str());
    EXPECT_EQ(true, ret >= EOK);

    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string chargeState = "1";
    ret = ThermalMgrPolicyTest::WriteFile(stateChargeBuf, chargeState, chargeState.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char cpuBuf[MAX_PATH] = {0};
        char freqValue[MAX_PATH] = {0};
        ret = snprintf_s(cpuBuf, MAX_PATH, sizeof(cpuBuf) - 1, CPU_FREQ_PATH);
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string freq = freqValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(freq);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 1990500) << "ThermalMgrPolicyTest029 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest029: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest030
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: charge = 1, scene = "cam"
 * @tc.result level 2, freq 1990800
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest030, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest030: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    char stateChargeBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(stateChargeBuf, MAX_PATH, sizeof(stateChargeBuf) - 1, stateChargePath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("cam");

    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string chargeState = "1";
    ret = ThermalMgrPolicyTest::WriteFile(stateChargeBuf, chargeState, chargeState.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char cpuBuf[MAX_PATH] = {0};
        char freqValue[MAX_PATH] = {0};
        ret = snprintf_s(cpuBuf, MAX_PATH, sizeof(cpuBuf) - 1, CPU_FREQ_PATH);
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string freq = freqValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(freq);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 1990800) << "ThermalMgrPolicyTest030 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest030: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest031
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: charge = 0, scene = "cam"
 * @tc.result level 2, freq 1990600
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest031, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest031: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    char stateChargeBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(stateChargeBuf, MAX_PATH, sizeof(stateChargeBuf) - 1, stateChargePath.c_str());
    EXPECT_EQ(true, ret >= EOK);

    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string chargeState = "0";
    ret = ThermalMgrPolicyTest::WriteFile(stateChargeBuf, chargeState, chargeState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("cam");

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char cpuBuf[MAX_PATH] = {0};
        char freqValue[MAX_PATH] = {0};
        ret = snprintf_s(cpuBuf, MAX_PATH, sizeof(cpuBuf) - 1, CPU_FREQ_PATH);
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string freq = freqValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(freq);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 1990600) << "ThermalMgrPolicyTest031 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest031: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest032
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: charge = 1, no scene
 * @tc.result level 3, freq 1989500
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest032, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest032: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    char stateChargeBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(stateChargeBuf, MAX_PATH, sizeof(stateChargeBuf) - 1, stateChargePath.c_str());
    EXPECT_EQ(true, ret >= EOK);

    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string chargeState = "1";
    ret = ThermalMgrPolicyTest::WriteFile(stateChargeBuf, chargeState, chargeState.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char cpuBuf[MAX_PATH] = {0};
        char freqValue[MAX_PATH] = {0};
        ret = snprintf_s(cpuBuf, MAX_PATH, sizeof(cpuBuf) - 1, CPU_FREQ_PATH);
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string freq = freqValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(freq);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 1989500) << "ThermalMgrPolicyTest032 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest032: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest033
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: charge = 1, scene = "cam"
 * @tc.result level 3, freq 1989800
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest033, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest033: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    char stateChargeBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(stateChargeBuf, MAX_PATH, sizeof(stateChargeBuf) - 1, stateChargePath.c_str());
    EXPECT_EQ(true, ret >= EOK);

    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string chargeState = "1";
    ret = ThermalMgrPolicyTest::WriteFile(stateChargeBuf, chargeState, chargeState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("cam");

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char cpuBuf[MAX_PATH] = {0};
        char freqValue[MAX_PATH] = {0};
        ret = snprintf_s(cpuBuf, MAX_PATH, sizeof(cpuBuf) - 1, CPU_FREQ_PATH);
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string freq = freqValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(freq);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 1989800) << "ThermalMgrPolicyTest033 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest033: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest034
 * @tc.desc: test get cpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: charge = 0, scene = "cam"
 * @tc.result level 3, freq 1989600
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest034, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest034: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    char stateChargeBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(stateChargeBuf, MAX_PATH, sizeof(stateChargeBuf) - 1, stateChargePath.c_str());
    EXPECT_EQ(true, ret >= EOK);

    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    std::string chargeState = "0";
    ret = ThermalMgrPolicyTest::WriteFile(stateChargeBuf, chargeState, chargeState.length());
    EXPECT_EQ(true, ret == ERR_OK);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("cam");

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char cpuBuf[MAX_PATH] = {0};
        char freqValue[MAX_PATH] = {0};
        ret = snprintf_s(cpuBuf, MAX_PATH, sizeof(cpuBuf) - 1, CPU_FREQ_PATH);
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(cpuBuf, freqValue, sizeof(freqValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string freq = freqValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(freq);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 1989600) << "ThermalMgrPolicyTest034 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest034: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest035: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);
        sleep(WAIT_TIME_5_SEC);
        char currentBuf[MAX_PATH] = {0};
        char currentValue[MAX_PATH] = {0};
        ret = snprintf_s(currentBuf, MAX_PATH, sizeof(currentBuf) - 1, BATTERY_CHARGER_CURRENT_PATH);
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(currentBuf, currentValue, sizeof(currentValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string current = currentValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(current);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 1800) << "ThermalMgrPolicyTest035 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest035: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest036
 * @tc.desc: test get charge currentby setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: scene = "cam"
 * @tc.result level 1, current 1200
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest036, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest036: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("cam");

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char currentBuf[MAX_PATH] = {0};
        char currentValue[MAX_PATH] = {0};
        ret = snprintf_s(currentBuf, MAX_PATH, sizeof(currentBuf) - 1, BATTERY_CHARGER_CURRENT_PATH);
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(currentBuf, currentValue, sizeof(currentValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string current = currentValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(current);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 1200) << "ThermalMgrPolicyTest036 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest036: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest035: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char currentBuf[MAX_PATH] = {0};
        char currentValue[MAX_PATH] = {0};
        ret = snprintf_s(currentBuf, MAX_PATH, sizeof(currentBuf) - 1, BATTERY_CHARGER_CURRENT_PATH);
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(currentBuf, currentValue, sizeof(currentValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string current = currentValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(current);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 1500) << "ThermalMgrPolicyTest037 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest037: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest038
 * @tc.desc: test get charge currentby setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: scene = "cam"
 * @tc.result level 2, current 1000
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest038, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest038: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("cam");

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char currentBuf[MAX_PATH] = {0};
        char currentValue[MAX_PATH] = {0};
        ret = snprintf_s(currentBuf, MAX_PATH, sizeof(currentBuf) - 1, BATTERY_CHARGER_CURRENT_PATH);
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(currentBuf, currentValue, sizeof(currentValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string current = currentValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(current);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 1000) << "ThermalMgrPolicyTest038 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest038: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest039: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char currentBuf[MAX_PATH] = {0};
        char currentValue[MAX_PATH] = {0};
        ret = snprintf_s(currentBuf, MAX_PATH, sizeof(currentBuf) - 1, BATTERY_CHARGER_CURRENT_PATH);
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(currentBuf, currentValue, sizeof(currentValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string current = currentValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(current);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 1300) << "ThermalMgrPolicyTest039 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest039: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest040
 * @tc.desc: test get charge currentby setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp, state: scene = "cam"
 * @tc.result level 3, current 800
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest040, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest040: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("cam");

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char currentBuf[MAX_PATH] = {0};
        char currentValue[MAX_PATH] = {0};
        ret = snprintf_s(currentBuf, MAX_PATH, sizeof(currentBuf) - 1, BATTERY_CHARGER_CURRENT_PATH);
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(currentBuf, currentValue, sizeof(currentValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string current = currentValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(current);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 800) << "ThermalMgrPolicyTest040 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest040: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest041: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = -10000;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char currentlBuf[MAX_PATH] = {0};
        char currentValue[MAX_PATH] = {0};
        ret = snprintf_s(currentlBuf, MAX_PATH, sizeof(currentlBuf) - 1, BATTERY_CHARGER_CURRENT_PATH);
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(currentlBuf, currentValue, sizeof(currentValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string current = currentValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(current);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 1850) << "ThermalMgrPolicyTest041 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest041: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest042: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = -14100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char currentlBuf[MAX_PATH] = {0};
        char currentValue[MAX_PATH] = {0};
        ret = snprintf_s(currentlBuf, MAX_PATH, sizeof(currentlBuf) - 1, BATTERY_CHARGER_CURRENT_PATH);
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(currentlBuf, currentValue, sizeof(currentValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string current = currentValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(current);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 1550) << "ThermalMgrPolicyTest042 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest042: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest043: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = -19100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char currentlBuf[MAX_PATH] = {0};
        char currentValue[MAX_PATH] = {0};
        ret = snprintf_s(currentlBuf, MAX_PATH, sizeof(currentlBuf) - 1, BATTERY_CHARGER_CURRENT_PATH);
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(currentlBuf, currentValue, sizeof(currentValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string current = currentValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(current);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 1150) << "ThermalMgrPolicyTest043 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest043: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest044
 * @tc.desc: test get brightness configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 1 brightness factor is 1.0
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest044, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest044: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char lcdBuf[MAX_PATH] = {0};
        char lcdValue[MAX_PATH]= {0};
        ret = snprintf_s(lcdBuf, MAX_PATH, sizeof(lcdBuf) - 1, lcdPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string lcd = lcdValue;
        THERMAL_HILOGD(LABEL_TEST, "lcd value: %{public}s", lcd.c_str());
        EXPECT_EQ(true, lcd.substr(0, 3) == "1.0") << "ThermalMgrPolicyTest044 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest044: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest045
 * @tc.desc: test get brightness configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 2 brightness factor is 0.9
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest045, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest045: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char lcdBuf[MAX_PATH] = {0};
        char lcdValue[MAX_PATH] = {0};
        ret = snprintf_s(lcdBuf, MAX_PATH, sizeof(lcdBuf) - 1, lcdPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string lcd = lcdValue;
        THERMAL_HILOGD(LABEL_TEST, "lcd value: %{public}s", lcd.c_str());
        EXPECT_EQ(true, lcd.substr(0, 3) == "0.9") << "ThermalMgrPolicyTest045 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest045: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest046
 * @tc.desc: test get brightness configured level by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 3 brightness factor is 0.8
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest046, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest046: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char lcdBuf[MAX_PATH] = {0};
        char lcdValue[MAX_PATH] = {0};
        ret = snprintf_s(lcdBuf, MAX_PATH, sizeof(lcdBuf) - 1, lcdPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string lcd = lcdValue;
        THERMAL_HILOGD(LABEL_TEST, "lcd value: %{public}s", lcd.c_str());
        EXPECT_EQ(true, lcd.substr(0, 3) == "0.8") << "ThermalMgrPolicyTest046 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest046: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest047
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set PA temp, High Temp With Aux sensor
 * @tc.result level 1 brightness factor is 0.7
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest047, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest047: start.");
    int32_t ret = -1;
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(paTempBuf, MAX_PATH, sizeof(paTempBuf) - 1, paPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(amTempBuf, MAX_PATH, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= EOK);

    int32_t paTemp = 41000;
    std::string sTemp = to_string(paTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 10000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);
        char lcdBuf[MAX_PATH] = {0};
        char lcdValue[MAX_PATH] = {0};
        ret = snprintf_s(lcdBuf, MAX_PATH, sizeof(lcdBuf) - 1, lcdPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string lcd = lcdValue;
        THERMAL_HILOGD(LABEL_TEST, "lcd value: %{public}s", lcd.c_str());
        EXPECT_EQ(true, lcd.substr(0, 3) == "0.7") << "ThermalMgrPolicyTest047 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest047: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest048
 * @tc.desc: test level asc logic by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set PA temp, High Temp With Aux sensor
 * @tc.result level 2 brightness factor is 0.6
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest048, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest047: start.");
    int32_t ret = -1;
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(paTempBuf, MAX_PATH, sizeof(paTempBuf) - 1, paPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(amTempBuf, MAX_PATH, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= EOK);

    int32_t paTemp = 44000;
    std::string sTemp = to_string(paTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 10000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);
        char lcdBuf[MAX_PATH] = {0};
        char lcdValue[MAX_PATH] = {0};
        ret = snprintf_s(lcdBuf, MAX_PATH, sizeof(lcdBuf) - 1, lcdPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string lcd = lcdValue;
        THERMAL_HILOGD(LABEL_TEST, "lcd value: %{public}s", lcd.c_str());
        EXPECT_EQ(true, lcd.substr(0, 3) == "0.6") << "ThermalMgrPolicyTest048 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest048: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest049: start.");
    int32_t ret = -1;
    char apTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    char shellTempBuf[MAX_PATH] = {0};
    char stateScreenBuf[MAX_PATH] = {0};
    ret = snprintf_s(apTempBuf, MAX_PATH, sizeof(apTempBuf) - 1, apPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(amTempBuf, MAX_PATH, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(shellTempBuf, MAX_PATH, sizeof(shellTempBuf) - 1, shellPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(stateScreenBuf, MAX_PATH, sizeof(stateScreenBuf) - 1, stateScreenPath.c_str());
    EXPECT_EQ(true, ret >= EOK);

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

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);
        char procsessBuf[MAX_PATH] = {0};
        char procsesValue[MAX_PATH] = {0};
        ret = snprintf_s(procsessBuf, MAX_PATH, sizeof(procsessBuf) - 1, processPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(procsessBuf, procsesValue, sizeof(procsesValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string process = procsesValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(process);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 3) << "ThermalMgrPolicyTest049 failed";

        char shutdownBuf[MAX_PATH] = {0};
        char shutdownValue[MAX_PATH] = {0};
        ret = snprintf_s(shutdownBuf, MAX_PATH, sizeof(shutdownBuf) - 1, shutdownPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(shutdownBuf, shutdownValue, sizeof(shutdownValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string shutdown = shutdownValue;
        value = ThermalMgrPolicyTest::ConvertInt(shutdown);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 1) << "ThermalMgrPolicyTest049 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest049: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest050: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char procsessBuf[MAX_PATH] = {0};
        char procsesValue[MAX_PATH] = {0};
        ret = snprintf_s(procsessBuf, MAX_PATH, sizeof(procsessBuf) - 1, processPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(procsessBuf, procsesValue, sizeof(procsesValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string process = procsesValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(process);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 3) << "ThermalMgrPolicyTest050 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest050: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest051: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char procsessBuf[MAX_PATH] = {0};
        char procsesValue[MAX_PATH] = {0};
        ret = snprintf_s(procsessBuf, MAX_PATH, sizeof(procsessBuf) - 1, processPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(procsessBuf, procsesValue, sizeof(procsesValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string process = procsesValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(process);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 2) << "ThermalMgrPolicyTest051 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest051: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest052: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char procsessBuf[MAX_PATH] = {0};
        char procsesValue[MAX_PATH] = {0};
        ret = snprintf_s(procsessBuf, MAX_PATH, sizeof(procsessBuf) - 1, processPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(procsessBuf, procsesValue, sizeof(procsesValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string process = procsesValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(process);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 1) << "ThermalMgrPolicyTest052 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest052: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest053: start.");
    int32_t ret = -1;
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(paTempBuf, MAX_PATH, sizeof(paTempBuf) - 1, paPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(amTempBuf, MAX_PATH, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= EOK);

    int32_t paTemp = 41000;
    std::string sTemp = to_string(paTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 10000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);
        char procsessBuf[MAX_PATH] = {0};
        char procsesValue[MAX_PATH] = {0};
        ret = snprintf_s(procsessBuf, MAX_PATH, sizeof(procsessBuf) - 1, processPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(procsessBuf, procsesValue, sizeof(procsesValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string process = procsesValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(process);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 2) << "ThermalMgrPolicyTest053 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest053: end.");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest054: start.");
    int32_t ret = -1;
    char paTempBuf[MAX_PATH] = {0};
    char amTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(paTempBuf, MAX_PATH, sizeof(paTempBuf) - 1, paPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(amTempBuf, MAX_PATH, sizeof(amTempBuf) - 1, ambientPath.c_str());
    EXPECT_EQ(true, ret >= EOK);

    int32_t paTemp = 44000;
    std::string sTemp = to_string(paTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(paTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t amTemp = 10000;
    sTemp = to_string(amTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(amTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);
        char procsessBuf[MAX_PATH] = {0};
        char procsesValue[MAX_PATH] = {0};
        ret = snprintf_s(procsessBuf, MAX_PATH, sizeof(procsessBuf) - 1, processPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(procsessBuf, procsesValue, sizeof(procsesValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string process = procsesValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(process);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 3) << "ThermalMgrPolicyTest054 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest054: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest055
 * @tc.desc: get the config current by setting battery temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp, charge_type: buck
 * @tc.result level 1, current 1200
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest055, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest055: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME_5_SEC);

    char buckCurrentBuf[MAX_PATH] = {0};
    char buckCurrentValue[MAX_PATH] = {0};
    ret = snprintf_s(buckCurrentBuf, MAX_PATH, sizeof(buckCurrentBuf) - 1, BUCK_CURRENT_PATH);
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrPolicyTest::ReadFile(buckCurrentBuf, buckCurrentValue, sizeof(buckCurrentValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string valueStr = buckCurrentValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(valueStr);\
    THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
    EXPECT_EQ(true, value == 1200) << "ThermalMgrPolicyTest055 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest055: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest056
 * @tc.desc: get the config voltage by setting battery temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp, charge_type: sc
 * @tc.result level 1, voltage 4000
 */

HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest056, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest056: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME_5_SEC);

    char scVoltageBuf[MAX_PATH] = {0};
    char scVoltageValue[MAX_PATH] = {0};
    ret = snprintf_s(scVoltageBuf, MAX_PATH, sizeof(scVoltageBuf) - 1, SC_VOLTAGE_PATH);
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrPolicyTest::ReadFile(scVoltageBuf, scVoltageValue, sizeof(scVoltageValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string valueStr = scVoltageValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(valueStr);
    THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
    EXPECT_EQ(true, value == 4000) << "ThermalMgrPolicyTest056 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest056: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest057
 * @tc.desc: get the config voltage by setting battery temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp, charge_type: buck
 * @tc.result level 1, voltage 3000
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest057, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest057: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME_5_SEC);

    char buckVoltageBuf[MAX_PATH] = {0};
    char buckVoltageValue[MAX_PATH] = {0};
    ret = snprintf_s(buckVoltageBuf, MAX_PATH, sizeof(buckVoltageBuf) - 1, BUCK_VOLTAGE_PATH);
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrPolicyTest::ReadFile(buckVoltageBuf, buckVoltageValue, sizeof(buckVoltageValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string valueStr = buckVoltageValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(valueStr);
    THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
    EXPECT_EQ(true, value == 3000) << "ThermalMgrPolicyTest057 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest057: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest058
 * @tc.desc: get the config current by setting battery temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp, charge_type: buck
 * @tc.result level 2, current 1000
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest058, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest058: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME_5_SEC);

    char buckCurrentBuf[MAX_PATH] = {0};
    char buckCurrentValue[MAX_PATH] = {0};
    ret = snprintf_s(buckCurrentBuf, MAX_PATH, sizeof(buckCurrentBuf) - 1, BUCK_CURRENT_PATH);
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrPolicyTest::ReadFile(buckCurrentBuf, buckCurrentValue, sizeof(buckCurrentValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string valueStr = buckCurrentValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(valueStr);
    THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
    EXPECT_EQ(true, value == 1000) << "ThermalMgrPolicyTest058 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest058: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest059
 * @tc.desc: get the config voltage by setting battery temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp, charge_type: sc
 * @tc.result level 2， voltage 3000
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest059, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest059: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME_5_SEC);

    char scVoltageBuf[MAX_PATH] = {0};
    char scVoltageValue[MAX_PATH] = {0};
    ret = snprintf_s(scVoltageBuf, MAX_PATH, sizeof(scVoltageBuf) - 1, SC_VOLTAGE_PATH);
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrPolicyTest::ReadFile(scVoltageBuf, scVoltageValue, sizeof(scVoltageValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string valueStr = scVoltageValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(valueStr);
    THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
    EXPECT_EQ(true, value == 3000) << "ThermalMgrPolicyTest059 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest059: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest060
 * @tc.desc: get the config voltage by setting battery temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp, charge_type: buck
 * @tc.result level 2, voltage 2000
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest060, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest060: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME_5_SEC);

    char buckVoltageBuf[MAX_PATH] = {0};
    char buckVoltageValue[MAX_PATH] = {0};
    ret = snprintf_s(buckVoltageBuf, MAX_PATH, sizeof(buckVoltageBuf) - 1, BUCK_VOLTAGE_PATH);
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrPolicyTest::ReadFile(buckVoltageBuf, buckVoltageValue, sizeof(buckVoltageValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string valueStr = buckVoltageValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(valueStr);
    THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
    EXPECT_EQ(true, value == 2000) << "ThermalMgrPolicyTest060 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest060: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest061
 * @tc.desc: get the config current by setting battery temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp, charge_type: buck
 * @tc.result level 3, current 800
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest061, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest061: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME_5_SEC);

    char buckCurrentBuf[MAX_PATH] = {0};
    char buckCurrentValue[MAX_PATH] = {0};
    ret = snprintf_s(buckCurrentBuf, MAX_PATH, sizeof(buckCurrentBuf) - 1, BUCK_CURRENT_PATH);
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrPolicyTest::ReadFile(buckCurrentBuf, buckCurrentValue, sizeof(buckCurrentValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string valueStr = buckCurrentValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(valueStr);
    THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
    EXPECT_EQ(true, value == 800) << "ThermalMgrPolicyTest061 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest061: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest062
 * @tc.desc: get the config voltage by setting battery temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp, charge_type: sc
 * @tc.result level 3, voltage 2000
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest062, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest062: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME_5_SEC);

    char scVoltageBuf[MAX_PATH] = {0};
    char scVoltageValue[MAX_PATH] = {0};
    ret = snprintf_s(scVoltageBuf, MAX_PATH, sizeof(scVoltageBuf) - 1, SC_VOLTAGE_PATH);
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrPolicyTest::ReadFile(scVoltageBuf, scVoltageValue, sizeof(scVoltageValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string valueStr = scVoltageValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(valueStr);
    THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
    EXPECT_EQ(true, value == 2000) << "ThermalMgrPolicyTest062 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest062: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest063
 * @tc.desc: get the config voltage by setting battery temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, High Temp, charge_type: buck
 * @tc.result level 3, voltage 1000
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest063, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest063: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(WAIT_TIME_5_SEC);

    char buckVoltageBuf[MAX_PATH] = {0};
    char buckVoltageValue[MAX_PATH] = {0};
    ret = snprintf_s(buckVoltageBuf, MAX_PATH, sizeof(buckVoltageBuf) - 1, BUCK_VOLTAGE_PATH);
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrPolicyTest::ReadFile(buckVoltageBuf, buckVoltageValue, sizeof(buckVoltageValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string valueStr = buckVoltageValue;
    int32_t value = ThermalMgrPolicyTest::ConvertInt(valueStr);
    THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
    EXPECT_EQ(true, value == 1000) << "ThermalMgrPolicyTest063 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest063: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest064
 * @tc.desc: test get gpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp
 * @tc.result level 1, screen 1, freq 512000
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest064, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest064: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    char stateScreenBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(stateScreenBuf, MAX_PATH, sizeof(stateScreenBuf) - 1, stateScreenPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t stateScreen = 1;
    sTemp = to_string(stateScreen);
    ret = ThermalMgrPolicyTest::WriteFile(stateScreenBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char gpuBuf[MAX_PATH] = {0};
        char freqValue[MAX_PATH] = {0};
        ret = snprintf_s(gpuBuf, MAX_PATH, sizeof(gpuBuf) - 1, GPU_FREQ_PATH);
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(gpuBuf, freqValue, sizeof(freqValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string freq = freqValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(freq);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 512000) << "ThermalMgrPolicyTest064 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest064: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest065
 * @tc.desc: test get gpu freq by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set BATTERY temp
 * @tc.result level 1, screen 0, freq 524288
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest065, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest065: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    char stateScreenBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = snprintf_s(stateScreenBuf, MAX_PATH, sizeof(stateScreenBuf) - 1, stateScreenPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    int32_t stateScreen = 0;
    sTemp = to_string(stateScreen);
    ret = ThermalMgrPolicyTest::WriteFile(stateScreenBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char gpuBuf[MAX_PATH] = {0};
        char freqValue[MAX_PATH] = {0};
        ret = snprintf_s(gpuBuf, MAX_PATH, sizeof(gpuBuf) - 1, GPU_FREQ_PATH);
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(gpuBuf, freqValue, sizeof(freqValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string freq = freqValue;
        int32_t value = ThermalMgrPolicyTest::ConvertInt(freq);
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
        EXPECT_EQ(true, value == 524288) << "ThermalMgrPolicyTest065 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest065: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest066
 * @tc.desc: test set brightness according to the scene
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 1, scene cam, brightness factor is 0.99
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest066, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest066: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("cam");

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char lcdBuf[MAX_PATH] = {0};
        char lcdValue[MAX_PATH]= {0};
        ret = snprintf_s(lcdBuf, MAX_PATH, sizeof(lcdBuf) - 1, lcdPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string lcd = lcdValue;
        THERMAL_HILOGD(LABEL_TEST, "lcd value: %{public}s", lcd.c_str());
        EXPECT_EQ(true, lcd.substr(0, 4) == "0.99") << "ThermalMgrPolicyTest066 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest066: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest067
 * @tc.desc: test set brightness according to the scene
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 2, scene cam, brightness factor is 0.98
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest067, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest067: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("cam");

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char lcdBuf[MAX_PATH] = {0};
        char lcdValue[MAX_PATH]= {0};
        ret = snprintf_s(lcdBuf, MAX_PATH, sizeof(lcdBuf) - 1, lcdPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string lcd = lcdValue;
        THERMAL_HILOGD(LABEL_TEST, "lcd value: %{public}s", lcd.c_str());
        EXPECT_EQ(true, lcd.substr(0, 4) == "0.89") << "ThermalMgrPolicyTest067 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest067: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest068
 * @tc.desc: test set brightness according to the scene
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 3, scene cam, brightness factor is 0.97
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest068, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest068: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("cam");

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char lcdBuf[MAX_PATH] = {0};
        char lcdValue[MAX_PATH]= {0};
        ret = snprintf_s(lcdBuf, MAX_PATH, sizeof(lcdBuf) - 1, lcdPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string lcd = lcdValue;
        THERMAL_HILOGD(LABEL_TEST, "lcd value: %{public}s", lcd.c_str());
        EXPECT_EQ(true, lcd.substr(0, 4) == "0.79") << "ThermalMgrPolicyTest068 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest068: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest069
 * @tc.desc: test set brightness according to the scene
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 1, scene call, brightness factor is 0.98
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest069, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest069: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("call");

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char lcdBuf[MAX_PATH] = {0};
        char lcdValue[MAX_PATH]= {0};
        ret = snprintf_s(lcdBuf, MAX_PATH, sizeof(lcdBuf) - 1, lcdPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string lcd = lcdValue;
        THERMAL_HILOGD(LABEL_TEST, "lcd value: %{public}s", lcd.c_str());
        EXPECT_EQ(true, lcd.substr(0, 4) == "0.98") << "ThermalMgrPolicyTest069 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest069: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest070
 * @tc.desc: test set brightness according to the scene
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 2, scene call, brightness factor is 0.88
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest070, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest070: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("call");

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char lcdBuf[MAX_PATH] = {0};
        char lcdValue[MAX_PATH]= {0};
        ret = snprintf_s(lcdBuf, MAX_PATH, sizeof(lcdBuf) - 1, lcdPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string lcd = lcdValue;
        THERMAL_HILOGD(LABEL_TEST, "lcd value: %{public}s", lcd.c_str());
        EXPECT_EQ(true, lcd.substr(0, 4) == "0.88") << "ThermalMgrPolicyTest070 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest070: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest071
 * @tc.desc: test set brightness according to the scene
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 3, scene call, brightness factor is 0.78
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest071, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest071: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("call");

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char lcdBuf[MAX_PATH] = {0};
        char lcdValue[MAX_PATH]= {0};
        ret = snprintf_s(lcdBuf, MAX_PATH, sizeof(lcdBuf) - 1, lcdPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string lcd = lcdValue;
        THERMAL_HILOGD(LABEL_TEST, "lcd value: %{public}s", lcd.c_str());
        EXPECT_EQ(true, lcd.substr(0, 4) == "0.78") << "ThermalMgrPolicyTest071 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest071: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest072
 * @tc.desc: test set brightness according to the scene
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 1, scene game, brightness factor is 0.97
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest072, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest072: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("game");

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char lcdBuf[MAX_PATH] = {0};
        char lcdValue[MAX_PATH]= {0};
        ret = snprintf_s(lcdBuf, MAX_PATH, sizeof(lcdBuf) - 1, lcdPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string lcd = lcdValue;
        THERMAL_HILOGD(LABEL_TEST, "lcd value: %{public}s", lcd.c_str());
        EXPECT_EQ(true, lcd.substr(0, 4) == "0.97") << "ThermalMgrPolicyTest072 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest072: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest073
 * @tc.desc: test set brightness according to the scene
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 2, scene game, brightness factor is 0.87
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest073, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest073: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("game");

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char lcdBuf[MAX_PATH] = {0};
        char lcdValue[MAX_PATH]= {0};
        ret = snprintf_s(lcdBuf, MAX_PATH, sizeof(lcdBuf) - 1, lcdPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string lcd = lcdValue;
        THERMAL_HILOGD(LABEL_TEST, "lcd value: %{public}s", lcd.c_str());
        EXPECT_EQ(true, lcd.substr(0, 4) == "0.87") << "ThermalMgrPolicyTest073 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest073: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest074
 * @tc.desc: test set brightness according to the scene
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 3, scene game, brightness factor is 0.77
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest074, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest074: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("game");

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char lcdBuf[MAX_PATH] = {0};
        char lcdValue[MAX_PATH]= {0};
        ret = snprintf_s(lcdBuf, MAX_PATH, sizeof(lcdBuf) - 1, lcdPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string lcd = lcdValue;
        THERMAL_HILOGD(LABEL_TEST, "lcd value: %{public}s", lcd.c_str());
        EXPECT_EQ(true, lcd.substr(0, 4) == "0.77") << "ThermalMgrPolicyTest074 failed";
    }

    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest074: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest075
 * @tc.desc: test set brightness according to the scene
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 1, scene test, brightness factor is 0.91
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest075, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest075: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("test");

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char lcdBuf[MAX_PATH] = {0};
        char lcdValue[MAX_PATH]= {0};
        ret = snprintf_s(lcdBuf, MAX_PATH, sizeof(lcdBuf) - 1, lcdPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string lcd = lcdValue;
        THERMAL_HILOGD(LABEL_TEST, "lcd value: %{public}s", lcd.c_str());
        EXPECT_EQ(true, lcd.substr(0, 4) == "0.91") << "ThermalMgrPolicyTest075 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest075: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest076
 * @tc.desc: test set brightness according to the scene
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 1, brightness factor is 1.0; scene game, brightness factor is 0.97
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest076, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest076: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char lcdBuf[MAX_PATH] = {0};
        char lcdValue[MAX_PATH]= {0};
        ret = snprintf_s(lcdBuf, MAX_PATH, sizeof(lcdBuf) - 1, lcdPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string lcd = lcdValue;
        THERMAL_HILOGD(LABEL_TEST, "lcd value: %{public}s", lcd.c_str());
        EXPECT_EQ(true, lcd.substr(0, 3) == "1.0") << "ThermalMgrPolicyTest076 failed";
    }

    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("game");
    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char lcdBuf[MAX_PATH] = {0};
        char lcdValue[MAX_PATH]= {0};
        ret = snprintf_s(lcdBuf, MAX_PATH, sizeof(lcdBuf) - 1, lcdPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string lcd = lcdValue;
        THERMAL_HILOGD(LABEL_TEST, "lcd value: %{public}s", lcd.c_str());
        EXPECT_EQ(true, lcd.substr(0, 4) == "0.97") << "ThermalMgrPolicyTest076 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest076: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest077
 * @tc.desc: test set brightness according to the scene
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 1, scene call, brightness factor is 0.98; scene empty, brightness factor is 1.0
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest077, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest077: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("call");

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char lcdBuf[MAX_PATH] = {0};
        char lcdValue[MAX_PATH]= {0};
        ret = snprintf_s(lcdBuf, MAX_PATH, sizeof(lcdBuf) - 1, lcdPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string lcd = lcdValue;
        THERMAL_HILOGD(LABEL_TEST, "lcd value: %{public}s", lcd.c_str());
        EXPECT_EQ(true, lcd.substr(0, 4) == "0.98") << "ThermalMgrPolicyTest077 failed";
    }

    thermalMgrClient.SetScene("");
    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char lcdBuf[MAX_PATH] = {0};
        char lcdValue[MAX_PATH]= {0};
        ret = snprintf_s(lcdBuf, MAX_PATH, sizeof(lcdBuf) - 1, lcdPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string lcd = lcdValue;
        THERMAL_HILOGD(LABEL_TEST, "lcd value: %{public}s", lcd.c_str());
        EXPECT_EQ(true, lcd.substr(0, 3) == "1.0") << "ThermalMgrPolicyTest077 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest077: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest078
 * @tc.desc: test set brightness according to the scene
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result scene cam, level 1, brightness factor is 0.99
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest078, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest078: start.");
    int32_t ret = -1;
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("cam");

    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char lcdBuf[MAX_PATH] = {0};
        char lcdValue[MAX_PATH]= {0};
        ret = snprintf_s(lcdBuf, MAX_PATH, sizeof(lcdBuf) - 1, lcdPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string lcd = lcdValue;
        THERMAL_HILOGD(LABEL_TEST, "lcd value: %{public}s", lcd.c_str());
        EXPECT_EQ(true, lcd.substr(0, 4) == "0.99") << "ThermalMgrPolicyTest078 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest078: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest079
 * @tc.desc: test set brightness according to the scene
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result scene call, level 2, brightness factor is 0.88
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest079, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest079: start.");
    int32_t ret = -1;
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("call");

    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char lcdBuf[MAX_PATH] = {0};
        char lcdValue[MAX_PATH]= {0};
        ret = snprintf_s(lcdBuf, MAX_PATH, sizeof(lcdBuf) - 1, lcdPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string lcd = lcdValue;
        THERMAL_HILOGD(LABEL_TEST, "lcd value: %{public}s", lcd.c_str());
        EXPECT_EQ(true, lcd.substr(0, 4) == "0.88") << "ThermalMgrPolicyTest079 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest079: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest080
 * @tc.desc: test set brightness according to the scene
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result scene game, level 3, brightness factor is 0.77
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest080, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest080: start.");
    int32_t ret = -1;
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("game");

    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);

        char lcdBuf[MAX_PATH] = {0};
        char lcdValue[MAX_PATH]= {0};
        ret = snprintf_s(lcdBuf, MAX_PATH, sizeof(lcdBuf) - 1, lcdPath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(lcdBuf, lcdValue, sizeof(lcdValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string lcd = lcdValue;
        THERMAL_HILOGD(LABEL_TEST, "lcd value: %{public}s", lcd.c_str());
        EXPECT_EQ(true, lcd.substr(0, 4) == "0.77") << "ThermalMgrPolicyTest080 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest080: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest081
 * @tc.desc: get the config volume by setting battery temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp
 * @tc.result level 1, volume 1.0
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest081, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest081: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);
        char volumeCurrentBuf[MAX_PATH] = {0};
        char volumeCurrentValue[MAX_PATH] = {0};
        ret = snprintf_s(volumeCurrentBuf, MAX_PATH, sizeof(volumeCurrentBuf) - 1, volumePath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(volumeCurrentBuf, volumeCurrentValue, sizeof(volumeCurrentValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string value = volumeCurrentValue;
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}s", value.c_str());
        EXPECT_EQ(true, value.substr(0, 3) == "1.0") << "ThermalMgrPolicyTest081 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest081: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest082
 * @tc.desc: get the config current by setting battery temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp
 * @tc.result level 2, volume 0.8
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest082, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest082: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 43100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);
        char volumeCurrentBuf[MAX_PATH] = {0};
        char volumeCurrentValue[MAX_PATH] = {0};
        ret = snprintf_s(volumeCurrentBuf, MAX_PATH, sizeof(volumeCurrentBuf) - 1, volumePath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(volumeCurrentBuf, volumeCurrentValue, sizeof(volumeCurrentValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string value = volumeCurrentValue;
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}s", value.c_str());
        EXPECT_EQ(true, value.substr(0, 3) == "0.8") << "ThermalMgrPolicyTest082 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest082: end.");
}

/**
 * @tc.name: ThermalMgrPolicyTest083
 * @tc.desc: get the config current by setting battery temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp
 * @tc.result level 3, volume 0.7
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest083, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest083: start.");
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrPolicyTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(vendorConfig.c_str(), 0) != 0) {
        sleep(WAIT_TIME_5_SEC);
        char volumeCurrentBuf[MAX_PATH] = {0};
        char volumeCurrentValue[MAX_PATH] = {0};
        ret = snprintf_s(volumeCurrentBuf, MAX_PATH, sizeof(volumeCurrentBuf) - 1, volumePath.c_str());
        EXPECT_EQ(true, ret >= EOK);
        ret = ThermalMgrPolicyTest::ReadFile(volumeCurrentBuf, volumeCurrentValue, sizeof(volumeCurrentValue));
        EXPECT_EQ(true, ret == ERR_OK);
        std::string value = volumeCurrentValue;
        THERMAL_HILOGD(LABEL_TEST, "value: %{public}s", value.c_str());
        EXPECT_EQ(true, value.substr(0, 3) == "0.7") << "ThermalMgrPolicyTest083 failed";
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrPolicyTest083: end.");
}
}
