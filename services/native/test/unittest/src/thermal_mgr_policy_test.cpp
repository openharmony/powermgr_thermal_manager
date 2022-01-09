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

#include "thermal_mgr_policy_test.h"

#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <fcntl.h>
#include <unistd.h>
#include "securec.h"

#include "thermal_service.h"
#include "thermal_mgr_client.h"
#include "thermal_action.h"
#include "thermal_common.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

static sptr<ThermalService> service;
static std::mutex g_mtx;

static int32_t WriteFile(std::string path, std::string buf, size_t size)
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

static int32_t ReadFile(const char *path, char *buf, size_t size)
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

static int32_t ConvertInt(const std::string &value)
{
    return std::stoi(value);
}

void ThermalMgrPolicyTest::SetUpTestCase(void)
{
    service = DelayedSpSingleton<ThermalService>::GetInstance();
    service->OnStart();
}

void ThermalMgrPolicyTest::TearDownTestCase(void)
{
    service->OnStop();
    DelayedSpSingleton<ThermalService>::DestroyInstance();
}

void ThermalMgrPolicyTest::SetUp(void)
{
}

void ThermalMgrPolicyTest::TearDown(void)
{
}

namespace {
/**
 * @tc.name: ThermalMgrPolicyTest001
 * @tc.desc: Verify to get battery current value by set battery temp in level 2 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest001, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char currentBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest001: Failed to get ThermalService";
    }
    int32_t temp = 45000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    snprintf_s(currentBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryCurrentPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest001: Failed to write file ";
    }
    sleep(WAIT_TIME);
    ret = ReadFile(currentBuf, currentValue, sizeof(currentValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest001: Failed to read file ";
    }
    std::string current = currentValue;
    value = ConvertInt(current);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest001: current is: " << value;
    ASSERT_TRUE(value == 1500) << "failed to get current value";
}

/**
 * @tc.name: ThermalMgrPolicyTest002
 * @tc.desc: Verify to get cpu freq value by set battery temp in level 2 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest002, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;

    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest002: Failed to get ThermalService";
    }
    int32_t temp = 45000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, cpuFreqPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest002: Failed to write file ";
    }

    sleep(WAIT_TIME);
    ret = ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest002: Failed to read file ";
    }
    std::string freq = freqValue;
    value = ConvertInt(freq);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest002: freq is: " << value;
    ASSERT_TRUE(value == 90000) << "failed to get freq value";
}

/**
 * @tc.name: ThermalMgrPolicyTest003
 * @tc.desc: Verify to get battery current value by set battery temp in level 1 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest003, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char currentBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest003: Failed to get ThermalService";
    }
    int32_t temp = 42000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    snprintf_s(currentBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryCurrentPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest003: Failed to write file ";
    }
    sleep(WAIT_TIME);
    ret = ReadFile(currentBuf, currentValue, sizeof(currentValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest003: Failed to read file ";
    }
    std::string current = currentValue;
    value = ConvertInt(current);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest003: current is: " << value;
    ASSERT_TRUE(value == 1800) << "failed to get current value";
}

/**
 * @tc.name: ThermalMgrPolicyTest004
 * @tc.desc: Verify to get cpu freq value by set battery temp in level 1 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest004, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;

    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest004: Failed to get ThermalService";
    }
    int32_t temp = 42000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, cpuFreqPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest004: Failed to write file ";
    }

    sleep(WAIT_TIME);
    ret = ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest004: Failed to read file ";
    }
    std::string freq = freqValue;
    value = ConvertInt(freq);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest004: freq is: " << value;
    ASSERT_TRUE(value == 99000) << "failed to get freq value";
}

/**
 * @tc.name: ThermalMgrPolicyTest005
 * @tc.desc: Verify to get battery current value by set battery temp and camare scene in level 2 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest005, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char currentBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest005: Failed to get ThermalService";
    }
    tms->GetStateMachine()->SetSceneState(true);
    int32_t temp = 45000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    snprintf_s(currentBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryCurrentPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest005: Failed to write file ";
    }
    sleep(WAIT_TIME);
    ret = ReadFile(currentBuf, currentValue, sizeof(currentValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest005: Failed to read file ";
    }
    std::string current = currentValue;
    value = ConvertInt(current);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest005: current is: " << value;
    ASSERT_TRUE(value == 1000) << "failed to get current value";
}

/**
 * @tc.name: ThermalMgrPolicyTest006
 * @tc.desc: Verify to get brightness value by set battery temp in level 2 of warm 5G mode.
 * @tc.cond: aux temp - 5000_35000
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest006, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char auxtempBuf[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest006: Failed to get ThermalService";
    }
    int32_t temp = 44000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, paPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest006: Failed to write file ";
    }

    int32_t amtemp = 32000;
    std::string sAmbientTemp = to_string(amtemp) + "\n";
    snprintf_s(auxtempBuf, PATH_MAX, sizeof(auxtempBuf) - 1, ambientPath.c_str());
    ret = WriteFile(auxtempBuf, sAmbientTemp, sAmbientTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest006: Failed to write file ";
    }
    sleep(WAIT_TIME);
    value = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::THERMAL_FLAG_LCD_BL_LIMIT);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest006: Brightness " << value;
    ASSERT_TRUE(value == 100) << "failed to get brightness value";
}

/**
 * @tc.name: ThermalMgrPolicyTest007
 * @tc.desc: Verify to get brightness value by set battery temp in level 2 of warm 5G mode.
 * @tc.cond: aux temp - 35000_900000
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest007, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char auxtempBuf[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest007: Failed to get ThermalService";
    }
    int32_t temp = 46000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, paPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest007: Failed to write file ";
    }

    int32_t amtemp = 40000;
    std::string sAmbientTemp = to_string(amtemp) + "\n";
    snprintf_s(auxtempBuf, PATH_MAX, sizeof(auxtempBuf) - 1, ambientPath.c_str());
    ret = WriteFile(auxtempBuf, sAmbientTemp, sAmbientTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest007: Failed to write file ";
    }
    sleep(WAIT_TIME);
    value = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::THERMAL_FLAG_LCD_BL_LIMIT);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest007: Brightness " << value;
    ASSERT_TRUE(value == 100) << "failed to get brightness value";
}

/**
 * @tc.name: ThermalMgrPolicyTest008
 * @tc.desc: Verify to get brightness value by set battery temp in level 1 of warm 5G mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest008, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char auxtempBuf[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest008: Failed to get ThermalService";
    }
    int32_t temp = 42000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, paPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest008: Failed to write file ";
    }

    int32_t amtemp = 32000;
    std::string sAmbientTemp = to_string(amtemp) + "\n";
    snprintf_s(auxtempBuf, PATH_MAX, sizeof(auxtempBuf) - 1, ambientPath.c_str());
    ret = WriteFile(auxtempBuf, sAmbientTemp, sAmbientTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest008: Failed to write file ";
    }
    sleep(WAIT_TIME);
    value = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::THERMAL_FLAG_LCD_BL_LIMIT);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest008: Brightness " << value;
    ASSERT_TRUE(value == 130) << "failed to get brightness value";
}

/**
 * @tc.name: ThermalMgrPolicyTest009
 * @tc.desc: Verify to get battery current value by set battery temp in level 1 of code safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest009, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char currentBuf[MAX_PATH] = {0};
    int32_t ret = -1, value = 0, level = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest009: Failed to get ThermalService";
    }
    int32_t temp = -13000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    snprintf_s(currentBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryCurrentPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest009: Failed to write file ";
    }
    sleep(WAIT_TIME);
    level = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::THERMAL_FLAG_LEVEL_NOTIFY);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest009: level is: " << level;
    ASSERT_TRUE(level == 0) << "failed to get level value";
    value = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::THERMAL_FLAG_BATTERY_LIMIT);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest009: current is: " << value;
    ASSERT_TRUE(value == 1850) << "failed to get current value";
}

/**
 * @tc.name: ThermalMgrPolicyTest010
 * @tc.desc: Verify to get battery current value by set battery temp in level 2 of code safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest010, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char currentBuf[MAX_PATH] = {0};
    int32_t ret = -1, value = 0, level = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest010: Failed to get ThermalService";
    }
    int32_t temp = -15000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    snprintf_s(currentBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryCurrentPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest010: Failed to write file ";
    }
    sleep(WAIT_TIME);
    level = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::THERMAL_FLAG_LEVEL_NOTIFY);
    ASSERT_TRUE(level == 0) << "failed to get level value";
    value = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::THERMAL_FLAG_BATTERY_LIMIT);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest010: current is: " << value;
    ASSERT_TRUE(value == 1550) << "failed to get current value";
}

/**
 * @tc.name: ThermalMgrPolicyTest011
 * @tc.desc: Verify to get battery current value by set battery temp in level 3 of code safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest011, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char currentBuf[MAX_PATH] = {0};
    int32_t ret = -1, value = 0, level = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest011: Failed to get ThermalService";
    }
    int32_t temp = -20000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    snprintf_s(currentBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryCurrentPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest011: Failed to write file ";
    }
    sleep(WAIT_TIME);
    level = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::THERMAL_FLAG_LEVEL_NOTIFY);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest012: level is: " << level;
    ASSERT_TRUE(level == 0) << "failed to get level value";
    value = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::THERMAL_FLAG_BATTERY_LIMIT);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest012: current is: " << value;
    ASSERT_TRUE(value == 1150) << "failed to get current value";
}

/**
 * @tc.name: ThermalMgrPolicyTest012
 * @tc.desc: Verify to get battery current value by set battery temp in level 0 of code safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest012, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char currentBuf[MAX_PATH] = {0};
    int32_t ret = -1, value = 0, level = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest012: Failed to get ThermalService";
    }
    int32_t temp = -8000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    snprintf_s(currentBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryCurrentPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest012: Failed to write file ";
    }
    sleep(WAIT_TIME);
    level = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::THERMAL_FLAG_LEVEL_NOTIFY);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest012: level is: " << level;

    value = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::THERMAL_FLAG_BATTERY_LIMIT);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest012: current is: " << value;
    ASSERT_TRUE(value > 0) << "failed to Get process action value";
}

/**
 * @tc.name: ThermalMgrPolicyTest013
 * @tc.desc: Verify delay shutdown and repeatedly kill process by set ap temp in level 1 of warm safe mode
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest013, TestSize.Level2)
{
    char apBuf[MAX_PATH] = {0};
    char ambientBuf[MAX_PATH] = {0};
    char batteryBuf[MAX_PATH] = {0};

    int32_t ret = -1, shutdownValue = 0, processValue = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest013: Failed to get ThermalService";
    }
    tms->GetStateMachine()->SetStateFlag(false);
    tms->GetStateMachine()->SetState(1);
    int32_t aptemp = 80000;
    std::string sApTemp = to_string(aptemp) + "\n";
    snprintf_s(apBuf, PATH_MAX, sizeof(apBuf) - 1, apPath.c_str());
    ret = WriteFile(apBuf, sApTemp, sApTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest013: Failed to write file ";
    }

    int32_t amTemp = 50000;
    std::string sAmTemp = to_string(amTemp) + "\n";
    snprintf_s(ambientBuf, PATH_MAX, sizeof(ambientBuf) - 1, ambientPath.c_str());
    ret = WriteFile(ambientBuf, sAmTemp, sAmTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest013: Failed to write file ";
    }
    int32_t baTemp = 60000;
    std::string sBaTemp = to_string(baTemp) + "\n";
    snprintf_s(batteryBuf, PATH_MAX, sizeof(batteryBuf) - 1, batteryPath.c_str());
    ret = WriteFile(batteryBuf, sBaTemp, sBaTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest013: Failed to write file ";
    }
    sleep(WAIT_TIME);
    shutdownValue = tms->GetThermalAction()->GetPolicyActionHubValue(ThermalAction::THERMAL_HUB_POWER_SHUTDOWN);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest013: shutdown value is: " << shutdownValue;
    ASSERT_TRUE(shutdownValue == 1) << "failed to Get shutdown value";

    processValue = tms->GetThermalAction()->GetPolicyActionHubValue(ThermalAction::THERMAL_HUB_CLEAR_ALL_PROCESS);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest013: process value is: " << processValue;
    ASSERT_TRUE(processValue == 1) << "failed to Get process action value";
}

/**
 * @tc.name: ThermalMgrPolicyTest014
 * @tc.desc: Verify to get gear value by set battery temp in level 2 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest014, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest014: Failed to get ThermalService";
    }
    int32_t temp = 45000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest014: Failed to write file ";
    }
    sleep(WAIT_TIME);
    value = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::THERMAL_FLAG_LEVEL_NOTIFY);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest014: level is: " << value;
    ASSERT_TRUE(value == 2) << "failed to Get level value";
}

/**
 * @tc.name: ThermalMgrPolicyTest015
 * @tc.desc: Verify to get gear value by set battery temp in level 1 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest015, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char currentBuf[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest015: Failed to get ThermalService";
    }
    int32_t temp = 42000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    snprintf_s(currentBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryCurrentPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest015: Failed to write file ";
    }
    sleep(WAIT_TIME);
    value = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::THERMAL_FLAG_LEVEL_NOTIFY);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest015: level is: " << value;
    ASSERT_TRUE(value == 1) << "failed to get level value";
}

/**
 * @tc.name: ThermalMgrPolicyTest016
 * @tc.desc: Verify to get gear value by set battery temp in level 3 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest016, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char currentBuf[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest016: Failed to get ThermalService";
    }
    int32_t temp = 48000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    snprintf_s(currentBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryCurrentPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest016: Failed to write file ";
    }
    sleep(WAIT_TIME);
    value = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::THERMAL_FLAG_LEVEL_NOTIFY);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest016: level is: " << value;
    ASSERT_TRUE(value == 3) << "failed to get level value";
}

/**
 * @tc.name: ThermalMgrPolicyTest017
 * @tc.desc: Verify delay shutdown and repeatedly kill process by set ap temp in level 1 of warm safe mode
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest017, TestSize.Level2)
{
    char apBuf[MAX_PATH] = {0};
    char ambientBuf[MAX_PATH] = {0};
    char batteryBuf[MAX_PATH] = {0};

    int32_t ret = -1, shutdownValue = 0, processValue = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest017: Failed to get ThermalService";
    }
    tms->GetStateMachine()->SetStateFlag(false);
    tms->GetStateMachine()->SetState(1);
    int32_t aptemp = 90000;
    std::string sApTemp = to_string(aptemp) + "\n";
    snprintf_s(apBuf, PATH_MAX, sizeof(apBuf) - 1, apPath.c_str());
    ret = WriteFile(apBuf, sApTemp, sApTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest017: Failed to write file ";
    }

    int32_t amTemp = 60000;
    std::string sAmTemp = to_string(amTemp) + "\n";
    snprintf_s(ambientBuf, PATH_MAX, sizeof(ambientBuf) - 1, ambientPath.c_str());
    ret = WriteFile(ambientBuf, sAmTemp, sAmTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest017: Failed to write file ";
    }
    int32_t baTemp = 70000;
    std::string sBaTemp = to_string(baTemp) + "\n";
    snprintf_s(batteryBuf, PATH_MAX, sizeof(batteryBuf) - 1, batteryPath.c_str());
    ret = WriteFile(batteryBuf, sBaTemp, sBaTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest017: Failed to write file ";
    }
    sleep(WAIT_TIME);
    shutdownValue = tms->GetThermalAction()->GetPolicyActionHubValue(ThermalAction::THERMAL_HUB_POWER_SHUTDOWN);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest017: shutdown value is: " << shutdownValue;
    ASSERT_TRUE(shutdownValue == 1) << "failed to Get shutdown value";

    processValue = tms->GetThermalAction()->GetPolicyActionHubValue(ThermalAction::THERMAL_HUB_CLEAR_ALL_PROCESS);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest017: process value is: " << processValue;
    ASSERT_TRUE(processValue == 1) << "failed to Get process action value";
}

/**
 * @tc.name: ThermalMgrPolicyTest018
 * @tc.desc: Verify to get process_ctrl value by set battery temp in level 1 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest018, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char currentBuf[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest018: Failed to get ThermalService";
    }
    int32_t temp = 42000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    snprintf_s(currentBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryCurrentPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest018: Failed to write file ";
    }
    sleep(WAIT_TIME);
    value = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::ThERMAL_FLAG_CLEAR_BG_CONTROL);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest018: process value is: " << value;
    ASSERT_TRUE(value == 3) << "failed to Get process action value";
}

/**
 * @tc.name: ThermalMgrPolicyTest019
 * @tc.desc: Verify to get process_ctrl value by set battery temp in level 2 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest019, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char currentBuf[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest019: Failed to get ThermalService";
    }
    int32_t temp = 45000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    snprintf_s(currentBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryCurrentPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest019: Failed to write file ";
    }
    sleep(WAIT_TIME);
    value = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::ThERMAL_FLAG_CLEAR_BG_CONTROL);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest019: process value  is: " << value;
    ASSERT_TRUE(value == 2) << "failed to Get process action value";
}

/**
 * @tc.name: ThermalMgrPolicyTest020
 * @tc.desc: Verify to get process_ctrl value by set battery temp in level 3 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest020, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char currentBuf[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest020: Failed to get ThermalService";
    }
    int32_t temp = 49000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    snprintf_s(currentBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryCurrentPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest020: Failed to write file ";
    }
    sleep(WAIT_TIME);
    value = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::ThERMAL_FLAG_CLEAR_BG_CONTROL);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest020: process value  is: " << value;
    ASSERT_TRUE(value == 1) << "failed to Get process action value";
}

/**
 * @tc.name: ThermalMgrPolicyTest021
 * @tc.desc: Verify to get battery current value by set battery temp in level 3 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest021, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char currentBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest021: Failed to get ThermalService";
    }
    tms->GetStateMachine()->SetSceneState(false);
    int32_t temp = 48000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    snprintf_s(currentBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryCurrentPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest021: Failed to write file ";
    }
    sleep(WAIT_TIME);
    ret = ReadFile(currentBuf, currentValue, sizeof(currentValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest021: Failed to read file ";
    }
    std::string current = currentValue;
    value = ConvertInt(current);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest021: current is: " << value;
    ASSERT_TRUE(value == 1300) << "failed to get current value";
}

/**
 * @tc.name: ThermalMgrPolicyTest022
 * @tc.desc: Verify to get cpu freq value by set battery temp in level 3 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest022, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;

    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest022: Failed to get ThermalService";
    }

    int32_t temp = 50000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, cpuFreqPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest022: Failed to write file ";
    }

    sleep(WAIT_TIME);
    ret = ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest022: Failed to read file ";
    }
    std::string freq = freqValue;
    value = ConvertInt(freq);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest022: freq is: " << value;
    ASSERT_TRUE(value == 80000) << "failed to get freq value";
}

/**
 * @tc.name: ThermalMgrPolicyTest005
 * @tc.desc: Verify to get battery current value by set battery temp and camare scene in level 1 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest023, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char currentBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest023: Failed to get ThermalService";
    }
    tms->GetStateMachine()->SetSceneState(true);
    int32_t temp = 42000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    snprintf_s(currentBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryCurrentPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest023: Failed to write file ";
    }
    sleep(WAIT_TIME);
    ret = ReadFile(currentBuf, currentValue, sizeof(currentValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest023: Failed to read file ";
    }
    std::string current = currentValue;
    value = ConvertInt(current);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest023: current is: " << value;
    ASSERT_TRUE(value == 1200) << "failed to get current value";
}

/**
 * @tc.name: ThermalMgrPolicyTest024
 * @tc.desc: Verify to get battery current value by set battery temp and camare scene in level 3 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest024, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char currentBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest024: Failed to get ThermalService";
    }
    tms->GetStateMachine()->SetSceneState(true);
    int32_t temp = 48900;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    snprintf_s(currentBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryCurrentPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest024: Failed to write file ";
    }
    sleep(WAIT_TIME);
    ret = ReadFile(currentBuf, currentValue, sizeof(currentValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest024: Failed to read file ";
    }
    std::string current = currentValue;
    value = ConvertInt(current);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest024: current is: " << value;
    ASSERT_TRUE(value == 800) << "failed to get current value";
}

/**
 * @tc.name: ThermalMgrPolicyTest025
 * @tc.desc: Verify to get cpu freq value by set battery temp in level 1 of base safe mode.
 * @tc.cond: scene-cam, state- enable charge
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest025, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;

    sptr<ThermalService> service = DelayedSpSingleton<ThermalService>::GetInstance();
    if (service == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrMock002: Failed to get ThermalService";
    }
    service->GetStateMachine()->SetSceneState(true);
    service->GetStateMachine()->SetStateFlag(false);
    service->GetStateMachine()->SetState(1);
    int32_t temp = 42000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, cpuFreqPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest025: Failed to write file ";
    }

    sleep(WAIT_TIME);
    ret = ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest025: Failed to read file ";
    }
    std::string freq = freqValue;
    value = ConvertInt(freq);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest025: freq is: " << value;
    ASSERT_TRUE(value == 80000) << "failed to get freq value";
}

/**
 * @tc.name: ThermalMgrPolicyTest026
 * @tc.desc: Verify to get cpu freq value by set battery temp in level 1 of base safe mode.
 * @tc.cond: scene-cam, state- disable charge
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest026, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;

    sptr<ThermalService> service = DelayedSpSingleton<ThermalService>::GetInstance();
    if (service == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrMock002: Failed to get ThermalService";
    }
    service->GetStateMachine()->SetSceneState(true);
    service->GetStateMachine()->SetStateFlag(false);
    service->GetStateMachine()->SetState(0);
    int32_t temp = 42200;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, cpuFreqPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest026: Failed to write file ";
    }

    sleep(WAIT_TIME);
    ret = ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest026: Failed to read file ";
    }
    std::string freq = freqValue;
    value = ConvertInt(freq);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest026: freq is: " << value;
    ASSERT_TRUE(value == 90000) << "failed to get freq value";
}

/**
 * @tc.name: ThermalMgrPolicyTest027
 * @tc.desc: Verify to get cpu freq value by set battery temp in level 2 of base safe mode.
 * @tc.cond: scene-cam, state- enable charge
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest027, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;

    sptr<ThermalService> service = DelayedSpSingleton<ThermalService>::GetInstance();
    if (service == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrMock002: Failed to get ThermalService";
    }
    service->GetStateMachine()->SetSceneState(true);
    service->GetStateMachine()->SetStateFlag(false);
    service->GetStateMachine()->SetState(1);
    int32_t temp = 45000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, cpuFreqPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest027: Failed to write file ";
    }

    sleep(WAIT_TIME);
    ret = ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest027: Failed to read file ";
    }
    std::string freq = freqValue;
    value = ConvertInt(freq);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest027: freq is: " << value;
    ASSERT_TRUE(value == 70000) << "failed to get freq value";
}

/**
 * @tc.name: ThermalMgrPolicyTest028
 * @tc.desc: Verify to get cpu freq value by set battery temp in level 2 of base safe mode.
 * @tc.cond: scene-cam, state- disable charge
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest028, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;

    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest028: Failed to get ThermalService";
    }
    sptr<ThermalService> service = DelayedSpSingleton<ThermalService>::GetInstance();
    if (service == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrMock002: Failed to get ThermalService";
    }
    service->GetStateMachine()->SetSceneState(true);
    service->GetStateMachine()->SetStateFlag(false);
    service->GetStateMachine()->SetState(0);
    int32_t temp = 45200;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, cpuFreqPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest028: Failed to write file ";
    }

    sleep(WAIT_TIME);
    ret = ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest028: Failed to read file ";
    }
    std::string freq = freqValue;
    value = ConvertInt(freq);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest028: freq is: " << value;
    ASSERT_TRUE(value == 80000) << "failed to get freq value";
}

/**
 * @tc.name: ThermalMgrPolicyTest029
 * @tc.desc: Verify to get cpu freq value by set battery temp in level 3 of base safe mode.
 * @tc.cond: scene-cam, state- enable charge
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest029, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;

    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest029: Failed to get ThermalService";
    }
    sptr<ThermalService> service = DelayedSpSingleton<ThermalService>::GetInstance();
    if (service == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrMock002: Failed to get ThermalService";
    }
    service->GetStateMachine()->SetSceneState(true);
    service->GetStateMachine()->SetStateFlag(false);
    service->GetStateMachine()->SetState(1);
    int32_t temp = 48000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, cpuFreqPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest029: Failed to write file ";
    }

    sleep(WAIT_TIME);
    ret = ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest029: Failed to read file ";
    }
    std::string freq = freqValue;
    value = ConvertInt(freq);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest029: freq is: " << value;
    ASSERT_TRUE(value == 60000) << "failed to get freq value";
}

/**
 * @tc.name: ThermalMgrPolicyTest030
 * @tc.desc: Verify to get cpu freq value by set battery temp in level 3 of base safe mode.
 * @tc.cond: scene-cam, state- disable charge
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest030, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;

    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest030: Failed to get ThermalService";
    }
    sptr<ThermalService> service = DelayedSpSingleton<ThermalService>::GetInstance();
    if (service == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrMock002: Failed to get ThermalService";
    }
    service->GetStateMachine()->SetSceneState(true);
    service->GetStateMachine()->SetStateFlag(false);
    service->GetStateMachine()->SetState(0);
    int32_t temp = 48200;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, cpuFreqPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest030: Failed to write file ";
    }

    sleep(WAIT_TIME);
    ret = ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest030: Failed to read file ";
    }
    std::string freq = freqValue;
    value = ConvertInt(freq);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest030: freq is: " << value;
    ASSERT_TRUE(value == 70000) << "failed to get freq value";
}

/**
 * @tc.name: ThermalMgrPolicyTest031
 * @tc.desc: Verify to get lcd brightness value by set battery temp in level 1 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest031, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char cpuBuf[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;

    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest031: Failed to get ThermalService";
    }
    int32_t temp = 42000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, cpuFreqPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest031: Failed to write file ";
    }
    sleep(WAIT_TIME);
    value = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::THERMAL_FLAG_LCD_BL_LIMIT);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest031: brightness is: " << value;
    ASSERT_TRUE(value == 188) << "failed to get brightness value";
}

/**
 * @tc.name: ThermalMgrPolicyTest031
 * @tc.desc: Verify to get lcd brightness value by set battery temp in level 2 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest032, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char cpuBuf[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;

    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest032: Failed to get ThermalService";
    }
    int32_t temp = 45000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, cpuFreqPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest032: Failed to write file ";
    }
    sleep(WAIT_TIME);
    value = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::THERMAL_FLAG_LCD_BL_LIMIT);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest032: brightness is: " << value;
    ASSERT_TRUE(value == 155) << "failed to get brightness value";
}

/**
 * @tc.name: ThermalMgrPolicyTest033
 * @tc.desc: Verify to get lcd brightness value by set battery temp in level 3 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest033, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char cpuBuf[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;

    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest033: Failed to get ThermalService";
    }
    int32_t temp = 48000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, cpuFreqPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest033: Failed to write file ";
    }
    sleep(WAIT_TIME);
    value = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::THERMAL_FLAG_LCD_BL_LIMIT);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest033: freq is: " << value;
    ASSERT_TRUE(value == 120) << "failed to get brightness value";
}

/**
 * @tc.name: ThermalMgrPolicyTest036
 * @tc.desc: Verify to get process action value by set pa temp in level 2 of warm 5G mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest036, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char auxtempBuf[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest036: Failed to get ThermalService";
    }
    int32_t temp = 48000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, paPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest036: Failed to write file ";
    }

    int32_t amtemp = 32000;
    std::string sAmbientTemp = to_string(amtemp) + "\n";
    snprintf_s(auxtempBuf, PATH_MAX, sizeof(auxtempBuf) - 1, ambientPath.c_str());
    ret = WriteFile(auxtempBuf, sAmbientTemp, sAmbientTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest036: Failed to write file ";
    }
    sleep(WAIT_TIME);
    value = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::ThERMAL_FLAG_CLEAR_BG_CONTROL);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest036: process value is: " << value;
    ASSERT_TRUE(value == 3) << "failed to Get process action value";
}

/**
 * @tc.name: ThermalMgrPolicyTest037
 * @tc.desc: Verify to get gear action value by set pa temp in level 1 of warm 5G mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest037, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char auxtempBuf[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest037: Failed to get ThermalService";
    }
    int32_t temp = 42000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, paPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest037: Failed to write file ";
    }

    int32_t amtemp = 32000;
    std::string sAmbientTemp = to_string(amtemp) + "\n";
    snprintf_s(auxtempBuf, PATH_MAX, sizeof(auxtempBuf) - 1, ambientPath.c_str());
    ret = WriteFile(auxtempBuf, sAmbientTemp, sAmbientTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest037: Failed to write file ";
    }
    sleep(WAIT_TIME);
    value = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::THERMAL_FLAG_LEVEL_NOTIFY);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest037: level is: " << value;
    ASSERT_TRUE(value == 4) << "failed to Get level value";
}

/**
 * @tc.name: ThermalMgrPolicyTest038
 * @tc.desc: Verify to get gear value by set pa temp in level 2 of warm 5G mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest038, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char auxtempBuf[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest038: Failed to get ThermalService";
    }
    int32_t temp = 48000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, paPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest038: Failed to write file ";
    }

    int32_t amtemp = 32000;
    std::string sAmbientTemp = to_string(amtemp) + "\n";
    snprintf_s(auxtempBuf, PATH_MAX, sizeof(auxtempBuf) - 1, ambientPath.c_str());
    ret = WriteFile(auxtempBuf, sAmbientTemp, sAmbientTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest038: Failed to write file ";
    }
    sleep(WAIT_TIME);
    value = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::THERMAL_FLAG_LEVEL_NOTIFY);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest038: level is: " << value;
    ASSERT_TRUE(value == 5) << "failed to Get level value";
}

/**
 * @tc.name: ThermalMgrPolicyTest039
 * @tc.desc: Verify to get battery current value by set charger temp in level 2 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest039, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char currentBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest039: Failed to get ThermalService";
    }
    tms->GetStateMachine()->SetSceneState(false);
    int32_t temp = 43110;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, chargerPath.c_str());
    snprintf_s(currentBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryCurrentPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest039: Failed to write file ";
    }
    sleep(WAIT_TIME);
    ret = ReadFile(currentBuf, currentValue, sizeof(currentValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest039: Failed to read file ";
    }
    std::string current = currentValue;
    value = ConvertInt(current);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest039: current is: " << value;
    ASSERT_TRUE(value == 1500) << "failed to get current value";
}

/**
 * @tc.name: ThermalMgrPolicyTest040
 * @tc.desc: Verify to get cpu freq value by set charger temp in level 2 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest040, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;

    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest040: Failed to get ThermalService";
    }
    tms->GetStateMachine()->SetSceneState(false);
    int32_t temp = 43000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, chargerPath.c_str());
    snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, cpuFreqPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest040: Failed to write file ";
    }

    sleep(WAIT_TIME);
    ret = ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest040: Failed to read file ";
    }
    std::string freq = freqValue;
    value = ConvertInt(freq);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest040: freq is: " << value;
    ASSERT_TRUE(value == 90000) << "failed to get freq value";
}

/**
 * @tc.name: ThermalMgrPolicyTest041
 * @tc.desc: Verify to get battery current value by set charger temp in level 1 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest041, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char currentBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest041: Failed to get ThermalService";
    }
    tms->GetStateMachine()->SetSceneState(false);
    int32_t temp = 40200;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, chargerPath.c_str());
    snprintf_s(currentBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryCurrentPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest041: Failed to write file ";
    }
    sleep(WAIT_TIME);
    ret = ReadFile(currentBuf, currentValue, sizeof(currentValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest041: Failed to read file ";
    }
    std::string current = currentValue;
    value = ConvertInt(current);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest041: current is: " << value;
    ASSERT_TRUE(value == 1800) << "failed to get current value";
}

/**
 * @tc.name: ThermalMgrPolicyTest042
 * @tc.desc: Verify to get cpu freq value by set charger temp in level 1 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest042, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;

    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest042: Failed to get ThermalService";
    }
    tms->GetStateMachine()->SetSceneState(false);
    int32_t temp = 40300;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, chargerPath.c_str());
    snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, cpuFreqPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest042: Failed to write file ";
    }

    sleep(WAIT_TIME);
    ret = ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest042: Failed to read file ";
    }
    std::string freq = freqValue;
    value = ConvertInt(freq);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest042: freq is: " << value;
    ASSERT_TRUE(value == 99000) << "failed to get freq value";
}

/**
 * @tc.name: ThermalMgrPolicyTest043
 * @tc.desc: Verify to get battery current value by set charger temp and camare scene in level 2 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest043, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char currentBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest043: Failed to get ThermalService";
    }
    tms->GetStateMachine()->SetSceneState(true);
    int32_t temp = 43000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, chargerPath.c_str());
    snprintf_s(currentBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryCurrentPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest043: Failed to write file ";
    }
    sleep(WAIT_TIME);
    ret = ReadFile(currentBuf, currentValue, sizeof(currentValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest043: Failed to read file ";
    }
    std::string current = currentValue;
    value = ConvertInt(current);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest043: current is: " << value;
    ASSERT_TRUE(value == 1000) << "failed to get current value";
}

/**
 * @tc.name: ThermalMgrPolicyTest044
 * @tc.desc: Verify to get gear value by set charger temp in level 2 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest044, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char currentBuf[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest044: Failed to get ThermalService";
    }
    int32_t temp = 43000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, chargerPath.c_str());
    snprintf_s(currentBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryCurrentPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest044: Failed to write file ";
    }
    sleep(WAIT_TIME);
    value = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::THERMAL_FLAG_LEVEL_NOTIFY);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest044: level is: " << value;
    ASSERT_TRUE(value == 2) << "failed to Get level value";
}

/**
 * @tc.name: ThermalMgrPolicyTest045
 * @tc.desc: Verify to get gear value by set charger temp in level 1 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest045, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char currentBuf[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest045: Failed to get ThermalService";
    }
    int32_t temp = 40500;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, chargerPath.c_str());
    snprintf_s(currentBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryCurrentPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest045: Failed to write file ";
    }
    sleep(WAIT_TIME);
    value = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::THERMAL_FLAG_LEVEL_NOTIFY);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest045: level is: " << value;
    ASSERT_TRUE(value == 1) << "failed to Get level value";
}

/**
 * @tc.name: ThermalMgrPolicyTest046
 * @tc.desc: Verify to get gear value by set charger temp in level 3 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest046, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char currentBuf[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest046: Failed to get ThermalService";
    }
    int32_t temp = 48500;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, chargerPath.c_str());
    snprintf_s(currentBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryCurrentPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest046: Failed to write file ";
    }
    sleep(WAIT_TIME);
    value = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::THERMAL_FLAG_LEVEL_NOTIFY);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest046: level is: " << value;
    ASSERT_TRUE(value == 3) << "failed to get level value";
}

/**
 * @tc.name: ThermalMgrPolicyTest047
 * @tc.desc: Verify to get process_ctrl value by set charger temp in level 1 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest047, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char currentBuf[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest047: Failed to get ThermalService";
    }
    int32_t temp = 40600;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, chargerPath.c_str());
    snprintf_s(currentBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryCurrentPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest047: Failed to write file ";
    }
    sleep(WAIT_TIME);
    value = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::ThERMAL_FLAG_CLEAR_BG_CONTROL);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest047: current is: " << value;
    ASSERT_TRUE(value == 3) << "failed to Get process action value";
}

/**
 * @tc.name: ThermalMgrPolicyTest048
 * @tc.desc: Verify to get process_ctrl value by set charger temp in level 2 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest048, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char currentBuf[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest048: Failed to get ThermalService";
    }
    int32_t temp = 43000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, chargerPath.c_str());
    snprintf_s(currentBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryCurrentPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest048: Failed to write file ";
    }
    sleep(WAIT_TIME);
    value = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::ThERMAL_FLAG_CLEAR_BG_CONTROL);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest048: current is: " << value;
    ASSERT_TRUE(value == 2) << "failed to Get process action value";
}

/**
 * @tc.name: ThermalMgrPolicyTest049
 * @tc.desc: Verify to get process_ctrl value by set charger temp in level 3 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest049, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char currentBuf[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest049: Failed to get ThermalService";
    }
    int32_t temp = 48400;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, chargerPath.c_str());
    snprintf_s(currentBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryCurrentPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest049: Failed to write file ";
    }
    sleep(WAIT_TIME);
    value = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::ThERMAL_FLAG_CLEAR_BG_CONTROL);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest049: current is: " << value;
    ASSERT_TRUE(value == 1) << "failed to Get process action value";
}

/**
 * @tc.name: ThermalMgrPolicyTest050
 * @tc.desc: Verify to get battery current value by set charger temp in level 3 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest050, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char currentBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest050: Failed to get ThermalService";
    }
    int32_t temp = 48200;
    tms->GetStateMachine()->SetSceneState(false);
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, chargerPath.c_str());
    snprintf_s(currentBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryCurrentPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest050: Failed to write file ";
    }
    sleep(WAIT_TIME);
    ret = ReadFile(currentBuf, currentValue, sizeof(currentValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest050: Failed to read file ";
    }
    std::string current = currentValue;
    value = ConvertInt(current);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest050: current is: " << value;
    ASSERT_TRUE(value == 1300) << "failed to get current value";
}

/**
 * @tc.name: ThermalMgrPolicyTest051
 * @tc.desc: Verify to get cpu freq value by set charger temp in level 3 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest051, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;

    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest051: Failed to get ThermalService";
    }
    tms->GetStateMachine()->SetSceneState(false);
    int32_t temp = 48300;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, chargerPath.c_str());
    snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, cpuFreqPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest051: Failed to write file ";
    }

    sleep(WAIT_TIME);
    ret = ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest051: Failed to read file ";
    }
    std::string freq = freqValue;
    value = ConvertInt(freq);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest051: freq is: " << value;
    ASSERT_TRUE(value == 80000) << "failed to get freq value";
}

/**
 * @tc.name: ThermalMgrPolicyTest052
 * @tc.desc: Verify to get battery current value by set charger temp and camare scene in level 1 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest052, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char currentBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest052: Failed to get ThermalService";
    }
    tms->GetStateMachine()->SetSceneState(true);
    int32_t temp = 40200;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, chargerPath.c_str());
    snprintf_s(currentBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryCurrentPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest052: Failed to write file ";
    }
    sleep(WAIT_TIME);
    ret = ReadFile(currentBuf, currentValue, sizeof(currentValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest052: Failed to read file ";
    }
    std::string current = currentValue;
    value = ConvertInt(current);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest052: current is: " << value;
    ASSERT_TRUE(value == 1200) << "failed to get current value";
}

/**
 * @tc.name: ThermalMgrPolicyTest053
 * @tc.desc: Verify to get battery current value by set charger temp and camare scene in level 3 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest053, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char currentBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest053: Failed to get ThermalService";
    }
    tms->GetStateMachine()->SetSceneState(true);
    int32_t temp = 47000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, chargerPath.c_str());
    snprintf_s(currentBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryCurrentPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest053: Failed to write file ";
    }
    sleep(WAIT_TIME);
    ret = ReadFile(currentBuf, currentValue, sizeof(currentValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest053: Failed to read file ";
    }
    std::string current = currentValue;
    value = ConvertInt(current);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest053: current is: " << value;
    ASSERT_TRUE(value == 800) << "failed to get current value";
}

/**
 * @tc.name: ThermalMgrPolicyTest054
 * @tc.desc: Verify to get cpu freq value by set charger temp in level 1 of base safe mode.
 * @tc.cond: scene-cam, state- enable charge
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest054, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;

    sptr<ThermalService> service = DelayedSpSingleton<ThermalService>::GetInstance();
    if (service == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrMock002: Failed to get ThermalService";
    }
    service->GetStateMachine()->SetSceneState(true);
    service->GetStateMachine()->SetStateFlag(false);
    service->GetStateMachine()->SetState(1);
    int32_t temp = 40000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, chargerPath.c_str());
    snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, cpuFreqPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest054: Failed to write file ";
    }

    sleep(WAIT_TIME);
    ret = ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest054: Failed to read file ";
    }
    std::string freq = freqValue;
    value = ConvertInt(freq);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest054: freq is: " << value;
    ASSERT_TRUE(value == 80000) << "failed to get freq value";
}

/**
 * @tc.name: ThermalMgrPolicyTest055
 * @tc.desc: Verify to get cpu freq value by set charger temp in level 1 of base safe mode.
 * @tc.cond: scene-cam, state- disable charge
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest055, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;

    sptr<ThermalService> service = DelayedSpSingleton<ThermalService>::GetInstance();
    if (service == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrMock002: Failed to get ThermalService";
    }
    service->GetStateMachine()->SetSceneState(true);
    service->GetStateMachine()->SetStateFlag(false);
    service->GetStateMachine()->SetState(0);
    int32_t temp = 40500;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, chargerPath.c_str());
    snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, cpuFreqPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest055: Failed to write file ";
    }

    sleep(WAIT_TIME);
    ret = ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest055: Failed to read file ";
    }
    std::string freq = freqValue;
    value = ConvertInt(freq);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest055: freq is: " << value;
    ASSERT_TRUE(value == 90000) << "failed to get freq value";
}

/**
 * @tc.name: ThermalMgrPolicyTest056
 * @tc.desc: Verify to get cpu freq value by set charger temp in level 2 of base safe mode.
 * @tc.cond: scene-cam, state- enable charge
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest056, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;

    sptr<ThermalService> service = DelayedSpSingleton<ThermalService>::GetInstance();
    if (service == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrMock002: Failed to get ThermalService";
    }
    service->GetStateMachine()->SetSceneState(true);
    service->GetStateMachine()->SetStateFlag(false);
    service->GetStateMachine()->SetState(1);
    int32_t temp = 43500;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, chargerPath.c_str());
    snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, cpuFreqPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest056: Failed to write file ";
    }

    sleep(WAIT_TIME);
    ret = ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest056: Failed to read file ";
    }
    std::string freq = freqValue;
    value = ConvertInt(freq);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest056: freq is: " << value;
    ASSERT_TRUE(value == 70000) << "failed to get freq value";
}

/**
 * @tc.name: ThermalMgrPolicyTest057
 * @tc.desc: Verify to get cpu freq value by set charger temp in level 2 of base safe mode.
 * @tc.cond: scene-cam, state- disable charge
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest057, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> service = DelayedSpSingleton<ThermalService>::GetInstance();
    if (service == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrMock002: Failed to get ThermalService";
    }
    service->GetStateMachine()->SetSceneState(true);
    service->GetStateMachine()->SetStateFlag(false);
    service->GetStateMachine()->SetState(0);
    int32_t temp = 43000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, chargerPath.c_str());
    snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, cpuFreqPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest057: Failed to write file ";
    }

    sleep(WAIT_TIME);
    ret = ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest057: Failed to read file ";
    }
    std::string freq = freqValue;
    value = ConvertInt(freq);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest057: freq is: " << value;
    ASSERT_TRUE(value == 80000) << "failed to get freq value";
}

/**
 * @tc.name: ThermalMgrPolicyTest058
 * @tc.desc: Verify to get cpu freq value by set charger temp in level 3 of base safe mode.
 * @tc.cond: scene-cam, state- enable charge
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest058, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;

    sptr<ThermalService> service = DelayedSpSingleton<ThermalService>::GetInstance();
    if (service == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrMock002: Failed to get ThermalService";
    }
    service->GetStateMachine()->SetSceneState(true);
    service->GetStateMachine()->SetStateFlag(false);
    service->GetStateMachine()->SetState(1);
    int32_t temp = 48000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, chargerPath.c_str());
    snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, cpuFreqPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest058: Failed to write file ";
    }

    sleep(WAIT_TIME);
    ret = ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest058: Failed to read file ";
    }
    std::string freq = freqValue;
    value = ConvertInt(freq);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest058: freq is: " << value;
    ASSERT_TRUE(value == 60000) << "failed to get freq value";
}

/**
 * @tc.name: ThermalMgrPolicyTest059
 * @tc.desc: Verify to get cpu freq value by set charger temp in level 3 of base safe mode.
 * @tc.cond: scene-cam, state- disable charge
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest059, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    char cpuBuf[MAX_PATH] = {0};
    char freqValue[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> service = DelayedSpSingleton<ThermalService>::GetInstance();
    if (service == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrMock002: Failed to get ThermalService";
    }
    service->GetStateMachine()->SetSceneState(true);
    service->GetStateMachine()->SetStateFlag(false);
    service->GetStateMachine()->SetState(0);
    int32_t temp = 49000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, chargerPath.c_str());
    snprintf_s(cpuBuf, PATH_MAX, sizeof(cpuBuf) - 1, cpuFreqPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest059: Failed to write file ";
    }

    sleep(WAIT_TIME);
    ret = ReadFile(cpuBuf, freqValue, sizeof(freqValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest059: Failed to read file ";
    }
    std::string freq = freqValue;
    value = ConvertInt(freq);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest059: freq is: " << value;
    ASSERT_TRUE(value == 70000) << "failed to get freq value";
}

/**
 * @tc.name: ThermalMgrPolicyTest060
 * @tc.desc: Verify to get lcd brightness value by set charger temp in level 1 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest060, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;

    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest060: Failed to get ThermalService";
    }
    int32_t temp = 40100;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, chargerPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest060: Failed to write file ";
    }
    sleep(WAIT_TIME);
    value = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::THERMAL_FLAG_LCD_BL_LIMIT);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest060: brightness is: " << value;
    ASSERT_TRUE(value == 188) << "failed to get brightness value";
}

/**
 * @tc.name: ThermalMgrPolicyTest061
 * @tc.desc: Verify to get lcd brightness value by set charger temp in level 2 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest061, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;

    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest061: Failed to get ThermalService";
    }
    int32_t temp = 43100;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, chargerPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest061: Failed to write file ";
    }
    sleep(WAIT_TIME);
    value = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::THERMAL_FLAG_LCD_BL_LIMIT);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest061: brightness is: " << value;
    ASSERT_TRUE(value == 155) << "failed to get brightness value";
}

/**
 * @tc.name: ThermalMgrPolicyTest062
 * @tc.desc: Verify to get lcd brightness value by set charger temp in level 3 of base safe mode.
 * @tc.type: FUNC
 */
HWTEST_F (ThermalMgrPolicyTest, ThermalMgrPolicyTest062, TestSize.Level2)
{
    char tempBuf[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;

    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest062: Failed to get ThermalService";
    }
    int32_t temp = 48100;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, chargerPath.c_str());
    ret = WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalMgrPolicyTest062: Failed to write file ";
    }
    sleep(WAIT_TIME);
    value = tms->GetThermalAction()->GetPolicyActionValue(ThermalAction::THERMAL_FLAG_LCD_BL_LIMIT);
    GTEST_LOG_(INFO) << "ThermalMgrPolicyTest062: brightness is: " << value;
    ASSERT_TRUE(value == 120) << "failed to get brightness value";
}
}