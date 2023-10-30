/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef THERMAL_MGR_INTERFACE_TEST_H
#define THERMAL_MGR_INTERFACE_TEST_H

#include <gtest/gtest.h>

#include "ithermal_level_callback.h"
#include "ithermal_temp_callback.h"
#include "thermal_level_callback_stub.h"
#include "thermal_temp_callback_stub.h"
#include "thermal_test.h"

namespace OHOS {
namespace PowerMgr {
class ThermalMgrInterfaceTest : public testing::Test, public ThermalTest {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void TearDown();
    void InitData();

    class ThermalTempTest1Callback : public ThermalTempCallbackStub {
    public:
        ThermalTempTest1Callback() {};
        virtual ~ThermalTempTest1Callback() {};
        virtual bool OnThermalTempChanged(TempCallbackMap& tempCbMap) override;
    };

    class ThermalTempTest2Callback : public ThermalTempCallbackStub {
    public:
        ThermalTempTest2Callback() {};
        virtual ~ThermalTempTest2Callback() {};
        virtual bool OnThermalTempChanged(TempCallbackMap& tempCbMap) override;
    };

    class ThermalLevelTest1Callback : public ThermalLevelCallbackStub {
    public:
        ThermalLevelTest1Callback() {};
        virtual ~ThermalLevelTest1Callback() {};
        virtual bool OnThermalLevelChanged(ThermalLevel level) override;
    };

    class ThermalLevelTest2Callback : public ThermalLevelCallbackStub {
    public:
        ThermalLevelTest2Callback() {};
        virtual ~ThermalLevelTest2Callback() {};
        virtual bool OnThermalLevelChanged(ThermalLevel level) override;
    };

    class ThermalLevelTest3Callback : public ThermalLevelCallbackStub {
    public:
        ThermalLevelTest3Callback() {};
        virtual ~ThermalLevelTest3Callback() {};
        virtual bool OnThermalLevelChanged(ThermalLevel level) override;
    };

    class ThermalLevelTest4Callback : public ThermalLevelCallbackStub {
    public:
        ThermalLevelTest4Callback() {};
        virtual ~ThermalLevelTest4Callback() {};
        virtual bool OnThermalLevelChanged(ThermalLevel level) override;
    };

    class ThermalLevelTest5Callback : public ThermalLevelCallbackStub {
    public:
        ThermalLevelTest5Callback() {};
        virtual ~ThermalLevelTest5Callback() {};
        virtual bool OnThermalLevelChanged(ThermalLevel level) override;
    };
};
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_MGR_INTERFACE_TEST_H
