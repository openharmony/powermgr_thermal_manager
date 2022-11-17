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

#include "thermal_utils_test.h"

#include "errors.h"
#include "file_operation.h"
#include "string_operation.h"
#include "thermal_hisysevent.h"
#include "thermal_log.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
/**
 * @tc.name: ThermalUtilsTest001
 * @tc.desc: utils test
 * @tc.type: FUNC
 * @tc.require: issueI5YZQ2
 */
HWTEST_F(ThermalUtilsTest, ThermalUtilsTest001, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalUtilsTest001 start.");
    std::string dir = "";
    char* buf = nullptr;
    EXPECT_EQ(true, FileOperation::CreateNodeDir(dir) != ERR_OK);
    EXPECT_EQ(true, FileOperation::CreateNodeFile(dir) != ERR_OK);
    dir = "/thermaltest/";
    EXPECT_EQ(true, FileOperation::CreateNodeDir(dir) == ERR_OK);
    dir = "/thermaltest/thermaltest.txt";
    EXPECT_EQ(true, FileOperation::CreateNodeFile(dir) == ERR_OK);
    size_t size = 1;
    EXPECT_EQ(true, FileOperation::WriteFile(dir, dir, size) == ERR_OK);
    dir = "";
    EXPECT_EQ(true, FileOperation::ReadFile(dir.c_str(), buf, size) != ERR_OK);
    dir = "/thermaltest/thermaltest.txt";
    EXPECT_EQ(false, FileOperation::ReadFile(dir.c_str(), buf, size) == ERR_OK);
    dir = "";
    EXPECT_EQ(true, FileOperation::WriteFile(dir, dir, size) != ERR_OK);
    dir = "1";
    EXPECT_EQ(1, FileOperation::ConvertInt(dir));
    THERMAL_HILOGD(LABEL_TEST, "ThermalUtilsTest001 end.");
}

/**
 * @tc.name: ThermalUtilsTest002
 * @tc.desc: utils test
 * @tc.require: issueI5YZQ2
 * @tc.type: FUNC
 */
HWTEST_F(ThermalUtilsTest, ThermalUtilsTest002, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalUtilsTest002 start.");
    std::string str;
    std::vector<std::string> ret;
    std::string sep;
    StringOperation::SplitString(str, ret, sep);
    StringOperation::ClearAllSpace(str);
    str = "str";
    StringOperation::ClearAllSpace(str);
    EXPECT_EQ(false, StringOperation::Compare(str, sep));
    sep = "str";
    EXPECT_EQ(true, StringOperation::Compare(str, sep));
    EXPECT_EQ(true, StringOperation::Find(str, sep));
    str = "";
    EXPECT_EQ(false, StringOperation::Find(str, sep));
    std::string configDir = "/data/service/el0/thermal/config/%s";
    WriteLevelChangedHiSysEvent(true, 1);
    WriteLevelChangedHiSysEvent(false, 1);
    WriteActionTriggeredHiSysEvent(true, configDir, 1);
    WriteActionTriggeredHiSysEvent(false, configDir, 1);
    THERMAL_HILOGD(LABEL_TEST, "ThermalUtilsTest002 end.");
}
} // namespace
