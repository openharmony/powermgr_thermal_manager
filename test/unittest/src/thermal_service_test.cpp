/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "thermal_service_test.h"
#include "thermal_xcollie.h"

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
constexpr const char* VENDOR_CONFIG = "/vendor/etc/thermal_config/thermal_service_config.xml";
constexpr const char* SYSTEM_CONFIG = "/system/etc/thermal_config/thermal_service_config.xml";
} // namespace

char* GetOneCfgFile(const char *pathSuffix, char *buf, unsigned int bufLength)
{
    THERMAL_HILOGD(LABEL_TEST, "mock GetOneCfgFile.");
    return nullptr;
}

void ThermalServiceTest::SetUpTestCase()
{
    g_service = ThermalService::GetInstance();
    g_service->InitSystemTestModules();
    g_service->OnStart();
}

namespace {
/**
 * @tc.name: ThermalServiceTest000
 * @tc.desc: test OnAddSystemAbility
 * @tc.type: FUNC
 * @tc.require: issueI6KRS8
 */
HWTEST_F(ThermalServiceTest, ThermalServiceTest000, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalServiceTest000 function start!");
    std::string deviceId = "";
    EXPECT_FALSE(g_service == nullptr);
    g_service->OnAddSystemAbility(COMMON_EVENT_SERVICE_ID, deviceId);
    g_service->OnAddSystemAbility(POWER_MANAGER_THERMAL_SERVICE_ID, deviceId);
    THERMAL_HILOGI(LABEL_TEST, "ThermalServiceTest000 function end!");
}

/**
 * @tc.name: ThermalServiceTest001
 * @tc.desc: test OnStart and OnStop
 * @tc.type: FUNC
 */
HWTEST_F(ThermalServiceTest, ThermalServiceTest001, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalServiceTest001 function start!");
    g_service->ready_ = true;
    g_service->InitSystemTestModules();
    g_service->OnStart();

    g_service->ready_ = false;
    g_service->OnStop();

    g_service->ready_ = true;
    g_service->RegisterHdiStatusListener();
    g_service->GetThermalInfo();
    g_service->OnStop();
    EXPECT_FALSE(g_service->ready_);

    g_service->ready_ = true;
    g_service->OnStop();
    EXPECT_FALSE(g_service->ready_);

    THERMAL_HILOGI(LABEL_TEST, "ThermalServiceTest001 function end!");
}

/**
 * @tc.name: ThermalServiceTest002
 * @tc.desc: test Init
 * @tc.type: FUNC
 */
HWTEST_F(ThermalServiceTest, ThermalServiceTest002, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalServiceTest002 function start!");
    EXPECT_TRUE(g_service->Init());
    EXPECT_TRUE(g_service->CreateConfigModule());
    EXPECT_TRUE(g_service->Init());
    EXPECT_TRUE(g_service->CreateConfigModule());
    EXPECT_TRUE(g_service->InitStateMachine());
    g_service->InitSystemTestModules();
    THERMAL_HILOGI(LABEL_TEST, "ThermalServiceTest002 function end!");
}

/**
 * @tc.name: ThermalServiceTest003
 * @tc.desc: test InitConfigFile
 * @tc.type: FUNC
 */
HWTEST_F(ThermalServiceTest, ThermalServiceTest003, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalServiceTest003 function start!");

    std::string VENDOR_CONFIG_BACKUP = "/vendor/etc/thermal_config/thermal_service_config_backup.xml";
    std::string SYSTEM_CONFIG_BACKUP = "/system/etc/thermal_config/thermal_service_config_backup.xml";

    int32_t ret = rename(VENDOR_CONFIG_BACKUP.c_str(), VENDOR_CONFIG);
    EXPECT_NE(ret, ERR_OK);
    ret = rename(SYSTEM_CONFIG_BACKUP.c_str(), SYSTEM_CONFIG);
    EXPECT_NE(ret, ERR_OK);

    THERMAL_HILOGI(LABEL_TEST, "ThermalServiceTest003 function end!");
}

/**
 * @tc.name: ThermalServiceTest004
 * @tc.desc: test Service Dump
 * @tc.type: FUNC
 */
HWTEST_F(ThermalServiceTest, ThermalServiceTest004, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalServiceTest004 function start!");
    ThermalXCollieTestable thermalXCollie("test_log", true);
    int fd = 0;
    std::vector<std::u16string> args;
    args.push_back(u"-h");
    g_service->isBootCompleted_ = true;
    EXPECT_EQ(ERR_OK, g_service->Dump(fd, args));

    fd = -1;
    EXPECT_EQ(ERR_OK, g_service->Dump(fd, args));

    thermalXCollie.CallCancel();
    thermalXCollie.CallCancel();
    THERMAL_HILOGI(LABEL_TEST, "ThermalServiceTest004 function end!");
}
} // namespace