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

#include "thermal_policy_test.h"

#ifdef THERMAL_GTEST
#define private   public
#define protected public
#define final
#endif

#include <map>
#include <string>
#include <vector>

#include "action_cpu_big.h"
#include "config_policy_utils.h"
#include "modulemgr.h"
#include "power_mgr_client.h"
#include "screen_state_collection.h"
#include "thermal_log.h"
#include "thermal_policy.h"
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

void ThermalPolicyTest::SetUpTestCase()
{
    g_service = ThermalService::GetInstance();
    g_service->InitSystemTestModules();
    g_service->OnStart();
}

namespace {
/**
 * @tc.name: ThermalPolicyTest001
 * @tc.desc: test GetClusterLevelMap
 * @tc.type: FUNC
 * @tc.require: issueI6KRS8
 */
HWTEST_F(ThermalPolicyTest, ThermalPolicyTest001, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalPolicyTest001 function start!");
    ThermalPolicy policy;
    EXPECT_TRUE(policy.GetClusterLevelMap().empty());
    THERMAL_HILOGI(LABEL_TEST, "ThermalPolicyTest001 function end!");
}

/**
 * @tc.name: ThermalPolicyTest002
 * @tc.desc: test PolicyDecision continue in a loop
 * @tc.type: FUNC
 * @tc.require: issueI6KRS8
 */
HWTEST_F(ThermalPolicyTest, ThermalPolicyTest002, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalPolicyTest002 function start!");
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
    THERMAL_HILOGI(LABEL_TEST, "ThermalPolicyTest002 function end!");
}

/**
 * @tc.name: ThermalPolicyTest003
 * @tc.desc: test ActionDecision continue in a loop
 * @tc.type: FUNC
 * @tc.require: issueI6KRS8
 */
HWTEST_F(ThermalPolicyTest, ThermalPolicyTest003, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalPolicyTest003 function start!");
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
    THERMAL_HILOGI(LABEL_TEST, "ThermalPolicyTest003 function end!");
}

/**
 * @tc.name: ThermalPolicyTest004
 * @tc.desc: test ActionDecision Execute the if else branch
 * @tc.type: FUNC
 * @tc.require: issueI6KRS8
 */
HWTEST_F(ThermalPolicyTest, ThermalPolicyTest004, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalPolicyTest004 function start!");
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
    THERMAL_HILOGI(LABEL_TEST, "ThermalPolicyTest004 function end!");
}

/**
 * @tc.name: ThermalPolicyTest005
 * @tc.desc: test StateMachineDecision
 * @tc.type: FUNC
 */
HWTEST_F(ThermalPolicyTest, ThermalPolicyTest005, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalPolicyTest005 function start!");
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
    THERMAL_HILOGI(LABEL_TEST, "ThermalPolicyTest005 function end!");
}

/**
 * @tc.name: ThermalPolicyTest006
 * @tc.desc: test DecryptConfig
 * @tc.type: FUNC
 */
HWTEST_F (ThermalPolicyTest, ThermalPolicyTest006, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalPolicyTest006 function start!");
    string path = "test/path";
    string result = "";
#if (defined(__aarch64__) || defined(__x86_64__))
    constexpr const char* THERMAL_PLUGIN_AUTORUN_PATH = "/system/lib64/thermalplugin/autorun";
#else
    constexpr const char* THERMAL_PLUGIN_AUTORUN_PATH = "/system/lib/thermalplugin/autorun";
#endif
    ModuleMgrScan(THERMAL_PLUGIN_AUTORUN_PATH);
    bool ret = g_service->GetConfigParser().DecryptConfig(path, result);
    if (result.empty()) {
        EXPECT_FALSE(ret);
    } else {
        EXPECT_TRUE(ret);
    }
    THERMAL_HILOGI(LABEL_TEST, "ThermalPolicyTest006 function end!");
}
} // namespace
