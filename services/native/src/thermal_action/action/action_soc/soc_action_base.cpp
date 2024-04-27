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

#include "soc_action_base.h"

#ifdef SOC_PERF_ENABLE
#include "socperf_client.h"
#endif

namespace OHOS {
namespace PowerMgr {
namespace {
#ifdef SOC_PERF_ENABLE
constexpr int32_t ACTION_TYPE_THERMAL_ID = 2;
constexpr int32_t ACTION_TYPE_CPU_ISOLATE_CMDID = 12361;
#endif
}

void SocActionBase::SocLimitRequest(int32_t tag, int64_t value)
{
#ifdef SOC_PERF_ENABLE
    std::vector<int32_t> tags;
    std::vector<int64_t> configs;
    tags.push_back(tag);
    configs.push_back(value);
    OHOS::SOCPERF::SocPerfClient::GetInstance().LimitRequest(ACTION_TYPE_THERMAL_ID, tags, configs, "");
#endif
}

void SocActionBase::SetSocPerfThermalLevel(uint32_t level)
{
#ifdef SOC_PERF_ENABLE
    OHOS::SOCPERF::SocPerfClient::GetInstance().SetThermalLevel(level);
#endif
}

void SocActionBase::SocIsolateRequest(bool enable)
{
#ifdef SOC_PERF_ENABLE
    OHOS::SOCPERF::SocPerfClient::GetInstance().PerfRequestEx(ACTION_TYPE_CPU_ISOLATE_CMDID, enable, "");
#endif
}
} // namespace PowerMgr
} // namespace OHOS
