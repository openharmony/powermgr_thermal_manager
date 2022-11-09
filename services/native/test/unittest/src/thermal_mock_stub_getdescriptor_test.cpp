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

#include "thermal_mock_stub_getdescriptor_test.h"

#include "mock_thermal_remote_object.h"
#include "thermal_action_callback_stub.h"
#include "thermal_level_callback_stub.h"
#include "thermal_log.h"
#include "thermal_mgr_errors.h"
#include "thermal_srv_stub.h"
#include "thermal_temp_callback_stub.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
/**
 * @tc.name: ThermalMockStubGetdescriptorTest001
 * @tc.desc: stub test
 * @tc.type: FUNC
 * @tc.require: issueI5YZQ2
 */
HWTEST_F(ThermalMockStubGetdescriptorTest, ThermalMockStubGetdescriptorTest001, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubGetdescriptorTest001 start.");
    uint32_t code = 0;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    sptr<ThermalActionCallbackStub> actionStub = new ThermalActionCallbackStub();
    EXPECT_FALSE(actionStub == nullptr);
    EXPECT_TRUE(E_GET_THERMAL_SERVICE_FAILED == actionStub->OnRemoteRequest(code, data, reply, option));
    sptr<ThermalLevelCallbackStub> levelStub = new ThermalLevelCallbackStub();
    EXPECT_FALSE(levelStub == nullptr);
    EXPECT_TRUE(E_GET_THERMAL_SERVICE_FAILED == levelStub->OnRemoteRequest(code, data, reply, option));
    sptr<ThermalTempCallbackStub> tempStub = new ThermalTempCallbackStub();
    EXPECT_FALSE(tempStub == nullptr);
    EXPECT_TRUE(E_GET_THERMAL_SERVICE_FAILED != tempStub->OnRemoteRequest(code, data, reply, option));
    code++;
    EXPECT_TRUE(E_GET_THERMAL_SERVICE_FAILED != tempStub->OnRemoteRequest(code, data, reply, option));
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubGetdescriptorTest001 end.");
}
} // namespace
