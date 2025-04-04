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

#include "thermal_client_test.h"

#include "hdf_base.h"
#include "hdi_service_status_listener.h"
#include "mock_thermal_remote_object.h"
#include "thermal_action_callback_proxy.h"
#include "thermal_callback.h"
#include "thermal_level_callback_proxy.h"
#include "thermal_log.h"
#include "thermal_mgr_client.h"
#include "thermal_srv_sensor_info.h"
#include "thermal_temp_callback_proxy.h"
#include "thermal_mgr_listener.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
const int32_t INVAILID_VALUE = -1;
namespace {
int32_t MockEventCb(const HdfThermalCallbackInfo& event)
{
    return 0;
}

bool MockStatusCb(const OHOS::HDI::ServiceManager::V1_0::ServiceStatus&)
{
    return true;
}

/**
 * @tc.name: ThermalClientTest001
 * @tc.desc: register thermal event
 * @tc.type: FUNC
 * @tc.require: issueI5YZQ2
 */
HWTEST_F(ThermalClientTest, ThermalClientTest001, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalClientTest001 function start!");
    ThermalCallback* thermalCb = new ThermalCallback();
    HdfThermalCallbackInfo* info = new HdfThermalCallbackInfo();
    EXPECT_TRUE(thermalCb->OnThermalDataEvent(*info) == HDF_FAILURE);
    using ThermalEventCallback = std::function<int32_t(const HdfThermalCallbackInfo& event)>;
    ThermalEventCallback cb = MockEventCb;
    EXPECT_TRUE(thermalCb->RegisterThermalEvent(cb) == HDF_SUCCESS);
    EXPECT_TRUE(thermalCb->OnThermalDataEvent(*info) != HDF_FAILURE);
    delete thermalCb;
    thermalCb = nullptr;
    delete info;
    info = nullptr;
    THERMAL_HILOGI(LABEL_TEST, "ThermalClientTest001 function end!");
}

/**
 * @tc.name: ThermalClientTest002
 * @tc.desc: listener on receive
 * @tc.type: FUNC
 * @tc.require: issueI5YZQ2
 */
HWTEST_F(ThermalClientTest, ThermalClientTest002, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalClientTest002 function start!");
    using StatusCallback = std::function<void(const OHOS::HDI::ServiceManager::V1_0::ServiceStatus&)>;
    StatusCallback cb = MockStatusCb;
    HdiServiceStatusListener* listener = new HdiServiceStatusListener(cb);
    EXPECT_FALSE(listener == nullptr);
    OHOS::HDI::ServiceManager::V1_0::ServiceStatus status = {"a", 0, 0, "a"};
    listener->OnReceive(status);
    delete listener;
    listener = nullptr;
    THERMAL_HILOGI(LABEL_TEST, "ThermalClientTest002 function end!");
}

/**
 * @tc.name: ThermalClientTest003
 * @tc.desc: subscribe callback and unsubscribe callback
 * @tc.type: FUNC
 * @tc.require: issueI5YZQ2
 */
HWTEST_F(ThermalClientTest, ThermalClientTest003, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalClientTest003 function start!");
    auto& client = ThermalMgrClient::GetInstance();
    std::vector<std::string> typeList;
    sptr<IThermalTempCallback> tempCallback = nullptr;
    EXPECT_FALSE(client.SubscribeThermalTempCallback(typeList, tempCallback));
    EXPECT_FALSE(client.UnSubscribeThermalTempCallback(tempCallback));
    sptr<MockThermalRemoteObject> sptrRemoteObj = new MockThermalRemoteObject();
    EXPECT_FALSE(sptrRemoteObj == nullptr);
    tempCallback = new ThermalTempCallbackProxy(sptrRemoteObj);
    EXPECT_FALSE(tempCallback == nullptr);
    EXPECT_TRUE(client.SubscribeThermalTempCallback(typeList, tempCallback));
    EXPECT_TRUE(client.UnSubscribeThermalTempCallback(tempCallback));
    sptr<IThermalLevelCallback> levelCallback = nullptr;
    EXPECT_FALSE(client.SubscribeThermalLevelCallback(levelCallback));
    EXPECT_FALSE(client.UnSubscribeThermalLevelCallback(levelCallback));
    levelCallback = new ThermalLevelCallbackProxy(sptrRemoteObj);
    EXPECT_FALSE(levelCallback == nullptr);
    EXPECT_TRUE(client.SubscribeThermalLevelCallback(levelCallback));
    EXPECT_TRUE(client.UnSubscribeThermalLevelCallback(levelCallback));
    sptr<IThermalActionCallback> actionCallback = nullptr;
    std::string actionList;
    EXPECT_FALSE(client.SubscribeThermalActionCallback(typeList, actionList, actionCallback));
    EXPECT_FALSE(client.UnSubscribeThermalActionCallback(actionCallback));
    actionCallback = new ThermalActionCallbackProxy(sptrRemoteObj);
    EXPECT_FALSE(actionCallback == nullptr);
    EXPECT_TRUE(client.SubscribeThermalActionCallback(typeList, actionList, actionCallback));
    EXPECT_TRUE(client.UnSubscribeThermalActionCallback(actionCallback));
    THERMAL_HILOGI(LABEL_TEST, "ThermalClientTest003 function end!");
}

/**
 * @tc.name: ThermalClientTest004
 * @tc.desc: ThermalSrvSensorInfo Marshalling and Unmarshalling test
 * @tc.type: FUNC
 * @tc.require: issueI5YZQ2
 */
HWTEST_F(ThermalClientTest, ThermalClientTest004, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalClientTest004 function start!");
    sptr<ThermalSrvSensorInfo> info = new ThermalSrvSensorInfo();
    MessageParcel parcel;
    info->Unmarshalling(parcel);
    EXPECT_TRUE(info->Marshalling(parcel));
    THERMAL_HILOGI(LABEL_TEST, "ThermalClientTest004 function end!");
}

/**
 * @tc.name: ThermalClientTest005
 * @tc.desc: thermalListener additional test
 * @tc.type: FUNC
 * @tc.require: issueI5YZQ2
 */
HWTEST_F(ThermalClientTest, ThermalClientTest005, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalClientTest005 function start!");
    std::shared_ptr<ThermalMgrListener> thermalListener = std::make_shared<ThermalMgrListener>();
    ASSERT_NE(thermalListener, nullptr);
    int32_t ret = thermalListener->SubscribeLevelEvent(nullptr);
    EXPECT_NE(ret, ERR_OK);
    ret = thermalListener->UnSubscribeLevelEvent();
    EXPECT_NE(ret, ERR_OK);
    ThermalLevel level = thermalListener->GetThermalLevel();
    EXPECT_NE(static_cast<int32_t>(level), INVAILID_VALUE);
    sptr<IThermalLevelCallback> callback = new ThermalMgrListener::ThermalLevelCallback(nullptr);
    bool result = callback->OnThermalLevelChanged(level);
    EXPECT_EQ(result, false);
    THERMAL_HILOGI(LABEL_TEST, "ThermalClientTest005 function end!");
}
} // namespace
