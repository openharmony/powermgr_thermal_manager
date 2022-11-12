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

#include "action_shutdown.h"

#include <map>
#include <thread>
#include "event_runner.h"
#include "event_handler.h"
#include "event_queue.h"
#include "power_mgr_client.h"
#include "file_operation.h"
#include "thermal_hisysevent.h"
#include "thermal_service.h"
#include "securec.h"

using namespace OHOS::PowerMgr;
using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace PowerMgr {
namespace {
constexpr const char* SHUTDOWN_REASON = "DeviceTempTooHigh";
auto g_service = DelayedSpSingleton<ThermalService>::GetInstance();
constexpr const char* SHUTDOWN_PATH = "/data/service/el0/thermal/config/shut_down";
const int MAX_PATH = 256;
}
ActionShutdown::ActionShutdown(const std::string& actionName)
{
    actionName_ = actionName;
}

void ActionShutdown::InitParams(const std::string& params)
{
}

void ActionShutdown::SetStrict(bool flag)
{
    flag_ = flag;
}

void ActionShutdown::SetEnableEvent(bool enable)
{
    enableEvent_ = enable;
}

void ActionShutdown::AddActionValue(std::string value)
{
    THERMAL_HILOGD(COMP_SVC, "value=%{public}s", value.c_str());
    valuesList_.push_back(atoi(value.c_str()));
}

void ActionShutdown::Execute()
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    int32_t value;
    THERMAL_RETURN_IF (g_service == nullptr);
    std::string scene = g_service->GetScene();
    auto iter = g_sceneMap.find(scene);
    if (iter != g_sceneMap.end()) {
        value = static_cast<int32_t>(atoi(iter->second.c_str()));
        if ((value != lastValue_) && (!g_service->GetSimulationXml())) {
            ShutdownRequest(static_cast<bool>(value));
        } else if (value != lastValue_) {
            ShutdownExecution(static_cast<bool>(value));
        } else {
            THERMAL_HILOGD(COMP_SVC, "value is not change");
        }
        WriteActionTriggeredHiSysEvent(enableEvent_, actionName_, value);
        g_service->GetObserver()->SetDecisionValue(actionName_, iter->second);
        lastValue_ = value;
        valuesList_.clear();
        return;
    }

    if (valuesList_.empty()) {
        value = 0;
    } else {
        if (flag_) {
            value = *max_element(valuesList_.begin(), valuesList_.end());
        } else {
            value = *min_element(valuesList_.begin(), valuesList_.end());
        }
        valuesList_.clear();
    }

    if (value != lastValue_) {
        if (!g_service->GetFlag()) {
            ShutdownExecution(static_cast<bool>(value));
        } else {
            ShutdownRequest(static_cast<bool>(value));
        }
        WriteActionTriggeredHiSysEvent(enableEvent_, actionName_, value);
        g_service->GetObserver()->SetDecisionValue(actionName_, std::to_string(value));
        lastValue_ = value;
    }
}

uint32_t ActionShutdown::ShutdownRequest(bool isShutdown)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    if (isShutdown) {
        PowerMgrClient::GetInstance().ShutDownDevice(SHUTDOWN_REASON);
        THERMAL_HILOGI(COMP_SVC, "device start shutdown");
    }
    return ERR_OK;
}

int32_t ActionShutdown::ShutdownExecution(bool isShutdown)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
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
    THERMAL_HILOGD(COMP_SVC, "Enter");
    uint32_t delay = 50000;
    auto handler = g_service->GetHandler();
    if (handler == nullptr) {
        return ERR_INVALID_VALUE;
    }
    auto runner = g_service->GetEventRunner();
    if (runner == nullptr) {
        return ERR_INVALID_VALUE;
    }
    auto shutDownTask = [&]() {
        THERMAL_HILOGI(COMP_SVC, "shutdown run");
        ShutdownRequest(isShutdown);
        runner->Stop();
    };

    /**
     * @steps: post delay task to start runner to shutdown,
     * then new a thread to start the same runner cancell shutdown.
     * @expected: return runner is already running error.
     */
    auto f = [&]() {
        if (temp < thresholdClr) {
            auto runResult = runner->Stop();
            if (!runResult) {
                THERMAL_HILOGE(COMP_SVC, "HandleShutdownActionHUb: success to cancell");
            }
        }
    };
    handler->PostTask(shutDownTask, delay, EventQueue::Priority::HIGH);
    std::thread newThread(f);
    newThread.detach();
    runner->Run();
    return ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS
