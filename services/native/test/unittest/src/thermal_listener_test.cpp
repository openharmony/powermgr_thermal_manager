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

#include "thermal_listener_test.h"

#include "securec.h"

#include "file_operation.h"
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

int32_t ThermalListenerTest::WriteFile(std::string path, std::string buf, size_t size)
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

void ThermalListenerTest::SetSensorTemp(int32_t temperature, const std::string& path)
{
    char tempBuf[MAX_PATH] = {0};
    int32_t ret = snprintf_s(tempBuf, MAX_PATH, sizeof(tempBuf) - 1, path.c_str());
    EXPECT_EQ(true, ret >= EOK);

    std::string sTemp = to_string(temperature) + "\n";
    ret = ThermalListenerTest::WriteFile(tempBuf, sTemp, sTemp.length());
    EXPECT_EQ(true, ret == ERR_OK);
}

void ThermalListenerTest::ThermalLevelTestEvent::OnThermalLevelResult(const ThermalLevel& level)
{
    THERMAL_HILOGD(LABEL_TEST, "OnThermalLevelResult Enter");
    g_thermalLevel = level;
}

void ThermalListenerTest::TearDown()
{
    SetSensorTemp(0, BATTERY_PATH);
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
    THERMAL_HILOGD(LABEL_TEST, "ThermalListenerTest001 start.");
    std::shared_ptr<ThermalMgrListener> thermalListener = std::make_shared<ThermalMgrListener>();
    ASSERT_NE(thermalListener, nullptr);
    const std::shared_ptr<ThermalMgrListener::ThermalLevelEvent> thermalEvent =
        std::make_shared<ThermalListenerTest::ThermalLevelTestEvent>();
    ASSERT_NE(thermalEvent, nullptr);

    int32_t ret = thermalListener->SubscribeLevelEvent(thermalEvent);
    EXPECT_EQ(true, ret == ERR_OK);

    g_thermalLevel = ThermalLevel::COOL;
    SetSensorTemp(46100, BATTERY_PATH);
    MockThermalMgrClient::GetInstance().GetThermalInfo();
    EXPECT_EQ(true, g_thermalLevel == ThermalLevel::HOT);
    thermalListener->UnSubscribeLevelEvent();

    ThermalLevel levelListener = thermalListener->GetThermalLevel();
    ThermalLevel levelClient = ThermalMgrClient::GetInstance().GetThermalLevel();
    EXPECT_EQ(levelListener, levelClient);
    THERMAL_HILOGD(LABEL_TEST, "ThermalListenerTest001 end.");
}

/**
 * @tc.name: ThermalListenerTest002
 * @tc.desc: test class ThermalMgrListener function(thermal level event is nullptr)
 * @tc.type: FUNC
 * @tc.require: issueI63SZ4
 */
HWTEST_F(ThermalListenerTest, ThermalListenerTest002, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalListenerTest002 start.");
    std::shared_ptr<ThermalMgrListener> thermalListener = std::make_shared<ThermalMgrListener>();
    ASSERT_NE(thermalListener, nullptr);

    int32_t ret = thermalListener->SubscribeLevelEvent(nullptr);
    EXPECT_EQ(true, ret == ERR_INVALID_VALUE);
    THERMAL_HILOGD(LABEL_TEST, "ThermalListenerTest002 end.");
}
} // namespace
