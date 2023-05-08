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

#include "thermal_listener_test.h"

#include "mock_thermal_mgr_client.h"
#include "thermal_log.h"
#include "thermal_mgr_client.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
static ThermalLevel g_thermalLevel = ThermalLevel::COOL;
}

void ThermalListenerTest::ThermalLevelTestEvent::OnThermalLevelResult(const ThermalLevel& level)
{
    THERMAL_HILOGD(LABEL_TEST, "OnThermalLevelResult Enter");
    g_thermalLevel = level;
}

void ThermalListenerTest::TearDown()
{
    InitNode();
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    thermalMgrClient.SetScene("");
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    g_thermalLevel = ThermalLevel::COOL;
}

namespace {
/**
 * @tc.name: ThermalListenerTest001
 * @tc.desc: test class ThermalMgrListener function(thermal level event is not nullptr)
 * @tc.type: FUNC
 * @tc.require: issueI63SZ4
 */
HWTEST_F(ThermalListenerTest, ThermalListenerTest001, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalListenerTest001 start");
    if (!IsMock(BATTERY_PATH) || IsVendor()) {
        return;
    }
    std::shared_ptr<ThermalMgrListener> thermalListener = std::make_shared<ThermalMgrListener>();
    ASSERT_NE(thermalListener, nullptr);
    const std::shared_ptr<ThermalMgrListener::ThermalLevelEvent> thermalEvent =
        std::make_shared<ThermalListenerTest::ThermalLevelTestEvent>();
    ASSERT_NE(thermalEvent, nullptr);

    int32_t ret = thermalListener->SubscribeLevelEvent(thermalEvent);
    EXPECT_EQ(true, ret == ERR_OK);

    g_thermalLevel = ThermalLevel::COOL;
    SetNodeValue(46100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    EXPECT_TRUE(g_thermalLevel == ThermalLevel::HOT);
    thermalListener->UnSubscribeLevelEvent();

    ThermalLevel levelListener = thermalListener->GetThermalLevel();
    ThermalLevel levelClient = ThermalMgrClient::GetInstance().GetThermalLevel();
    EXPECT_EQ(levelListener, levelClient);
    THERMAL_HILOGD(LABEL_TEST, "ThermalListenerTest001 end");
}

/**
 * @tc.name: ThermalListenerTest002
 * @tc.desc: test class ThermalMgrListener function(thermal level event is nullptr)
 * @tc.type: FUNC
 * @tc.require: issueI63SZ4
 */
HWTEST_F(ThermalListenerTest, ThermalListenerTest002, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalListenerTest002 start");
    std::shared_ptr<ThermalMgrListener> thermalListener = std::make_shared<ThermalMgrListener>();
    ASSERT_NE(thermalListener, nullptr);

    int32_t ret = thermalListener->SubscribeLevelEvent(nullptr);
    EXPECT_EQ(true, ret == ERR_INVALID_VALUE);
    THERMAL_HILOGD(LABEL_TEST, "ThermalListenerTest002 end");
}
} // namespace
