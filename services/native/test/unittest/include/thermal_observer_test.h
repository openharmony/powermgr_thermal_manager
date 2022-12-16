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

#ifndef THERMAL_OBSERVER_TEST_H
#define THERMAL_OBSERVER_TEST_H

#include <gtest/gtest.h>
#include <common_event_manager.h>
#include "thermal_action_callback_stub.h"
#include "thermal_temp_callback_stub.h"

namespace OHOS {
namespace PowerMgr {
class ThermalObserverTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    void HandleScreenOnCompleted(const EventFwk::CommonEventData& data __attribute__((__unused__)));
    class ThermalTempTestCallback : public ThermalTempCallbackStub {
    public:
        ThermalTempTestCallback() {};
        virtual ~ThermalTempTestCallback() {};
        virtual bool OnThermalTempChanged(TempCallbackMap& tempCbMap) override;
    };
    class ThermalActionTestCallback : public ThermalActionCallbackStub {
    public:
        ThermalActionTestCallback() {};
        virtual ~ThermalActionTestCallback() {};
        virtual bool OnThermalActionChanged(ActionCallbackMap& actionCbMap) override;
    };
};
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_OBSERVER_TEST_H
