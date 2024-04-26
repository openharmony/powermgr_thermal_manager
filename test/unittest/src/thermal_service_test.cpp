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

#include "thermal_service_test.h"

#ifdef THERMAL_GTEST
#define private   public
#define protected public
#define final
#endif

#include <map>
#include <string>
#include <vector>

#include "system_ability_definition.h"

#include "action_cpu_big.h"
#include "config_policy_utils.h"
#include "power_mgr_client.h"
#include "screen_state_collection.h"
#include "thermal_config_sensor_cluster.h"
#include "thermal_log.h"
#include "thermal_mgr_dumper.h"
#include "thermal_policy.h"
#include "thermal_service.h"

using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace testing::ext;
using namespace std;

namespace {
sptr<ThermalService> g_service = nullptr;
constexpr const char* VENDOR_CONFIG = "/vendor/etc/thermal_config/thermal_service_config.xml";
constexpr const char* SYSTEM_CONFIG = "/system/etc/thermal_config/thermal_service_config.xml";
} // namespace

char* GetOneCfgFile(const char *pathSuffix, char *buf, unsigned int bufLength)
{
    THERMAL_HILOGD(LABEL_TEST, "mock GetOneCfgFile.");
    return nullptr;
}

void ThermalServiceTest::SetUpTestCase()
{
    g_service = ThermalService::GetInstance();
    g_service->InitSystemTestModules();
    g_service->OnStart();
}

namespace {
/**
 * @tc.name: ThermalMgrDumperTest001
 * @tc.desc: test dump help
 * @tc.type: FUNC
 */
HWTEST_F(ThermalServiceTest, ThermalMgrDumperTest001, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrDumperTest001 start.");
    std::vector<std::string> args;
    std::string result;
    EXPECT_TRUE(ThermalMgrDumper::Dump(args, result));
    EXPECT_TRUE(!result.empty());

    result.clear();
    args.push_back("-h");
    EXPECT_TRUE(ThermalMgrDumper::Dump(args, result));
    EXPECT_TRUE(!result.empty());

    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrDumperTest001 end.");
}

/**
 * @tc.name: ThermalMgrDumperTest002
 * @tc.desc: test dump -d
 * @tc.type: FUNC
 */
HWTEST_F(ThermalServiceTest, ThermalMgrDumperTest002, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrDumperTest002 start.");
    std::vector<std::string> args;
    std::string result;

    args.push_back("-d");
    EXPECT_TRUE(ThermalMgrDumper::Dump(args, result));
    EXPECT_TRUE(result.empty());
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrDumperTest002 end.");
}

/**
 * @tc.name: ThermalMgrDumperTest003
 * @tc.desc: test dump -t
 * @tc.type: FUNC
 */
HWTEST_F(ThermalServiceTest, ThermalMgrDumperTest003, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrDumperTest003 start.");
    std::vector<std::string> args;
    std::string result;

    g_service->RegisterThermalHdiCallback();
    args.push_back("-t");
    EXPECT_TRUE(ThermalMgrDumper::Dump(args, result));
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrDumperTest003 end.");
}

/**
 * @tc.name: ThermalMgrDumperTest004
 * @tc.desc: test dump Invalid value
 * @tc.type: FUNC
 */
HWTEST_F(ThermalServiceTest, ThermalMgrDumperTest004, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrDumperTest004 start.");
    std::vector<std::string> args;
    std::string result;

    args.push_back("---");
    EXPECT_TRUE(ThermalMgrDumper::Dump(args, result));
    EXPECT_TRUE(result.empty());
    THERMAL_HILOGD(LABEL_TEST, "ThermalMgrDumperTest004 end.");
}

/**
 * @tc.name: ThermalMgrDumperTest005
 * @tc.desc: test dump switch temp report
 * @tc.type: FUNC
 */
HWTEST_F(ThermalServiceTest, ThermalMgrDumperTest005, TestSize.Level0)
{
    std::vector<std::string> args;
    std::string result;

    args.push_back("-st");
    args.push_back("0");
    EXPECT_TRUE(ThermalMgrDumper::Dump(args, result));
    args.clear();
    args.push_back("-st");
    args.push_back("1");
    EXPECT_TRUE(ThermalMgrDumper::Dump(args, result));
}

/**
 * @tc.name: ThermalMgrDumperTest006
 * @tc.desc: test dump temp emul
 * @tc.type: FUNC
 */
HWTEST_F(ThermalServiceTest, ThermalMgrDumperTest006, TestSize.Level0)
{
    std::vector<std::string> args;
    std::string result;

    args.push_back("-te");
    args.push_back("battery");
    args.push_back("43000");
    EXPECT_TRUE(ThermalMgrDumper::Dump(args, result));
    args.clear();
    args.push_back("-te");
    EXPECT_TRUE(ThermalMgrDumper::Dump(args, result));
}

/**
 * @tc.name: ThermalServiceTest000
 * @tc.desc: test OnAddSystemAbility
 * @tc.type: FUNC
 * @tc.require: issueI6KRS8
 */
HWTEST_F(ThermalServiceTest, ThermalServiceTest000, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalServiceTest000 start.");
    std::string deviceId = "";
    EXPECT_FALSE(g_service == nullptr);
    g_service->OnAddSystemAbility(COMMON_EVENT_SERVICE_ID, deviceId);
    g_service->OnAddSystemAbility(POWER_MANAGER_THERMAL_SERVICE_ID, deviceId);
    THERMAL_HILOGD(LABEL_TEST, "ThermalServiceTest000 end.");
}

/**
 * @tc.name: ThermalServiceTest001
 * @tc.desc: test OnStart and OnStop
 * @tc.type: FUNC
 */
HWTEST_F(ThermalServiceTest, ThermalServiceTest001, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalServiceTest001 start.");
    g_service->ready_ = true;
    g_service->InitSystemTestModules();
    g_service->OnStart();

    g_service->ready_ = false;
    g_service->OnStop();

    g_service->ready_ = true;
    g_service->RegisterHdiStatusListener();
    g_service->GetThermalInfo();
    g_service->OnStop();
    EXPECT_FALSE(g_service->ready_);

    g_service->ready_ = true;
    g_service->OnStop();
    EXPECT_FALSE(g_service->ready_);

    THERMAL_HILOGD(LABEL_TEST, "ThermalServiceTest001 end.");
}

/**
 * @tc.name: ThermalServiceTest002
 * @tc.desc: test Init
 * @tc.type: FUNC
 */
HWTEST_F(ThermalServiceTest, ThermalServiceTest002, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalServiceTest002 start.");
    EXPECT_TRUE(g_service->Init());
    EXPECT_TRUE(g_service->CreateConfigModule());
    EXPECT_TRUE(g_service->Init());
    EXPECT_TRUE(g_service->CreateConfigModule());
    EXPECT_TRUE(g_service->InitStateMachine());
    g_service->InitSystemTestModules();
    THERMAL_HILOGD(LABEL_TEST, "ThermalServiceTest002 end.");
}

/**
 * @tc.name: ThermalServiceTest003
 * @tc.desc: test InitConfigFile
 * @tc.type: FUNC
 */
HWTEST_F(ThermalServiceTest, ThermalServiceTest003, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalServiceTest003 start.");

    std::string VENDOR_CONFIG_BACKUP = "/vendor/etc/thermal_config/thermal_service_config_backup.xml";
    std::string SYSTEM_CONFIG_BACKUP = "/system/etc/thermal_config/thermal_service_config_backup.xml";

    rename(VENDOR_CONFIG_BACKUP.c_str(), VENDOR_CONFIG);
    rename(SYSTEM_CONFIG_BACKUP.c_str(), SYSTEM_CONFIG);

    THERMAL_HILOGD(LABEL_TEST, "ThermalServiceTest003 end.");
}

/**
 * @tc.name: ThermalServiceTest004
 * @tc.desc: test Service Dump
 * @tc.type: FUNC
 */
HWTEST_F(ThermalServiceTest, ThermalServiceTest004, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalServiceTest004 start.");

    int fd = 0;
    std::vector<std::u16string> args;
    args.push_back(u"-h");
    g_service->isBootCompleted_ = true;
    EXPECT_EQ(ERR_OK, g_service->Dump(fd, args));

    fd = -1;
    EXPECT_EQ(ERR_OK, g_service->Dump(fd, args));

    THERMAL_HILOGD(LABEL_TEST, "ThermalServiceTest004 end.");
}

/**
 * @tc.name: ThermalConfigSensorCluster001
 * @tc.desc: test CheckStandard
 * @tc.type: FUNC
 */
HWTEST_F(ThermalServiceTest, ThermalConfigSensorCluster001, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalConfigSensorCluster001 start.");

    ThermalConfigSensorCluster cluster;
    cluster.sensorInfolist_.clear();
    EXPECT_FALSE(cluster.CheckStandard());

    // The first for loop returns the value false
    LevelItem item;
    item.level = 2;
    std::vector<LevelItem> vecLevel;
    vecLevel.push_back(item);
    cluster.sensorInfolist_["test"] = vecLevel;
    EXPECT_FALSE(cluster.CheckStandard());

    // continue
    vecLevel.clear();
    item.level = 1;
    vecLevel.push_back(item);
    cluster.sensorInfolist_["test"] = vecLevel;
    std::vector<AuxLevelItem> auxLevel;
    cluster.auxSensorInfolist_["test"] = auxLevel;
    EXPECT_TRUE(cluster.CheckStandard());

    // The second for loop returns the value false
    AuxLevelItem auxItem;
    auxLevel.push_back(auxItem);
    auxLevel.push_back(auxItem);
    cluster.auxSensorInfolist_["test"] = auxLevel;
    EXPECT_FALSE(cluster.CheckStandard());

    THERMAL_HILOGD(LABEL_TEST, "ThermalConfigSensorCluster001 end.");
}

/**
 * @tc.name: ThermalConfigSensorCluster002
 * @tc.desc: test UpdateThermalLevel
 * @tc.type: FUNC
 */
HWTEST_F(ThermalServiceTest, ThermalConfigSensorCluster002, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalConfigSensorCluster002 start.");

    // Null data return
    ThermalConfigSensorCluster cluster;
    TypeTempMap typeTempInfo;
    cluster.latestLevel_ = 0;
    cluster.UpdateThermalLevel(typeTempInfo);
    EXPECT_EQ(cluster.latestLevel_, 0);

    THERMAL_HILOGD(LABEL_TEST, "ThermalConfigSensorCluster002 end.");
}

/**
 * @tc.name: ThermalConfigSensorCluster003
 * @tc.desc: test AscJudgment if branch
 * @tc.type: FUNC
 */
HWTEST_F(ThermalServiceTest, ThermalConfigSensorCluster003, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalConfigSensorCluster003 start.");
    // inner if branch (curTemp >= threshold)
    LevelItem item1;
    item1.threshold = 0;
    LevelItem item2;
    item2.threshold = 0;
    item2.level = 999;
    std::vector<LevelItem> levItems1;
    levItems1.push_back(item1);
    levItems1.push_back(item2);
    int32_t curTemp = 1;
    uint32_t level = 1;
    ThermalConfigSensorCluster cluster;
    cluster.AscJudgment(levItems1, curTemp, level);
    EXPECT_EQ(level, item2.level);

    const int32_t INDEX0 = 0;
    const int32_t INDEX1 = 1;
    // The break branch in the for loop
    levItems1.at(INDEX1).threshold = 3;
    level = 1;
    cluster.AscJudgment(levItems1, curTemp, level);
    EXPECT_EQ(level, 1);

    // inner else if branch (curTemp < thresholdClr)
    levItems1.at(INDEX0).thresholdClr = 2;
    levItems1.at(INDEX0).level = 999;
    levItems1.at(INDEX1).threshold = 2;
    levItems1.at(INDEX1).thresholdClr = 2;
    level = 1;
    cluster.AscJudgment(levItems1, curTemp, level);
    EXPECT_EQ(level, levItems1.at(INDEX0).level - 1);
    THERMAL_HILOGD(LABEL_TEST, "ThermalConfigSensorCluster003 end.");
}

/**
 * @tc.name: ThermalConfigSensorCluster004
 * @tc.desc: test AscJudgment else if branch
 * @tc.type: FUNC
 */
HWTEST_F(ThermalServiceTest, ThermalConfigSensorCluster004, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalConfigSensorCluster004 start.");

    LevelItem item;
    item.thresholdClr = 2;
    item.level = 999;
    std::vector<LevelItem> levItems;
    levItems.push_back(item);
    int32_t curTemp = 1;
    uint32_t level = 1;
    ThermalConfigSensorCluster cluster;
    cluster.AscJudgment(levItems, curTemp, level);
    EXPECT_EQ(level, item.level - 1);

    THERMAL_HILOGD(LABEL_TEST, "ThermalConfigSensorCluster004 end.");
}

/**
 * @tc.name: ThermalConfigSensorCluster005
 * @tc.desc: test AscJudgment else branch
 * @tc.type: FUNC
 */
HWTEST_F(ThermalServiceTest, ThermalConfigSensorCluster005, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalConfigSensorCluster005 start.");

    LevelItem item;
    item.threshold = 1;
    item.level = 999;
    std::vector<LevelItem> levItems;
    levItems.push_back(item);
    levItems.push_back(item);
    int32_t curTemp = 1;
    uint32_t level = 0;
    ThermalConfigSensorCluster cluster;
    cluster.AscJudgment(levItems, curTemp, level);
    EXPECT_EQ(level, item.level);

    THERMAL_HILOGD(LABEL_TEST, "ThermalConfigSensorCluster005 end.");
}

/**
 * @tc.name: ThermalConfigSensorCluster006
 * @tc.desc: test DescJudgment if branch
 * @tc.type: FUNC
 */
HWTEST_F(ThermalServiceTest, ThermalConfigSensorCluster006, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalConfigSensorCluster006 start.");
    // inner if branch (curTemp <= nextUptemp)
    LevelItem item1;
    item1.threshold = 1;
    LevelItem item2;
    item2.threshold = 1;
    item2.level = 999;
    std::vector<LevelItem> levItems;
    levItems.push_back(item1);
    levItems.push_back(item2);
    int32_t curTemp = 1;
    uint32_t level = 1;
    ThermalConfigSensorCluster cluster;
    cluster.DescJudgment(levItems, curTemp, level);
    EXPECT_EQ(level, item2.level);

    const int32_t INDEX0 = 0;
    const int32_t INDEX1 = 1;
    // inner else if branch (curTemp > curDownTemp)
    levItems.at(INDEX0).thresholdClr = 0;
    levItems.at(INDEX0).level = 999;
    levItems.at(INDEX1).threshold = 0;
    levItems.at(INDEX1).thresholdClr = 0;
    level = 1;
    cluster.DescJudgment(levItems, curTemp, level);
    EXPECT_EQ(level, levItems.at(INDEX0).level - 1);
    THERMAL_HILOGD(LABEL_TEST, "ThermalConfigSensorCluster006 end.");
}

/**
 * @tc.name: ThermalConfigSensorCluster007
 * @tc.desc: test DescJudgment else if branch
 * @tc.type: FUNC
 */
HWTEST_F(ThermalServiceTest, ThermalConfigSensorCluster007, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalConfigSensorCluster007 start.");

    LevelItem item;
    item.thresholdClr = 2;
    item.level = 999;
    std::vector<LevelItem> levItems;
    levItems.push_back(item);
    int32_t curTemp = 3;
    uint32_t level = 1;
    ThermalConfigSensorCluster cluster;
    cluster.DescJudgment(levItems, curTemp, level);
    EXPECT_EQ(level, item.level - 1);

    THERMAL_HILOGD(LABEL_TEST, "ThermalConfigSensorCluster007 end.");
}

/**
 * @tc.name: ThermalConfigSensorCluster008
 * @tc.desc: test DescJudgment else branch
 * @tc.type: FUNC
 */
HWTEST_F(ThermalServiceTest, ThermalConfigSensorCluster008, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalConfigSensorCluster008 start.");

    LevelItem item;
    item.threshold = 2;
    item.level = 999;
    std::vector<LevelItem> levItems;
    levItems.push_back(item);
    levItems.push_back(item);
    int32_t curTemp = 2;
    uint32_t level = 0;
    ThermalConfigSensorCluster cluster;
    cluster.DescJudgment(levItems, curTemp, level);
    EXPECT_EQ(level, item.level);

    THERMAL_HILOGD(LABEL_TEST, "ThermalConfigSensorCluster008 end.");
}

/**
 * @tc.name: ThermalConfigSensorCluster009
 * @tc.desc: test IsAuxSensorTrigger
 * @tc.type: FUNC
 */
HWTEST_F(ThermalServiceTest, ThermalConfigSensorCluster009, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalConfigSensorCluster009 start.");

    TypeTempMap typeTempInfo;
    uint32_t level = 0;
    ThermalConfigSensorCluster cluster;
    // Returns true when level = 0
    EXPECT_TRUE(cluster.IsAuxSensorTrigger(typeTempInfo, level));

    // No matching item is found
    std::vector<AuxLevelItem> auxLevel;
    cluster.auxSensorInfolist_["test1"] = auxLevel;
    level = 1;
    EXPECT_TRUE(cluster.IsAuxSensorTrigger(typeTempInfo, level));
    EXPECT_EQ(level, 1);

    AuxLevelItem item;
    item.lowerTemp = 1;
    item.upperTemp = 1;
    auxLevel.push_back(item);
    cluster.auxSensorInfolist_["test1"] = auxLevel;
    cluster.auxSensorInfolist_["test"] = auxLevel;
    cluster.auxSensorInfolist_["test2"] = auxLevel;
    typeTempInfo["test"] = 1;  // The range is lowerTemp and upperTemp
    typeTempInfo["test2"] = 5; // The range is not lowerTemp or upperTemp
    level = 1;
    EXPECT_FALSE(cluster.IsAuxSensorTrigger(typeTempInfo, level));
    EXPECT_EQ(level, 0);

    THERMAL_HILOGD(LABEL_TEST, "ThermalConfigSensorCluster009 end.");
}

/**
 * @tc.name: ThermalConfigSensorCluster010
 * @tc.desc: test IsTempRateTrigger rateMap and sensorInfolist_ No match
 * @tc.type: FUNC
 */
HWTEST_F(ThermalServiceTest, ThermalConfigSensorCluster010, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalConfigSensorCluster010 start.");
    g_service->RegisterThermalHdiCallback();
    ThermalConfigSensorCluster cluster;
    std::vector<LevelItem> vecLevel;
    cluster.sensorInfolist_["test"] = vecLevel;
    uint32_t level = 2;
    EXPECT_TRUE(cluster.IsTempRateTrigger(level));

    // continue
    auto& rateMap = g_service->serviceSubscriber_->sensorsRateMap_;
    rateMap["test"] = 3.14;
    LevelItem item1;
    item1.level = 1;
    LevelItem item2;
    item2.level = 2;
    item2.tempRiseRate = 2.14;
    vecLevel.push_back(item1);
    vecLevel.push_back(item2);
    cluster.sensorInfolist_["test"] = vecLevel;
    EXPECT_TRUE(cluster.IsTempRateTrigger(level));

    // false is returned if the condition is not met
    vecLevel.clear();
    item1.level = 2;
    item1.tempRiseRate = 4.14;
    vecLevel.push_back(item1);
    cluster.sensorInfolist_["test"] = vecLevel;
    EXPECT_FALSE(cluster.IsTempRateTrigger(level));
    EXPECT_EQ(level, 0);
    THERMAL_HILOGD(LABEL_TEST, "ThermalConfigSensorCluster010 end.");
}

/**
 * @tc.name: ThermalPolicy001
 * @tc.desc: test GetClusterLevelMap
 * @tc.type: FUNC
 * @tc.require: issueI6KRS8
 */
HWTEST_F(ThermalServiceTest, ThermalPolicy001, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalPolicy001 start.");
    ThermalPolicy policy;
    EXPECT_TRUE(policy.GetClusterLevelMap().empty());
    THERMAL_HILOGD(LABEL_TEST, "ThermalPolicy001 end.");
}

/**
 * @tc.name: ThermalPolicy002
 * @tc.desc: test PolicyDecision continue in a loop
 * @tc.type: FUNC
 * @tc.require: issueI6KRS8
 */
HWTEST_F(ThermalServiceTest, ThermalPolicy002, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalPolicy002 start.");
    ThermalPolicy policy;
    EXPECT_TRUE(policy.Init());
    std::vector<PolicyConfig> vecConfig;
    policy.clusterPolicyMap_[""] = vecConfig;
    policy.clusterPolicyMap_["test"] = vecConfig;
    PolicyConfig config;
    vecConfig.push_back(config);
    policy.clusterPolicyMap_["test1"] = vecConfig;
    policy.PolicyDecision();

    // ActionExecution Let the function return false
    auto mgrTmp = g_service->actionMgr_;
    g_service->actionMgr_ = nullptr;
    policy.PolicyDecision();
    g_service->actionMgr_ = mgrTmp;
    EXPECT_TRUE(g_service->actionMgr_ != nullptr);
    THERMAL_HILOGD(LABEL_TEST, "ThermalPolicy002 end.");
}

/**
 * @tc.name: ThermalPolicy003
 * @tc.desc: test ActionDecision continue in a loop
 * @tc.type: FUNC
 * @tc.require: issueI6KRS8
 */
HWTEST_F(ThermalServiceTest, ThermalPolicy003, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalPolicy003 start.");
    // No matching item
    ThermalPolicy policy;
    std::vector<PolicyAction> actionList;
    PolicyAction action;
    action.actionName = "test";
    actionList.push_back(action);
    EXPECT_FALSE(actionList.empty());
    policy.ActionDecision(actionList);

    // second is nullptr
    auto& actionMap = g_service->actionMgr_->actionMap_;
    actionMap["test"] = nullptr;
    policy.ActionDecision(actionList);
    EXPECT_FALSE(actionMap.empty());
    THERMAL_HILOGD(LABEL_TEST, "ThermalPolicy003 end.");
}

/**
 * @tc.name: ThermalPolicy004
 * @tc.desc: test ActionDecision Execute the if else branch
 * @tc.type: FUNC
 * @tc.require: issueI6KRS8
 */
HWTEST_F(ThermalServiceTest, ThermalPolicy004, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalPolicy004 start.");
    ThermalPolicy policy;
    std::vector<PolicyAction> actionList;
    PolicyAction action1;
    action1.actionName = "test";
    action1.isProp = true;
    PolicyAction action2;
    action2.actionName = "test1";
    action1.isProp = false;
    PolicyAction action3;
    action2.actionName = "test2";
    action1.isProp = true;
    actionList.push_back(action1);
    actionList.push_back(action2);
    actionList.push_back(action3);
    EXPECT_FALSE(actionList.empty());

    auto& actionMap = g_service->actionMgr_->actionMap_;
    actionMap["test"] = std::make_shared<ActionCpuBig>(CPU_BIG_ACTION_NAME);
    actionMap["test1"] = std::make_shared<ActionCpuBig>(CPU_BIG_ACTION_NAME);
    actionMap["test2"] = std::make_shared<ActionCpuBig>(CPU_BIG_ACTION_NAME);
    policy.ActionDecision(actionList);
    EXPECT_FALSE(actionMap.empty());
    THERMAL_HILOGD(LABEL_TEST, "ThermalPolicy004 end.");
}

/**
 * @tc.name: ThermalPolicy005
 * @tc.desc: test StateMachineDecision
 * @tc.type: FUNC
 */
HWTEST_F(ThermalServiceTest, ThermalPolicy005, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalPolicy005 start.");
    // No match
    ThermalPolicy policy;
    g_service->observer_ = nullptr;
    policy.FindSubscribeActionValue();

    std::map<std::string, std::string> stateMap;
    stateMap["test"] = "test";
    EXPECT_FALSE(policy.StateMachineDecision(stateMap));

    // The second term is nullptr
    auto& collectionMap = g_service->state_->stateCollectionMap_;
    collectionMap["test"] = nullptr;
    EXPECT_FALSE(policy.StateMachineDecision(stateMap));

    // for loop return false
    collectionMap["test"] = std::make_shared<ScreenStateCollection>();
    EXPECT_FALSE(policy.StateMachineDecision(stateMap));

    // for loop continue or retrun true
    stateMap["test"] = "1";
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    if (powerMgrClient.IsScreenOn()) {
        EXPECT_TRUE(policy.StateMachineDecision(stateMap));
    } else {
        EXPECT_FALSE(policy.StateMachineDecision(stateMap));
    }
    stateMap["test1"] = "0";
    collectionMap["test1"] = std::make_shared<ScreenStateCollection>();
    EXPECT_FALSE(policy.StateMachineDecision(stateMap));
    THERMAL_HILOGD(LABEL_TEST, "ThermalPolicy005 end.");
}
} // namespace
