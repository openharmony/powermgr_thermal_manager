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


#include "thermal_mgr_system_test.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <fcntl.h>
#include <libxml/tree.h>
#include <mutex>
#include <unistd.h>
#include "securec.h"

#include "constants.h"
#include "mock_thermal_mgr_client.h"
#include "string_operation.h"
#include "thermal_common.h"
#include "thermal_config_file_parser.h"
#include "thermal_log.h"
#include "thermal_mgr_client.h"
#include "thermal_service.h"
#include "thermal_srv_config_parser.h"

using namespace OHOS::HiviewDFX;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
using namespace testing::ext;

namespace {
static sptr<ThermalService> g_service;
static std::map<std::string, SensorInfoMap> g_sensorInfoMap;
static int32_t g_temp = 0;
std::unique_ptr<ThermalConfigFileParser> g_parser;
const std::string SYSTEM_CONFIG = "/vendor/etc/thermal_config/thermal_service_config.xml";
} // namespace

int32_t ThermalMgrSystemTest::WriteFile(std::string path, std::string buf, size_t size)
{
    FILE* stream = fopen(path.c_str(), "w+");
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

int32_t ThermalMgrSystemTest::ReadFile(const char* path, char* buf, size_t size)
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
        ret = snprintf_s(bufTemp, MAX_PATH, sizeof(bufTemp) - 1, SIMULATION_TEMP_DIR.c_str(), iter.first.c_str());
        if (ret < EOK) {
            return ret;
        }
        std::string temp = std::to_string(iter.second) + "\n";
        WriteFile(bufTemp, temp, temp.length());
    }
    return ERR_OK;
}

int32_t ThermalMgrSystemTest::ConvertInt(const std::string& value)
{
    return std::stoi(value);
}

void ThermalMgrSystemTest::SetUpTestCase()
{
    system("setenforce 0");
    system("mount -o rw,remount /vendor");
    g_service = ThermalService::GetInstance();
    g_service->InitSystemTestModules();
    g_parser = std::make_unique<ThermalConfigFileParser>();
    g_parser->ParseXmlFile(SYSTEM_CONFIG);
}

void ThermalMgrSystemTest::TearDownTestCase()
{
    system("setenforce 1");
}

void ThermalMgrSystemTest::SetUp()
{
    InitNode();
}

void ThermalMgrSystemTest::TearDown()
{
}

static void GetSystemTestTemp(std::vector<LevelItem>& iter, const int32_t& needLevel)
{
    THERMAL_HILOGD(LABEL_TEST, "GetSystemTestTemp: start.");
    for (auto info : iter) {
        THERMAL_HILOGD(LABEL_TEST, "info.level = %{public}d", info.level);
        if (info.level == static_cast<uint32_t>(needLevel)) {
            g_temp = info.threshold;
        }
    }
}

static void GetSensorName(SensorInfoMap& info, const std::string& sensorName, const int32_t& needLevel)
{
    THERMAL_HILOGD(LABEL_TEST, "GetSensorName: start.");
    for (auto iter = info.begin(); iter != info.end(); ++iter) {
        THERMAL_HILOGD(COMP_SVC, "SENSOR name = %{public}s", iter->first.c_str());
        if (iter->first == sensorName) {
            GetSystemTestTemp(iter->second, needLevel);
        }
    }
}

static void GetSensorClusterName(const std::string& sensorClusterName, const std::string& sensorName,
    const int32_t& needLevel)
{
    THERMAL_HILOGD(LABEL_TEST, "GetSensorClusterName: start.");
    if (g_parser == nullptr) {
        THERMAL_HILOGD(LABEL_TEST, "g_parser: is nullptr.");
        return;
    }

    std::map<std::string, SensorInfoMap> sensorInfoMap = g_parser->GetSensorInfoMap();
    for (auto info = sensorInfoMap.begin(); info != sensorInfoMap.end(); ++info) {
        THERMAL_HILOGD(COMP_SVC, "sensor_cluster name = %{public}s", info->first.c_str());
        if (info->first == sensorClusterName) {
            GetSensorName(info->second, sensorName, needLevel);
        }
    }
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
    char batteryTempBuf[MAX_PATH] = {0};
    int32_t ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    GetSensorClusterName("base_safe", "battery", 1);
    int32_t batteryTemp = g_temp + 100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(VENDOR_CONFIG.c_str(), 0) != 0) {
        THERMAL_HILOGD(LABEL_TEST, "SIMUL PATH has been in ");
        return;
    }

    MockThermalMgrClient::GetInstance().GetThermalInfo();

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, CONFIG_LEVEL_PATH.c_str());
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
    char batteryTempBuf[MAX_PATH] = {0};
    int32_t ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    GetSensorClusterName("base_safe", "battery", 2);
    int32_t batteryTemp = g_temp +100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(VENDOR_CONFIG.c_str(), 0) != 0) {
        THERMAL_HILOGD(LABEL_TEST, "SIMUL PATH has been in ");
        return;
    }

    MockThermalMgrClient::GetInstance().GetThermalInfo();

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, CONFIG_LEVEL_PATH.c_str());
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
    char batteryTempBuf[MAX_PATH] = {0};
    int32_t ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    GetSensorClusterName("base_safe", "battery", 3);
    int32_t batteryTemp = g_temp + 100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(VENDOR_CONFIG.c_str(), 0) != 0) {
        THERMAL_HILOGD(LABEL_TEST, "SIMUL PATH has been in ");
        return;
    }

    MockThermalMgrClient::GetInstance().GetThermalInfo();

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, CONFIG_LEVEL_PATH.c_str());
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
    char batteryTempBuf[MAX_PATH] = {0};
    int32_t ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    GetSensorClusterName("base_safe", "battery", 4);
    int32_t batteryTemp = g_temp + 100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(VENDOR_CONFIG.c_str(), 0) != 0) {
        THERMAL_HILOGD(LABEL_TEST, "SIMUL PATH has been in ");
        return;
    }

    MockThermalMgrClient::GetInstance().GetThermalInfo();

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, CONFIG_LEVEL_PATH.c_str());
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
    char batteryTempBuf[MAX_PATH] = {0};
    int32_t ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    GetSensorClusterName("base_safe", "battery", 1);
    int32_t batteryTemp = g_temp + 100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    std::string level;
    int32_t value;
    if (access(VENDOR_CONFIG.c_str(), 0) != 0) {
        THERMAL_HILOGD(LABEL_TEST, "SIMUL PATH has been in ");
        return;
    }

    MockThermalMgrClient::GetInstance().GetThermalInfo();
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, CONFIG_LEVEL_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 1) << "ThermalMgrSystemTest005 failed";
 
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    GetSensorClusterName("base_safe", "battery", 4);
    batteryTemp = g_temp + 100;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    if (access(VENDOR_CONFIG.c_str(), 0) != 0) {
        return;
    }

    MockThermalMgrClient::GetInstance().GetThermalInfo();
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, CONFIG_LEVEL_PATH.c_str());
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
    char batteryTempBuf[MAX_PATH] = {0};
    int32_t ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    GetSensorClusterName("base_safe", "battery", 2);
    int32_t batteryTemp = g_temp + 100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    std::string level;
    int32_t value;
    if (access(VENDOR_CONFIG.c_str(), 0) != 0) {
        return;
    }

    MockThermalMgrClient::GetInstance().GetThermalInfo();
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, CONFIG_LEVEL_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 2) << "ThermalMgrSystemTest006 failed";
 
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    GetSensorClusterName("base_safe", "battery", 4);
    batteryTemp = g_temp + 100;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    if (access(VENDOR_CONFIG.c_str(), 0) != 0) {
        return;
    }

    MockThermalMgrClient::GetInstance().GetThermalInfo();
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, CONFIG_LEVEL_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrSystemTest::ConvertInt(level);
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
    char batteryTempBuf[MAX_PATH] = {0};
    int32_t ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    GetSensorClusterName("base_safe", "battery", 4);
    int32_t batteryTemp = g_temp + 100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    std::string level;
    int32_t value;
    if (access(VENDOR_CONFIG.c_str(), 0) != 0) {
        return;
    }

    MockThermalMgrClient::GetInstance().GetThermalInfo();
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, CONFIG_LEVEL_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 4) << "ThermalMgrSystemTest007 failed";
 
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    GetSensorClusterName("base_safe", "battery", 1);
    batteryTemp = g_temp + 100;

    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    if (access(VENDOR_CONFIG.c_str(), 0) != 0) {
        return;
    }

    MockThermalMgrClient::GetInstance().GetThermalInfo();
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, CONFIG_LEVEL_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrSystemTest::ConvertInt(level);
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
    char batteryTempBuf[MAX_PATH] = {0};
    int32_t ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    GetSensorClusterName("base_safe", "battery", 3);
    int32_t batteryTemp = g_temp + 100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    std::string level;
    int32_t value;
    if (access(VENDOR_CONFIG.c_str(), 0) != 0) {
        return;
    }

    MockThermalMgrClient::GetInstance().GetThermalInfo();
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) -1, CONFIG_LEVEL_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 3) << "ThermalMgrSystemTest008 failed";
 
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    GetSensorClusterName("base_safe", "battery", 1);
    batteryTemp = g_temp - 3000;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    if (access(VENDOR_CONFIG.c_str(), 0) != 0) {
        return;
    }

    MockThermalMgrClient::GetInstance().GetThermalInfo();
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, CONFIG_LEVEL_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 0) << "ThermalMgrSystemTest008 failed";
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

    char batteryTempBuf[MAX_PATH] = {0};
    int32_t ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    GetSensorClusterName("cold_safe", "battery", 1);
    int32_t batteryTemp = g_temp - 2000;
    THERMAL_HILOGD(LABEL_TEST, "TEMP: %{public}d", batteryTemp);
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(VENDOR_CONFIG.c_str(), 0) != 0) {
        THERMAL_HILOGD(LABEL_TEST, "SIMUL PATH has been in ");
        return;
    }

    MockThermalMgrClient::GetInstance().GetThermalInfo();

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, CONFIG_LEVEL_PATH.c_str());
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

    char batteryTempBuf[MAX_PATH] = {0};
    int32_t ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    GetSensorClusterName("cold_safe", "battery", 2);
    int32_t batteryTemp = g_temp - 2000;
    THERMAL_HILOGD(LABEL_TEST, "TEMP: %{public}d", batteryTemp);
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(VENDOR_CONFIG.c_str(), 0) != 0) {
        THERMAL_HILOGD(LABEL_TEST, "SIMUL PATH has been in ");
        return;
    }

    MockThermalMgrClient::GetInstance().GetThermalInfo();

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, CONFIG_LEVEL_PATH.c_str());
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

    char batteryTempBuf[MAX_PATH] = {0};
    int32_t ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    GetSensorClusterName("cold_safe", "battery", 3);
    int32_t batteryTemp = g_temp - 1000;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(VENDOR_CONFIG.c_str(), 0) != 0) {
        THERMAL_HILOGD(LABEL_TEST, "SIMUL PATH has been in ");
        return;
    }

    MockThermalMgrClient::GetInstance().GetThermalInfo();

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, CONFIG_LEVEL_PATH.c_str());
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

    char batteryTempBuf[MAX_PATH] = {0};
    int32_t ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    GetSensorClusterName("cold_safe", "battery", 4);
    int32_t batteryTemp = g_temp - 2000;
    THERMAL_HILOGD(LABEL_TEST, "TEMP: %{public}d", batteryTemp);
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(VENDOR_CONFIG.c_str(), 0) != 0) {
        THERMAL_HILOGD(LABEL_TEST, "SIMUL PATH has been in ");
        return;
    }

    MockThermalMgrClient::GetInstance().GetThermalInfo();

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, CONFIG_LEVEL_PATH.c_str());
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

    char batteryTempBuf[MAX_PATH] = {0};
    int32_t ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    GetSensorClusterName("cold_safe", "battery", 1);
    int32_t batteryTemp = g_temp - 2000;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    std::string level;
    int32_t value;
    if (access(VENDOR_CONFIG.c_str(), 0) != 0) {
        return;
    }

    MockThermalMgrClient::GetInstance().GetThermalInfo();
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, CONFIG_LEVEL_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 1) << "ThermalMgrSystemTest013 failed";

    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    GetSensorClusterName("cold_safe", "battery", 4);
    batteryTemp = g_temp - 2000;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    if (access(VENDOR_CONFIG.c_str(), 0) != 0) {
        return;
    }

    MockThermalMgrClient::GetInstance().GetThermalInfo();
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, CONFIG_LEVEL_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrSystemTest::ConvertInt(level);
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

    char batteryTempBuf[MAX_PATH] = {0};
    int32_t ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    GetSensorClusterName("cold_safe", "battery", 2);
    int32_t batteryTemp = g_temp - 2000;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    std::string level;
    int32_t value;
    if (access(VENDOR_CONFIG.c_str(), 0) != 0) {
        return;
    }

    MockThermalMgrClient::GetInstance().GetThermalInfo();
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, CONFIG_LEVEL_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 2) << "ThermalMgrSystemTest014 failed";

    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    GetSensorClusterName("cold_safe", "battery", 4);
    batteryTemp = g_temp - 2000;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    if (access(VENDOR_CONFIG.c_str(), 0) != 0) {
        return;
    }

    MockThermalMgrClient::GetInstance().GetThermalInfo();
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, CONFIG_LEVEL_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrSystemTest::ConvertInt(level);
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

    char batteryTempBuf[MAX_PATH] = {0};
    int32_t ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    GetSensorClusterName("cold_safe", "battery", 4);
    int32_t batteryTemp = g_temp - 2000;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    std::string level;
    int32_t value;
    if (access(VENDOR_CONFIG.c_str(), 0) != 0) {
        return;
    }

    MockThermalMgrClient::GetInstance().GetThermalInfo();
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, CONFIG_LEVEL_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 4) << "ThermalMgrSystemTest015 failed";
 
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    GetSensorClusterName("cold_safe", "battery", 1);
    batteryTemp = g_temp - 2000;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    if (access(VENDOR_CONFIG.c_str(), 0) != 0) {
        return;
    }

    MockThermalMgrClient::GetInstance().GetThermalInfo();
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, CONFIG_LEVEL_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrSystemTest::ConvertInt(level);
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

    char batteryTempBuf[MAX_PATH] = {0};
    int32_t ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    GetSensorClusterName("cold_safe", "battery", 3);
    int32_t batteryTemp = g_temp - 1000;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    std::string level;
    int32_t value;
    if (access(VENDOR_CONFIG.c_str(), 0) != 0) {
        return;
    }

    MockThermalMgrClient::GetInstance().GetThermalInfo();
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, CONFIG_LEVEL_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 3) << "ThermalMgrSystemTest016 failed";

    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    GetSensorClusterName("cold_safe", "battery", 1);
    batteryTemp = g_temp + 10000;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    if (access(VENDOR_CONFIG.c_str(), 0) != 0) {
        return;
    }

    MockThermalMgrClient::GetInstance().GetThermalInfo();
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, CONFIG_LEVEL_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    level = levelValue;
    value = ThermalMgrSystemTest::ConvertInt(level);
    EXPECT_EQ(true, value == 0) << "ThermalMgrSystemTest016 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest016: end.");
}

/**
 * @tc.name: ThermalMgrSystemTest017
 * @tc.desc: test get process value by setting temp
 * @tc.type: FEATURE
  * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 1 procss 3
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest017, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest017: start.");
    char batteryTempBuf[MAX_PATH] = {0};
    int32_t ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);

    GetSensorClusterName("base_safe", "battery", 1);
    int32_t batteryTemp = g_temp + 1000;
    THERMAL_HILOGD(LABEL_TEST, "TEMP: %{public}d", batteryTemp);
    std::string sTemp = to_string(batteryTemp);
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(VENDOR_CONFIG.c_str(), 0) != 0) {
        THERMAL_HILOGD(LABEL_TEST, "SIMUL PATH has been in ");
        return;
    }

    MockThermalMgrClient::GetInstance().GetThermalInfo();

    char procsessBuf[MAX_PATH] = {0};
    char procsesValue[MAX_PATH] = {0};
    ret = snprintf_s(procsessBuf, MAX_PATH, sizeof(procsessBuf) - 1, PROCESS_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);

    ret = ThermalMgrSystemTest::ReadFile(procsessBuf, procsesValue, sizeof(procsesValue));
    EXPECT_EQ(true, ret == ERR_OK);

    std::string process = procsesValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(process);
    THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
    EXPECT_EQ(true, value == 3) << "ThermalMgrSystemTest017 failed";

    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest017: end.");
}

/**
 * @tc.name: ThermalMgrSystemTest018
 * @tc.desc: test get process value by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 2 procss 1
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest018, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest018: start.");
    char batteryTempBuf[MAX_PATH] = {0};
    int32_t ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);

    GetSensorClusterName("base_safe", "battery", 2);
    int32_t batteryTemp = g_temp + 1000;
    THERMAL_HILOGD(LABEL_TEST, "TEMP: %{public}d", batteryTemp);
    std::string sTemp = to_string(batteryTemp);
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    if (access(VENDOR_CONFIG.c_str(), 0) != 0) {
        THERMAL_HILOGD(LABEL_TEST, "SIMUL PATH has been in ");
        return;
    }

    MockThermalMgrClient::GetInstance().GetThermalInfo();

    char procsessBuf[MAX_PATH] = {0};
    char procsesValue[MAX_PATH] = {0};
    ret = snprintf_s(procsessBuf, MAX_PATH, sizeof(procsessBuf) - 1, PROCESS_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(procsessBuf, procsesValue, sizeof(procsesValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string process = procsesValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(process);
    THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
    EXPECT_EQ(true, value == 2) << "ThermalMgrSystemTest018 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest018: end.");
}

/**
 * @tc.name: ThermalMgrSystemTest019
 * @tc.desc: test get process value by setting temp
 * @tc.type: FEATURE
 * @tc.cond: Set Battery temp, Lower Temp
 * @tc.result level 3 procss 1
 */
HWTEST_F (ThermalMgrSystemTest, ThermalMgrSystemTest019, Function|MediumTest|Level2)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest019: start.");

    char batteryTempBuf[MAX_PATH] = {0};
    int32_t ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    GetSensorClusterName("base_safe", "battery", 3);
    int32_t batteryTemp = g_temp + 1000;
    THERMAL_HILOGD(LABEL_TEST, "TEMP: %{public}d", batteryTemp);
    std::string sTemp = to_string(batteryTemp);
    ret = ThermalMgrSystemTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    if (access(VENDOR_CONFIG.c_str(), 0) != 0) {
        THERMAL_HILOGD(LABEL_TEST, "SIMUL PATH has been in ");
        return;
    }

    MockThermalMgrClient::GetInstance().GetThermalInfo();
    char procsessBuf[MAX_PATH] = {0};
    char procsesValue[MAX_PATH] = {0};
    ret = snprintf_s(procsessBuf, MAX_PATH, sizeof(procsessBuf) - 1, PROCESS_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    ret = ThermalMgrSystemTest::ReadFile(procsessBuf, procsesValue, sizeof(procsesValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string process = procsesValue;
    int32_t value = ThermalMgrSystemTest::ConvertInt(process);
    THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
    EXPECT_EQ(true, value == 1) << "ThermalMgrSystemTest019 failed";
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrSystemTest019: end.");
}
}

