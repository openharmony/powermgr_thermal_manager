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

#include "thermal_serivice_test.h"

#include "thermal_policy.h"
#include "thermal_service.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

static sptr<ThermalService> service;
void ThermalServiceTest::SetUpTestCase(void)
{
}

void ThermalServiceTest::TearDownTestCase(void)
{
}

void ThermalServiceTest::SetUp(void)
{
    // create singleton service object at the beginning
    service = DelayedSpSingleton<ThermalService>::GetInstance();
    service->OnStart();
    service->SetActionExecutionFlag(true);
}

void ThermalServiceTest::TearDown(void)
{
    service->OnStop();
    DelayedSpSingleton<ThermalService>::DestroyInstance();
}

namespace {
/**
 * @tc.name: ThermalPolicyTest001
 * @tc.desc: test level decision
 * @tc.type: FUNC
 */
HWTEST_F (ThermalServiceTest, ThermalPolicyTest001, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ThermalPolicyTest001: start.";

    if (service == nullptr) {
        GTEST_LOG_(INFO) << "ThermalPolicyTest001: Failed to get ThermalService";
    }
    service->SetSensorTemp("battery", 48000);
    service->SetSensorTemp("charger", 41600);
    sleep(WAIT_TIME);

    std::map<std::string, uint32_t> map = service->GetPolicy()->GetClusterLevelMap();
    for (auto item : map) {
        GTEST_LOG_(INFO) << "ThermalPolicyTest001 cluster: " << item.first << " level: " << item.second;
    }
    GTEST_LOG_(INFO) << "ThermalPolicyTest001: end.";
}

/**
 * @tc.name: ThermalPolicyTest002
 * @tc.desc: test level decision
 * @tc.type: FUNC
 */
HWTEST_F (ThermalServiceTest, ThermalPolicyTest002, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ThermalPolicyTest002: start.";

    if (service == nullptr) {
        GTEST_LOG_(INFO) << "ThermalPolicyTest002: Failed to get ThermalService";
    }
    service->SetSensorTemp("battery", 42000);
    service->SetSensorTemp("charger", 45000);

    sleep(WAIT_TIME);

    std::map<std::string, uint32_t> map = service->GetPolicy()->GetClusterLevelMap();
    for (auto item : map) {
        GTEST_LOG_(INFO) << "ThermalPolicyTest002 cluster: " << item.first << " level: " << item.second;
    }
    GTEST_LOG_(INFO) << "ThermalPolicyTest002: end.";
}
}