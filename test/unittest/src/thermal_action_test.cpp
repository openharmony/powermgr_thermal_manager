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

#include "thermal_action_test.h"

#ifdef THERMAL_GTEST
#define private   public
#endif

#include <memory>
#include <singleton.h>
#include "action_airplane.h"
#include "action_application_process.h"
#include "action_charger.h"
#include "action_cpu_big.h"
#include "action_cpu_med.h"
#include "action_cpu_lit.h"
#include "action_cpu_isolate.h"
#include "action_display.h"
#include "action_gpu.h"
#include "action_node.h"
#include "action_shutdown.h"
#include "action_thermal_level.h"
#include "action_popup.h"
#include "action_volume.h"
#include "action_voltage.h"
#include "file_operation.h"
#include "iremote_object.h"
#include "mock_thermal_remote_object.h"
#include "thermal_service.h"
#include "thermal_timer.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
constexpr int32_t BUF_LEN = 32;
static sptr<ThermalService> g_service = nullptr;
std::shared_ptr<ActionApplicationProcess> g_actionApplicationProcess =
    std::make_shared<ActionApplicationProcess>("process_ctrl");
std::shared_ptr<ActionAirplane> g_actionAirplane = std::make_shared<ActionAirplane>("airplane");
std::shared_ptr<ActionCharger> g_actionCharger = std::make_shared<ActionCharger>("current_sc");
std::shared_ptr<ActionCpuBig> g_actionCpuBig = std::make_shared<ActionCpuBig>("cpu_big");
std::shared_ptr<ActionCpuMed> g_actionCpuMed = std::make_shared<ActionCpuMed>("cpu_med");
std::shared_ptr<ActionCpuLit> g_actionCpuLit = std::make_shared<ActionCpuLit>("cpu_lit");
std::shared_ptr<ActionCpuIsolate> g_actionCpuIsolate = std::make_shared<ActionCpuIsolate>("isolate");
std::shared_ptr<ActionDisplay> g_actionDisplay = std::make_shared<ActionDisplay>("lcd");
std::shared_ptr<ActionGpu> g_actionGpu = std::make_shared<ActionGpu>("gpu");
std::shared_ptr<ActionPopup> g_actionPopup = std::make_shared<ActionPopup>("popup");
std::shared_ptr<ActionShutdown> g_actionShutdown = std::make_shared<ActionShutdown>("shut_down");
std::shared_ptr<ActionThermalLevel> g_actionThermalLevel = std::make_shared<ActionThermalLevel>("thermallevel");
std::shared_ptr<ActionVolume> g_actionVolume = std::make_shared<ActionVolume>("volume");
std::shared_ptr<ActionVoltage> g_actionVoltage = std::make_shared<ActionVoltage>("voltage");
std::shared_ptr<ActionNode> g_actionNode = std::make_shared<ActionNode>("action_node");
}

void ThermalActionTest::SetUpTestCase()
{
    g_service = ThermalService::GetInstance();
    g_service->InitSystemTestModules();
    g_service->OnStart();
}

void ThermalActionTest::TearDownTestCase()
{
    g_service->OnStop();
}

void ThermalActionTest::SetUp() {}

void ThermalActionTest::TearDown() {}

namespace {
/**
 * @tc.name: ThermalActionTest001
 * @tc.desc: Action Volume Function Test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalActionTest, ThermalActionTest001, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest001 start");
    g_actionVolume->InitParams("volume");
    g_actionVolume->SetEnableEvent(false);
    g_actionVolume->AddActionValue("");
    g_actionVolume->AddActionValue("1.0");
    g_actionVolume->Execute();
    g_actionVolume->SetStrict(true);
    g_actionVolume->AddActionValue("2.0");
    g_actionVolume->GetActionValue();
    g_actionVolume->VolumeRequest(1.0);
    int32_t ret = g_actionVolume->VolumeExecution(1.0);
    EXPECT_TRUE(ret == ERR_OK);
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest001 end");
}

/**
 * @tc.name: ThermalActionTest002
 * @tc.desc: Action Voltage Function Test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalActionTest, ThermalActionTest002, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest002 start");
    g_actionVoltage->AddActionValue("");
    g_actionVoltage->AddActionValue("1.0");
    g_actionVoltage->Execute();
    g_actionVoltage->SetStrict(true);
    g_actionVoltage->AddActionValue("2.0");
    g_actionVoltage->GetActionValue();
    g_actionVoltage->SetVoltage(123456);
    g_actionVoltage->ExecuteVoltageLimit();
    int32_t ret = g_actionVoltage->WriteMockNode(123456);
    EXPECT_FALSE(ret == ERR_OK);
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest002 end");
}

/**
 * @tc.name: ThermalActionTest003
 * @tc.desc: Action Thermal Level Function Test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalActionTest, ThermalActionTest003, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest003 start");
    g_actionThermalLevel->AddActionValue("");
    g_actionThermalLevel->AddActionValue("1.0");
    g_actionThermalLevel->Execute();
    g_actionThermalLevel->GetThermalLevel();
    g_actionThermalLevel->LevelRequest(1);
    g_actionThermalLevel->LevelRequest(9);
    g_actionThermalLevel->LevelRequest(-1);
    g_actionThermalLevel->SetStrict(true);
    g_actionThermalLevel->AddActionValue("2.0");
    g_actionThermalLevel->GetActionValue();
    g_actionThermalLevel->SubscribeThermalLevelCallback(nullptr);
    g_actionThermalLevel->UnSubscribeThermalLevelCallback(nullptr);
    std::shared_ptr<IRemoteObject::DeathRecipient> deathRecipient =
        std::make_shared<ActionThermalLevel::ThermalLevelCallbackDeathRecipient>();
    wptr<IRemoteObject> remoteObj = nullptr;
    EXPECT_NE(deathRecipient, nullptr);
    deathRecipient->OnRemoteDied(remoteObj);

    sptr<IRemoteObject> sptrRemoteObj = new MockThermalRemoteObject();
    EXPECT_FALSE(sptrRemoteObj == nullptr);
    deathRecipient->OnRemoteDied(sptrRemoteObj);

    g_actionThermalLevel->NotifyThermalLevelChanged(1);
    bool ret = g_actionThermalLevel->
        PublishLevelChangedEvents(ThermalCommonEventCode::CODE_THERMAL_LEVEL_CHANGED, 1);
    EXPECT_TRUE(ret);
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest003 end");
}

/**
 * @tc.name: ThermalActionTest004
 * @tc.desc: Action Shutdown Function Test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalActionTest, ThermalActionTest004, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest004 start");
    g_actionShutdown->AddActionValue("");
    g_actionShutdown->AddActionValue("1.0");
    g_actionShutdown->Execute();
    g_actionShutdown->SetStrict(true);
    g_actionShutdown->AddActionValue("2.0");
    g_actionShutdown->GetActionValue();
    int32_t ret = g_actionShutdown->ShutdownRequest(false);
    EXPECT_TRUE(ret == ERR_OK);
    g_actionShutdown->ShutdownExecution(false);
    g_actionShutdown->ShutdownExecution(false);
    ret = g_actionShutdown->DelayShutdown(false, 0, 0);
    EXPECT_TRUE(ret == ERR_OK);
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest004 end");
}

/**
 * @tc.name: ThermalActionTest005
 * @tc.desc: Action Display Function Test
 * @tc.type: FUNC
 * @tc.require: issueI6KRS8
 */
HWTEST_F(ThermalActionTest, ThermalActionTest005, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest005 start");
    g_actionDisplay->AddActionValue("");
    g_actionDisplay->AddActionValue("1.0");
    g_actionDisplay->Execute();
    g_actionDisplay->SetStrict(true);
    g_actionDisplay->AddActionValue("2.0");
    g_actionDisplay->GetActionValue();
    EXPECT_FALSE(g_actionDisplay->valueList_.empty());
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest005 end");
}

/**
 * @tc.name: ThermalActionTest006
 * @tc.desc: Action Charger Function Test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalActionTest, ThermalActionTest006, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest006 start");
    g_actionCharger->AddActionValue("");
    g_actionCharger->AddActionValue("1.0");
    g_actionCharger->Execute();
    g_actionCharger->ChargerRequest(0);
    g_actionCharger->ExecuteCurrentLimit();
    int32_t ret = g_actionCharger->WriteSimValue(0);
    EXPECT_TRUE(ret);
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest006 end");
}

/**
 * @tc.name: ThermalActionTest007
 * @tc.desc: Action Application Process Test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalActionTest, ThermalActionTest007, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest007 start");
    g_actionApplicationProcess->InitParams("");
    g_actionApplicationProcess->AddActionValue("");
    g_actionApplicationProcess->AddActionValue("1");
    g_actionApplicationProcess->Execute();
    g_actionApplicationProcess->KillApplicationAction("");
    g_actionApplicationProcess->KillProcess(0);
    g_actionApplicationProcess->GetAppProcessInfoByName("");
    g_actionApplicationProcess->GetAllRunnningAppProcess();
    g_actionApplicationProcess->KillBgAppProcess();
    g_actionApplicationProcess->KillAllAppProcess();
    g_actionApplicationProcess->ProcessAppActionRequest(0);
    g_actionApplicationProcess->ProcessAppActionRequest(1);
    g_actionApplicationProcess->ProcessAppActionRequest(2);
    g_actionApplicationProcess->ProcessAppActionRequest(3);
    g_actionApplicationProcess->ProcessAppActionExecution(0);
    EXPECT_TRUE(g_actionApplicationProcess->valueList_.empty());
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest007 end");
}

/**
 * @tc.name: ThermalActionTest008
 * @tc.desc: Action CPU Big Test
 * @tc.type: FUNC
 * @tc.require: issueI6KRS8
 */
HWTEST_F(ThermalActionTest, ThermalActionTest008, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest008 start");
    g_actionCpuBig->AddActionValue("");
    g_actionCpuBig->AddActionValue("1.0");
    g_actionCpuBig->Execute();
    EXPECT_TRUE(g_actionCpuBig->valueList_.empty());
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest008 end");
}

/**
 * @tc.name: ThermalActionTest009
 * @tc.desc: Action CPU Med Test
 * @tc.type: FUNC
 * @tc.require: issueI6KRS8
 */
HWTEST_F(ThermalActionTest, ThermalActionTest009, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest009 start");
    g_actionCpuMed->AddActionValue("");
    g_actionCpuMed->AddActionValue("1.0");
    g_actionCpuMed->Execute();
    EXPECT_TRUE(g_actionCpuMed->valueList_.empty());
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest009 end");
}

/**
 * @tc.name: ThermalActionTest010
 * @tc.desc: Action CPU Lit Test
 * @tc.type: FUNC
 * @tc.require: issueI6KRS8
 */
HWTEST_F(ThermalActionTest, ThermalActionTest010, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest010 start");
    g_actionCpuLit->AddActionValue("");
    g_actionCpuLit->AddActionValue("1.0");
    g_actionCpuLit->Execute();
    EXPECT_TRUE(g_actionCpuLit->valueList_.empty());
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest010 end");
}

/**
 * @tc.name: ThermalActionTest011
 * @tc.desc: Action GPU Test
 * @tc.type: FUNC
 * @tc.require: issueI6KRS8
 */
HWTEST_F(ThermalActionTest, ThermalActionTest011, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest011 start");
    g_actionGpu->AddActionValue("");
    g_actionGpu->AddActionValue("1.0");
    g_actionGpu->Execute();
    EXPECT_TRUE(g_actionGpu->valueList_.empty());
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest011 end");
}

/**
 * @tc.name: ThermalActionTest012
 * @tc.desc: Action Isolate CPU Test
 * @tc.type: FUNC
  */
HWTEST_F(ThermalActionTest, ThermalActionTest012, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest012 start");
    g_actionCpuIsolate->AddActionValue("");
    g_actionCpuIsolate->AddActionValue("1.0");
    g_actionCpuIsolate->Execute();
    g_actionCpuIsolate->SetStrict(true);
    g_actionCpuIsolate->AddActionValue("2.0");
    g_actionCpuIsolate->GetActionValue();
    EXPECT_FALSE(g_actionCpuIsolate->valueList_.empty());
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest012 end");
}

/**
 * @tc.name: ThermalActionTest013
 * @tc.desc: Action node Test
 * @tc.type: FUNC
  */
HWTEST_F(ThermalActionTest, ThermalActionTest013, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest013 start");
    std::string input = "1";
    g_actionNode->InitParams("/data/service/el0/thermal/config/lcd");
    g_actionNode->AddActionValue(input);
    g_actionNode->Execute();
    char buf[BUF_LEN];
    FileOperation::ReadFile("/data/service/el0/thermal/config/lcd", buf, BUF_LEN);
    std::string ret = buf;
    EXPECT_EQ(input, ret);
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest013 end");
}

/**
 * @tc.name: ThermalActionTest014
 * @tc.desc: Action Airplane Function Test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalActionTest, ThermalActionTest014, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest014 start");
    g_actionAirplane->InitParams("airplane");
    g_actionAirplane->AddActionValue("");
    g_actionAirplane->AddActionValue("0");
    g_actionAirplane->AddActionValue("air");
    g_actionAirplane->Execute();
    EXPECT_TRUE(g_actionAirplane->valueList_.empty());
    std::string input = "1";
    g_actionAirplane->AddActionValue(input);
    g_actionCpuIsolate->SetStrict(true);
    uint32_t value = g_actionAirplane->GetActionValue();
    int32_t ret = g_actionAirplane->AirplaneRequest(value);
    g_actionAirplane->AirplaneRequest(0);
    g_actionAirplane->AirplaneRequest(1);
    EXPECT_TRUE(ret == ERR_OK);
    ret = g_actionAirplane->AirplaneExecution(value);
    EXPECT_TRUE(ret == ERR_OK);
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest014 end");
}

/**
 * @tc.name: ThermalActionTest015
 * @tc.desc: Action Timer Test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalActionTest, ThermalActionTest015, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest015 start");
    auto timerInfo = std::make_shared<ThermalTimerInfo>();
    ASSERT_NE(timerInfo, nullptr);
    OHOS::PowerMgr::ThermalTimerInfo::TimerInfoCallback callback;
    timerInfo->SetCallbackInfo(callback);
    timerInfo->OnTrigger();
    auto timer = std::make_shared<ThermalTimer>();
    ASSERT_NE(timer, nullptr);
    uint64_t triggerTime = 1;
    std::shared_ptr<ThermalTimerInfo> info;
    uint64_t timerId = timer->CreateTimer(info);
    EXPECT_EQ(timerId, 0);
    timer->StartTimer(timerId, triggerTime);
    timer->DestroyTimer(timerId);
    timer->StopTimer(timerId);
    auto timerUtils = std::make_shared<ThermalTimerUtils>();
    ASSERT_NE(timerUtils, nullptr);
    timerUtils->Stop();
    int delay = 1;
    OHOS::PowerMgr::ThermalTimerUtils::NotifyTask task;
    timerUtils->Start(delay, task);
    timerUtils->Start(delay, task);
    timerUtils->Stop();
    timerUtils->Stop();
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest015 end");
}

/**
 * @tc.name: ThermalActionTest016
 * @tc.desc: Action Popup Test
 * @tc.type: FUNC
 */
HWTEST_F(ThermalActionTest, ThermalActionTest016, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest016 start");
    g_actionPopup->AddActionValue("pop");
    g_actionPopup->Execute();
    EXPECT_TRUE(g_actionPopup->valueList_.empty());
    THERMAL_HILOGD(LABEL_TEST, "ThermalActionTest016 end");
}
} // namespace
