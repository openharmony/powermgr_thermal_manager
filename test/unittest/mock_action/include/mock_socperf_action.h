/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef MOCK_SOCPERF_ACTION_H
#define MOCK_SOCPERF_ACTION_H

#include <cstdint>
#include <map>

namespace OHOS {
namespace PowerMgr {
class MockSocPerfAction {
public:
    static void LimitRequest(int32_t tag, int64_t value);
    static int64_t GetLimitValue(int32_t tag);
    static void ClearLimit();
    static void BoostRequest();
    static uint32_t GetBoostRequestCounter();
    static void ClearBoost();

private:
    static std::map<int32_t, int64_t> limitMap_;
    static uint32_t boostCounter_;
};
} // namespace Power
} // namespace OHOS
#endif // MOCK_SOCPERF_ACTION_H
