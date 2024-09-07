/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "thermal_action_report_test.h"

#include "securec.h"
#include <fcntl.h>
#include <string>
#include <unistd.h>
#ifdef BATTERY_MANAGER_ENABLE
#include "battery_srv_client.h"
#endif
#ifdef BATTERY_STATS_ENABLE
#include "battery_stats_client.h"
#endif
#include "power_mgr_client.h"
#include "thermal_config_file_parser.h"
#include "thermal_mgr_client.h"

#define private   public
#define protected public
#include "thermal_service.h"
#include "thermal_srv_config_parser.h"
#include "v1_1/ithermal_interface.h"
#include "v1_1/thermal_types.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
using namespace OHOS::HDI::Thermal::V1_1;

namespace {
static std::shared_ptr<ThermalConfigFileParser> g_configParser = nullptr;
static std::vector<std::string> g_dumpArgs;
static std::string g_sceneState;
static const std::string POLICY_CFG_NAME = "base_safe";
constexpr int32_t THERMAL_RATIO_BEGIN = 0;
constexpr int32_t THERMAL_RATIO_LENGTH = 4;
const std::string SYSTEM_THERMAL_SERVICE_CONFIG_PATH = "/system/etc/thermal_config/thermal_service_config.xml";
sptr<ThermalService> g_service = nullptr;
} // namespace

void ThermalActionReportTest::ParserThermalSrvConfigFile()
{
    if (g_configParser == nullptr) {
        g_configParser = std::make_shared<ThermalConfigFileParser>();
        if (!g_configParser->Init()) {
            THERMAL_HILOGE(LABEL_TEST, "Thermal service config file parser initialization failed");
        }
    }
}

int32_t ThermalActionReportTest::SetCondition(int32_t value, const std::string& scene)
{
    THERMAL_HILOGD(LABEL_TEST, "battery = %{public}d, scene = %{public}s", value, scene.c_str());
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = value;
    event.info.push_back(info1);
    g_sceneState = scene;
    g_service->SetScene(g_sceneState);
    return g_service->HandleThermalCallbackEvent(event);
}

int32_t ThermalActionReportTest::GetThermalLevel(int32_t expectValue)
{
    ThermalLevel level;
    g_service->GetThermalLevel(level);
    int32_t value = static_cast<int32_t>(level);
    THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
    EXPECT_EQ(value, expectValue) << "Thermal action policy failed";
    return value;
}

std::string ThermalActionReportTest::GetActionValue(const std::string& actionName, uint32_t level)
{
    THERMAL_HILOGD(LABEL_TEST, "action name = %{public}s, level = %{public}d", actionName.c_str(), level);
    std::string value = "0";
    std::vector<PolicyAction> vAction;
    if (!g_configParser->GetActionPolicy(POLICY_CFG_NAME, level, vAction)) {
        THERMAL_HILOGD(LABEL_TEST, "Get policy failed name = %{public}s, return", POLICY_CFG_NAME.c_str());
        return value;
    }
    return ActionDecision(actionName, vAction);
}

std::string ThermalActionReportTest::ActionDecision(const std::string& actionName, std::vector<PolicyAction>& vAction)
{
    THERMAL_HILOGD(LABEL_TEST, "action name = %{public}s", actionName.c_str());
    if (actionName == LCD_ACTION_NAME) {
        return LcdValueDecision(actionName, vAction);
    } else {
        return ActionValueDecision(actionName, vAction);
    }
}

std::string ThermalActionReportTest::ActionValueDecision(
    const std::string& actionName, std::vector<PolicyAction>& vAction)
{
    THERMAL_HILOGD(LABEL_TEST, "action name = %{public}s", actionName.c_str());
    int32_t value = -1;
    std::vector<uint32_t> valueList;
    for (auto actionIter : vAction) {
        if (actionIter.actionName == actionName) {
            if (actionIter.isProp) {
                if (StateDecision(actionIter.actionPropMap)) {
                    valueList.push_back(stoi(actionIter.actionValue));
                }
            } else {
                valueList.push_back(stoi(actionIter.actionValue));
            }
        }
    }

    bool strict = g_configParser->GetActionStrict(actionName);
    if (valueList.empty()) {
        value = 0;
    } else {
        if (strict) {
            value = *max_element(valueList.begin(), valueList.end());
        } else {
            value = *min_element(valueList.begin(), valueList.end());
        }
    }
    std::string strValue = to_string(value);
    return strValue;
}

std::string ThermalActionReportTest::LcdValueDecision(const std::string& actionName, std::vector<PolicyAction>& vAction)
{
    THERMAL_HILOGD(LABEL_TEST, "action name = %{public}s", actionName.c_str());
    float value = -1.0;
    std::vector<float> valueList;
    std::map<std::string, std::string> sceneMap;
    for (auto actionIter : vAction) {
        if (actionIter.actionName == actionName) {
            if (actionIter.isProp) {
                if (StateDecision(actionIter.actionPropMap)) {
                    valueList.push_back(stof(actionIter.actionValue));
                    sceneMap.emplace(std::pair(actionIter.actionPropMap.begin()->second, actionIter.actionValue));
                }
            } else {
                valueList.push_back(stof(actionIter.actionValue));
            }
        }
    }

    for (auto sceneIter : sceneMap) {
        if (g_sceneState == sceneIter.first) {
            return sceneIter.second.c_str();
        }
    }

    bool strict = g_configParser->GetActionStrict(actionName);
    if (valueList.empty()) {
        value = 0;
    } else {
        if (strict) {
            value = *max_element(valueList.begin(), valueList.end());
        } else {
            value = *min_element(valueList.begin(), valueList.end());
        }
    }
    std::string strValue = to_string(value).substr(THERMAL_RATIO_BEGIN, THERMAL_RATIO_LENGTH);
    return strValue;
}

bool ThermalActionReportTest::StateDecision(std::map<std::string, std::string>& actionPropMap)
{
    bool ret = true;
    std::map<std::string, std::string> stateMap;
    GetStateMap(stateMap);
    for (auto prop : actionPropMap) {
        auto stateIter = stateMap.find(prop.first);
        THERMAL_HILOGD(LABEL_TEST, "state = %{public}s, value = %{public}s", prop.first.c_str(), prop.second.c_str());
        if (stateIter != stateMap.end()) {
            THERMAL_HILOGD(LABEL_TEST, "state iter = %{public}s, iter value = %{public}s", stateIter->first.c_str(),
                stateIter->second.c_str());
            if (stateIter->second.compare(prop.second) == 0) {
                continue;
            } else {
                ret = false;
                break;
            }
        }
    }
    return ret;
}

std::string ThermalActionReportTest::GetScreenState()
{
    std::string state = "0";
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    if (powerMgrClient.IsScreenOn()) {
        state = "1";
    }
    return state;
}

std::string ThermalActionReportTest::GetChargeState()
{
    std::string state = "";
#ifdef BATTERY_MANAGER_ENABLE
    auto& batterySrvClient = BatterySrvClient::GetInstance();
    BatteryChargeState chargeState = batterySrvClient.GetChargingStatus();
    if (chargeState == BatteryChargeState::CHARGE_STATE_ENABLE) {
        state = "1";
    } else if (chargeState == BatteryChargeState::CHARGE_STATE_NONE) {
        state = "0";
    }
#endif
    return state;
}

void ThermalActionReportTest::GetStateMap(std::map<std::string, std::string>& stateMap)
{
    std::vector<StateItem> stateItem = g_configParser->GetStateItem();
    for (auto stateIter : stateItem) {
        std::string state = "";
        if (stateIter.name == "scene") {
            state = g_sceneState;
        } else if (stateIter.name == "screen") {
            state = GetScreenState();
        } else if (stateIter.name == "charge") {
            state = GetChargeState();
        }
        stateMap.emplace(std::pair(stateIter.name, state));
    }
    for (auto iter : stateMap) {
        THERMAL_HILOGD(
            LABEL_TEST, "stateMap name = %{public}s, value = %{public}s", iter.first.c_str(), iter.second.c_str());
    }
}

void ThermalActionReportTest::ThermalActionTriggered(
    const std::string& actionName, int32_t level, const std::string& dumpInfo, bool isReversed)
{
    bool enableEvent = g_configParser->GetActionEnableEvent(actionName);
    THERMAL_HILOGD(LABEL_TEST, "action name = %{public}s, event flag = %{public}d", actionName.c_str(), enableEvent);
    if (!enableEvent) {
        GTEST_LOG_(INFO) << __func__ << " action name: " << actionName << " enalbe event flag is false, return";
        return;
    }
    std::string value = GetActionValue(actionName, level);
    std::string expectedDumpInfo;
    std::string valueLabel = " Value = ";
    if (actionName == LCD_ACTION_NAME) {
        valueLabel = " Ratio = ";
    }
    expectedDumpInfo.append("Additional debug info: ")
        .append("Event name = ACTION_TRIGGERED")
        .append(" Action name = ")
        .append(actionName)
        .append(valueLabel)
        .append(value);
    THERMAL_HILOGD(LABEL_TEST, "value: %{public}s", value.c_str());
    GTEST_LOG_(INFO) << __func__ << " action name: " << actionName << " expected debug info: " << expectedDumpInfo;
    auto index = dumpInfo.find(expectedDumpInfo);
    if (!isReversed) {
        EXPECT_TRUE(index != string::npos) << " Thermal action fail due to not found related debug info."
                                           << " action name = " << actionName;
    } else {
        EXPECT_TRUE(index == string::npos) << " Thermal action fail due to not found related debug info."
                                           << " action name = " << actionName;
    }
}

void ThermalActionReportTest::SetUpTestCase()
{
    ParserThermalSrvConfigFile();
    g_dumpArgs.push_back("-batterystats");
    g_service = ThermalService::GetInstance();
    g_service->InitSystemTestModules();
    g_service->OnStart();
    g_service->InitStateMachine();
    g_service->InitActionManager();
}

void ThermalActionReportTest::TearDownTestCase()
{
    g_service->OnStop();
}

void ThermalActionReportTest::TearDown()
{
    g_service->SetScene("");
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info1;
    info1.type = "battery";
    info1.temp = 0;
    event.info.push_back(info1);
    g_service->HandleThermalCallbackEvent(event);
}

#ifdef BATTERY_STATS_ENABLE
namespace {
/**
 * @tc.name: ThermalActionReportTest001
 * @tc.desc: test dump info when thermal action is triggered
 * @tc.type: FEATURE
 * @tc.cond: Set battery temp = 40100, scence = cam
 * @tc.result battery stats dump info
 */
HWTEST_F(ThermalActionReportTest, ThermalActionReportTest001, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.001 start");
    auto& statsClient = BatteryStatsClient::GetInstance();
    statsClient.Reset();

    int32_t ret = -1;
    int32_t batteryTemp = 40100;
    std::string sceneState = "cam";
    int32_t expectLevel = 1;
    ret = SetCondition(batteryTemp, sceneState);
    EXPECT_EQ(ret, ERR_OK) << " Thermal action fail due to set condition error";
    int32_t level = ThermalActionReportTest::GetThermalLevel(expectLevel);
    // sleep 100ms for hisys_event handle
    usleep(100000);
    std::string actualDumpInfo = statsClient.Dump(g_dumpArgs);
    GTEST_LOG_(INFO) << __func__ << ": actual dump info: " << actualDumpInfo;
    ThermalActionReportTest::ThermalActionTriggered(CPU_BIG_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_MED_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_LIT_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(GPU_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(LCD_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(PROCESS_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(THERMAL_LEVEL_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CURRENT_SC_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CURRENT_BUCK_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(VOLATAGE_SC_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(VOLATAGE_BUCK_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_BOOST_ACTION_NAME, level, actualDumpInfo);
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.001 end");
}

/**
 * @tc.name: ThermalActionReportTest002
 * @tc.desc: test dump info when thermal action is triggered
 * @tc.type: FEATURE
 * @tc.cond: Set battery temp = 40100, scence = call
 * @tc.result battery stats dump info
 */
HWTEST_F(ThermalActionReportTest, ThermalActionReportTest002, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.002 start");
    auto& statsClient = BatteryStatsClient::GetInstance();
    statsClient.Reset();

    int32_t ret = -1;
    int32_t batteryTemp = 40100;
    std::string sceneState = "call";
    int32_t expectLevel = 1;
    ret = SetCondition(batteryTemp, sceneState);
    EXPECT_EQ(ret, ERR_OK) << " Thermal action fail due to set condition error";
    int32_t level = ThermalActionReportTest::GetThermalLevel(expectLevel);
    // sleep 100ms for hisys_event handle
    usleep(100000);
    std::string actualDumpInfo = statsClient.Dump(g_dumpArgs);
    GTEST_LOG_(INFO) << __func__ << ": actual dump info: " << actualDumpInfo;
    ThermalActionReportTest::ThermalActionTriggered(CPU_BIG_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_MED_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_LIT_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(GPU_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(LCD_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(PROCESS_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(THERMAL_LEVEL_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CURRENT_SC_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CURRENT_BUCK_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(VOLATAGE_SC_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(VOLATAGE_BUCK_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_BOOST_ACTION_NAME, level, actualDumpInfo);
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.002 end");
}

/**
 * @tc.name: ThermalActionReportTest003
 * @tc.desc: test dump info when thermal action is triggered
 * @tc.type: FEATURE
 * @tc.cond: Set battery temp = 40100, scence = game
 * @tc.result battery stats dump info
 */
HWTEST_F(ThermalActionReportTest, ThermalActionReportTest003, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.003 start");
    auto& statsClient = BatteryStatsClient::GetInstance();
    statsClient.Reset();

    int32_t ret = -1;
    int32_t batteryTemp = 40100;
    std::string sceneState = "game";
    int32_t expectLevel = 1;
    ret = SetCondition(batteryTemp, sceneState);
    EXPECT_EQ(ret, ERR_OK) << " Thermal action fail due to set condition error";

    int32_t level = ThermalActionReportTest::GetThermalLevel(expectLevel);
    // sleep 100ms for hisys_event handle
    usleep(100000);
    std::string actualDumpInfo = statsClient.Dump(g_dumpArgs);
    GTEST_LOG_(INFO) << __func__ << ": actual dump info: " << actualDumpInfo;
    ThermalActionReportTest::ThermalActionTriggered(CPU_BIG_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_MED_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_LIT_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(GPU_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(LCD_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(PROCESS_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(THERMAL_LEVEL_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CURRENT_SC_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CURRENT_BUCK_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(VOLATAGE_SC_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(VOLATAGE_BUCK_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_BOOST_ACTION_NAME, level, actualDumpInfo);
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.003 end");
}

/**
 * @tc.name: ThermalActionReportTest004
 * @tc.desc: test dump info when thermal action is triggered
 * @tc.type: FEATURE
 * @tc.cond: Set battery temp = 40100, scence = ""
 * @tc.result battery stats dump info
 */
HWTEST_F(ThermalActionReportTest, ThermalActionReportTest004, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.004 start");
    auto& statsClient = BatteryStatsClient::GetInstance();
    statsClient.Reset();

    int32_t ret = -1;
    int32_t batteryTemp = 40100;
    std::string sceneState = "";
    int32_t expectLevel = 1;
    ret = SetCondition(batteryTemp, sceneState);
    EXPECT_EQ(ret, ERR_OK) << " Thermal action fail due to set condition error";

    int32_t level = ThermalActionReportTest::GetThermalLevel(expectLevel);
    // sleep 100ms for hisys_event handle
    usleep(100000);
    std::string actualDumpInfo = statsClient.Dump(g_dumpArgs);
    GTEST_LOG_(INFO) << __func__ << ": actual dump info: " << actualDumpInfo;
    ThermalActionReportTest::ThermalActionTriggered(CPU_BIG_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_MED_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_LIT_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(GPU_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(LCD_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(PROCESS_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(THERMAL_LEVEL_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CURRENT_SC_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CURRENT_BUCK_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(VOLATAGE_SC_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(VOLATAGE_BUCK_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_BOOST_ACTION_NAME, level, actualDumpInfo);
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.004 end");
}

/**
 * @tc.name: ThermalActionReportTest005
 * @tc.desc: test dump info when thermal action is triggered
 * @tc.type: FEATURE
 * @tc.cond: Set battery temp = 43100, scence = cam
 * @tc.result battery stats dump info
 */
HWTEST_F(ThermalActionReportTest, ThermalActionReportTest005, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.005 start");
    auto& statsClient = BatteryStatsClient::GetInstance();
    statsClient.Reset();

    int32_t ret = -1;
    int32_t batteryTemp = 43100;
    std::string sceneState = "cam";
    int32_t expectLevel = 2;
    ret = SetCondition(batteryTemp, sceneState);
    EXPECT_EQ(ret, ERR_OK) << " Thermal action fail due to set condition error";

    int32_t level = ThermalActionReportTest::GetThermalLevel(expectLevel);
    // sleep 100ms for hisys_event handle
    usleep(100000);
    std::string actualDumpInfo = statsClient.Dump(g_dumpArgs);
    GTEST_LOG_(INFO) << __func__ << ": actual dump info: " << actualDumpInfo;
    ThermalActionReportTest::ThermalActionTriggered(CPU_BIG_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_MED_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_LIT_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(GPU_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(LCD_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(PROCESS_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(THERMAL_LEVEL_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CURRENT_SC_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CURRENT_BUCK_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(VOLATAGE_SC_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(VOLATAGE_BUCK_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_BOOST_ACTION_NAME, level, actualDumpInfo);
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.005 end");
}

/**
 * @tc.name: ThermalActionReportTest006
 * @tc.desc: test dump info when thermal action is triggered
 * @tc.type: FEATURE
 * @tc.cond: Set battery temp = 43100, scence = call
 * @tc.result battery stats dump info
 */
HWTEST_F(ThermalActionReportTest, ThermalActionReportTest006, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.006 start");
    auto& statsClient = BatteryStatsClient::GetInstance();
    statsClient.Reset();

    int32_t ret = -1;
    int32_t batteryTemp = 43100;
    std::string sceneState = "call";
    int32_t expectLevel = 2;
    ret = SetCondition(batteryTemp, sceneState);
    EXPECT_EQ(ret, ERR_OK) << " Thermal action fail due to set condition error";

    int32_t level = ThermalActionReportTest::GetThermalLevel(expectLevel);
    // sleep 100ms for hisys_event handle
    usleep(100000);
    std::string actualDumpInfo = statsClient.Dump(g_dumpArgs);
    GTEST_LOG_(INFO) << __func__ << ": actual dump info: " << actualDumpInfo;
    ThermalActionReportTest::ThermalActionTriggered(CPU_BIG_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_MED_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_LIT_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(GPU_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(LCD_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(PROCESS_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(THERMAL_LEVEL_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CURRENT_SC_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CURRENT_BUCK_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(VOLATAGE_SC_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(VOLATAGE_BUCK_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_BOOST_ACTION_NAME, level, actualDumpInfo);
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.006 end");
}

/**
 * @tc.name: ThermalActionReportTest007
 * @tc.desc: test dump info when thermal action is triggered
 * @tc.type: FEATURE
 * @tc.cond: Set battery temp = 43100, scence = game
 * @tc.result battery stats dump info
 */
HWTEST_F(ThermalActionReportTest, ThermalActionReportTest007, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.007 start");
    auto& statsClient = BatteryStatsClient::GetInstance();
    statsClient.Reset();

    int32_t ret = -1;
    int32_t batteryTemp = 43100;
    std::string sceneState = "game";
    int32_t expectLevel = 2;
    ret = SetCondition(batteryTemp, sceneState);
    EXPECT_EQ(ret, ERR_OK) << " Thermal action fail due to set condition error";

    int32_t level = ThermalActionReportTest::GetThermalLevel(expectLevel);
    // sleep 100ms for hisys_event handle
    usleep(100000);
    std::string actualDumpInfo = statsClient.Dump(g_dumpArgs);
    GTEST_LOG_(INFO) << __func__ << ": actual dump info: " << actualDumpInfo;
    ThermalActionReportTest::ThermalActionTriggered(CPU_BIG_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_MED_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_LIT_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(GPU_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(LCD_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(PROCESS_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(THERMAL_LEVEL_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CURRENT_SC_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CURRENT_BUCK_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(VOLATAGE_SC_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(VOLATAGE_BUCK_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_BOOST_ACTION_NAME, level, actualDumpInfo);
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.007 end");
}

/**
 * @tc.name: ThermalActionReportTest008
 * @tc.desc: test dump info when thermal action is triggered
 * @tc.type: FEATURE
 * @tc.cond: Set battery temp = 43100, scence = ""
 * @tc.result battery stats dump info
 */
HWTEST_F(ThermalActionReportTest, ThermalActionReportTest008, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.008 start");
    auto& statsClient = BatteryStatsClient::GetInstance();
    statsClient.Reset();

    int32_t ret = -1;
    int32_t batteryTemp = 43100;
    std::string sceneState = "";
    int32_t expectLevel = 2;
    ret = SetCondition(batteryTemp, sceneState);
    EXPECT_EQ(ret, ERR_OK) << " Thermal action fail due to set condition error";

    int32_t level = ThermalActionReportTest::GetThermalLevel(expectLevel);
    // sleep 100ms for hisys_event handle
    usleep(100000);
    std::string actualDumpInfo = statsClient.Dump(g_dumpArgs);
    GTEST_LOG_(INFO) << __func__ << ": actual dump info: " << actualDumpInfo;
    ThermalActionReportTest::ThermalActionTriggered(CPU_BIG_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_MED_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_LIT_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(GPU_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(LCD_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(PROCESS_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(THERMAL_LEVEL_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CURRENT_SC_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CURRENT_BUCK_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(VOLATAGE_SC_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(VOLATAGE_BUCK_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_BOOST_ACTION_NAME, level, actualDumpInfo);
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.008 end");
}

/**
 * @tc.name: ThermalActionReportTest009
 * @tc.desc: test dump info when thermal action is triggered
 * @tc.type: FEATURE
 * @tc.cond: Set battery temp = 46100, scence = cam
 * @tc.result battery stats dump info
 */
HWTEST_F(ThermalActionReportTest, ThermalActionReportTest009, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.009 start");
    auto& statsClient = BatteryStatsClient::GetInstance();
    statsClient.Reset();

    int32_t ret = -1;
    int32_t batteryTemp = 46100;
    std::string sceneState = "cam";
    int32_t expectLevel = 3;
    ret = SetCondition(batteryTemp, sceneState);
    EXPECT_EQ(ret, ERR_OK) << " Thermal action fail due to set condition error";

    int32_t level = ThermalActionReportTest::GetThermalLevel(expectLevel);
    // sleep 100ms for hisys_event handle
    usleep(100000);
    std::string actualDumpInfo = statsClient.Dump(g_dumpArgs);
    GTEST_LOG_(INFO) << __func__ << ": actual dump info: " << actualDumpInfo;
    ThermalActionReportTest::ThermalActionTriggered(CPU_BIG_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_MED_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_LIT_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(GPU_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(LCD_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(PROCESS_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(THERMAL_LEVEL_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CURRENT_SC_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CURRENT_BUCK_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(VOLATAGE_SC_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(VOLATAGE_BUCK_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(SHUTDOWN_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_BOOST_ACTION_NAME, level, actualDumpInfo, true);
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.009 end");
}

/**
 * @tc.name: ThermalActionReportTest010
 * @tc.desc: test dump info when thermal action is triggered
 * @tc.type: FEATURE
 * @tc.cond: Set battery temp = 46100, scence = call
 * @tc.result battery stats dump info
 */
HWTEST_F(ThermalActionReportTest, ThermalActionReportTest010, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.010 start");
    auto& statsClient = BatteryStatsClient::GetInstance();
    statsClient.Reset();

    int32_t ret = -1;
    int32_t batteryTemp = 46100;
    std::string sceneState = "call";
    int32_t expectLevel = 3;
    ret = SetCondition(batteryTemp, sceneState);
    EXPECT_EQ(ret, ERR_OK) << " Thermal action fail due to set condition error";

    int32_t level = ThermalActionReportTest::GetThermalLevel(expectLevel);
    // sleep 100ms for hisys_event handle
    usleep(100000);
    std::string actualDumpInfo = statsClient.Dump(g_dumpArgs);
    GTEST_LOG_(INFO) << __func__ << ": actual dump info: " << actualDumpInfo;
    ThermalActionReportTest::ThermalActionTriggered(CPU_BIG_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_MED_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_LIT_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(GPU_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(LCD_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(PROCESS_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(THERMAL_LEVEL_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CURRENT_SC_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CURRENT_BUCK_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(VOLATAGE_SC_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(VOLATAGE_BUCK_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(SHUTDOWN_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_BOOST_ACTION_NAME, level, actualDumpInfo, true);
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.010 end");
}

/**
 * @tc.name: ThermalActionReportTest011
 * @tc.desc: test dump info when thermal action is triggered
 * @tc.type: FEATURE
 * @tc.cond: Set battery temp = 46100, scence = game
 * @tc.result battery stats dump info
 */
HWTEST_F(ThermalActionReportTest, ThermalActionReportTest011, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.011 start");
    auto& statsClient = BatteryStatsClient::GetInstance();
    statsClient.Reset();

    int32_t ret = -1;
    int32_t batteryTemp = 46100;
    std::string sceneState = "game";
    int32_t expectLevel = 3;
    ret = SetCondition(batteryTemp, sceneState);
    EXPECT_EQ(ret, ERR_OK) << " Thermal action fail due to set condition error";

    int32_t level = ThermalActionReportTest::GetThermalLevel(expectLevel);
    // sleep 100ms for hisys_event handle
    usleep(100000);
    std::string actualDumpInfo = statsClient.Dump(g_dumpArgs);
    GTEST_LOG_(INFO) << __func__ << ": actual dump info: " << actualDumpInfo;
    ThermalActionReportTest::ThermalActionTriggered(CPU_BIG_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_MED_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_LIT_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(GPU_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(LCD_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(PROCESS_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(THERMAL_LEVEL_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CURRENT_SC_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CURRENT_BUCK_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(VOLATAGE_SC_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(VOLATAGE_BUCK_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(SHUTDOWN_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_BOOST_ACTION_NAME, level, actualDumpInfo, true);
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.011 end");
}

/**
 * @tc.name: ThermalActionReportTest012
 * @tc.desc: test dump info when thermal action is triggered
 * @tc.type: FEATURE
 * @tc.cond: Set battery temp = 46100, scence = ""
 * @tc.result battery stats dump info
 */
HWTEST_F(ThermalActionReportTest, ThermalActionReportTest012, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.012 start");
    auto& statsClient = BatteryStatsClient::GetInstance();
    statsClient.Reset();

    int32_t ret = -1;
    int32_t batteryTemp = 46100;
    std::string sceneState = "";
    int32_t expectLevel = 3;
    ret = SetCondition(batteryTemp, sceneState);
    EXPECT_EQ(ret, ERR_OK) << " Thermal action fail due to set condition error";

    int32_t level = ThermalActionReportTest::GetThermalLevel(expectLevel);
    // sleep 100ms for hisys_event handle
    usleep(100000);
    std::string actualDumpInfo = statsClient.Dump(g_dumpArgs);
    GTEST_LOG_(INFO) << __func__ << ": actual dump info: " << actualDumpInfo;
    ThermalActionReportTest::ThermalActionTriggered(CPU_BIG_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_MED_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_LIT_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(GPU_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(LCD_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(PROCESS_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(THERMAL_LEVEL_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CURRENT_SC_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CURRENT_BUCK_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(VOLATAGE_SC_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(VOLATAGE_BUCK_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(SHUTDOWN_ACTION_NAME, level, actualDumpInfo);
    ThermalActionReportTest::ThermalActionTriggered(CPU_BOOST_ACTION_NAME, level, actualDumpInfo, true);
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.012 end");
}
} // namespace
#endif // BATTERY_STATS_ENABLE
