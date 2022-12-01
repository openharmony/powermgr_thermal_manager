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

#include "thermal_service_death_test.h"

#include "thermal_mgr_client.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
/**
 * @tc.name: ThermalServiceDeathTest_001
 * @tc.desc: test OnRemoteDied function(Input remoteObj is nullptr, function don't reset proxy)
 * @tc.type: FUNC
 * @tc.require: issueI63SZ4
 */
HWTEST_F (ThermalServiceDeathTest, ThermalServiceDeathTest_001, TestSize.Level0)
{
    auto& thermalClient = ThermalMgrClient::GetInstance();
    EXPECT_EQ(thermalClient.Connect(), ERR_OK);

    std::shared_ptr<IRemoteObject::DeathRecipient> deathRecipient =
        std::make_shared<ThermalMgrClient::ThermalMgrDeathRecipient>();
    wptr<IRemoteObject> remoteObj = nullptr;
    EXPECT_NE(deathRecipient, nullptr);
    deathRecipient->OnRemoteDied(remoteObj);
    EXPECT_NE(thermalClient.thermalSrv_, nullptr);
}
}