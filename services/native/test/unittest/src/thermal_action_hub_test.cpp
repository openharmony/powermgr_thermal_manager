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
#include <iostream>
#include <list>
#include <mutex>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <datetime_ex.h>
#include <gtest/gtest.h>
#include <ipc_skeleton.h>
#include <string_ex.h>

#include "ithermal_srv.h"
#include "securec.h"
#include "thermal_mgr_client.h"
#include "thermal_srv_sensor_info.h"
#include "constants.h"
#include "thermal_common.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

static std::mutex g_mtx;
std::vector<std::string> typeList;

int32_t ThermalActionHubTest::WriteFile(std::string path, std::string buf, size_t size)
{
    FILE *stream = fopen(path.c_str(), "w+");
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

int32_t ThermalActionHubTest::ReadFile(const char *path, char *buf, size_t size)
{
    int32_t ret;

    int32_t fd = open(path, O_RDONLY);
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

int32_t ThermalActionHubTest::ConvertInt(const std::string &value)
{
    return std::stoi(value);
}

void ThermalActionHubTest::SetUpTestCase()
{
}

void ThermalActionHubTest::TearDownTestCase()
{
}

void ThermalActionHubTest::SetUp()
{
}

void ThermalActionHubTest::TearDown()
{
}

void ThermalActionHubTest::InitData()
{
    typeList.push_back(BATTERY);
    typeList.push_back(SOC);
}

void ThermalActionHubTest::ThermalActionTest1Callback::OnThermalActionChanged(ActionCallbackMap &actionCbMap)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest1Callback::OnThermalActionChanged Enter");
    for (auto iter : actionCbMap) {
        GTEST_LOG_(INFO) << "actionName: " << iter.first << " actionValue: " << iter.second;
    }
}

void ThermalActionHubTest::ThermalActionTest2Callback::OnThermalActionChanged(ActionCallbackMap &actionCbMap)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest2Callback::OnThermalActionChanged Enter");
    for (auto iter : actionCbMap) {
        GTEST_LOG_(INFO) << "actionName: " << iter.first << " actionValue: " << iter.second;
    }
}

void ThermalActionHubTest::ThermalActionTest3Callback::OnThermalActionChanged(ActionCallbackMap &actionCbMap)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest3Callback::OnThermalActionChanged Enter");
    for (auto iter : actionCbMap) {
        GTEST_LOG_(INFO) << "actionName: " << iter.first << " actionValue: " << iter.second;
    }
}

namespace {
/**
 * @tc.name: ThermalActionHubTest001
 * @tc.desc: register action is cpu test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalActionHubTest, ThermalActionHubTest001, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest001 start.");
    std::vector<std::string> actionList;
    actionList.push_back(CPU);

    std::string desc = "";
    char batteryTempBuf[MAX_PATH] = {0};
    char socTempBuf[MAX_PATH] = {0};
    int32_t ret = -1;
    InitData();
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);

    ret = snprintf_s(socTempBuf, MAX_PATH, sizeof(socTempBuf) - 1, socPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    const sptr<IThermalActionCallback> cb1 = new ThermalActionTest1Callback();

    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest001 start register");
    thermalMgrClient.SubscribeThermalActionCallback(actionList, desc, cb1);

    int32_t batteryTemp = 40100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalActionHubTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(5);
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
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);

    ret = snprintf_s(socTempBuf, MAX_PATH, sizeof(socTempBuf) - 1, socPath.c_str());
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

    sleep(5);
    thermalMgrClient.UnSubscribeThermalActionCallback(cb2);
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest002 end.");
}

/**
 * @tc.name: ThermalActionHubTest003
 * @tc.desc: register action is cpu and lcd test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalActionHubTest, ThermalActionHubTest003, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest003 start.");
    const std::string LCD = "lcd";
    std::vector<std::string> actionList;
    actionList.push_back(CPU);
    actionList.push_back(LCD);

    std::string desc = "";
    char batteryTempBuf[MAX_PATH] = {0};
    char socTempBuf[MAX_PATH] = {0};
    int32_t ret = -1;
    InitData();
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, batteryPath.c_str());
    EXPECT_EQ(true, ret >= EOK);

    ret = snprintf_s(socTempBuf, MAX_PATH, sizeof(socTempBuf) - 1, socPath.c_str());
    EXPECT_EQ(true, ret >= EOK);
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    const sptr<IThermalActionCallback> cb3 = new ThermalActionTest3Callback();

    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest003 start register");
    thermalMgrClient.SubscribeThermalActionCallback(actionList, desc, cb3);

    int32_t batteryTemp = 46100;
    std::string sTemp = to_string(batteryTemp) + "\n";
    ret = ThermalActionHubTest::WriteFile(batteryTempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);

    sleep(5);
    thermalMgrClient.UnSubscribeThermalActionCallback(cb3);
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionHubTest003 end.");
}
}
