/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "thermal_config_sensor_cluster_test.h"

#ifdef THERMAL_GTEST
#define private   public
#define protected public
#define final
#endif

#include <map>
#include <string>
#include <vector>

#include "config_policy_utils.h"
#include "power_mgr_client.h"
#include "thermal_config_sensor_cluster.h"
#include "thermal_log.h"
#include "thermal_service.h"

using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace testing::ext;
using namespace std;

namespace {
sptr<ThermalService> g_service = nullptr;
} // namespace

char* GetOneCfgFile(const char *pathSuffix, char *buf, unsigned int bufLength)
{
    THERMAL_HILOGI(LABEL_TEST, "mock GetOneCfgFile.");
    return nullptr;
}

void ThermalConfigSensorClusterTest::SetUpTestCase()
{
    g_service = ThermalService::GetInstance();
    g_service->InitSystemTestModules();
    g_service->OnStart();
}

namespace {
/**
 * @tc.name: ThermalConfigSensorClusterTest001
 * @tc.desc: test CheckStandard
 * @tc.type: FUNC
 */
HWTEST_F(ThermalConfigSensorClusterTest, ThermalConfigSensorClusterTest001, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalConfigSensorClusterTest001 function start!");

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

    THERMAL_HILOGI(LABEL_TEST, "ThermalConfigSensorClusterTest001 function end!");
}

/**
 * @tc.name: ThermalConfigSensorClusterTest002
 * @tc.desc: test UpdateThermalLevel
 * @tc.type: FUNC
 */
HWTEST_F(ThermalConfigSensorClusterTest, ThermalConfigSensorClusterTest002, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalConfigSensorClusterTest002 function start!");

    // Null data return
    ThermalConfigSensorCluster cluster;
    TypeTempMap typeTempInfo;
    cluster.UpdateThermalLevel(typeTempInfo);
    EXPECT_NE(cluster.latestLevel_, 1);

    THERMAL_HILOGI(LABEL_TEST, "ThermalConfigSensorClusterTest002 function end!");
}

/**
 * @tc.name: ThermalConfigSensorClusterTest003
 * @tc.desc: test AscJudgment if branch
 * @tc.type: FUNC
 */
HWTEST_F(ThermalConfigSensorClusterTest, ThermalConfigSensorClusterTest003, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalConfigSensorClusterTest003 function start!");
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
    TypeTempMap type;
    ThermalConfigSensorCluster cluster;
    std::string sensorType = "test";
    cluster.AscJudgment(levItems1, curTemp, level, type, sensorType);
    EXPECT_EQ(level, item2.level);

    const int32_t INDEX0 = 0;
    const int32_t INDEX1 = 1;
    // The break branch in the for loop
    levItems1.at(INDEX1).threshold = 3;
    level = 1;
    cluster.AscJudgment(levItems1, curTemp, level, type, sensorType);
    EXPECT_NE(level, item2.level);

    // inner else if branch (curTemp < thresholdClr)
    levItems1.at(INDEX0).thresholdClr = 2;
    levItems1.at(INDEX0).level = 999;
    levItems1.at(INDEX1).threshold = 2;
    levItems1.at(INDEX1).thresholdClr = 2;
    level = 1;
    cluster.AscJudgment(levItems1, curTemp, level, type, sensorType);
    EXPECT_EQ(level, levItems1.at(INDEX0).level - 1);
    THERMAL_HILOGI(LABEL_TEST, "ThermalConfigSensorClusterTest003 function end!");
}

/**
 * @tc.name: ThermalConfigSensorClusterTest004
 * @tc.desc: test AscJudgment else if branch
 * @tc.type: FUNC
 */
HWTEST_F(ThermalConfigSensorClusterTest, ThermalConfigSensorClusterTest004, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalConfigSensorClusterTest004 function start!");

    LevelItem item;
    item.thresholdClr = 2;
    item.level = 999;
    std::vector<LevelItem> levItems;
    levItems.push_back(item);
    int32_t curTemp = 1;
    uint32_t level = 1;
    TypeTempMap type;
    ThermalConfigSensorCluster cluster;
    std::string sensorType = "test";
    cluster.AscJudgment(levItems, curTemp, level, type, sensorType);
    EXPECT_EQ(level, item.level - 1);

    THERMAL_HILOGI(LABEL_TEST, "ThermalConfigSensorClusterTest004 function end!");
}

/**
 * @tc.name: ThermalConfigSensorClusterTest005
 * @tc.desc: test AscJudgment else branch
 * @tc.type: FUNC
 */
HWTEST_F(ThermalConfigSensorClusterTest, ThermalConfigSensorClusterTest005, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalConfigSensorClusterTest005 function start!");

    LevelItem item;
    item.threshold = 1;
    item.level = 999;
    std::vector<LevelItem> levItems;
    levItems.push_back(item);
    levItems.push_back(item);
    int32_t curTemp = 1;
    uint32_t level = 0;
    TypeTempMap type;
    ThermalConfigSensorCluster cluster;
    std::string sensorType = "test";
    cluster.AscJudgment(levItems, curTemp, level, type, sensorType);
    EXPECT_EQ(level, item.level);

    THERMAL_HILOGI(LABEL_TEST, "ThermalConfigSensorClusterTest005 function end!");
}

/**
 * @tc.name: ThermalConThermalConfigSensorClusterTest006
 * @tc.desc: test DescJudgment if branch
 * @tc.type: FUNC
 */
HWTEST_F(ThermalConfigSensorClusterTest, ThermalConfigSensorClusterTest006, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalConfigSensorClusterTest006 function start!");
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
    TypeTempMap type;
    ThermalConfigSensorCluster cluster;
    std::string sensorType = "test";
    cluster.DescJudgment(levItems, curTemp, level, type, sensorType);
    EXPECT_EQ(level, item2.level);

    const int32_t INDEX0 = 0;
    const int32_t INDEX1 = 1;
    // inner else if branch (curTemp > curDownTemp)
    levItems.at(INDEX0).thresholdClr = 0;
    levItems.at(INDEX0).level = 999;
    levItems.at(INDEX1).threshold = 0;
    levItems.at(INDEX1).thresholdClr = 0;
    level = 1;
    cluster.DescJudgment(levItems, curTemp, level, type, sensorType);
    EXPECT_EQ(level, levItems.at(INDEX0).level - 1);
    THERMAL_HILOGI(LABEL_TEST, "ThermalConfigSensorClusterTest006 function end!");
}

/**
 * @tc.name: ThermalConfigSensorClusterTest007
 * @tc.desc: test DescJudgment else if branch
 * @tc.type: FUNC
 */
HWTEST_F(ThermalConfigSensorClusterTest, ThermalConfigSensorClusterTest007, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalConfigSensorClusterTest007 function start!");

    LevelItem item;
    item.thresholdClr = 2;
    item.level = 999;
    std::vector<LevelItem> levItems;
    levItems.push_back(item);
    int32_t curTemp = 3;
    uint32_t level = 1;
    TypeTempMap type;
    ThermalConfigSensorCluster cluster;
    std::string sensorType = "test";
    cluster.DescJudgment(levItems, curTemp, level, type, sensorType);
    EXPECT_EQ(level, item.level - 1);

    THERMAL_HILOGI(LABEL_TEST, "ThermalConfigSensorClusterTest007 function end!");
}

/**
 * @tc.name: ThermalConfigSensorClusterTest008
 * @tc.desc: test DescJudgment else branch
 * @tc.type: FUNC
 */
HWTEST_F(ThermalConfigSensorClusterTest, ThermalConfigSensorClusterTest008, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalConfigSensorClusterTest008 function start!");

    LevelItem item;
    item.threshold = 2;
    item.level = 999;
    std::vector<LevelItem> levItems;
    levItems.push_back(item);
    levItems.push_back(item);
    int32_t curTemp = 2;
    uint32_t level = 0;
    TypeTempMap type;
    ThermalConfigSensorCluster cluster;
    std::string sensorType = "test";
    cluster.DescJudgment(levItems, curTemp, level, type, sensorType);
    EXPECT_EQ(level, item.level);

    THERMAL_HILOGI(LABEL_TEST, "ThermalConfigSensorClusterTest008 function end!");
}

/**
 * @tc.name: ThermalConfigSensorClusterTest009
 * @tc.desc: test IsAuxSensorTrigger
 * @tc.type: FUNC
 */
HWTEST_F(ThermalConfigSensorClusterTest, ThermalConfigSensorClusterTest009, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalConfigSensorClusterTest009 function start!");

    TypeTempMap typeTempInfo;
    uint32_t level = 0;
    ThermalConfigSensorCluster cluster;
    // Returns true when level = 0
    EXPECT_TRUE(cluster.IsAuxSensorTrigger(typeTempInfo, level));

    // No matching item is found
    std::vector<AuxLevelItem> auxLevel;
    cluster.auxSensorInfolist_["test1"] = auxLevel;
    level = 1;
    EXPECT_FALSE(cluster.IsAuxSensorTrigger(typeTempInfo, level));

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

    THERMAL_HILOGI(LABEL_TEST, "ThermalConfigSensorClusterTest009 function end!");
}

/**
 * @tc.name: ThermalConfigSensorClusterTest010
 * @tc.desc: test IsTempRateTrigger rateMap and sensorInfolist_ No match
 * @tc.type: FUNC
 */
HWTEST_F(ThermalConfigSensorClusterTest, ThermalConfigSensorClusterTest010, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalConfigSensorClusterTest010 function start!");
    g_service->RegisterThermalHdiCallback();
    ThermalConfigSensorCluster cluster;
    std::vector<LevelItem> vecLevel;
    uint32_t level = 0;
    std::string sensorType = "test";
    bool isCritical = true;
    EXPECT_TRUE(cluster.IsTempRateTrigger(level, sensorType, vecLevel, isCritical));

    // false is returned if rate list not full
    level = 2;
    auto& rateMap = g_service->serviceSubscriber_->sensorsRateMap_;
    rateMap["test"].push_back(3.14);
    LevelItem item1;
    item1.level = 1;
    LevelItem item2;
    item2.level = 2;
    item2.tempRiseRate = 2.14;
    vecLevel.push_back(item1);
    vecLevel.push_back(item2);
    EXPECT_FALSE(cluster.IsTempRateTrigger(level, sensorType, vecLevel, isCritical));

    // rate list is full
    rateMap["test"].push_back(3.04);
    rateMap["test"].push_back(2.94);
    EXPECT_TRUE(cluster.IsTempRateTrigger(level, sensorType, vecLevel, isCritical));

    // false is returned if not all critical
    rateMap["test"].clear();
    rateMap["test"].push_back(3.14);
    rateMap["test"].push_back(2.14);
    rateMap["test"].push_back(1.14);
    EXPECT_FALSE(cluster.IsTempRateTrigger(level, sensorType, vecLevel, isCritical));

    // false is returned if the type is not found
    sensorType = "test2";
    EXPECT_FALSE(cluster.IsTempRateTrigger(level, sensorType, vecLevel, isCritical));

    // not critical
    sensorType = "test";
    isCritical = false;
    rateMap["test"].clear();
    rateMap["test"].push_back(1.14);
    rateMap["test"].push_back(1.04);
    rateMap["test"].push_back(0.94);
    EXPECT_TRUE(cluster.IsTempRateTrigger(level, sensorType, vecLevel, isCritical));
    THERMAL_HILOGI(LABEL_TEST, "ThermalConfigSensorClusterTest010 function end!");
}
 
/**
 * @tc.name: ThermalConfigSensorClusterTest011
 * @tc.desc: test IsTempDiffTrigger
 * @tc.type: FUNC
*/
HWTEST_F(ThermalConfigSensorClusterTest, ThermalConfigSensorClusterTest011, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalConfigSensorClusterTest011 function start!");

    TypeTempMap typeTempInfo;
    
    uint32_t level = 0;
    ThermalConfigSensorCluster cluster;
    EXPECT_TRUE(cluster.IsTempDiffTrigger(typeTempInfo, level));

    // illegal level parameter
    level = INT_MAX;
    EXPECT_FALSE(cluster.IsTempDiffTrigger(typeTempInfo, level));
    level = -1;
    EXPECT_FALSE(cluster.IsTempDiffTrigger(typeTempInfo, level));

    TempDiffInfoList tempDiffInfoList;
    TempDiffItem tdItem1, tdItem2;

    tdItem1.sensor1 = "test1";
    tdItem1.sensor2 = "test2";
    tdItem1.tempDiff = 5000;
    tdItem1.level = 1;

    tdItem2.sensor1 = "test1";
    tdItem2.sensor2 = "test2";
    tdItem2.tempDiff = 900000;
    tdItem2.level = 2;

    tempDiffInfoList.push_back(tdItem2);
    tempDiffInfoList.push_back(tdItem1);
    cluster.SetTempDiffInfo(tempDiffInfoList);
    cluster.SetTempDiffFlag(true);

    // no sensor test2 info
    level = 1;
    typeTempInfo["test1"] = 41000;
    EXPECT_FALSE(cluster.IsTempDiffTrigger(typeTempInfo, level));
    EXPECT_EQ(level, 1);

    // greater than level 1 diff temperature 5000
    level = 1;
    typeTempInfo["test2"] = 35000;
    EXPECT_FALSE(cluster.IsTempDiffTrigger(typeTempInfo, level));
    EXPECT_EQ(level, 1);

    // less than or equal to level 1 diff temperature 5000
    level = 1;
    typeTempInfo["test2"] = 36000;
    EXPECT_TRUE(cluster.IsTempDiffTrigger(typeTempInfo, level));
    EXPECT_EQ(level, 1);

    level = 2;
    EXPECT_TRUE(cluster.IsTempDiffTrigger(typeTempInfo, level));
    EXPECT_EQ(level, 2);
    std::string sensorType = "test";
    bool isCritical = true;
    std::vector<LevelItem> vecLevel;
    cluster.CheckExtraCondition(typeTempInfo, level, sensorType, vecLevel, isCritical);

    THERMAL_HILOGI(LABEL_TEST, "ThermalConfigSensorClusterTest011 function end!");
}
} // namespace