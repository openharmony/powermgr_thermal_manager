/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "thermal_client_api_test.h"

#include "thermal_mgr_client.h"

using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace testing::ext;

namespace {
/*
 * @tc.number: ThermalClientApiTest_001
 * @tc.name: ThermalClientApiTest_001
 * @tc.desc: Test possible security issue caused by ShellDump API
 */
HWTEST_F(ThermalClientApiTest, ThermalClientApiTest_001, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalClientApiTest_001:: Test Start!!");
    size_t size = 1000;
    std::vector<std::string> cmdsList;
    for (size_t i = 0; i < size; i++) {
        std::string cmd = "test_cmd" + std::to_string(i);
        cmdsList.push_back(cmd);
    }
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    std::string result = thermalMgrClient.Dump(cmdsList);

    THERMAL_HILOGD(LABEL_TEST, "result : %{public}s", result.c_str());
    EXPECT_EQ(true, result == "remote error");
    THERMAL_HILOGD(LABEL_TEST, "ThermalClientApiTest_001:: Test End!!");
}

/*
 * @tc.number: ThermalClientApiTest_002
 * @tc.name: ThermalClientApiTest_002
 * @tc.desc: Test possible security issue caused by ShellDump API
 */
HWTEST_F(ThermalClientApiTest, ThermalClientApiTest_002, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalClientApiTest_002:: Test Start!!");
    size_t size = 5;
    std::vector<std::string> cmdsList;
    for (size_t i = 0; i < size; i++) {
        std::string cmd = "test_cmd" + std::to_string(i);
        cmdsList.push_back(cmd);
    }
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    std::string result = thermalMgrClient.Dump(cmdsList);

    THERMAL_HILOGD(LABEL_TEST, "result : %{public}s", result.c_str());
    EXPECT_EQ(true, result.empty());
    THERMAL_HILOGD(LABEL_TEST, "ThermalClientApiTest_002:: Test End!!");
}

/*
 * @tc.number: ThermalClientApiTest_003
 * @tc.name: ThermalClientApiTest_003
 * @tc.desc: Test get thermal level
 */
HWTEST_F(ThermalClientApiTest, ThermalClientApiTest_003, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalClientApiTest_003:: Test Start!!");

    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    ThermalLevel level = thermalMgrClient.GetThermalLevel();

    EXPECT_EQ(true, level <= ThermalLevel::EMERGENCY && level >= ThermalLevel::COOL);
    THERMAL_HILOGD(LABEL_TEST, "ThermalClientApiTest_003:: Test End!!");
}
}