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

#ifndef THERMAL_MGR_POLICY_TEST
#define THERMAL_MGR_POLICY_TEST

#include <gtest/gtest.h>
#include <string>

namespace OHOS {
namespace PowerMgr {
class ThermalMockActionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static void SetSensorTemp(int32_t temperature, const std::string& path);
    static int32_t WriteFile(std::string path, std::string buf, size_t size);
    static int32_t InitNode();
};
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_MGR_POLICY_TEST
