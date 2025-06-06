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

#ifndef THERMAL_LISTENER_TEST_H
#define THERMAL_LISTENER_TEST_H

#include <gtest/gtest.h>

#include "thermal_mgr_listener.h"
#include "thermal_test.h"

namespace OHOS {
namespace PowerMgr {
class ThermalListenerTest : public testing::Test, public ThermalTest {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void TearDown();

    class ThermalLevelTestEvent : public ThermalMgrListener::ThermalLevelEvent {
    public:
        ThermalLevelTestEvent() = default;
        virtual ~ThermalLevelTestEvent() = default;
        virtual void OnThermalLevelResult(const ThermalLevel& level) override;
    };
};
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_LISTENER_TEST_H
