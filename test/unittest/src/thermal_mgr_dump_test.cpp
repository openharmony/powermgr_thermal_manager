/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "thermal_mgr_dump_test.h"

#ifdef THERMAL_GTEST
#define private   public
#define protected public
#define final
#endif

#include <map>
#include <string>
#include <vector>

#include "thermal_log.h"
#include "thermal_mgr_dumper.h"


using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace testing::ext;
using namespace std;

namespace {
sptr<ThermalService> g_service = nullptr;
} // namespace

char* GetOneCfgFile(const char *pathSuffix, char *buf, unsigned int bufLength)
{
    THERMAL_HILOGI(LABEL_TEST, "mock GetOneCfgFile.");
    return nullptr;
}

void ThermalMgrDumperTest::SetUpTestCase()
{
    g_service = ThermalService::GetInstance();
    g_service->InitSystemTestModules();
    g_service->OnStart();
}

namespace {
/**
 * @tc.name: ThermalMgrDumperTest001
 * @tc.desc: test dump help
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrDumperTest, ThermalMgrDumperTest001, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalMgrDumperTest001 start.");
    std::vector<std::string> args;
    std::string result;
    EXPECT_TRUE(ThermalMgrDumper::Dump(args, result));
    EXPECT_TRUE(!result.empty());

    result.clear();
    args.push_back("-h");
    EXPECT_TRUE(ThermalMgrDumper::Dump(args, result));
    EXPECT_TRUE(!result.empty());

    THERMAL_HILOGI(LABEL_TEST, "ThermalMgrDumperTest001 end.");
}

/**
 * @tc.name: ThermalMgrDumperTest002
 * @tc.desc: test dump -d
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrDumperTest, ThermalMgrDumperTest002, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalMgrDumperTest002 start.");
    std::vector<std::string> args;
    std::string result;

    args.push_back("-d");
    EXPECT_TRUE(ThermalMgrDumper::Dump(args, result));
    EXPECT_TRUE(result.empty());
    THERMAL_HILOGI(LABEL_TEST, "ThermalMgrDumperTest002 end.");
}

/**
 * @tc.name: ThermalMgrDumperTest003
 * @tc.desc: test dump -t
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrDumperTest, ThermalMgrDumperTest003, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalMgrDumperTest003 start.");
    std::vector<std::string> args;
    std::string result;

    g_service->RegisterThermalHdiCallback();
    args.push_back("-t");
    EXPECT_TRUE(ThermalMgrDumper::Dump(args, result));
    THERMAL_HILOGI(LABEL_TEST, "ThermalMgrDumperTest003 end.");
}

/**
 * @tc.name: ThermalMgrDumperTest004
 * @tc.desc: test dump Invalid value
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrDumperTest, ThermalMgrDumperTest004, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalMgrDumperTest004 start.");
    std::vector<std::string> args;
    std::string result;

    args.push_back("---");
    EXPECT_TRUE(ThermalMgrDumper::Dump(args, result));
    EXPECT_TRUE(result.empty());
    THERMAL_HILOGI(LABEL_TEST, "ThermalMgrDumperTest004 end.");
}

/**
 * @tc.name: ThermalMgrDumperTest005
 * @tc.desc: test dump switch temp report
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrDumperTest, ThermalMgrDumperTest005, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalMgrDumperTest005 start.");
    std::vector<std::string> args;
    std::string result;

    args.push_back("-st");
    args.push_back("0");
    EXPECT_TRUE(ThermalMgrDumper::Dump(args, result));
    args.clear();
    args.push_back("-st");
    args.push_back("1");
    EXPECT_TRUE(ThermalMgrDumper::Dump(args, result));
    THERMAL_HILOGI(LABEL_TEST, "ThermalMgrDumperTest005 end.");
}

/**
 * @tc.name: ThermalMgrDumperTest006
 * @tc.desc: test dump temp emul
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMgrDumperTest, ThermalMgrDumperTest006, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalMgrDumperTest006 start.");
    std::vector<std::string> args;
    std::string result;

    args.push_back("-te");
    args.push_back("battery");
    args.push_back("43000");
    EXPECT_TRUE(ThermalMgrDumper::Dump(args, result));
    args.clear();
    args.push_back("-te");
    EXPECT_TRUE(ThermalMgrDumper::Dump(args, result));
    THERMAL_HILOGI(LABEL_TEST, "ThermalMgrDumperTest006 end.");
}
} // namespace
