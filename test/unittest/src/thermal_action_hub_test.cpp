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

#include "thermal_action_hub_test.h"

#include <cstdio>
#include <cstdlib>
#include <datetime_ex.h>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <iostream>
#include <ipc_skeleton.h>
#include <string>
#include <string_ex.h>

#include "constants.h"
#include "ithermal_srv.h"
#include "mock_thermal_mgr_client.h"
#include "securec.h"
#include "thermal_common.h"
#include "thermal_mgr_client.h"
#include "thermal_srv_sensor_info.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
std::vector<std::string> g_typeList;
}

int32_t ThermalActionHubTest::WriteFile(std::string path, std::string buf, size_t size)
{
    FILE* stream = fopen(path.c_str(), "w+");
    if (stream == nullptr) {
        return ERR_INVALID_VALUE;
    }
    size_t ret = fwrite(buf.c_str(), strlen(buf.c_str()), 1, stream);
    if (ret == ERR_OK) {
        THERMAL_HILOGE(LABEL_TEST, "ret=%{public}zu", ret);
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

int32_t ThermalActionHubTest::ReadFile(const char* path, char* buf, size_t size)
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

void ThermalActionHubTest::SetUpTestCase() {}

void ThermalActionHubTest::TearDownTestCase()
{
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("");
}

void ThermalActionHubTest::SetUp() {}

void ThermalActionHubTest::TearDown() {}

void ThermalActionHubTest::InitData()
{
    g_typeList.push_back(BATTERY);
    g_typeList.push_back(SOC);
}

bool ThermalActionHubTest::ThermalActionTest1Callback::OnThermalActionChanged(ActionCallbackMap& actionCbMap)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest1Callback::OnThermalActionChanged Enter");
    int32_t cpuBigFreq = 1992000;
    for (auto iter : actionCbMap) {
        if (iter.first == "cpu_big") {
            EXPECT_EQ(iter.second, cpuBigFreq);
        }
        GTEST_LOG_(INFO) << "actionName: " << iter.first << " actionValue: " << iter.second;
    }
    return true;
}

bool ThermalActionHubTest::ThermalActionTest2Callback::OnThermalActionChanged(ActionCallbackMap& actionCbMap)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest2Callback::OnThermalActionChanged Enter");
    std::string lcd = "0.9";
    for (auto iter : actionCbMap) {
        if (iter.first == "lcd") {
            // 0： begin position; 3: end position
            EXPECT_EQ(std::to_string(iter.second).substr(0, 3), lcd);
        }
        GTEST_LOG_(INFO) << "actionName: " << iter.first << " actionValue: " << iter.second;
    }
    return true;
}

bool ThermalActionHubTest::ThermalActionTest3Callback::OnThermalActionChanged(ActionCallbackMap& actionCbMap)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest3Callback::OnThermalActionChanged Enter");
    int32_t cpuMedFreq = 1989500;
    std::string lcd = "0.8";
    for (auto iter : actionCbMap) {
        if (iter.first == "cpu_med") {
            EXPECT_EQ(iter.second, cpuMedFreq);
        }
        if (iter.first == "lcd") {
            // 0： begin position; 3: end position
            EXPECT_EQ(std::to_string(iter.second).substr(0, 3), lcd);
        }
        GTEST_LOG_(INFO) << "actionName: " << iter.first << " actionValue: " << iter.second;
    }
    return true;
}

bool ThermalActionHubTest::ThermalActionTest4Callback::OnThermalActionChanged(ActionCallbackMap& actionCbMap)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest4Callback::OnThermalActionChanged Enter");
    std::string lcd = "0.99";
    for (auto iter : actionCbMap) {
        if (iter.first == "lcd") {
            // 0： begin position; 4: end position
            EXPECT_EQ(std::to_string(iter.second).substr(0, 4), lcd);
        }
        GTEST_LOG_(INFO) << "actionName: " << iter.first << " actionValue: " << iter.second;
    }
    return true;
}

bool ThermalActionHubTest::ThermalActionTest5Callback::OnThermalActionChanged(ActionCallbackMap& actionCbMap)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest5Callback::OnThermalActionChanged Enter");
    std::string lcd = "0.88";
    for (auto iter : actionCbMap) {
        if (iter.first == "lcd") {
            // 0： begin position; 4: end position
            EXPECT_EQ(std::to_string(iter.second).substr(0, 4), lcd);
        }
        GTEST_LOG_(INFO) << "actionName: " << iter.first << " actionValue: " << iter.second;
    }
    return true;
}

bool ThermalActionHubTest::ThermalActionTest6Callback::OnThermalActionChanged(ActionCallbackMap& actionCbMap)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest6Callback::OnThermalActionChanged Enter");
    std::string lcd = "0.77";
    for (auto iter : actionCbMap) {
        if (iter.first == "lcd") {
            // 0： begin position; 4: end position
            EXPECT_EQ(std::to_string(iter.second).substr(0, 4), lcd);
        }
        GTEST_LOG_(INFO) << "actionName: " << iter.first << " actionValue: " << iter.second;
    }
    return true;
}

namespace {
/**
 * @tc.name: ThermalActionHubTest001
 * @tc.desc: register action is cpu_big test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalActionHubTest, ThermalActionHubTest001, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest001 start.");
    std::vector<std::string> actionList;
    actionList.push_back("cpu_big");

    std::string desc = "";
    char batteryTempBuf[MAX_PATH] = {0};
    char socTempBuf[MAX_PATH] = {0};
    int32_t ret = -1;
    InitData();
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);

    ret = snprintf_s(socTempBuf, MAX_PATH, sizeof(socTempBuf) - 1, SOC_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    const sptr<IThermalActionCallback> cb1 = new ThermalActionTest1Callback();

    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest001 start register");
    thermalMgrClient.SubscribeThermalActionCallback(actionList, desc, cb1);

    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalActionHubTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    MockThermalMgrClient::GetInstance().GetThermalInfo();
    thermalMgrClient.UnSubscribeThermalActionCallback(cb1);
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest001 end.");
}

/**
 * @tc.name: ThermalActionHubTest002
 * @tc.desc: register action is lcd test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalActionHubTest, ThermalActionHubTest002, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest002 start.");
    const std::string LCD = "lcd";
    std::vector<std::string> actionList;
    actionList.push_back(LCD);

    std::string desc = "";
    char batteryTempBuf[MAX_PATH] = {0};
    char socTempBuf[MAX_PATH] = {0};
    int32_t ret = -1;
    InitData();
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);

    ret = snprintf_s(socTempBuf, MAX_PATH, sizeof(socTempBuf) - 1, SOC_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    const sptr<IThermalActionCallback> cb2 = new ThermalActionTest2Callback();

    int32_t batteryTemp = 0;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalActionHubTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest002 start register");
    thermalMgrClient.SubscribeThermalActionCallback(actionList, desc, cb2);

    batteryTemp = 43100;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalActionHubTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    MockThermalMgrClient::GetInstance().GetThermalInfo();
    thermalMgrClient.UnSubscribeThermalActionCallback(cb2);
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest002 end.");
}

/**
 * @tc.name: ThermalActionHubTest003
 * @tc.desc: register action is cpu_med and lcd test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalActionHubTest, ThermalActionHubTest003, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest003 start.");
    const std::string LCD = "lcd";
    std::vector<std::string> actionList;
    actionList.push_back("cpu_med");
    actionList.push_back(LCD);

    std::string desc = "";
    char batteryTempBuf[MAX_PATH] = {0};
    char socTempBuf[MAX_PATH] = {0};
    int32_t ret = -1;
    InitData();
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);

    ret = snprintf_s(socTempBuf, MAX_PATH, sizeof(socTempBuf) - 1, SOC_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    const sptr<IThermalActionCallback> cb3 = new ThermalActionTest3Callback();

    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest003 start register");
    thermalMgrClient.SubscribeThermalActionCallback(actionList, desc, cb3);

    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalActionHubTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    MockThermalMgrClient::GetInstance().GetThermalInfo();
    thermalMgrClient.UnSubscribeThermalActionCallback(cb3);
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest003 end.");
}

/**
 * @tc.name: ThermalActionHubTest004
 * @tc.desc: register action is lcd test, scene cam, level 1
 * @tc.type: FUNC
 */
HWTEST_F(ThermalActionHubTest, ThermalActionHubTest004, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest004 start.");
    const std::string LCD = "lcd";
    std::vector<std::string> actionList;
    actionList.push_back(LCD);

    std::string desc = "";
    char batteryTempBuf[MAX_PATH] = {0};
    char socTempBuf[MAX_PATH] = {0};
    int32_t ret = -1;
    InitData();
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);

    ret = snprintf_s(socTempBuf, MAX_PATH, sizeof(socTempBuf) - 1, SOC_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    const sptr<IThermalActionCallback> cb4 = new ThermalActionTest4Callback();

    int32_t batteryTemp = 0;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalActionHubTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest004 start register");
    thermalMgrClient.SubscribeThermalActionCallback(actionList, desc, cb4);

    thermalMgrClient.SetScene("cam");
    batteryTemp = 40100;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalActionHubTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    MockThermalMgrClient::GetInstance().GetThermalInfo();
    thermalMgrClient.UnSubscribeThermalActionCallback(cb4);
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest004 end.");
}

/**
 * @tc.name: ThermalActionHubTest005
 * @tc.desc: register action is lcd test, scene call, level 2
 * @tc.type: FUNC
 */
HWTEST_F(ThermalActionHubTest, ThermalActionHubTest005, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest005 start.");
    const std::string LCD = "lcd";
    std::vector<std::string> actionList;
    actionList.push_back(LCD);

    std::string desc = "";
    char batteryTempBuf[MAX_PATH] = {0};
    char socTempBuf[MAX_PATH] = {0};
    int32_t ret = -1;
    InitData();
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);

    ret = snprintf_s(socTempBuf, MAX_PATH, sizeof(socTempBuf) - 1, SOC_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    const sptr<IThermalActionCallback> cb5 = new ThermalActionTest5Callback();

    int32_t batteryTemp = 0;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalActionHubTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest005 start register");
    thermalMgrClient.SubscribeThermalActionCallback(actionList, desc, cb5);

    thermalMgrClient.SetScene("call");
    batteryTemp = 43100;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalActionHubTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    MockThermalMgrClient::GetInstance().GetThermalInfo();
    thermalMgrClient.UnSubscribeThermalActionCallback(cb5);
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest005 end.");
}

/**
 * @tc.name: ThermalActionHubTest006
 * @tc.desc: register action is lcd test, scene game, level 3
 * @tc.type: FUNC
 */
HWTEST_F(ThermalActionHubTest, ThermalActionHubTest006, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest006 start.");
    const std::string LCD = "lcd";
    std::vector<std::string> actionList;
    actionList.push_back(LCD);

    std::string desc = "";
    char batteryTempBuf[MAX_PATH] = {0};
    char socTempBuf[MAX_PATH] = {0};
    int32_t ret = -1;
    InitData();
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);

    ret = snprintf_s(socTempBuf, MAX_PATH, sizeof(socTempBuf) - 1, SOC_PATH.c_str());
    EXPECT_EQ(true, ret >= EOK);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    const sptr<IThermalActionCallback> cb6 = new ThermalActionTest6Callback();

    int32_t batteryTemp = 0;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalActionHubTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest006 start register");
    thermalMgrClient.SubscribeThermalActionCallback(actionList, desc, cb6);

    thermalMgrClient.SetScene("game");
    batteryTemp = 46100;
    sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalActionHubTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    MockThermalMgrClient::GetInstance().GetThermalInfo();
    thermalMgrClient.UnSubscribeThermalActionCallback(cb6);
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest006 end.");
}
} // namespace
