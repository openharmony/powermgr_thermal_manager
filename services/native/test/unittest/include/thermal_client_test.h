/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef THERMAL_CLIENT_TEST_H
#define THERMAL_CLIENT_TEST_H

#include <gtest/gtest.h>
#include "thermal_mgr_client.h"
#include "thermal_temp_callback_stub.h"
#include "thermal_level_callback_stub.h"
#include "thermal_service.h"
#include "thermal_level_info.h"

namespace OHOS {
namespace PowerMgr {
constexpr int WAIR_TIME = 10;
constexpr int WAIR_TIME_MS = 300;
class ThermalClientTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    class ThermalObserverTest1Callback : public ThermalTempCallbackStub {
    public:
        ThermalObserverTest1Callback() {};
        virtual ~ThermalObserverTest1Callback() {};
        virtual void OnThermalTempChanged(int32_t temp) override;
    };
    class ThermalLevelTest1Callback : public ThermalLevelCallbackStub {
    public:
        ThermalLevelTest1Callback() {};
        virtual ~ThermalLevelTest1Callback() {};
        virtual void GetThermalLevel(ThermalLevel level) override;
    };
    class ThermalLevelTest2Callback : public ThermalLevelCallbackStub {
    public:
        ThermalLevelTest2Callback() {};
        virtual ~ThermalLevelTest2Callback() {};
        virtual void GetThermalLevel(ThermalLevel level) override;
    };
    class ThermalLevelTest3Callback : public ThermalLevelCallbackStub {
    public:
        ThermalLevelTest3Callback() {};
        virtual ~ThermalLevelTest3Callback() {};
        virtual void GetThermalLevel(ThermalLevel level) override;
    };
};
}
}
#endif // THERMAL_CLIENT_TEST_H