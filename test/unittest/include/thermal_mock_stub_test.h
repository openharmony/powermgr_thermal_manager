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

#ifndef THERMAL_MOCK_STUB_TEST_H
#define THERMAL_MOCK_STUB_TEST_H

#include <gtest/gtest.h>

namespace OHOS {
namespace PowerMgr {
class ThermalMockStubTest : public testing::Test {
public:
    static void SetUpTestCase();
protected:
    const int VECTOR_MAX_SIZE = 102400;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_MOCK_STUB_TEST_H
