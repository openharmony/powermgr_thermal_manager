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

#ifndef THERMAL_MGR_POLICY_TEST_H
#define THERMAL_MGR_POLICY_TEST_H

#include <mutex>
#include <string>
#include <gtest/gtest.h>

namespace OHOS {
namespace PowerMgr {
const uint32_t MAX_PATH = 256;
constexpr uint32_t WAIT_TIME = 10;
std::string batteryPath = "/data/thermal/battery/temp";
std::string paPath = "/data/thermal/pa/temp";
std::string ambientPath = "/data/thermal/ambient/temp";
std::string apPath = "/data/thermal/ap/temp";
std::string chargerPath = "/data/thermal/charger/temp";
std::string batteryCurrentPath = "/data/mitigation/charger/current";
std::string cpuFreqPath = "/data/mitigation/cpu/freq";
class ThermalMgrPolicyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_MGR_POLICY_TEST_H