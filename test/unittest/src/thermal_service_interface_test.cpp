/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "thermal_service_interface_test.h"

#ifdef THERMAL_GTEST
#define private   public
#define protected public
#define final
#endif

#include <map>
#include <string>
#include <vector>

#include "system_ability_definition.h"

#include "config_policy_utils.h"
#include "power_mgr_client.h"
#include "thermal_log.h"
#include "thermal_service.h"

using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace testing::ext;
using namespace std;

namespace {
sptr<ThermalService> g_service = nullptr;
} // namespace

void ThermalServiceInterfaceTest::SetUpTestCase()
{
    g_service = ThermalService::GetInstance();
    g_service->InitSystemTestModules();
    g_service->OnStart();
}

sptr<IThermalInterface> ThermalService::GetThermalInterfaceInner()
{
    return nullptr;
}

namespace {
/**
 * @tc.name: ThermalServiceInterfaceTest000
 * @tc.desc: test thermalInterface_
 * @tc.type: FUNC
 * @tc.require: issueI6KRS8
 */
HWTEST_F(ThermalServiceInterfaceTest, ThermalServiceInterfaceTest000, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalServiceInterfaceTest000 function start!");
    int32_t ret = g_service->GetThermalInfo();
    constexpr int32_t param = -1;
    EXPECT_FALSE(g_service == nullptr);
    g_service->RegisterThermalHdiCallback();
    g_service->UnRegisterThermalHdiCallback();
    g_service->RegisterFanHdiCallback();
    g_service->thermalInterface_ = nullptr;
    g_service->RegisterHdiStatusListener();
    EXPECT_TRUE(ret == param);
    THERMAL_HILOGI(LABEL_TEST, "ThermalServiceInterfaceTest000 function end!");
}
} // namespace