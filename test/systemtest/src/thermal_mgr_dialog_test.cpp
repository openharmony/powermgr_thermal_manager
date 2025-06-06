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

#include "thermal_mgr_dialog_test.h"
#include "action_popup.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;

const char *g_perms[] = { "ohos.permission.START_INVISIBLE_ABILITY" };
uint64_t g_token;
AccessTokenID ThermalMgrDialogTest::tokenID_ = 0;

NativeTokenInfoParams infoInstance = {
    .dcapsNum = 0,
    .permsNum = 1,
    .aclsNum = 0,
    .dcaps = nullptr,
    .perms = g_perms,
    .acls = nullptr,
    .processName = "ThermalMgrDialogTest",
    .aplStr = "system_basic",
};

void ThermalMgrDialogTest::SetUpTestCase()
{
    g_token = GetSelfTokenID();
    tokenID_ = GetAccessTokenId(&infoInstance);
    ASSERT_NE(0, tokenID_);
    ASSERT_EQ(0, SetSelfTokenID(tokenID_));
    AccessTokenKit::ReloadNativeTokenInfo();
}

void ThermalMgrDialogTest::TearDownTestCase()
{
    AccessTokenKit::DeleteToken(tokenID_);
    SetSelfTokenID(g_token);
}

void ThermalMgrDialogTest::SetUp() {}

void ThermalMgrDialogTest::TearDown() {}

namespace {
/**
 * @tc.name: ThermalMgrDialogTest001
 * @tc.desc: test pull up HighThermalDialog in StartPowerDialog
 * @tc.type: FUNC
 * @tc.require: issueissueI5I9BF
 */
HWTEST_F(ThermalMgrDialogTest, ThermalMgrDialogTest001, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalMgrDialogTest001 function start!");
    auto popup = std::make_unique<ActionPopup>("Start HighThermal Dialog");
    EXPECT_TRUE(popup->ShowThermalDialog(ActionPopup::TempStatus::HIGHER_TEMP));
    THERMAL_HILOGI(LABEL_TEST, "ThermalMgrDialogTest001 function end!");
}

/**
 * @tc.name: ThermalMgrDialogTest002
 * @tc.desc: test pull up LowThermalDialog in StartPowerDialog
 * @tc.type: FUNC
 * @tc.require: issueissueI5I9BF
 */
HWTEST_F(ThermalMgrDialogTest, ThermalMgrDialogTest002, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalMgrDialogTest002 function start!");
    auto popup = std::make_unique<ActionPopup>("Start LowThermal Dialog");
    EXPECT_TRUE(popup->ShowThermalDialog(ActionPopup::TempStatus::LOWER_TEMP));
    THERMAL_HILOGI(LABEL_TEST, "ThermalMgrDialogTest002 function end!");
}
} // namespace