/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "thermal_module_service_test.h"

#include "securec.h"

#include "thermal_common.h"
#include "thermal_common_file.h"
#include "thermal_mgr_client.h"
#include "thermal_service.h"

using namespace OHOS;
using namespace OHOS::PowerMgr;
using namespace std;
using namespace testing::ext;
using namespace OHOS::HiviewDFX;

static constexpr HiLogLabel LABEL = {LOG_CORE, 0, "ThermalMST"};
static sptr<ThermalService> g_service;

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

void ThermalModuleServiceTest::SetUpTestCase(void)
{
    g_service = DelayedSpSingleton<ThermalService>::GetInstance();
    g_service->OnStart();
}

void ThermalModuleServiceTest::TearDownTestCase(void)
{
    g_service->OnStop();
    DelayedSpSingleton<ThermalService>::DestroyInstance();
}

void ThermalModuleServiceTest::SetUp(void)
{
}

void ThermalModuleServiceTest::TearDown(void)
{
}

/*
 * Feature: Get action by polocy decision
 * Function: ThermalCommonFile::ReadFile
 * CaseDescription:
 *
 */
HWTEST_F (ThermalModuleServiceTest, ThermalModuleServiceTest001, TestSize.Level0)
{
    HiLog::Info(LABEL, "ThermalModuleServiceTest001 start");
    char tempBuf[MAX_PATH] = {0};
    char currentBuf[MAX_PATH] = {0};
    char currentValue[MAX_PATH] = {0};
    int32_t ret = -1, value = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalModuleServiceTest001: Failed to get ThermalService";
    }
    int32_t temp = 45000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    snprintf_s(currentBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryCurrentPath.c_str());
    ret = ThermalCommonFile::WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalModuleServiceTest001: Failed to write file ";
    }
    sleep(WAIT_TIME);
    ret = ThermalCommonFile::ReadFile(currentBuf, currentValue, sizeof(currentValue));
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalModuleServiceTest001: Failed to read file ";
    }
    std::string current = currentValue;
    value = ThermalCommonFile::ConvertInt(current);
    GTEST_LOG_(INFO) << "ThermalModuleServiceTest001: current is: " << value;
    ASSERT_TRUE(value == 1500) << "failed to get current value";
    HiLog::Info(LABEL, "ThermalModuleServiceTest001 end");
}

/*
 * Feature: Verify delay shutdown and repeatedly kill process by policy decision.
 * Function: GetPolicyActionHubValue
 * CaseDescription:
 *
 */
HWTEST_F (ThermalModuleServiceTest, ThermalModuleServiceTest002, TestSize.Level0)
{
    char apBuf[MAX_PATH] = {0};
    char ambientBuf[MAX_PATH] = {0};
    char batteryBuf[MAX_PATH] = {0};

    int32_t ret = -1, shutdownValue = 0, processValue = 0;
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalModuleServiceTest002: Failed to get ThermalService";
    }
    tms->GetStateMachine()->SetStateFlag(true);
    tms->GetStateMachine()->SetStateFlag(false);
    tms->GetStateMachine()->SetState(1);
    int32_t aptemp = 80000;
    std::string sApTemp = to_string(aptemp) + "\n";
    snprintf_s(apBuf, PATH_MAX, sizeof(apBuf) - 1, apPath.c_str());
    ret = ThermalCommonFile::WriteFile(apBuf, sApTemp, sApTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalModuleServiceTest002: Failed to write file ";
    }

    int32_t amTemp = 50000;
    std::string sAmTemp = to_string(amTemp) + "\n";
    snprintf_s(ambientBuf, PATH_MAX, sizeof(ambientBuf) - 1, ambientPath.c_str());
    ret = ThermalCommonFile::WriteFile(ambientBuf, sAmTemp, sAmTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalModuleServiceTest002: Failed to write file ";
    }
    int32_t baTemp = 60000;
    std::string sBaTemp = to_string(baTemp) + "\n";
    snprintf_s(batteryBuf, PATH_MAX, sizeof(batteryBuf) - 1, batteryPath.c_str());
    ret = ThermalCommonFile::WriteFile(batteryBuf, sBaTemp, sBaTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalModuleServiceTest002: Failed to write file ";
    }
    sleep(WAIT_TIME);
    shutdownValue = tms->GetThermalAction()->GetPolicyActionHubValue(ThermalAction::THERMAL_HUB_POWER_SHUTDOWN);
    GTEST_LOG_(INFO) << "ThermalModuleServiceTest002: shutdown value is: " << shutdownValue;
    ASSERT_TRUE(shutdownValue == 1) << "failed to get shutdown value";

    processValue = tms->GetThermalAction()->GetPolicyActionHubValue(ThermalAction::THERMAL_HUB_CLEAR_ALL_PROCESS);
    GTEST_LOG_(INFO) << "ThermalModuleServiceTest002: process value is: " << processValue;
    ASSERT_TRUE(processValue == 1) << "failed to get process action value";
}

/*
 * Feature: Get battery temp by service.
 * Function: GetInfo
 * CaseDescription:
 *
 */
HWTEST_F (ThermalModuleServiceTest, ThermalModuleServiceTest003, TestSize.Level0)
{
    HiLog::Info(LABEL, "ThermalModuleServiceTest003 start");
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalModuleServiceTest004: Failed to get ThermalService";
    }
    char tempBuf[MAX_PATH] = {0};
    int32_t ret = -1;
    int32_t temp = 45000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    ret = ThermalCommonFile::WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalModuleServiceTest003: Failed to write file ";
    }
    sleep(WAIT_TIME);
    for (auto info: tms->GetObserver()->GetInfo()) {
        if (info.first.compare("battery") == 0) {
            ASSERT_TRUE(info.second == 45000) << "failed to get temp value";
        }
    }
    HiLog::Info(LABEL, "ThermalModuleServiceTest003 end");
}

/*
 * Feature: Get battery temp by hdi.
 * Function: GetSubscriberInfo
 * CaseDescription:
 *
 */
HWTEST_F (ThermalModuleServiceTest, ThermalModuleServiceTest004, TestSize.Level0)
{
    HiLog::Info(LABEL, "ThermalModuleServiceTest006 start");
    sptr<ThermalService> tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        GTEST_LOG_(INFO) << "ThermalModuleServiceTest005: Failed to get ThermalService";
    }
    tms->OnStart();
    char tempBuf[MAX_PATH] = {0};
    int32_t ret = -1;
    int32_t temp = 45000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    ret = ThermalCommonFile::WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalModuleServiceTest004: Failed to write file ";
    }
    sleep(WAIT_TIME);
    for (auto info: tms->GetSubscriber()->GetSubscriberInfo()) {
        if (info.first.compare("battery") == 0) {
            ASSERT_TRUE(info.second == 45000) << "failed to get temp value";
        }
    }
    HiLog::Info(LABEL, "ThermalModuleServiceTest004 end");
}

/*
 * Feature: Run thermal protect executable file.
 * Function: StartThermalProtector
 * CaseDescription:
 *
 */
HWTEST_F (ThermalModuleServiceTest, ThermalModuleServiceTest005, TestSize.Level0)
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