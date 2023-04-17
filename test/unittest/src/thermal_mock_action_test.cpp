/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "thermal_mock_action_test.h"

#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include "securec.h"

#include "mock_socperf_action.h"
#include "thermal_service.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
namespace {
const uint32_t MAX_PATH = 256;
const std::string BATTERY_PATH = "/data/service/el0/thermal/sensor/battery/temp";
constexpr const char* SIMULATION_TEMP_DIR = "/data/service/el0/thermal/sensor/%s/temp";
}
static sptr<ThermalService> g_thermalSvc = nullptr;

int32_t ThermalMockActionTest::WriteFile(std::string path, std::string buf, size_t size)
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

int32_t ThermalMockActionTest::InitNode()
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

void ThermalMockActionTest::SetSensorTemp(int32_t temperature, const std::string& path)
{
    char tempBuf[MAX_PATH] = {0};
    int32_t ret = snprintf_s(tempBuf, MAX_PATH, sizeof(tempBuf) - 1, path.c_str());
    EXPECT_EQ(true, ret >= EOK);

    std::string sTemp = to_string(temperature) + "\n";
    ret = ThermalMockActionTest::WriteFile(tempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
}

void ThermalMockActionTest::SetUpTestCase()
{
    g_thermalSvc = DelayedSpSingleton<ThermalService>::GetInstance();
    g_thermalSvc->OnStart();
}

void ThermalMockActionTest::TearDownTestCase()
{
    g_thermalSvc->OnStop();
    DelayedSpSingleton<ThermalService>::DestroyInstance();
}

void ThermalMockActionTest::SetUp()
{
    InitNode();
}

void ThermalMockActionTest::TearDown()
{
    InitNode();
}

namespace {
/**
 * @tc.name: ThermalMockActionTest001
 * @tc.desc: test cpu boost action
 * @tc.type: FUNC
 * @tc.cond: Set Battery temp
 * @tc.result: level 1, socperf function execution once
 * @tc.require: issueI6JSQD
 */
HWTEST_F (ThermalMockActionTest, ThermalMockActionTest001, Function|MediumTest|Level2)
{
    ASSERT_NE(g_thermalSvc, nullptr);
    int32_t temp = 40100;
    ThermalLevel level = ThermalLevel::COOL;
    int32_t expectLevel = 1;
    SetSensorTemp(temp, BATTERY_PATH);
    g_thermalSvc->GetThermalInfo();
    g_thermalSvc->GetThermalLevel(level);
    EXPECT_EQ(expectLevel, static_cast<int32_t>(level));
    EXPECT_EQ(1, MockSocPerfAction::GetBoostRequestCounter());
    MockSocPerfAction::ClearBoost();
}

/**
 * @tc.name: ThermalMockActionTest001
 * @tc.desc: test cpu boost action
 * @tc.type: FUNC
 * @tc.cond: Set Battery temp
 * @tc.result: level 3, socperf function execution once
 * @tc.require: issueI6JSQD
 */
HWTEST_F (ThermalMockActionTest, ThermalMockActionTest002, Function|MediumTest|Level2)
{
    ASSERT_NE(g_thermalSvc, nullptr);
    int32_t temp = 46100;
    ThermalLevel level = ThermalLevel::COOL;
    int32_t expectLevel = 3;
    SetSensorTemp(temp, BATTERY_PATH);
    g_thermalSvc->GetThermalInfo();
    g_thermalSvc->GetThermalLevel(level);
    EXPECT_EQ(expectLevel, static_cast<int32_t>(level));
    EXPECT_EQ(1, MockSocPerfAction::GetBoostRequestCounter());
    MockSocPerfAction::ClearBoost();
}

/**
 * @tc.name: ThermalMockActionTest003
 * @tc.desc: test cpu boost action
 * @tc.type: FUNC
 * @tc.cond: Set Battery temp
 * @tc.result: level 2, socperf function execution once
 * @tc.require: issueI6JSQD
 */
HWTEST_F (ThermalMockActionTest, ThermalMockActionTest003, Function|MediumTest|Level2)
{
    ASSERT_NE(g_thermalSvc, nullptr);
    int32_t temp = 43100;
    ThermalLevel level = ThermalLevel::COOL;
    int32_t expectLevel = 2;
    SetSensorTemp(temp, BATTERY_PATH);
    g_thermalSvc->GetThermalInfo();
    g_thermalSvc->GetThermalLevel(level);
    EXPECT_EQ(expectLevel, static_cast<int32_t>(level));
    EXPECT_EQ(1, MockSocPerfAction::GetBoostRequestCounter());
    MockSocPerfAction::ClearBoost();
}
}
