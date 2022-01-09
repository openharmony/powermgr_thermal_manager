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

#include "thermal_module_api_test.h"

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

void ThermalModuleApiTest::SetUpTestCase(void)
{
}

void ThermalModuleApiTest::TearDownTestCase(void)
{
}

void ThermalModuleApiTest::SetUp(void)
{
}

void ThermalModuleApiTest::TearDown(void)
{
}

void ThermalModuleApiTest::ThermalModuleApiTest1Callback::OnThermalTempChanged(int32_t temp)
{
    int assertValue = 40000;
    GTEST_LOG_(INFO) << "charger temp: " << temp;
    ASSERT_TRUE(temp == assertValue) << "failed to get charger temp";
}

void ThermalModuleApiTest::ThermalModuleApiTest2Callback::GetThermalLevel(ThermalLevel level)
{
    int min = 0;
    int max = 6;
    uint32_t value = static_cast<uint32_t>(level);
    GTEST_LOG_(INFO) << "ThermalLevelTest1Callback level: " << value;
    ASSERT_TRUE(value >= min || value <= max) << "failed to get charger temp";
}

/*
 * Feature: Get sensor temp by register callback
 * Function: SubscribeThermalTempCallback
 * CaseDescription:
 *
 */
HWTEST_F(ThermalModuleApiTest, ThermalModuleApiTest001, TestSize.Level0)
{
    HiLog::Info(LABEL, "ThermalModuleApiTest001 start");
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    char tempBuf[MAX_PATH] = {0};
    int32_t ret = -1;
    int32_t temp = 40000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, chargerPath.c_str());
    ret = ThermalCommonFile::WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalModuleApiTest001: Failed to write file ";
    }
    sleep(WAIT_TIME);
    sptr<IThermalTempCallback> cb1 = new ThermalModuleApiTest1Callback();
    thermalMgrClient.SubscribeThermalTempCallback("charger", cb1);
    sleep(WAIT_TIME);
    thermalMgrClient.UnSubscribeThermalTempCallback(cb1);

    HiLog::Info(LABEL, "ThermalModuleApiTest001 end");
}

/*
 * Feature: Get system level by register callback
 * Function: SubscribeThermalLevelCallback
 * CaseDescription:
 *
 */
HWTEST_F(ThermalModuleApiTest, ThermalModuleApiTest002, TestSize.Level0)
{
    HiLog::Info(LABEL, "ThermalModuleApiTest002 start");
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    sptr<IThermalLevelCallback> cb2 = new ThermalModuleApiTest2Callback();
    thermalMgrClient.SubscribeThermalLevelCallback(cb2);
    sleep(2 * WAIT_TIME);
    thermalMgrClient.UnSubscribeThermalLevelCallback(cb2);
    HiLog::Info(LABEL, "ThermalModuleApiTest002 end");
}

/*
 * Feature: Get the thermal level
 * Function: GetThermalLevel
 * CaseDescription:
 *
 */
HWTEST_F (ThermalModuleApiTest, ThermalModuleApiTest003, TestSize.Level0)
{
    HiLog::Info(LABEL, "ThermalModuleApiTest003 start");
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    char tempBuf[MAX_PATH] = {0};
    int32_t ret = -1;
    int32_t temp = 41000;
    ThermalLevel level;

    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    ret = ThermalCommonFile::WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalModuleApiTest003: Failed to write file ";
    }
    sleep(WAIT_TIME);
    thermalMgrClient.GetThermalLevel(level);
    uint32_t value = static_cast<uint32_t>(level);
    GTEST_LOG_(INFO) << "level is:" << value;
    EXPECT_EQ(true, value == 1);
    HiLog::Info(LABEL, "ThermalModuleApiTest003 end");
}

/*
 * Feature: Get the Soc temp
 * Function: GetThermalSensorTemp
 * CaseDescription:
 *
 */
HWTEST_F (ThermalModuleApiTest, ThermalModuleApiTest004, TestSize.Level0)
{
    HiLog::Info(LABEL, "ThermalModuleApiTest004 start");
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    char tempBuf[MAX_PATH] = {0};
    int32_t ret = -1;
    int32_t temp = 45000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, socPath.c_str());
    ret = ThermalCommonFile::WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalModuleApiTest004: Failed to write file ";
    }
    sleep(WAIT_TIME);
    GTEST_LOG_(INFO) << "SOC temp:" << thermalMgrClient.GetThermalSensorTemp(SensorType::SOC);
    EXPECT_EQ(45000, thermalMgrClient.GetThermalSensorTemp(SensorType::SOC));
    HiLog::Info(LABEL, "ThermalModuleApiTest004 end");
}

/*
 * Feature: Get the Battery temp
 * Function: GetThermalSensorTemp
 * CaseDescription:
 *
 */
HWTEST_F (ThermalModuleApiTest, ThermalModuleApiTest005, TestSize.Level0)
{
    HiLog::Info(LABEL, "ThermalModuleApiTest005 start");
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    char tempBuf[MAX_PATH] = {0};
    int32_t ret = -1;
    int32_t temp = 45000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, batteryPath.c_str());
    ret = ThermalCommonFile::WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalModuleApiTest005: Failed to write file ";
    }
    sleep(WAIT_TIME);
    GTEST_LOG_(INFO) << "Battery temp:" << thermalMgrClient.GetThermalSensorTemp(SensorType::BATTERY);
    EXPECT_EQ(45000, thermalMgrClient.GetThermalSensorTemp(SensorType::BATTERY));
    HiLog::Info(LABEL, "ThermalModuleApiTest005 end");
}

/*
 * Feature: Get AP temp by sensor type
 * Function: GetThermalSensorTemp
 * CaseDescription:
 *
 */
HWTEST_F(ThermalModuleApiTest, ThermalModuleApiTest006, TestSize.Level0)
{
    HiLog::Info(LABEL, "ThermalModuleApiTest006 start");
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    char tempBuf[MAX_PATH] = {0};
    int32_t ret = -1;
    int32_t temp = 45000;
    std::string sTemp = to_string(temp) + "\n";
    snprintf_s(tempBuf, PATH_MAX, sizeof(tempBuf) - 1, apPath.c_str());
    ret = ThermalCommonFile::WriteFile(tempBuf, sTemp, sTemp.length());
    if (ret != ERR_OK) {
        GTEST_LOG_(INFO) << "ThermalModuleApiTest006: Failed to write file ";
    }
    sleep(WAIT_TIME);
    GTEST_LOG_(INFO) << "AP temp:" << thermalMgrClient.GetThermalSensorTemp(SensorType::AP);
    EXPECT_EQ(45000, thermalMgrClient.GetThermalSensorTemp(SensorType::AP));
    HiLog::Info(LABEL, "ThermalModuleApiTest006 end");
}