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

#include "action_shutdown.h"

#include <map>

#include "constants.h"
#include "power_mgr_client.h"
#include "file_operation.h"
#include "thermal_hisysevent.h"
#include "thermal_service.h"
#include "ffrt_utils.h"
#include "securec.h"

using namespace OHOS::PowerMgr;
using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace PowerMgr {
namespace {
constexpr const char* SHUTDOWN_REASON = "DeviceTempTooHigh";
constexpr const char* SHUTDOWN_PATH = "/data/service/el0/thermal/config/shut_down";
FFRTQueue g_queue("thermal_action_shutdown");
FFRTHandle g_shutdownTaskHandle;
const int MAX_PATH = 256;
}

ActionShutdown::ActionShutdown(const std::string& actionName)
{
    actionName_ = actionName;
}

void ActionShutdown::InitParams(const std::string& params)
{
    (void)params;
}

void ActionShutdown::SetStrict(bool enable)
{
    isStrict_ = enable;
}

void ActionShutdown::SetEnableEvent(bool enable)
{
    enableEvent_ = enable;
}

void ActionShutdown::AddActionValue(std::string value)
{
    if (value.empty()) {
        return;
    }
    valueList_.push_back(static_cast<uint32_t>(strtol(value.c_str(), nullptr, STRTOL_FORMART_DEC)));
}

void ActionShutdown::Execute()
{
    auto tms = ThermalService::GetInstance();
    THERMAL_RETURN_IF (tms == nullptr);
    uint32_t value = GetActionValue();
    if (value != lastValue_) {
        if (tms->GetSimulationXml()) {
            ShutdownExecution(static_cast<bool>(value));
        } else {
            ShutdownRequest(static_cast<bool>(value));
        }
        WriteActionTriggeredHiSysEvent(enableEvent_, actionName_, value);
        tms->GetObserver()->SetDecisionValue(actionName_, std::to_string(value));
        lastValue_ = value;
        THERMAL_HILOGD(COMP_SVC, "action execute: {%{public}s = %{public}u}", actionName_.c_str(), lastValue_);
    }
    valueList_.clear();
}

uint32_t ActionShutdown::GetActionValue()
{
    uint32_t value = FALLBACK_VALUE_UINT_ZERO;
    if (!valueList_.empty()) {
        if (isStrict_) {
            value = *min_element(valueList_.begin(), valueList_.end());
        } else {
            value = *max_element(valueList_.begin(), valueList_.end());
        }
    }
    return value;
}

uint32_t ActionShutdown::ShutdownRequest(bool isShutdown)
{
    if (isShutdown) {
        THERMAL_HILOGI(COMP_SVC, "device start shutdown");
        PowerMgrClient::GetInstance().ShutDownDevice(SHUTDOWN_REASON);
    }
    return ERR_OK;
}

int32_t ActionShutdown::ShutdownExecution(bool isShutdown)
{
    int32_t ret = -1;
    char shutdownBuf[MAX_PATH] = {0};
    ret = snprintf_s(shutdownBuf, MAX_PATH, sizeof(shutdownBuf) - 1, SHUTDOWN_PATH);
    if (ret < EOK) {
        return ret;
    }
    std::string valueString = std::to_string(isShutdown) + "\n";
    ret = FileOperation::WriteFile(shutdownBuf, valueString, valueString.length());
    if (ret != ERR_OK) {
        return ret;
    }
    return ERR_OK;
}

uint32_t ActionShutdown::DelayShutdown(bool isShutdown, int32_t temp, int32_t thresholdClr)
{
    if (g_shutdownTaskHandle && temp < thresholdClr) {
        THERMAL_HILOGI(COMP_SVC, "shutdown canceled");
        FFRTUtils::CancelTask(g_shutdownTaskHandle, g_queue);
        return ERR_OK;
    }

    uint32_t delay = 50000;
    FFRTTask shutdownTask = [&]() {
        THERMAL_HILOGI(COMP_SVC, "shutdown start");
        ShutdownRequest(isShutdown);
    };
    g_shutdownTaskHandle = FFRTUtils::SubmitDelayTask(shutdownTask, delay, g_queue);
    THERMAL_HILOGI(COMP_SVC, "shutdown device after %{public}u ms", delay);
    return ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS
