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

#include "thermal_observer_test.h"

#include <memory>
#include <singleton.h>
#include <common_event_support.h>
#include "iremote_object.h"
#include "mock_thermal_remote_object.h"
#include "thermal_observer.h"
#include "thermal_service.h"
#include "thermal_sensor_info.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
using namespace OHOS::EventFwk;

namespace {
static sptr<ThermalService> g_service = nullptr;
}

void ThermalObserverTest::SetUpTestCase()
{
    g_service = DelayedSpSingleton<ThermalService>::GetInstance();
    g_service->OnStart();
}

void ThermalObserverTest::TearDownTestCase()
{
    g_service->OnStop();
}

void ThermalObserverTest::SetUp() {}

void ThermalObserverTest::TearDown() {}

void ThermalObserverTest::HandleScreenOnCompleted(const CommonEventData& data __attribute__((__unused__)))
{
}

bool ThermalObserverTest::ThermalTempTestCallback::OnThermalTempChanged(TempCallbackMap& tempCbMap)
{
    return true;
}

bool ThermalObserverTest::ThermalActionTestCallback::OnThermalActionChanged(ActionCallbackMap& actionCbMap)
{
    return true;
}

namespace {
/**
 * @tc.name: ThermalObserverTest001
 * @tc.desc: Thermal Common Event Receiver Function Test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalObserverTest, ThermalObserverTest001, TestSize.Level0)
{
    EventHandle handlerOn = std::bind(&ThermalObserverTest::HandleScreenOnCompleted, this, std::placeholders::_1);
    auto receiver = std::make_shared<ThermalCommonEventReceiver>();
    bool on = receiver->Start(CommonEventSupport::COMMON_EVENT_SCREEN_ON, handlerOn);
    EXPECT_TRUE(on);
    CommonEventData data;
    receiver->HandleEventChanged(data);
}

/**
 * @tc.name: ThermalObserverTest002
 * @tc.desc: Thermal Observer Function Test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalObserverTest, ThermalObserverTest002, TestSize.Level0)
{
    auto observer = std::make_shared<ThermalObserver>(g_service);
    std::vector<std::string> typeList;
    sptr<IThermalTempCallback> tempCallback;
    observer->Init();
    observer->SubscribeThermalTempCallback(typeList, tempCallback);
    sptr<IThermalTempCallback> tempCb = new ThermalTempTestCallback();
    observer->SubscribeThermalTempCallback(typeList, tempCb);
    observer->UnSubscribeThermalTempCallback(tempCb);
    sptr<IThermalActionCallback> actionCb;
    observer->SubscribeThermalActionCallback(typeList, "desc", actionCb);
    actionCb = new ThermalActionTestCallback();
    observer->SubscribeThermalActionCallback(typeList, "desc", actionCb);
    observer->UnSubscribeThermalActionCallback(actionCb);
    observer->FindSubscribeActionValue();
    IThermalActionCallback::ActionCallbackMap actionCbMap;
    observer->DecisionActionValue(typeList, actionCbMap);
    typeList.push_back("cpu_big");
    typeList.push_back("cpu_med");
    typeList.push_back("cpu_lit");
    observer->DecisionActionValue(typeList, actionCbMap);
    IThermalActionCallback::ActionCallbackMap map1;
    IThermalActionCallback::ActionCallbackMap map2;
    map1.insert(std::make_pair("lcd", 1.0));
    map2.insert(std::make_pair("lcd", 1.0));
    bool ret = observer->CompareActionCallbackMap(map1, map2);
    EXPECT_TRUE(ret);
    IThermalTempCallback::TempCallbackMap tempCbMap;
    observer->NotifySensorTempChanged(tempCbMap);
    TypeTempMap info;
    observer->OnReceivedSensorInfo(info);
    SensorType type = SensorType::SOC;
    ThermalSrvSensorInfo sensorInfo;
    observer->GetThermalSrvSensorInfo(type, sensorInfo);
    observer->GetTemp(type);
}

/**
 * @tc.name: ThermalObserverTest003
 * @tc.desc: Thermal Observer OnRemoteDied Test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalObserverTest, ThermalObserverTest003, TestSize.Level0)
{
    std::shared_ptr<IRemoteObject::DeathRecipient> deathRecipient =
        std::make_shared<ThermalObserver::SensorTempCallbackDeathRecipient>();
    wptr<IRemoteObject> remoteObj = nullptr;
    EXPECT_NE(deathRecipient, nullptr);
    deathRecipient->OnRemoteDied(remoteObj);

    sptr<IRemoteObject> sptrRemoteObj = new MockThermalRemoteObject();
    EXPECT_FALSE(sptrRemoteObj == nullptr);
    deathRecipient->OnRemoteDied(sptrRemoteObj);
}

/**
 * @tc.name: ThermalObserverTest004
 * @tc.desc: Thermal Observer OnRemoteDied Test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalObserverTest, ThermalObserverTest004, TestSize.Level0)
{
    std::shared_ptr<IRemoteObject::DeathRecipient> deathRecipient =
        std::make_shared<ThermalObserver::ActionCallbackDeathRecipient>();
    wptr<IRemoteObject> remoteObj = nullptr;
    EXPECT_NE(deathRecipient, nullptr);
    deathRecipient->OnRemoteDied(remoteObj);

    sptr<IRemoteObject> sptrRemoteObj = new MockThermalRemoteObject();
    EXPECT_FALSE(sptrRemoteObj == nullptr);
    deathRecipient->OnRemoteDied(sptrRemoteObj);
}

/**
 * @tc.name: ThermalObserverTest005
 * @tc.desc: Thermal Sensor Info Test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalObserverTest, ThermalObserverTest005, TestSize.Level0)
{
    auto info = std::make_shared<ThermalSensorInfo>();
    if (info != nullptr) {
        info->GetTypeTempMap();
        TypeTempMap type;
        info->SetTypeTempMap(type);
        info->GetTemp("soc");
        info->GetHistoryTemperature("soc");
        info->NotifyObserver();
    }
}
} // namespace
