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

#include <gtest/gtest.h>

#include "ohos.thermal.proj.hpp"
#include "ohos.thermal.impl.hpp"
#include "ohos.thermal.user.hpp"
#include "taihe/runtime.hpp"
#include "thermal_mgr_client.h"
#include "thermal_log.h"
#include "thermal_level_callback.h"

using namespace taihe;
using namespace ohos::thermal;
using namespace OHOS::PowerMgr;

using namespace testing;
using namespace testing::ext;

namespace {
bool g_pass = false;
bool g_flag = false;
}

namespace taihe {
void set_business_error(int32_t err_code, taihe::string_view msg)
{
    (void)err_code;
    (void)msg;
}
}

namespace OHOS::PowerMgr {
bool ThermalMgrClient::SubscribeThermalLevelCallback(const sptr<IThermalLevelCallback>& callback)
{
    g_pass = true;
    return true;
}

bool ThermalMgrClient::UnSubscribeThermalLevelCallback(const sptr<IThermalLevelCallback>& callback)
{
    g_pass = true;
    return true;
}

ThermalLevel ThermalMgrClient::GetThermalLevel()
{
    g_pass = true;
    ThermalLevel level = ThermalLevel::COOL;
    return level;
}
} // namespace OHOS::PowerMgr

namespace {
class ThermalTaiheNativeTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown()
    {
        g_pass = false;
        g_flag = false;
    }
};

/**
 * @tc.name: ThermalTaiheNativeTest_001
 * @tc.desc: test thermal taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICAK9Z
 */
HWTEST_F(ThermalTaiheNativeTest, ThermalTaiheNativeTest_001, TestSize.Level1)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalTaiheNativeTest_001 start");
    struct MyCallbackImpl {
        void operator()(ohos::thermal::ThermalLevel level)
        {
            g_flag = true;
        }
    };
    callback<void(ohos::thermal::ThermalLevel)> thermalCb =
        ::taihe::make_holder<MyCallbackImpl, ::taihe::callback<void(::ohos::thermal::ThermalLevel)>>();
    RegisterThermalLevelCallback(thermalCb);

    std::shared_ptr<ThermalLevelCallback> cb = std::make_shared<ThermalLevelCallback>();
    cb->UpdateCallback(thermalCb);
    OHOS::PowerMgr::ThermalLevel level = OHOS::PowerMgr::ThermalLevel::COOL;
    bool ret = cb->OnThermalLevelChanged(level);
    EXPECT_TRUE(g_flag);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(g_pass);
    THERMAL_HILOGI(LABEL_TEST, "ThermalTaiheNativeTest_001 end");
}

/**
 * @tc.name: ThermalTaiheNativeTest_002
 * @tc.desc: test thermal taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICAK9Z
 */
HWTEST_F(ThermalTaiheNativeTest, ThermalTaiheNativeTest_002, TestSize.Level1)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalTaiheNativeTest_002 start");
    optional<callback<void(MyUndefined const&)>> thermalCb;
    UnregisterThermalLevelCallback(thermalCb);

    struct MyCallbackImpl {
        void operator()(MyUndefined const& level)
        {
            g_flag = true;
        }
    };
    ::taihe::callback<void(::ohos::thermal::MyUndefined const&)>* thermalCbPtr = nullptr;
    callback<void(MyUndefined const&)> undefCb =
        ::taihe::make_holder<MyCallbackImpl, ::taihe::callback<void(MyUndefined const&)>>();
    thermalCbPtr = new ::taihe::callback<void(::ohos::thermal::MyUndefined const&)>(std::move(undefCb));
    optional<callback<void(MyUndefined const&)>> notNullThermalCb(thermalCbPtr);
    UnregisterThermalLevelCallback(notNullThermalCb);
    EXPECT_TRUE(g_flag);
    EXPECT_TRUE(g_pass);
    THERMAL_HILOGI(LABEL_TEST, "ThermalTaiheNativeTest_002 end");
}

/**
 * @tc.name: ThermalTaiheNativeTest_003
 * @tc.desc: test thermal taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICAK9Z
 */
HWTEST_F(ThermalTaiheNativeTest, ThermalTaiheNativeTest_003, TestSize.Level1)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalTaiheNativeTest_003 start");
    ohos::thermal::ThermalLevel level = GetLevel();
    ohos::thermal::ThermalLevel expectLevel(ohos::thermal::ThermalLevel::key_t::COOL);
    EXPECT_TRUE(level == expectLevel);
    EXPECT_TRUE(g_pass);
    THERMAL_HILOGI(LABEL_TEST, "ThermalTaiheNativeTest_003 end");
}

/**
 * @tc.name: ThermalTaiheNativeTest_004
 * @tc.desc: test thermal taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICAK9Z
 */
HWTEST_F(ThermalTaiheNativeTest, ThermalTaiheNativeTest_004, TestSize.Level1)
{
    THERMAL_HILOGI(LABEL_TEST, "ThermalTaiheNativeTest_004 start");
    std::shared_ptr<ThermalLevelCallback> cb = std::make_shared<ThermalLevelCallback>();
    OHOS::PowerMgr::ThermalLevel level = OHOS::PowerMgr::ThermalLevel::COOL;
    bool ret = cb->OnThermalLevelChanged(level);
    EXPECT_TRUE(ret);
    EXPECT_FALSE(g_flag);
    THERMAL_HILOGI(LABEL_TEST, "ThermalTaiheNativeTest_004 end");
}
} // namespace