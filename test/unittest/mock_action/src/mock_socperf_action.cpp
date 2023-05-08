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

#include "mock_socperf_action.h"

namespace OHOS {
namespace PowerMgr {
std::map<int32_t, int64_t> MockSocPerfAction::limitMap_ = {};
uint32_t MockSocPerfAction::boostCounter_ = 0;
constexpr int64_t INVALID_VALUE = -1;

void MockSocPerfAction::LimitRequest(int32_t tag, int64_t value)
{
    auto it = limitMap_.find(tag);
    if (it == limitMap_.end()) {
        limitMap_.emplace(tag, value);
        return;
    }
    it->second = value;
}

int64_t MockSocPerfAction::GetLimitValue(int32_t tag)
{
    auto it = limitMap_.find(tag);
    if (it != limitMap_.end()) {
        return it->second;
    }
    return INVALID_VALUE;
}

void MockSocPerfAction::ClearLimit()
{
    limitMap_.clear();
}

void MockSocPerfAction::BoostRequest()
{
    boostCounter_++;
}

uint32_t MockSocPerfAction::GetBoostRequestCounter()
{
    return boostCounter_;
}

void MockSocPerfAction::ClearBoost()
{
    boostCounter_ = 0;
}
} // namespace Power
} // namespace OHOS
