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
constexpr int32_t BUF_LEN = 64;
constexpr int32_t NUM_ZERO = 0;
}

namespace {
/**
 * @tc.name: ThermalUtilsTest001
 * @tc.desc: utils test
 * @tc.type: FUNC
 * @tc.require: issueI5YZQ2
 */
HWTEST_F(ThermalUtilsTest, ThermalUtilsTest001, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalUtilsTest001 function start!");
    std::string dir = "";
    char buf[BUF_LEN];
    EXPECT_EQ(true, FileOperation::CreateNodeDir(dir) != ERR_OK);
    EXPECT_EQ(true, FileOperation::CreateNodeFile(dir) != ERR_OK);
    dir = "/data/thermaltest/";
    EXPECT_EQ(true, FileOperation::CreateNodeDir(dir) == ERR_OK);
    dir = "/data/thermaltest/thermaltest.txt";
    EXPECT_EQ(true, FileOperation::CreateNodeFile(dir) == ERR_OK);
    size_t size = 1;
    EXPECT_EQ(true, FileOperation::WriteFile(dir, dir, size) == ERR_OK);
    dir = "";
    EXPECT_EQ(true, FileOperation::ReadFile(dir.c_str(), buf, size) != ERR_OK);
    dir = "/data/thermaltest/thermaltest.txt";
    EXPECT_EQ(true, FileOperation::ReadFile(dir.c_str(), buf, size) == ERR_OK);
    dir = "";
    EXPECT_EQ(true, FileOperation::WriteFile(dir, dir, size) != ERR_OK);
    dir = "1";
    EXPECT_EQ(1, FileOperation::ConvertInt(dir));
    THERMAL_HILOGI(LABEL_TEST, "ThermalUtilsTest001 function end!");
}

/**
 * @tc.name: ThermalUtilsTest002
 * @tc.desc: utils test
 * @tc.type: FUNC
 * @tc.require: issueIBYAS8
 */
HWTEST_F(ThermalUtilsTest, ThermalUtilsTest002, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalUtilsTest002 function start!");
    std::string dir = "";
    char buf[BUF_LEN];
    EXPECT_EQ(true, FileOperation::CreateNodeDir(dir) != ERR_OK);
    EXPECT_EQ(true, FileOperation::CreateNodeFile(dir) != ERR_OK);
    dir = "/data/thermaltest/";
    EXPECT_EQ(true, FileOperation::CreateNodeDir(dir) == ERR_OK);
    dir = "/data/thermaltest/thermaltest.txt";
    EXPECT_EQ(true, FileOperation::CreateNodeFile(dir) == ERR_OK);
    size_t size = dir.size()+1;
    EXPECT_EQ(true, FileOperation::WriteFile(dir, dir, size) == ERR_OK);
    EXPECT_EQ(true, FileOperation::ReadFile(dir.c_str(), buf, size) == ERR_OK);
    EXPECT_EQ(true, dir.compare(buf) == NUM_ZERO);
    dir = "";
    EXPECT_EQ(true, FileOperation::ReadFile(dir.c_str(), buf, size) != ERR_OK);
    EXPECT_EQ(true, FileOperation::WriteFile(dir, dir, size) != ERR_OK);
    THERMAL_HILOGI(LABEL_TEST, "ThermalUtilsTest002 function end!");
}

/**
 * @tc.name: ThermalUtilsTest003
 * @tc.desc: utils test
 * @tc.require: issueI5YZQ2
 * @tc.type: FUNC
 */
HWTEST_F(ThermalUtilsTest, ThermalUtilsTest003, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalUtilsTest003 function start!");
    std::string str;
    std::vector<std::string> ret;
    std::string sep;
    StringOperation::SplitString(str, ret, sep);
    StringOperation::ClearAllSpace(str);
    str = "str";
    uint32_t val = 1;
    double value = 2;
    StringOperation::StrToUint("", val);
    StringOperation::StrToDouble("", value);
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
    WriteActionTriggeredHiSysEventWithRatio(true, configDir, 1);
    WriteActionTriggeredHiSysEventWithRatio(false, configDir, 1);
    THERMAL_HILOGI(LABEL_TEST, "ThermalUtilsTest003 function end!");
}

/**
 * @tc.name: ThermalUtilsTest004
 * @tc.desc: utils test ParseStrtollResult
 * @tc.require: issueI5YZQ2
 * @tc.type: FUNC
 */
HWTEST_F(ThermalUtilsTest, ThermalUtilsTest004, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalUtilsTest004 function start!");
    int64_t result = 0;
    std::string strfirst = "";
    EXPECT_EQ(false, StringOperation::ParseStrtollResult(strfirst, result));
    result = 0;
    std::string strsecond = "abc";
    EXPECT_EQ(false, StringOperation::ParseStrtollResult(strsecond, result));
    result = 0;
    std::string strthird = "abc123";
    EXPECT_EQ(false, StringOperation::ParseStrtollResult(strthird, result));
    result = 0;
    std::string strfourth = "123abc";
    EXPECT_EQ(true, StringOperation::ParseStrtollResult(strfourth, result));
    result = 0;
    std::string strfifth = "123";
    EXPECT_EQ(true, StringOperation::ParseStrtollResult(strfifth, result));
    result = 0;
    std::string strsixth = "12345678999987654321";
    EXPECT_EQ(false, StringOperation::ParseStrtollResult(strsixth, result));
    THERMAL_HILOGI(LABEL_TEST, "ThermalUtilsTest004 function end!");
}

/**
 * @tc.name: ThermalUtilsTest005
 * @tc.desc: utils test ParseStrtoulResult
 * @tc.require: issueI5YZQ2
 * @tc.type: FUNC
 */
HWTEST_F(ThermalUtilsTest, ThermalUtilsTest005, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalUtilsTest005 function start!");
    unsigned long result = 0;
    std::string strfirst = "";
    EXPECT_EQ(false, StringOperation::ParseStrtoulResult(strfirst, result));
    result = 0;
    std::string strsecond = "abc";
    EXPECT_EQ(false, StringOperation::ParseStrtoulResult(strsecond, result));
    result = 0;
    std::string strthird = "abc123";
    EXPECT_EQ(false, StringOperation::ParseStrtoulResult(strthird, result));
    result = 0;
    std::string strfourth = "123abc";
    EXPECT_EQ(true, StringOperation::ParseStrtoulResult(strfourth, result));
    result = 0;
    std::string strfifth = "123";
    EXPECT_EQ(true, StringOperation::ParseStrtoulResult(strfifth, result));
    result = 0;
    std::string strsixth = "123456789999987654321";
    EXPECT_EQ(false, StringOperation::ParseStrtoulResult(strsixth, result));
    THERMAL_HILOGI(LABEL_TEST, "ThermalUtilsTest005 function end!");
}
} // namespace
