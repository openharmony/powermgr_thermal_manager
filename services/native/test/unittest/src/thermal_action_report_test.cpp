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


#include "thermal_action_report_test.h"

#include "securec.h"
#include <fcntl.h>
#include <string>
#include <unistd.h>

#include "battery_srv_client.h"
#include "battery_stats_client.h"
#include "mock_thermal_mgr_client.h"
#include "power_mgr_client.h"
#include "thermal_config_file_parser.h"
#include "thermal_mgr_client.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
static std::shared_ptr<ThermalConfigFileParser> g_configParser = nullptr;
static std::vector<std::string> g_dumpArgs;
static std::string g_sceneState;
static const std::string policyCfgName = "base_safe";
constexpr int32_t NUM_ZERO = 0;
constexpr uint32_t MAX_PATH = 256;
constexpr int32_t THERMAL_RATIO_BEGIN = 0;
constexpr int32_t THERMAL_RATIO_LENGTH = 4;
constexpr const char* BATTERY_TEMP_PATH = "/data/service/el0/thermal/sensor/battery/temp";
constexpr const char* CONFIG_LEVEL_PATH = "/data/service/el0/thermal/config/configLevel";
constexpr const char* VENDOR_CONFIG = "/vendor/etc/thermal_config/thermal_service_config.xml";
constexpr const char* SIMULATION_TEMP_DIR = "/data/service/el0/thermal/sensor/%s/temp";
}

void ThermalActionReportTest::ParserThermalSrvConfigFile()
{
    if (g_configParser == nullptr) {
        g_configParser = std::make_shared<ThermalConfigFileParser>();
        if (!g_configParser->Init()) {
            THERMAL_HILOGE(LABEL_TEST, "Thermal service config file parser initialization failed");
        }
    }
}

int32_t ThermalActionReportTest::WriteFile(const std::string& path, const std::string& buf, size_t size)
{
    FILE* stream = fopen(path.c_str(), "w+");
    if (stream == nullptr) {
        return ERR_INVALID_VALUE;
    }
    size_t ret = fwrite(buf.c_str(), strlen(buf.c_str()), 1, stream);
    if (ret == ERR_OK) {
        THERMAL_HILOGE(LABEL_TEST, "ret=%{public}zu", ret);
    }
    int32_t state = fseek(stream, 0, SEEK_SET);
    if (state != ERR_OK) {
        fclose(stream);
        return state;
    }
    state = fclose(stream);
    if (state != ERR_OK) {
        return state;
    }
    return ERR_OK;
}

int32_t ThermalActionReportTest::ReadFile(const char* path, char* buf, size_t size)
{
    int32_t ret = ReadSysfsFile(path, buf, size);
    if (ret != NUM_ZERO) {
        THERMAL_HILOGD(LABEL_TEST, "failed to read file");
        return ret;
    }
    return ERR_OK;
}

int32_t ThermalActionReportTest::ReadSysfsFile(const char* path, char* buf, size_t size)
{
    int32_t readSize;
    int fd = open(path, O_RDONLY, S_IRUSR | S_IRGRP | S_IROTH);
    if (fd < NUM_ZERO) {
        THERMAL_HILOGE(LABEL_TEST, "failed to open file node");
        return ERR_INVALID_VALUE;
    }

    readSize = read(fd, buf, size - 1);
    if (readSize < NUM_ZERO) {
        THERMAL_HILOGE(LABEL_TEST, "failed to read file");
        close(fd);
        return ERR_INVALID_VALUE;
    }

    buf[readSize] = '\0';
    Trim(buf);
    close(fd);

    return ERR_OK;
}

int32_t ThermalActionReportTest::ConvertInt(const std::string& value)
{
    if (IsNumericStr(value)) {
        return std::stoi(value);
    }
    return -1;
}

void ThermalActionReportTest::Trim(char* str)
{
    if (str == nullptr) {
        return;
    }

    str[strcspn(str, "\n")] = 0;
}

int32_t ThermalActionReportTest::InitNode()
{
    char bufTemp[MAX_PATH] = {0};
    int32_t ret = -1;
    std::map<std::string, int32_t> sensor;
    sensor["battery"] = 0;
    sensor["charger"] = 0;
    sensor["pa"] = 0;
    sensor["ap"] = 0;
    sensor["ambient"] = 0;
    sensor["cpu"] = 0;
    sensor["soc"] = 0;
    sensor["shell"] = 0;
    for (auto iter : sensor) {
        ret = snprintf_s(bufTemp, MAX_PATH, sizeof(bufTemp) - 1, SIMULATION_TEMP_DIR, iter.first.c_str());
        if (ret < EOK) {
            return ret;
        }
        std::string temp = std::to_string(iter.second);
        WriteFile(bufTemp, temp, temp.length());
    }
    return ERR_OK;
}

void ThermalActionReportTest::SetScene(const std::string& scene)
{
    g_sceneState = scene;
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene(g_sceneState);
}

int32_t ThermalActionReportTest::SetCondition(int32_t value, const std::string& scene)
{
    THERMAL_HILOGD(LABEL_TEST, "battery = %{public}d, scene = %{public}s", value, scene.c_str());
    int32_t ret = -1;
    char batteryTempBuf[MAX_PATH] = {0};
    ret = snprintf_s(batteryTempBuf, MAX_PATH, sizeof(batteryTempBuf) - 1, BATTERY_TEMP_PATH);
    EXPECT_EQ(true, ret >= EOK);
    std::string strTemp = to_string(value) + "\n";
    ret = WriteFile(batteryTempBuf, strTemp, strTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
    SetScene(scene);
    return ret;
}

int32_t ThermalActionReportTest::GetThermalLevel(int32_t expectValue)
{
    int32_t ret = -1;
    char levelBuf[MAX_PATH] = {0};
    char levelValue[MAX_PATH] = {0};
    ret = snprintf_s(levelBuf, MAX_PATH, sizeof(levelBuf) - 1, CONFIG_LEVEL_PATH);
    EXPECT_EQ(true, ret >= EOK);
    ret = ReadFile(levelBuf, levelValue, sizeof(levelValue));
    EXPECT_EQ(true, ret == ERR_OK);
    std::string level = levelValue;
    int32_t value = ConvertInt(level);
    THERMAL_HILOGD(LABEL_TEST, "value: %{public}d", value);
    EXPECT_EQ(true, value == expectValue) << "Thermal action policy failed";
    return value;
}

std::string ThermalActionReportTest::GetActionValue(const std::string& actionName, uint32_t level)
{
    THERMAL_HILOGD(LABEL_TEST, "action name = %{public}s, level = %{public}d", actionName.c_str(), level);
    std::string value = "0";
    std::vector<PolicyAction> vAction;
    if (!g_configParser->GetActionPolicy(policyCfgName, level, vAction)) {
        THERMAL_HILOGD(LABEL_TEST, "Get policy failed name = %{public}s, return", policyCfgName.c_str());
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

std::string ThermalActionReportTest::ActionValueDecision(const std::string& actionName,
    std::vector<PolicyAction>& vAction)
{
    THERMAL_HILOGD(LABEL_TEST, "action name = %{public}s", actionName.c_str());
    int32_t value = -1;
    std::vector<uint32_t> valueList;
    for (auto actionIter : vAction) {
        if (actionIter.actionName == actionName) {
            if (actionIter.isProp) {
                if (StateDecision(actionIter.mActionProp)) {
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

std::string ThermalActionReportTest::LcdValueDecision(const std::string& actionName,
    std::vector<PolicyAction>& vAction)
{
    THERMAL_HILOGD(LABEL_TEST, "action name = %{public}s", actionName.c_str());
    float value = -1.0;
    std::vector<float> valueList;
    std::map<std::string, std::string> sceneMap;
    for (auto actionIter : vAction) {
        if (actionIter.actionName == actionName) {
            if (actionIter.isProp) {
                if (StateDecision(actionIter.mActionProp)) {
                    valueList.push_back(stof(actionIter.actionValue));
                    sceneMap.emplace(std::pair(actionIter.mActionProp.begin()->second, actionIter.actionValue));
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
        THERMAL_HILOGD(LABEL_TEST, "state = %{public}s, value = %{public}s",
            prop.first.c_str(), prop.second.c_str());
        THERMAL_HILOGD(LABEL_TEST, "state iter = %{public}s, iter value = %{public}s",
            stateIter->first.c_str(), stateIter->second.c_str());
        if (stateIter != stateMap.end()) {
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
    auto& batterySrvClient = BatterySrvClient::GetInstance();
    BatteryChargeState chargeState = batterySrvClient.GetChargingStatus();
    if (chargeState == BatteryChargeState::CHARGE_STATE_ENABLE) {
        state = "1";
    } else if (chargeState == BatteryChargeState::CHARGE_STATE_NONE) {
        state = "0";
    }
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
        THERMAL_HILOGD(LABEL_TEST, "stateMap name = %{public}s, value = %{public}s",
            iter.first.c_str(), iter.second.c_str());
    }
}

void ThermalActionReportTest::ThermalActionTriggered(const std::string& actionName, int32_t level,
    const std::string& dumpInfo)
{
    bool enableEvent = g_configParser->GetActionEnableEvent(actionName);
    THERMAL_HILOGD(LABEL_TEST, "action name = %{public}s, event flag = %{public}d", actionName.c_str(), enableEvent);
    if (!enableEvent) {
        GTEST_LOG_(INFO) << __func__ << " action name: " << actionName <<" enalbe event flag is false, return";
        return;
    }
    std::string value = GetActionValue(actionName, level);
    std::string expectedDumpInfo;
    std::string valueLabel = " Value = ";
    if (actionName == LCD_ACTION_NAME) {
        valueLabel = " Ratio = ";
    }
    expectedDumpInfo.append("Additional debug info: ")
    .append("Event name = THERMAL_ACTION_TRIGGERED")
    .append(" Action name = ")
    .append(actionName)
    .append(valueLabel)
    .append(value);
    THERMAL_HILOGD(LABEL_TEST, "value: %{public}s", value.c_str());
    GTEST_LOG_(INFO) << __func__ << " action name: " << actionName <<" expected debug info: " << expectedDumpInfo;
    auto index = dumpInfo.find(expectedDumpInfo);
    EXPECT_TRUE(index != string::npos) << " Thermal action fail due to not found related debug info."
        << " action name = " << actionName;
}

void ThermalActionReportTest::SetUpTestCase()
{
    ParserThermalSrvConfigFile();
    g_dumpArgs.push_back("-batterystats");
}

void ThermalActionReportTest::TearDownTestCase()
{
}

void ThermalActionReportTest::SetUp()
{
    InitNode();
    MockThermalMgrClient::GetInstance().GetThermalInfo();
}

void ThermalActionReportTest::TearDown()
{
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("");
    MockThermalMgrClient::GetInstance().GetThermalInfo();
}

namespace {
/**
 * @tc.name: ThermalActionReportTest001
 * @tc.desc: test dump info when thermal action is triggered
 * @tc.type: FEATURE
 * @tc.cond: Set battery temp = 40100, scence = cam
 * @tc.result battery stats dump info
 */
HWTEST_F (ThermalActionReportTest, ThermalActionReportTest001, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.001 start");
    auto& statsClient = BatteryStatsClient::GetInstance();
    statsClient.Reset();

    int32_t ret = -1;
    int32_t batteryTemp = 40100;
    std::string sceneState = "cam";
    int32_t expectLevel = 1;
    ret = SetCondition(batteryTemp, sceneState);
    EXPECT_EQ(true, ret == ERR_OK) << " Thermal action fail due to set condition error";

    if (access(VENDOR_CONFIG, 0) != 0) {
        MockThermalMgrClient::GetInstance().GetThermalInfo();
        int32_t level = ThermalActionReportTest::GetThermalLevel(expectLevel);
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
    }
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.001 end");
}

/**
 * @tc.name: ThermalActionReportTest002
 * @tc.desc: test dump info when thermal action is triggered
 * @tc.type: FEATURE
 * @tc.cond: Set battery temp = 40100, scence = call
 * @tc.result battery stats dump info
 */
HWTEST_F (ThermalActionReportTest, ThermalActionReportTest002, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.002 start");
    auto& statsClient = BatteryStatsClient::GetInstance();
    statsClient.Reset();

    int32_t ret = -1;
    int32_t batteryTemp = 40100;
    std::string sceneState = "call";
    int32_t expectLevel = 1;
    ret = SetCondition(batteryTemp, sceneState);
    EXPECT_EQ(true, ret == ERR_OK) << " Thermal action fail due to set condition error";

    if (access(VENDOR_CONFIG, 0) != 0) {
        MockThermalMgrClient::GetInstance().GetThermalInfo();
        int32_t level = ThermalActionReportTest::GetThermalLevel(expectLevel);
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
    }
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.002 end");
}

/**
 * @tc.name: ThermalActionReportTest003
 * @tc.desc: test dump info when thermal action is triggered
 * @tc.type: FEATURE
 * @tc.cond: Set battery temp = 40100, scence = game
 * @tc.result battery stats dump info
 */
HWTEST_F (ThermalActionReportTest, ThermalActionReportTest003, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.003 start");
    auto& statsClient = BatteryStatsClient::GetInstance();
    statsClient.Reset();

    int32_t ret = -1;
    int32_t batteryTemp = 40100;
    std::string sceneState = "game";
    int32_t expectLevel = 1;
    ret = SetCondition(batteryTemp, sceneState);
    EXPECT_EQ(true, ret == ERR_OK) << " Thermal action fail due to set condition error";

    if (access(VENDOR_CONFIG, 0) != 0) {
        MockThermalMgrClient::GetInstance().GetThermalInfo();
        int32_t level = ThermalActionReportTest::GetThermalLevel(expectLevel);
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
    }
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.003 end");
}

/**
 * @tc.name: ThermalActionReportTest004
 * @tc.desc: test dump info when thermal action is triggered
 * @tc.type: FEATURE
 * @tc.cond: Set battery temp = 40100, scence = ""
 * @tc.result battery stats dump info
 */
HWTEST_F (ThermalActionReportTest, ThermalActionReportTest004, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.004 start");
    auto& statsClient = BatteryStatsClient::GetInstance();
    statsClient.Reset();

    int32_t ret = -1;
    int32_t batteryTemp = 40100;
    std::string sceneState = "";
    int32_t expectLevel = 1;
    ret = SetCondition(batteryTemp, sceneState);
    EXPECT_EQ(true, ret == ERR_OK) << " Thermal action fail due to set condition error";

    if (access(VENDOR_CONFIG, 0) != 0) {
        MockThermalMgrClient::GetInstance().GetThermalInfo();
        int32_t level = ThermalActionReportTest::GetThermalLevel(expectLevel);
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
    }
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.004 end");
}

/**
 * @tc.name: ThermalActionReportTest005
 * @tc.desc: test dump info when thermal action is triggered
 * @tc.type: FEATURE
 * @tc.cond: Set battery temp = 43100, scence = cam
 * @tc.result battery stats dump info
 */
HWTEST_F (ThermalActionReportTest, ThermalActionReportTest005, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.005 start");
    auto& statsClient = BatteryStatsClient::GetInstance();
    statsClient.Reset();

    int32_t ret = -1;
    int32_t batteryTemp = 43100;
    std::string sceneState = "cam";
    int32_t expectLevel = 2;
    ret = SetCondition(batteryTemp, sceneState);
    EXPECT_EQ(true, ret == ERR_OK) << " Thermal action fail due to set condition error";

    if (access(VENDOR_CONFIG, 0) != 0) {
        MockThermalMgrClient::GetInstance().GetThermalInfo();
        int32_t level = ThermalActionReportTest::GetThermalLevel(expectLevel);
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
    }
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.005 end");
}

/**
 * @tc.name: ThermalActionReportTest006
 * @tc.desc: test dump info when thermal action is triggered
 * @tc.type: FEATURE
 * @tc.cond: Set battery temp = 43100, scence = call
 * @tc.result battery stats dump info
 */
HWTEST_F (ThermalActionReportTest, ThermalActionReportTest006, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.006 start");
    auto& statsClient = BatteryStatsClient::GetInstance();
    statsClient.Reset();

    int32_t ret = -1;
    int32_t batteryTemp = 43100;
    std::string sceneState = "call";
    int32_t expectLevel = 2;
    ret = SetCondition(batteryTemp, sceneState);
    EXPECT_EQ(true, ret == ERR_OK) << " Thermal action fail due to set condition error";

    if (access(VENDOR_CONFIG, 0) != 0) {
        MockThermalMgrClient::GetInstance().GetThermalInfo();
        int32_t level = ThermalActionReportTest::GetThermalLevel(expectLevel);
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
    }
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.006 end");
}

/**
 * @tc.name: ThermalActionReportTest007
 * @tc.desc: test dump info when thermal action is triggered
 * @tc.type: FEATURE
 * @tc.cond: Set battery temp = 43100, scence = game
 * @tc.result battery stats dump info
 */
HWTEST_F (ThermalActionReportTest, ThermalActionReportTest007, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.007 start");
    auto& statsClient = BatteryStatsClient::GetInstance();
    statsClient.Reset();

    int32_t ret = -1;
    int32_t batteryTemp = 43100;
    std::string sceneState = "game";
    int32_t expectLevel = 2;
    ret = SetCondition(batteryTemp, sceneState);
    EXPECT_EQ(true, ret == ERR_OK) << " Thermal action fail due to set condition error";

    if (access(VENDOR_CONFIG, 0) != 0) {
        MockThermalMgrClient::GetInstance().GetThermalInfo();
        int32_t level = ThermalActionReportTest::GetThermalLevel(expectLevel);
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
    }
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.007 end");
}

/**
 * @tc.name: ThermalActionReportTest008
 * @tc.desc: test dump info when thermal action is triggered
 * @tc.type: FEATURE
 * @tc.cond: Set battery temp = 43100, scence = ""
 * @tc.result battery stats dump info
 */
HWTEST_F (ThermalActionReportTest, ThermalActionReportTest008, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.008 start");
    auto& statsClient = BatteryStatsClient::GetInstance();
    statsClient.Reset();

    int32_t ret = -1;
    int32_t batteryTemp = 43100;
    std::string sceneState = "";
    int32_t expectLevel = 2;
    ret = SetCondition(batteryTemp, sceneState);
    EXPECT_EQ(true, ret == ERR_OK) << " Thermal action fail due to set condition error";

    if (access(VENDOR_CONFIG, 0) != 0) {
        MockThermalMgrClient::GetInstance().GetThermalInfo();
        int32_t level = ThermalActionReportTest::GetThermalLevel(expectLevel);
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
    }
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.008 end");
}

/**
 * @tc.name: ThermalActionReportTest009
 * @tc.desc: test dump info when thermal action is triggered
 * @tc.type: FEATURE
 * @tc.cond: Set battery temp = 46100, scence = cam
 * @tc.result battery stats dump info
 */
HWTEST_F (ThermalActionReportTest, ThermalActionReportTest009, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.009 start");
    auto& statsClient = BatteryStatsClient::GetInstance();
    statsClient.Reset();

    int32_t ret = -1;
    int32_t batteryTemp = 46100;
    std::string sceneState = "cam";
    int32_t expectLevel = 3;
    ret = SetCondition(batteryTemp, sceneState);
    EXPECT_EQ(true, ret == ERR_OK) << " Thermal action fail due to set condition error";

    if (access(VENDOR_CONFIG, 0) != 0) {
        MockThermalMgrClient::GetInstance().GetThermalInfo();
        int32_t level = ThermalActionReportTest::GetThermalLevel(expectLevel);
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
    }
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.009 end");
}

/**
 * @tc.name: ThermalActionReportTest010
 * @tc.desc: test dump info when thermal action is triggered
 * @tc.type: FEATURE
 * @tc.cond: Set battery temp = 46100, scence = call
 * @tc.result battery stats dump info
 */
HWTEST_F (ThermalActionReportTest, ThermalActionReportTest010, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.010 start");
    auto& statsClient = BatteryStatsClient::GetInstance();
    statsClient.Reset();

    int32_t ret = -1;
    int32_t batteryTemp = 46100;
    std::string sceneState = "call";
    int32_t expectLevel = 3;
    ret = SetCondition(batteryTemp, sceneState);
    EXPECT_EQ(true, ret == ERR_OK) << " Thermal action fail due to set condition error";

    if (access(VENDOR_CONFIG, 0) != 0) {
        MockThermalMgrClient::GetInstance().GetThermalInfo();
        int32_t level = ThermalActionReportTest::GetThermalLevel(expectLevel);
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
    }
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.010 end");
}

/**
 * @tc.name: ThermalActionReportTest011
 * @tc.desc: test dump info when thermal action is triggered
 * @tc.type: FEATURE
 * @tc.cond: Set battery temp = 46100, scence = game
 * @tc.result battery stats dump info
 */
HWTEST_F (ThermalActionReportTest, ThermalActionReportTest011, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.011 start");
    auto& statsClient = BatteryStatsClient::GetInstance();
    statsClient.Reset();

    int32_t ret = -1;
    int32_t batteryTemp = 46100;
    std::string sceneState = "game";
    int32_t expectLevel = 3;
    ret = SetCondition(batteryTemp, sceneState);
    EXPECT_EQ(true, ret == ERR_OK) << " Thermal action fail due to set condition error";

    if (access(VENDOR_CONFIG, 0) != 0) {
        MockThermalMgrClient::GetInstance().GetThermalInfo();
        int32_t level = ThermalActionReportTest::GetThermalLevel(expectLevel);
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
    }
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.011 end");
}

/**
 * @tc.name: ThermalActionReportTest012
 * @tc.desc: test dump info when thermal action is triggered
 * @tc.type: FEATURE
 * @tc.cond: Set battery temp = 46100, scence = ""
 * @tc.result battery stats dump info
 */
HWTEST_F (ThermalActionReportTest, ThermalActionReportTest012, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.012 start");
    auto& statsClient = BatteryStatsClient::GetInstance();
    statsClient.Reset();

    int32_t ret = -1;
    int32_t batteryTemp = 46100;
    std::string sceneState = "";
    int32_t expectLevel = 3;
    ret = SetCondition(batteryTemp, sceneState);
    EXPECT_EQ(true, ret == ERR_OK) << " Thermal action fail due to set condition error";

    if (access(VENDOR_CONFIG, 0) != 0) {
        MockThermalMgrClient::GetInstance().GetThermalInfo();
        int32_t level = ThermalActionReportTest::GetThermalLevel(expectLevel);
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
    }
    THERMAL_HILOGD(LABEL_TEST, "Thermal action report test No.012 end");
}
}

