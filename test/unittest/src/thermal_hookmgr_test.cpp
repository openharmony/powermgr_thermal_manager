/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <list.h>
#include "thermal_hookmgr.h"
#include <thermal_log.h>
#include <gtest/gtest.h>

using namespace OHOS;
using namespace PowerMgr;
using namespace testing;
using namespace ext;
namespace {
class ThermalHookMgrTest : public Test {
public:
    inline static HOOK_INFO info {};
};

int TestHook(const HOOK_INFO* info, void* context)
{
    ThermalHookMgrTest::info = *info;
    return *static_cast<int*>(context);
}

HWTEST_F(ThermalHookMgrTest, ThermalHookMgrTest000, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalHookMgrTest000 function start!");
    auto hookMgr = GetThermalHookMgr();
    ASSERT_NE(hookMgr, nullptr);
    auto hookMgr2 = GetThermalHookMgr();
    EXPECT_EQ(hookMgr, hookMgr2);
    THERMAL_HILOGI(LABEL_TEST, "ThermalHookMgrTest000 function end!");
}

// execution with TRAVERSE_STOP_WHEN_ERROR
HWTEST_F(ThermalHookMgrTest, ThermalHookMgrTest001, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalHookMgrTest001 function start!");
    auto hookMgr = GetThermalHookMgr();
    ASSERT_NE(hookMgr, nullptr);
    HOOK_EXEC_OPTIONS options {TRAVERSE_STOP_WHEN_ERROR, nullptr, nullptr};
    HookMgrAdd(hookMgr, 0, 0, TestHook);
    for (int retval : {0, -1, 1}) {
        int ret = HookMgrExecute(hookMgr, 0, &retval, &options);
        EXPECT_EQ(info.stage, 0);
        EXPECT_EQ(info.prio, 0);
        EXPECT_EQ(info.hook, TestHook);
        EXPECT_EQ(ret, retval);
    }
    THERMAL_HILOGI(LABEL_TEST, "ThermalHookMgrTest001 function end!");
}

// task with greater priority number is executed later.
HWTEST_F(ThermalHookMgrTest, ThermalHookMgrTest002, TestSize.Level0)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalHookMgrTest001 function start!");
    auto hookMgr = GetThermalHookMgr();
    ASSERT_NE(hookMgr, nullptr);
    HookMgrAdd(hookMgr, 0, 0, TestHook);
    HookMgrAdd(hookMgr, 0, 1, TestHook);
    int retval = 0;
    HookMgrExecute(hookMgr, 0, &retval, nullptr);
    EXPECT_EQ(info.prio, 1);
    THERMAL_HILOGI(LABEL_TEST, "ThermalHookMgrTest001 function end!");
}
} // namespace