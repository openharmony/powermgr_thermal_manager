/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "charger_state_collection.h"
#include "charge_delay_state_collection.h"
#include "cust_state_collection.h"
#include "startup_delay_state_collection.h"
#include "scene_state_collection.h"
#include "screen_state_collection.h"
#include "state_machine.h"

#ifdef BATTERY_MANAGER_ENABLE
#include "battery_info.h"
#include "battery_srv_client.h"
#endif

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
    g_service = ThermalService::GetInstance();
    g_service->InitSystemTestModules();
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

namespace {
/**
 * @tc.name: ThermalObserverTest001
 * @tc.desc: Thermal Common Event Receiver Function Test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalObserverTest, ThermalObserverTest001, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalObserverTest001 start");
    EventHandle handlerOn = std::bind(&ThermalObserverTest::HandleScreenOnCompleted, this, std::placeholders::_1);
    auto receiver = std::make_shared<ThermalCommonEventReceiver>();
    receiver->AddEvent(CommonEventSupport::COMMON_EVENT_SCREEN_ON, handlerOn);
    bool on = receiver->Register();
    EXPECT_TRUE(on);
    CommonEventData data;
    receiver->HandleEventChanged(data);
    THERMAL_HILOGI(LABEL_TEST, "ThermalObserverTest001 end");
}

/**
 * @tc.name: ThermalObserverTest002
 * @tc.desc: Thermal Observer Function Test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalObserverTest, ThermalObserverTest002, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalObserverTest002 start");
    auto observer = std::make_shared<ThermalObserver>();
    std::vector<std::string> typeList;
    sptr<IThermalTempCallback> tempCallback;
    observer->Init();
    observer->SubscribeThermalTempCallback(typeList, tempCallback);
    // implicit conversion from ThermalTempTestCallback* to IThermalTempCallback* before construct sptr
    // should be safe to cast back
    sptr<IThermalTempCallback> tempCb = new ThermalTempTestCallback();
    observer->SubscribeThermalTempCallback(typeList, tempCb);
    IThermalTempCallback::TempCallbackMap tempCbMap;
    EXPECT_CALL(*static_cast<ThermalTempTestCallback*>(tempCb.GetRefPtr()), OnThermalTempChanged(::testing::_));
    observer->NotifySensorTempChanged(tempCbMap);
    observer->UnSubscribeThermalTempCallback(tempCb);
    EXPECT_CALL(*static_cast<ThermalTempTestCallback*>(tempCb.GetRefPtr()), OnThermalTempChanged(::testing::_))
        .Times(0);
    observer->NotifySensorTempChanged(tempCbMap);
    sptr<IThermalActionCallback> actionCb;
    observer->SubscribeThermalActionCallback(typeList, "desc", actionCb);
    actionCb = new ThermalActionTestCallback();
    observer->SubscribeThermalActionCallback(typeList, "desc", actionCb);
    EXPECT_CALL(*static_cast<ThermalActionTestCallback*>(actionCb.GetRefPtr()), OnThermalActionChanged(::testing::_));
    observer->SetDecisionValue("", "");
    observer->FindSubscribeActionValue();
    observer->UnSubscribeThermalActionCallback(actionCb);
    IThermalActionCallback::ActionCallbackMap actionCbMap;
    observer->DecisionActionValue(typeList, actionCbMap);
    typeList.push_back("cpu_big");
    typeList.push_back("cpu_med");
    typeList.push_back("cpu_lit");
    observer->DecisionActionValue(typeList, actionCbMap);
    THERMAL_HILOGI(LABEL_TEST, "ThermalObserverTest002 end");
}

/**
 * @tc.name: ThermalObserverTest003
 * @tc.desc: Thermal Observer OnRemoteDied Test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalObserverTest, ThermalObserverTest003, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalObserverTest003 start");
    std::shared_ptr<IRemoteObject::DeathRecipient> deathRecipient =
        std::make_shared<ThermalObserver::SensorTempCallbackDeathRecipient>();
    wptr<IRemoteObject> remoteObj = nullptr;
    EXPECT_NE(deathRecipient, nullptr);
    deathRecipient->OnRemoteDied(remoteObj);

    sptr<IRemoteObject> sptrRemoteObj = new MockThermalRemoteObject();
    EXPECT_FALSE(sptrRemoteObj == nullptr);
    deathRecipient->OnRemoteDied(sptrRemoteObj);
    THERMAL_HILOGI(LABEL_TEST, "ThermalObserverTest003 end");
}

/**
 * @tc.name: ThermalObserverTest004
 * @tc.desc: Thermal Observer OnRemoteDied Test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalObserverTest, ThermalObserverTest004, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalObserverTest004 start");
    std::shared_ptr<IRemoteObject::DeathRecipient> deathRecipient =
        std::make_shared<ThermalObserver::ActionCallbackDeathRecipient>();
    wptr<IRemoteObject> remoteObj = nullptr;
    EXPECT_NE(deathRecipient, nullptr);
    deathRecipient->OnRemoteDied(remoteObj);

    sptr<IRemoteObject> sptrRemoteObj = new MockThermalRemoteObject();
    EXPECT_FALSE(sptrRemoteObj == nullptr);
    deathRecipient->OnRemoteDied(sptrRemoteObj);
    THERMAL_HILOGI(LABEL_TEST, "ThermalObserverTest004 end");
}

/**
 * @tc.name: ThermalObserverTest005
 * @tc.desc: Thermal Sensor Info Test
 * @tc.type: FUNC
 * @tc.require: issueI6KRS8
 */
HWTEST_F(ThermalObserverTest, ThermalObserverTest005, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalObserverTest005 start");
    auto info = std::make_shared<ThermalSensorInfo>();
    EXPECT_TRUE(info->GetTypeTempMap().empty());
    TypeTempMap type;
    info->SetTypeTempMap(type);
    info->GetTemp("soc");
    EXPECT_TRUE(info->GetHistoryTemperature("soc").empty());
    info->NotifyObserver();
    THERMAL_HILOGI(LABEL_TEST, "ThermalObserverTest005 end");
}

/**
 * @tc.name: ThermalObserverTest006
 * @tc.desc: Thermal Charger State Collection Test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalObserverTest, ThermalObserverTest006, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalObserverTest006 start");
    auto chargerState = std::make_shared<ChargerStateCollection>();
    chargerState->Init();
    string param = "charging";
    bool ret = chargerState->InitParam(param);
    EXPECT_TRUE(ret);
    chargerState->GetState();
    chargerState->RegisterEvent();
    CommonEventData data;
    chargerState->HandleChangerStatusCompleted(data);
    chargerState->SetState("");
    chargerState->DecideState("1");
    chargerState->HandleChargeIdleState();
    chargerState->HandleThermalLevelCompleted(data);
    THERMAL_HILOGI(LABEL_TEST, "ThermalObserverTest006 end");
}

/**
 * @tc.name: ThermalObserverTest007
 * @tc.desc: Thermal Scene State Collection Test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalObserverTest, ThermalObserverTest007, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalObserverTest007 start");
    auto sceneState = std::make_shared<SceneStateCollection>();
    sceneState->Init();
    string param = "game";
    bool ret = sceneState->InitParam(param);
    EXPECT_TRUE(ret);
    sceneState->SetState("");
    sceneState->DecideState("game");
    THERMAL_HILOGI(LABEL_TEST, "ThermalObserverTest007 end");
}

/**
 * @tc.name: ThermalObserverTest008
 * @tc.desc: Thermal Screen State Collection Test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalObserverTest, ThermalObserverTest008, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalObserverTest008 start");
    auto screenState = std::make_shared<ScreenStateCollection>();
    screenState->Init();
    string param = "on";
    bool ret = screenState->InitParam(param);
    EXPECT_TRUE(ret);
    screenState->GetState();
    screenState->RegisterEvent();
    CommonEventData data;
    screenState->HandleScreenOnCompleted(data);
    screenState->HandleScreenOffCompleted(data);
    screenState->SetState("");
    screenState->DecideState("0");
    THERMAL_HILOGI(LABEL_TEST, "ThermalObserverTest008 end");
}

/**
 * @tc.name: ThermalObserverTest009
 * @tc.desc: Thermal State Machine Test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalObserverTest, ThermalObserverTest009, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalObserverTest009 start");
    auto stateMachine = std::make_shared<StateMachine>();
    bool ret = stateMachine->Init();
    EXPECT_TRUE(ret);
    THERMAL_HILOGI(LABEL_TEST, "ThermalObserverTest009 end");
}

/**
 * @tc.name: ThermalObserverTest010
 * @tc.desc: Thermal Charger State Collection Test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalObserverTest, ThermalObserverTest010, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalObserverTest010 start");
#ifdef BATTERY_MANAGER_ENABLE
    auto chargerState = std::make_shared<ChargerStateCollection>();
    CommonEventData data;
    Want want;
    want.SetParam(BatteryInfo::COMMON_EVENT_KEY_CHARGE_STATE,
        static_cast<int32_t>(BatteryChargeState::CHARGE_STATE_DISABLE));
    data.SetWant(want);
    chargerState->HandleChangerStatusCompleted(data);
    want.SetParam(BatteryInfo::COMMON_EVENT_KEY_CHARGE_STATE,
        static_cast<int32_t>(BatteryChargeState::CHARGE_STATE_ENABLE));
    data.SetWant(want);
    chargerState->HandleChangerStatusCompleted(data);
    want.SetParam(BatteryInfo::COMMON_EVENT_KEY_CHARGE_STATE,
        static_cast<int32_t>(BatteryChargeState::CHARGE_STATE_FULL));
    data.SetWant(want);
    chargerState->HandleChangerStatusCompleted(data);
    want.SetParam(BatteryInfo::COMMON_EVENT_KEY_CHARGE_STATE,
        static_cast<int32_t>(BatteryChargeState::CHARGE_STATE_NONE));
    data.SetWant(want);
    chargerState->HandleChangerStatusCompleted(data);
    want.SetParam(BatteryInfo::COMMON_EVENT_KEY_CHARGE_STATE,
        static_cast<int32_t>(BatteryChargeState::CHARGE_STATE_BUTT));
    data.SetWant(want);
    chargerState->HandleChangerStatusCompleted(data);
    want.SetParam(BatteryInfo::COMMON_EVENT_KEY_CHARGE_STATE,
        static_cast<int32_t>(BatteryChargeState::CHARGE_STATE_BUTT));
    data.SetWant(want);
    chargerState->HandleChangerStatusCompleted(data);
    if (!g_service->GetSimulationXml()) {
        EXPECT_EQ(chargerState->GetState(), "");
    } else {
        EXPECT_EQ(chargerState->GetState(), ToString(ChargerStateCollection::BUTT));
    }
#endif
    THERMAL_HILOGI(LABEL_TEST, "ThermalObserverTest010 end");
}

/**
 * @tc.name: ThermalObserverTest011
 * @tc.desc: Thermal Charger State Collection Test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalObserverTest, ThermalObserverTest011, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalObserverTest011 start");
#ifdef BATTERY_MANAGER_ENABLE
    auto chargerDelayState = std::make_shared<ChargeDelayStateCollection>();
    auto chargerState =  ChargerStateCollection::GetInstance();
    chargerDelayState->Init();
    string delayTime = "60000";
    chargerDelayState->InitParam(delayTime);
    CommonEventData data;
    chargerDelayState->HandlerPowerDisconnected(data);
    EXPECT_EQ(chargerState->GetCharge(), false);
    EXPECT_EQ(chargerDelayState->GetState(), "1");
    chargerDelayState->HandlerPowerConnected(data);
    EXPECT_EQ(chargerDelayState->GetState(), "0");
    EXPECT_EQ(chargerState->GetCharge(), true);
    EXPECT_TRUE(chargerDelayState->DecideState("0"));
    chargerDelayState->SetState("");
    chargerDelayState->ResetState();

#endif
    THERMAL_HILOGI(LABEL_TEST, "ThermalObserverTest011 end");
}

/**
 * @tc.name: ThermalObserverTest012
 * @tc.desc: Thermal Charger State Collection Test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalObserverTest, ThermalObserverTest012, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalObserverTest012 start");
#ifdef BATTERY_MANAGER_ENABLE
    auto delayState = std::make_shared<StartupDelayStateCollection>();
    delayState->Init();
    EXPECT_EQ(delayState->GetState(), "1");
    string delayTime = "60000";
    delayState->InitParam(delayTime);
    delayState->ResetState();
    EXPECT_EQ(delayState->GetState(), "0");
    EXPECT_TRUE(delayState->DecideState("0"));
    delayState->SetState("");
#endif
    THERMAL_HILOGI(LABEL_TEST, "ThermalObserverTest012 end");
}

/**
 * @tc.name: ThermalObserverTest013
 * @tc.desc: Thermal Cust State Collection Test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalObserverTest, ThermalObserverTest013, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalObserverTest013 start");
    auto custState = std::make_shared<CustStateCollection>();
    custState->Init();
    string value = "67";
    custState->SetState(value);
    EXPECT_TRUE(custState->DecideState("1"));
    EXPECT_FALSE(custState->DecideState("4"));
    THERMAL_HILOGI(LABEL_TEST, "ThermalObserverTest013 end");
}
} // namespace
