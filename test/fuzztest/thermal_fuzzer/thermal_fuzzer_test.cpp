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
auto &g_thermalMgrClient = ThermalMgrClient::GetInstance();
constexpr int32_t MIN = 0;
constexpr int32_t MAX = 3;
constexpr int32_t WAIT_TIME = 1000;
}

static sptr<IThermalTempCallback> tempCb_;
static sptr<IThermalTempCallback> testTempCb_;
static sptr<IThermalLevelCallback> levelCb_;
static sptr<IThermalLevelCallback> testLevelCb_;

void ThermalFuzzerTest::ThermalTempTestCallback::OnThermalTempChanged(TempCallbackMap &tempCbMap)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "temp callback");
}

void ThermalFuzzerTest::ThermalLevelTestCallback::GetThermalLevel(ThermalLevel level)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "level is: %{public}d", static_cast<int32_t>(level));
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

void ThermalFuzzerTest::TestSubscribeLevel(const uint8_t* data)
{
    g_thermalMgrClient.SubscribeThermalLevelCallback(levelCb_);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    TestUnSubscribeTemp();
}

void ThermalFuzzerTest::TestUnSubscribeLevel()
{
    g_thermalMgrClient.UnSubscribeThermalLevelCallback(levelCb_);
}

void ThermalFuzzerTest::TestGetLevel()
{
    ThermalLevel level = g_thermalMgrClient.GetThermalLevel();
    cout << "Thermal level is: " << static_cast<int32_t>(level)  << endl;
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
    int32_t idSize = 8;
    int32_t cond[1];
    if (static_cast<int32_t>(size) > idSize) {
        if (memcpy_s(cond, sizeof(cond), data, idSize) != EOK) {
            return false;
        }

        tempCb_ = new ThermalTempTestCallback();
        levelCb_ = new ThermalLevelTestCallback();
        std::random_device rd;
        std::default_random_engine engine(rd());
        std::uniform_int_distribution<int32_t> randomNum(MIN, MAX);
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