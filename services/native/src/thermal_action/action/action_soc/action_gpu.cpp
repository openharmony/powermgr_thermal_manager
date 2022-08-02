/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "action_gpu.h"

#include <map>
#include <unistd.h>
#include "socperf_client.h"
#include "thermal_hisysevent.h"
#include "thermal_service.h"

namespace OHOS {
namespace PowerMgr {
namespace {
constexpr int32_t LIM_GPU_ID = 2001;
constexpr int32_t ACTION_TYPE_THERMAL_ID = 2;
auto g_service = DelayedSpSingleton<ThermalService>::GetInstance();
std::map<std::string, std::string> g_sceneMap;
}
ActionGpu::ActionGpu(const std::string& actionName)
{
    actionName_ = actionName;
}

void ActionGpu::InitParams(const std::string& params)
{
}

void ActionGpu::SetStrict(bool flag)
{
    flag_ = flag;
}

void ActionGpu::SetEnableEvent(bool enable)
{
    enableEvent_ = enable;
}

void ActionGpu::SetXmlScene(const std::string& scene, const std::string& value)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    for (auto iter = g_sceneMap.begin(); iter != g_sceneMap.end(); ++iter) {
        if (iter->first == scene) {
            if (iter->second != value) {
                iter->second = value;
            }
            return;
        }
    }

    g_sceneMap.insert(std::make_pair(scene, value));
}

void ActionGpu::AddActionValue(std::string value)
{
    valueList_.push_back(atoi(value.c_str()));
}

void ActionGpu::Execute()
{
    uint32_t value = lastValue_;
    if (valueList_.empty()) {
        value = 0;
    } else {
        if (flag_) {
            value = *max_element(valueList_.begin(), valueList_.end());
        } else {
            value = *min_element(valueList_.begin(), valueList_.end());
        }
        valueList_.clear();
    }

    THERMAL_HILOGD(COMP_SVC, "Enter value=%{public}d, lastValue_=%{public}d", value, lastValue_);
    if (value != lastValue_) {
        GpuRequest(value);
        WriteActionTriggeredHiSysEvent(enableEvent_, actionName_, value);
        lastValue_ = value;
    }
}

int32_t ActionGpu::GpuRequest(uint32_t freq)
{
    std::vector<int32_t> tags;
    std::vector<int32_t> configs;

    if (!g_service->GetSimulationXml()) {
        tags.push_back(LIM_GPU_ID);
        configs.push_back(freq);
        OHOS::SOCPERF::SocPerfClient::GetInstance().LimitRequest(ACTION_TYPE_THERMAL_ID, tags, configs, "");
    } else {
        auto thermalInterface = g_service->GetThermalInterface();
        if (thermalInterface != nullptr) {
            int32_t ret = thermalInterface->SetGpuFreq(freq);
            if (ret != ERR_OK) {
                return ret;
            }
        }
    }

    return ERR_OK;
}
} // namespace PowermMgr
} // namespace OHOS
