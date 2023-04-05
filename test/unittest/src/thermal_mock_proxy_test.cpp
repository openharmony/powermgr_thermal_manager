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

#include "thermal_mock_proxy_test.h"

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
 * @tc.name: ThermalMockProxyTest001
 * @tc.desc: proxy test
 * @tc.type: FUNC
 * @tc.require: issueI5YZQ2
 */
HWTEST_F(ThermalMockProxyTest, ThermalMockProxyTest001, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockProxyTest001 start.");
    std::string result = "a";
    sptr<MockThermalRemoteObject> sptrRemoteObj = new MockThermalRemoteObject();
    EXPECT_FALSE(sptrRemoteObj == nullptr);
    std::shared_ptr<ThermalActionCallbackProxy> actionProxy =
        std::make_shared<ThermalActionCallbackProxy>(sptrRemoteObj);
    EXPECT_FALSE(actionProxy == nullptr);
    using ActionCallbackMap = std::map<std::string, float>;
    ActionCallbackMap map;
    map.insert(std::make_pair(result, 1));
    EXPECT_TRUE(actionProxy->OnThermalActionChanged(map));
    std::shared_ptr<ThermalLevelCallbackProxy> levalProxy = std::make_shared<ThermalLevelCallbackProxy>(sptrRemoteObj);
    EXPECT_FALSE(levalProxy == nullptr);
    ThermalLevel leval = ThermalLevel::COOL;
    EXPECT_TRUE(levalProxy->GetThermalLevel(leval));
    std::shared_ptr<ThermalTempCallbackProxy> tempProxy = std::make_shared<ThermalTempCallbackProxy>(sptrRemoteObj);
    EXPECT_FALSE(tempProxy == nullptr);
    using TempCallbackMap = std::map<std::string, int32_t>;
    TempCallbackMap tempCbMap;
    tempCbMap.insert(std::make_pair(result, 1));
    EXPECT_TRUE(tempProxy->OnThermalTempChanged(tempCbMap));
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockProxyTest001 end.");
}

/**
 * @tc.name: ThermalMockProxyTest002
 * @tc.desc: proxy test
 * @tc.type: FUNC
 * @tc.require: issueI5YZQ2
 */
HWTEST_F(ThermalMockProxyTest, ThermalMockProxyTest002, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockProxyTest002 start.");
    sptr<MockThermalRemoteObject> sptrRemoteObj = new MockThermalRemoteObject();
    std::shared_ptr<ThermalSrvProxy> srvProxy = std::make_shared<ThermalSrvProxy>(sptrRemoteObj);
    EXPECT_FALSE(srvProxy == nullptr);
    std::vector<std::string> typeList;
    std::string desc;
    sptr<IThermalTempCallback> tempCallback;
    EXPECT_FALSE(srvProxy->SubscribeThermalTempCallback(typeList, tempCallback));
    EXPECT_FALSE(srvProxy->UnSubscribeThermalTempCallback(tempCallback));
    sptr<IThermalLevelCallback> levelCallback;
    EXPECT_FALSE(srvProxy->SubscribeThermalLevelCallback(levelCallback));
    EXPECT_FALSE(srvProxy->UnSubscribeThermalLevelCallback(levelCallback));
    sptr<IThermalActionCallback> actionCallback;
    EXPECT_FALSE(srvProxy->SubscribeThermalActionCallback(typeList, desc, actionCallback));
    EXPECT_FALSE(srvProxy->UnSubscribeThermalActionCallback(actionCallback));

    tempCallback = new ThermalTempCallbackProxy(sptrRemoteObj);
    EXPECT_FALSE(tempCallback == nullptr);
    EXPECT_TRUE(srvProxy->SubscribeThermalTempCallback(typeList, tempCallback));
    EXPECT_TRUE(srvProxy->UnSubscribeThermalTempCallback(tempCallback));
    levelCallback = new ThermalLevelCallbackProxy(sptrRemoteObj);
    EXPECT_FALSE(levelCallback == nullptr);
    EXPECT_TRUE(srvProxy->SubscribeThermalLevelCallback(levelCallback));
    EXPECT_TRUE(srvProxy->UnSubscribeThermalLevelCallback(levelCallback));
    actionCallback = new ThermalActionCallbackProxy(sptrRemoteObj);
    EXPECT_FALSE(actionCallback == nullptr);
    EXPECT_TRUE(srvProxy->SubscribeThermalActionCallback(typeList, desc, actionCallback));
    EXPECT_TRUE(srvProxy->UnSubscribeThermalActionCallback(actionCallback));
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockProxyTest002 end.");
}
} // namespace
