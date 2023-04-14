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

#include "thermal_mock_proxy_remote_test.h"

#include "mock_thermal_remote_object.h"
#include "thermal_action_callback_proxy.h"
#include "thermal_level_callback_proxy.h"
#include "thermal_level_info.h"
#include "thermal_log.h"
#include "thermal_mgr_client.h"
#include "thermal_srv_proxy.h"
#include "thermal_srv_sensor_info.h"
#include "thermal_temp_callback_proxy.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
/**
 * @tc.name: ThermalClientMockPeerTest001
 * @tc.desc: Thermal client test by mock peer
 * @tc.type: FUNC
 * @tc.require: issueI64U2R
 */
HWTEST_F(ThermalMockProxyRemoteTest, ThermalClientMockPeerTest001, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalClientMockPeerTest001 start");
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();

    sptr<MockThermalRemoteObject> sptrRemoteObj = new MockThermalRemoteObject();
    EXPECT_FALSE(sptrRemoteObj == nullptr);
    std::shared_ptr<ThermalSrvProxy> srvProxy = std::make_shared<ThermalSrvProxy>(sptrRemoteObj);
    EXPECT_FALSE(srvProxy == nullptr);
    std::vector<std::string> typeList {};
    std::string desc {};
    sptr<IThermalTempCallback> tempCallback = new ThermalTempCallbackProxy(sptrRemoteObj);
    EXPECT_FALSE(tempCallback == nullptr);
    EXPECT_FALSE(thermalMgrClient.SubscribeThermalTempCallback(typeList, tempCallback));
    EXPECT_FALSE(thermalMgrClient.UnSubscribeThermalTempCallback(tempCallback));
    sptr<IThermalLevelCallback> levelCallback = new ThermalLevelCallbackProxy(sptrRemoteObj);
    EXPECT_FALSE(levelCallback == nullptr);
    EXPECT_FALSE(thermalMgrClient.SubscribeThermalLevelCallback(levelCallback));
    EXPECT_FALSE(thermalMgrClient.UnSubscribeThermalLevelCallback(levelCallback));
    sptr<IThermalActionCallback> actionCallback = new ThermalActionCallbackProxy(sptrRemoteObj);
    EXPECT_FALSE(actionCallback == nullptr);
    EXPECT_FALSE(thermalMgrClient.SubscribeThermalActionCallback(typeList, desc, actionCallback));
    EXPECT_FALSE(thermalMgrClient.UnSubscribeThermalActionCallback(actionCallback));

    SensorType sensorType = SensorType::SOC;
    EXPECT_TRUE(thermalMgrClient.GetThermalSensorTemp(sensorType) == 0);
    EXPECT_TRUE(thermalMgrClient.GetThermalLevel() == ThermalLevel::COOL);
    std::string sence = "";
    EXPECT_FALSE(thermalMgrClient.SetScene(sence));
    std::vector<std::string> args {};
    EXPECT_TRUE(thermalMgrClient.Dump(args) == "can't connect service");
    THERMAL_HILOGD(LABEL_TEST, "ThermalClientMockPeerTest001 end");
}

/**
 * @tc.name: ThermalMockProxyRemoteTest001
 * @tc.desc: proxy test
 * @tc.type: FUNC
 * @tc.require: issueI5YZQ2
 */
HWTEST_F(ThermalMockProxyRemoteTest, ThermalMockProxyRemoteTest001, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockProxyRemoteTest001 start");
    sptr<MockThermalRemoteObject> sptrRemoteObj = new MockThermalRemoteObject();
    EXPECT_FALSE(sptrRemoteObj == nullptr);
    std::shared_ptr<ThermalTempCallbackProxy> tempProxy = std::make_shared<ThermalTempCallbackProxy>(sptrRemoteObj);
    EXPECT_FALSE(tempProxy == nullptr);
    using TempCallbackMap = std::map<std::string, int32_t>;
    TempCallbackMap tempCbMap;
    EXPECT_FALSE(tempProxy->OnThermalTempChanged(tempCbMap));
    std::shared_ptr<ThermalActionCallbackProxy> actionProxy =
        std::make_shared<ThermalActionCallbackProxy>(sptrRemoteObj);
    EXPECT_FALSE(actionProxy == nullptr);
    using ActionCallbackMap = std::map<std::string, float>;
    ActionCallbackMap map;
    EXPECT_FALSE(actionProxy->OnThermalActionChanged(map));
    std::shared_ptr<ThermalLevelCallbackProxy> levalProxy = std::make_shared<ThermalLevelCallbackProxy>(sptrRemoteObj);
    EXPECT_FALSE(levalProxy == nullptr);
    ThermalLevel level = ThermalLevel::COOL;
    EXPECT_FALSE(levalProxy->GetThermalLevel(level));

    std::shared_ptr<ThermalSrvProxy> srvProxy = std::make_shared<ThermalSrvProxy>(sptrRemoteObj);
    EXPECT_FALSE(srvProxy == nullptr);
    SensorType type = SensorType::SOC;
    sptr<ThermalSrvSensorInfo> sensorInfo = new ThermalSrvSensorInfo();
    EXPECT_FALSE(srvProxy->GetThermalSrvSensorInfo(type, *sensorInfo));
    EXPECT_FALSE(srvProxy->GetThermalLevel(level));
    std::string sence;
    EXPECT_FALSE(srvProxy->SetScene(sence));
    std::vector<std::string> args;
    uint32_t argc = 0;
    EXPECT_TRUE(srvProxy->ShellDump(args, argc) == "remote error");
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockProxyRemoteTest001 end");
}
} // namespace
