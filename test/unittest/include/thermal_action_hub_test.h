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

#ifndef THERMAL_ACTION_HUB_TEST_H
#define THERMAL_ACTION_HUB_TEST_H

#include <gtest/gtest.h>

#include "ithermal_action_callback.h"
#include "thermal_action_callback_stub.h"
#include "thermal_test.h"


namespace OHOS {
namespace PowerMgr {
class ThermalActionHubTest : public testing::Test, public ThermalTest {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void TearDown();
    void InitData();

    class ThermalActionTest1Callback : public ThermalActionCallbackStub {
    public:
        ThermalActionTest1Callback() {};
        virtual ~ThermalActionTest1Callback() {};
        virtual bool OnThermalActionChanged(ActionCallbackMap& actionCbMap) override;
    };

    class ThermalActionTest2Callback : public ThermalActionCallbackStub {
    public:
        ThermalActionTest2Callback() {};
        virtual ~ThermalActionTest2Callback() {};
        virtual bool OnThermalActionChanged(ActionCallbackMap& actionCbMap) override;
    };

    class ThermalActionTest3Callback : public ThermalActionCallbackStub {
    public:
        ThermalActionTest3Callback() {};
        virtual ~ThermalActionTest3Callback() {};
        virtual bool OnThermalActionChanged(ActionCallbackMap& actionCbMap) override;
    };

    class ThermalActionTest4Callback : public ThermalActionCallbackStub {
    public:
        ThermalActionTest4Callback() {};
        virtual ~ThermalActionTest4Callback() {};
        virtual bool OnThermalActionChanged(ActionCallbackMap& actionCbMap) override;
    };

    class ThermalActionTest5Callback : public ThermalActionCallbackStub {
    public:
        ThermalActionTest5Callback() {};
        virtual ~ThermalActionTest5Callback() {};
        virtual bool OnThermalActionChanged(ActionCallbackMap& actionCbMap) override;
    };

    class ThermalActionTest6Callback : public ThermalActionCallbackStub {
    public:
        ThermalActionTest6Callback() {};
        virtual ~ThermalActionTest6Callback() {};
        virtual bool OnThermalActionChanged(ActionCallbackMap& actionCbMap) override;
    };

    class ThermalActionTest7Callback : public ThermalActionCallbackStub {
    public:
        ThermalActionTest7Callback() = default;
        virtual ~ThermalActionTest7Callback() = default;
        virtual bool OnThermalActionChanged(ActionCallbackMap& actionCbMap) override;
    };

    class ThermalActionTest8Callback : public ThermalActionCallbackStub {
    public:
        ThermalActionTest8Callback() = default;
        virtual ~ThermalActionTest8Callback() = default;
        virtual bool OnThermalActionChanged(ActionCallbackMap& actionCbMap) override;
    };
};
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_ACTION_HUB_TEST_H
