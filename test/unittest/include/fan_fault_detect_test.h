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

#ifndef FAN_FAULT_DETECT_TEST_H
#define FAN_FAULT_DETECT_TEST_H

#include <gtest/gtest.h>
#include "fan_fault_detect.h"

namespace OHOS {
namespace PowerMgr {
class FanFaultDetectTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    void GetFaultId(int64_t &faultId, const FanSensorInfo& report);
private:
    void InitFanFaultInfoMap(const std::shared_ptr<FanFaultDetect>& fanFaultDetect);
};
} // namespace PowerMgr
} // namespace OHOS
#endif // FAN_FAULT_DETECT_TEST_H
