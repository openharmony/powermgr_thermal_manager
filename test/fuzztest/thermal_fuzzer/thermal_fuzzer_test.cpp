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

#include "thermal_service.h"
#include "thermal_common.h"
#include "message_parcel.h"
#include "securec.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::PowerMgr;
namespace {
auto& g_thermalMgrClient = ThermalMgrClient::GetInstance();
const int32_t REWIND_READ_DATA = 0;
sptr<ThermalService> g_service = nullptr;
} // namespace

static sptr<IThermalActionCallback> actionCb_;
static sptr<IThermalActionCallback> testActionCb_;
static sptr<IThermalTempCallback> tempCb_ = new ThermalFuzzerTest::ThermalTempTestCallback();
static sptr<IThermalTempCallback> testTempCb_;
static sptr<IThermalLevelCallback> levelCb_ = new ThermalFuzzerTest::ThermalLevelTestCallback();
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

static std::vector<std::string> GetVector(const uint8_t* data, size_t size)
{
    int32_t inputData;
    std::vector<std::string> list;
    if (size < sizeof(inputData)) {
        return list;
    }
    if (memcpy_s(&inputData, sizeof(inputData), data, sizeof(inputData)) != EOK) {
        return list;
    }
    int32_t minVectorSize = 0;
    int32_t maxVectorSize = 5000;
    int32_t length = (inputData < minVectorSize) ? minVectorSize : inputData;
    length = (length > maxVectorSize) ? maxVectorSize : length;
    list.resize(length);
    for (auto &item : list) {
        item = std::string(reinterpret_cast<const char *>(data), size);
    }
    return list;
}

static void TestUnSubscribeTemp()
{
    g_thermalMgrClient.UnSubscribeThermalTempCallback(tempCb_);
    g_thermalMgrClient.UnSubscribeThermalTempCallback(testTempCb_);
}

static void TestSubscribeTemp(const uint8_t* data, size_t size)
{
    std::vector<std::string> typeList = GetVector(data, size);
    g_thermalMgrClient.SubscribeThermalTempCallback(typeList, tempCb_);
    g_thermalMgrClient.SubscribeThermalTempCallback(typeList, testTempCb_);
    TestUnSubscribeTemp();
}

static void TestUnSubscribeAction()
{
    g_thermalMgrClient.UnSubscribeThermalActionCallback(actionCb_);
    g_thermalMgrClient.UnSubscribeThermalActionCallback(testActionCb_);
}

static void TestSubscribeAction(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }
    std::string desc(reinterpret_cast<const char *>(data), size);
    std::vector<std::string> actionList = GetVector(data, size);
    g_thermalMgrClient.SubscribeThermalActionCallback(actionList, desc, actionCb_);
    g_thermalMgrClient.SubscribeThermalActionCallback(actionList, desc, testActionCb_);
    TestUnSubscribeAction();
}

static void TestUnSubscribeLevel()
{
    g_thermalMgrClient.UnSubscribeThermalLevelCallback(levelCb_);
}

static void TestSubscribeLevel([[maybe_unused]] const uint8_t* data, [[maybe_unused]] size_t size)
{
    g_thermalMgrClient.SubscribeThermalLevelCallback(levelCb_);
    TestUnSubscribeLevel();
}

static void TestGetLevel([[maybe_unused]] const uint8_t* data, [[maybe_unused]] size_t size)
{
    g_thermalMgrClient.GetThermalLevel();
}

static void TestSetScene(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }
    std::string sceneName(reinterpret_cast<const char *>(data), size);
    g_thermalMgrClient.SetScene(sceneName);
}

static void TestGetSensorTemp(const uint8_t* data, size_t size)
{
    int32_t code = 0;
    if (size < sizeof(code)) {
        return;
    }
    if (memcpy_s(&code, sizeof(code), data, sizeof(code)) != EOK) {
        return;
    }

    SensorType sensorType = static_cast<SensorType>(code);
    g_thermalMgrClient.GetThermalSensorTemp(sensorType);
}

static void ThermalServiceStub(const uint8_t* data, size_t size)
{
    uint32_t code;
    if (size < sizeof(code)) {
        return;
    }
    if (memcpy_s(&code, sizeof(code), data, sizeof(code)) != EOK) {
        return;
    }

    MessageParcel datas;
    datas.WriteInterfaceToken(ThermalService::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(REWIND_READ_DATA);
    MessageParcel reply;
    MessageOption option;
    if (g_service == nullptr) {
        g_service = DelayedSpSingleton<ThermalService>::GetInstance();
        g_service->OnStart();
    }
    g_service->OnRemoteRequest(code, datas, reply, option);
}

static std::vector<std::function<void(const uint8_t*, size_t)>> fuzzFunc = {
    &TestSubscribeTemp,
    &TestSubscribeLevel,
    &TestGetLevel,
    &TestGetSensorTemp,
    &TestSubscribeAction,
    &TestSetScene
};

static bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    std::random_device rd;
    std::default_random_engine engine(rd());
    std::uniform_int_distribution<int32_t> randomNum(0, fuzzFunc.size() - 1);
    int32_t number = randomNum(engine);
    fuzzFunc[number](data, size);
    ThermalServiceStub(data, size);
    return true;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
