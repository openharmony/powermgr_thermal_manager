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

#include "thermal_mock_proxy_sendrequest_test.h"

#include "mock_thermal_remote_object.h"
#include "thermal_action_callback_proxy.h"
#include "thermal_level_callback_proxy.h"
#include "thermal_log.h"
#include "thermal_srv_proxy.h"
#include "thermal_temp_callback_proxy.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
/**
 * @tc.name: ThermalMockProxySendrequestTest001
 * @tc.desc: proxy test
 * @tc.type: FUNC
 * @tc.require: issueI5YZQ2
 */
HWTEST_F(ThermalMockProxySendrequestTest, ThermalMockProxySendrequestTest001, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalMockProxySendrequestTest001 function start!");
    sptr<MockThermalRemoteObject> sptrRemoteObj = new MockThermalRemoteObject();
    EXPECT_FALSE(sptrRemoteObj == nullptr);
    std::shared_ptr<ThermalActionCallbackProxy> actionProxy =
        std::make_shared<ThermalActionCallbackProxy>(sptrRemoteObj);
    EXPECT_FALSE(actionProxy == nullptr);
    using ActionCallbackMap = std::map<std::string, std::string>;
    ActionCallbackMap map;
    EXPECT_FALSE(actionProxy->OnThermalActionChanged(map));
    std::shared_ptr<ThermalLevelCallbackProxy> levalProxy = std::make_shared<ThermalLevelCallbackProxy>(sptrRemoteObj);
    EXPECT_FALSE(levalProxy == nullptr);
    ThermalLevel leval = ThermalLevel::COOL;
    EXPECT_FALSE(levalProxy->OnThermalLevelChanged(leval));

    std::shared_ptr<ThermalSrvProxy> srvProxy = std::make_shared<ThermalSrvProxy>(sptrRemoteObj);
    EXPECT_FALSE(srvProxy == nullptr);
    std::vector<std::string> typeList;
    std::string desc;
    sptr<IThermalTempCallback> tempCallback = new ThermalTempCallbackProxy(sptrRemoteObj);
    EXPECT_FALSE(tempCallback == nullptr);
    EXPECT_NE(srvProxy->SubscribeThermalTempCallback(typeList, tempCallback), ERR_OK);
    EXPECT_NE(srvProxy->UnSubscribeThermalTempCallback(tempCallback), ERR_OK);
    sptr<IThermalLevelCallback> levelCallback = new ThermalLevelCallbackProxy(sptrRemoteObj);
    EXPECT_FALSE(levelCallback == nullptr);
    EXPECT_NE(srvProxy->SubscribeThermalLevelCallback(levelCallback), ERR_OK);
    EXPECT_NE(srvProxy->UnSubscribeThermalLevelCallback(levelCallback), ERR_OK);
    sptr<IThermalActionCallback> actionCallback = new ThermalActionCallbackProxy(sptrRemoteObj);
    EXPECT_FALSE(actionCallback == nullptr);
    EXPECT_NE(srvProxy->SubscribeThermalActionCallback(typeList, desc, actionCallback), ERR_OK);
    EXPECT_NE(srvProxy->UnSubscribeThermalActionCallback(actionCallback), ERR_OK);
    THERMAL_HILOGI(LABEL_TEST, "ThermalMockProxySendrequestTest001 function end!");
}
} // namespace
