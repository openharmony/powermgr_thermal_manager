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

#include "thermal_application_test.h"
#include "thermal_log.h"

#ifdef THERMAL_GTEST
#define private   public
#define protected public
#endif
#include "thermal_sensor_provision.h"
#include "thermal_kernel_service.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
constexpr int32_t MAX_BUFF_SIZE = 128;
constexpr int32_t NUM_ZERO = 0;
}

namespace {
/**
* @tc.name: ThermalApplicationTest001
* @tc.desc: utils test
* @tc.type: FUNC
* @tc.require: issueI5YZQ2
*/
HWTEST_F(ThermalApplicationTest, ThermalApplicationTest001, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalApplicationTest001 function start!");
    auto &g_service = ThermalKernelService::GetInstance();
    g_service.OnStart();
    std::shared_ptr<ThermalSensorProvision> provision_ = std::make_shared<ThermalSensorProvision>();
    EXPECT_EQ(false, provision_->InitProvision());
    char bufType[MAX_BUFF_SIZE] = {0};
    const char* typePath1 = "/sys/class/thermal";
    int32_t ret = provision_->ReadThermalSysfsToBuff(typePath1, bufType, sizeof(bufType));
    EXPECT_NE(ret, NUM_ZERO);
    const char* typePath2 = "";
    ret = provision_->ReadThermalSysfsToBuff(typePath2, bufType, sizeof(bufType));
    EXPECT_NE(ret, NUM_ZERO);
    THERMAL_HILOGI(LABEL_TEST, "ThermalApplicationTest001 function end!");
}

} // namespace
 