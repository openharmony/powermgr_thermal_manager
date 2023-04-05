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

#ifndef THERMAL_ACTION_REPORT_TEST
#define THERMAL_ACTION_REPORT_TEST

#include <gtest/gtest.h>
#include <string>

#include "thermal_policy.h"

namespace OHOS {
namespace PowerMgr {
class ThermalActionReportTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static void ParserThermalSrvConfigFile();
    static int32_t WriteFile(const std::string& path, const std::string& buf, size_t size);
    static int32_t ReadFile(const char* path, char* buf, size_t size);
    static int32_t ReadSysfsFile(const char* path, char* buf, size_t size);
    static int32_t ConvertInt(const std::string& value);
    static void Trim(char* str);
    static int32_t InitNode();
    static int32_t SetCondition(int32_t value, const std::string& scene);
    static void SetScene(const std::string& scene);
    static int32_t GetThermalLevel(int32_t expectValue);
    static std::string GetActionValue(const std::string& actionName, uint32_t level);
    static std::string ActionDecision(const std::string& actionName, std::vector<PolicyAction>& vAction);
    static std::string ActionValueDecision(const std::string& actionName, std::vector<PolicyAction>& vAction);
    static std::string LcdValueDecision(const std::string& actionName, std::vector<PolicyAction>& vAction);
    static bool StateDecision(std::map<std::string, std::string>& actionPropMap);
    static std::string GetScreenState();
    static std::string GetChargeState();
    static void GetStateMap(std::map<std::string, std::string>& stateMap);
    static void ThermalActionTriggered(const std::string& actionName, int32_t level, const std::string& dumpInfo);
    static void ReturnWithLog(bool conditon, int32_t ret, int32_t level, const std::string& info);
};
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_ACTION_REPORT_TEST