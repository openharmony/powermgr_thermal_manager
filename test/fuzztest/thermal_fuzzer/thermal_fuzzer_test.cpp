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

/* This files contains faultlog fuzzer test modules. */

#include "thermal_fuzzer_test.h"

#include "thermal_common.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::PowerMgr;
namespace {
auto& g_thermalMgrClient = ThermalMgrClient::GetInstance();
constexpr int32_t WAIT_TIME = 1000;
} // namespace

static sptr<IThermalActionCallback> actionCb_;
static sptr<IThermalActionCallback> testActionCb_;
static sptr<IThermalTempCallback> tempCb_;
static sptr<IThermalTempCallback> testTempCb_;
static sptr<IThermalLevelCallback> levelCb_;
static sptr<IThermalLevelCallback> testLevelCb_;

bool ThermalFuzzerTest::ThermalActionTestCallback::OnThermalActionChanged(ActionCallbackMap& actionCbMap)
{
    THERMAL_HILOGI(COMP_SVC, "action callback");
    return true;
}

bool ThermalFuzzerTest::ThermalTempTestCallback::OnThermalTempChanged(TempCallbackMap& tempCbMap)
{
    THERMAL_HILOGI(COMP_SVC, "temp callback");
    return true;
}

bool ThermalFuzzerTest::ThermalLevelTestCallback::GetThermalLevel(ThermalLevel level)
{
    THERMAL_HILOGI(COMP_SVC, "level is: %{public}d", static_cast<int32_t>(level));
    return true;
}

void ThermalFuzzerTest::TestSubscribeTemp(const uint8_t* data)
{
    const int32_t NUMBER_FOUR = 4;
    int32_t type[1];
    int32_t idSize = 4;
    std::vector<std::string> typeList;
    for (int32_t i = 0; i < NUMBER_FOUR; i++) {
        if (memcpy_s(type, sizeof(type), data, idSize) != EOK) {
            return;
        }
        typeList.push_back(to_string(type[0]));
    }
    g_thermalMgrClient.SubscribeThermalTempCallback(typeList, tempCb_);
    g_thermalMgrClient.SubscribeThermalTempCallback(typeList, testTempCb_);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    TestUnSubscribeTemp();
}

void ThermalFuzzerTest::TestUnSubscribeTemp()
{
    g_thermalMgrClient.UnSubscribeThermalTempCallback(tempCb_);
    g_thermalMgrClient.UnSubscribeThermalTempCallback(testTempCb_);
}

void ThermalFuzzerTest::TestSubscribeAction(const uint8_t* data)
{
    std::string desc = "";
    const int32_t NUMBER_FOUR = 4;
    int32_t type[1];
    int32_t idSize = 4;
    std::vector<std::string> actionList;
    for (int32_t i = 0; i < NUMBER_FOUR; i++) {
        if (memcpy_s(type, sizeof(type), data, idSize) != EOK) {
            return;
        }
        actionList.push_back(to_string(type[0]));
    }
    g_thermalMgrClient.SubscribeThermalActionCallback(actionList, desc, actionCb_);
    g_thermalMgrClient.SubscribeThermalActionCallback(actionList, desc, testActionCb_);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    TestUnSubscribeAction();
}

void ThermalFuzzerTest::TestUnSubscribeAction()
{
    g_thermalMgrClient.UnSubscribeThermalActionCallback(actionCb_);
    g_thermalMgrClient.UnSubscribeThermalActionCallback(testActionCb_);
}

void ThermalFuzzerTest::TestSubscribeLevel(const uint8_t* data)
{
    g_thermalMgrClient.SubscribeThermalLevelCallback(levelCb_);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    TestUnSubscribeLevel();
}

void ThermalFuzzerTest::TestUnSubscribeLevel()
{
    g_thermalMgrClient.UnSubscribeThermalLevelCallback(levelCb_);
}

void ThermalFuzzerTest::TestGetLevel()
{
    ThermalLevel level = g_thermalMgrClient.GetThermalLevel();
    cout << "Thermal level is: " << static_cast<int32_t>(level) << endl;
}

void ThermalFuzzerTest::TestSetScene(const uint8_t* data)
{
    int32_t type[1];
    int32_t idSize = 4;
    if (memcpy_s(type, sizeof(type), data, idSize) != EOK) {
        return;
    }

    g_thermalMgrClient.SetScene(to_string(type[0]));
}

void ThermalFuzzerTest::TestGetSensorTemp(const uint8_t* data)
{
    int32_t type[1];
    int32_t idSize = 4;
    if (memcpy_s(type, sizeof(type), data, idSize) != EOK) {
        return;
    }

    SensorType sensorType = static_cast<SensorType>(type[0]);
    int32_t temp = g_thermalMgrClient.GetThermalSensorTemp(sensorType);
    cout << "Sensor temp is: " << temp << endl;
}

bool ThermalFuzzerTest::DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    int32_t idSize = 4;
    int32_t cond[1];
    if (static_cast<int32_t>(size) > idSize) {
        if (memcpy_s(cond, sizeof(cond), data, idSize) != EOK) {
            return false;
        }

        tempCb_ = new ThermalTempTestCallback();
        levelCb_ = new ThermalLevelTestCallback();
        std::random_device rd;
        std::default_random_engine engine(rd());
        std::uniform_int_distribution<int32_t> randomNum(
            static_cast<int32_t>(ApiNumber::NUM_ZERO), static_cast<int32_t>(ApiNumber::NUM_END) - 1);
        cond[0] = randomNum(engine);

        switch (static_cast<ApiNumber>(cond[0])) {
            case ApiNumber::NUM_ZERO:
                TestSubscribeTemp(data);
                break;
            case ApiNumber::NUM_ONE:
                TestSubscribeLevel(data);
                break;
            case ApiNumber::NUM_TWO:
                TestGetLevel();
                break;
            case ApiNumber::NUM_THREE:
                TestGetSensorTemp(data);
                break;
            case ApiNumber::NUM_FOUR:
                TestSubscribeAction(data);
                break;
            case ApiNumber::NUM_FIVE:
                TestSetScene(data);
                break;
            default:
                break;
        }
    }
    return true;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::PowerMgr::ThermalFuzzerTest::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
