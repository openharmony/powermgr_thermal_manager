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

#include "soc_action_base.h"

#include "mock_socperf_action.h"

namespace OHOS {
namespace PowerMgr {
void SocActionBase::SocLimitRequest(int32_t tag, int64_t value)
{
    MockSocPerfAction::LimitRequest(tag, value);
}

void SocActionBase::SocPerfRequest(bool onOffTag)
{
    (void)onOffTag;
    MockSocPerfAction::BoostRequest();
}
} // namespace PowerMgr
} // namespace OHOS