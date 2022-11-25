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

#include "action_application_process.h"

#include <csignal>
#include <map>
#include <sys/types.h>

#include "file_operation.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"
#include "if_system_ability_manager.h"
#include "iremote_object.h"
#include "singleton.h"
#include "securec.h"

#include "thermal_hisysevent.h"
#include "thermal_service.h"

using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace PowerMgr {
namespace {
const int32_t SIGNAL_KILL = 9;
auto g_service = DelayedSpSingleton<ThermalService>::GetInstance();
constexpr const char* PROCESS_PATH = "/data/service/el0/thermal/config/process_ctrl";
const int MAX_PATH = 256;
}
ActionApplicationProcess::ActionApplicationProcess(const std::string& actionName)
{
    actionName_ = actionName;
}

bool ActionApplicationProcess::Init()
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    if (appMgrClient_ == nullptr) {
        appMgrClient_ = std::make_unique<AppMgrClient>();
    }
    return true;
}

void ActionApplicationProcess::InitParams(const std::string& params)
{
}

void ActionApplicationProcess::SetStrict(bool flag)
{
    flag_ = flag;
}

void ActionApplicationProcess::SetEnableEvent(bool enable)
{
    enableEvent_ = enable;
}

void ActionApplicationProcess::AddActionValue(std::string value)
{
    THERMAL_HILOGD(COMP_SVC, "value=%{public}s", value.c_str());
    if (value.empty()) return;
    valueList_.push_back(atoi(value.c_str()));
}

void ActionApplicationProcess::Execute()
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    uint32_t value;
    THERMAL_RETURN_IF (g_service == nullptr);
    std::string scene = g_service->GetScene();
    auto iter = g_sceneMap.find(scene);
    if (iter != g_sceneMap.end()) {
        value = static_cast<uint32_t>(atoi(iter->second.c_str()));
        if ((value != lastValue_) && (!g_service->GetSimulationXml())) {
            ProcessAppActionRequest(value);
        } else if (value != lastValue_) {
            ProcessAppActionExecution(value);
        } else {
            THERMAL_HILOGD(COMP_SVC, "value is not change");
        }
        WriteActionTriggeredHiSysEvent(enableEvent_, actionName_, value);
        g_service->GetObserver()->SetDecisionValue(actionName_, iter->second);
        lastValue_ = value;
        valueList_.clear();
        return;
    }

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

    if (value != lastValue_) {
        if (!g_service->GetFlag()) {
            ProcessAppActionExecution(value);
        } else {
            ProcessAppActionRequest(value);
        }
        WriteActionTriggeredHiSysEvent(enableEvent_, actionName_, value);
        g_service->GetObserver()->SetDecisionValue(actionName_, std::to_string(value));
        lastValue_ = value;
    }
}

ErrCode ActionApplicationProcess::KillApplicationAction(const std::string& bundleName)
{
    int result = ERR_OK;
    result = appMgrClient_->KillApplication(bundleName);
    if (result == ERR_OK) {
        THERMAL_HILOGE(COMP_SVC, "kill application:%{public}s successfully.", bundleName.c_str());
    } else {
        THERMAL_HILOGE(COMP_SVC, "failed to kill application:%{public}s.", bundleName.c_str());
    }
    return result;
}

ErrCode ActionApplicationProcess::GetRunningProcessInfo(std::vector<RunningProcessInfo>& info)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    ErrCode result = ERR_OK;
    result = appMgrClient_->GetAllRunningProcesses(info);
    if (result == ERR_OK) {
        THERMAL_HILOGI(COMP_SVC, "get running process info successfully.");
    } else {
        THERMAL_HILOGE(COMP_SVC, "failed to get running process info.");
    }
    return result;
}

ErrCode ActionApplicationProcess::KillProcess(const pid_t pid)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    int32_t ret = -1;
    if (pid > 0) {
        THERMAL_HILOGI(COMP_SVC, "KillProcess: kill pid %{public}d", pid);
        ret = kill(pid, SIGNAL_KILL);
        if (ret == ERR_OK) {
            THERMAL_HILOGI(COMP_SVC, "KillProcess: success kill");
        } else {
            THERMAL_HILOGE(COMP_SVC, "KillProcess: failed to kill");
        }
    }
    return ret;
}

RunningProcessInfo ActionApplicationProcess::GetAppProcessInfoByName(const std::string& processName)
{
    RunningProcessInfo appProcessInfo;
    appProcessInfo.pid_ = 0;
    if (ERR_OK == GetRunningProcessInfo(allAppProcessInfos_)) {
        const auto& it = std::find_if(allAppProcessInfos_.begin(), allAppProcessInfos_.end(), [&](auto& info) {
            return processName == info.processName_;
        });
        appProcessInfo = (it != allAppProcessInfos_.end()) ? *it : appProcessInfo;
    }
    return appProcessInfo;
}


void ActionApplicationProcess::GetAllRunnningAppProcess()
{
    if (ERR_OK == GetRunningProcessInfo(allAppProcessInfos_)) {
        for (const auto& info : allAppProcessInfos_) {
            if (info.state_ ==  AppProcessState::APP_STATE_BACKGROUND) {
                bgAppProcessInfos_.push_back(info);
            } else if (info.state_ == AppProcessState::APP_STATE_FOREGROUND) {
                fgAppProcessInfos_.push_back(info);
            }
        }
    }
}

void ActionApplicationProcess::KillBgAppProcess()
{
    for (auto bg : bgAppProcessInfos_) {
        if (KillProcess(bg.pid_) != ERR_OK) {
            THERMAL_HILOGE(COMP_SVC, "failed to kill bg process");
        }
    }
}

void ActionApplicationProcess::KillFgAppProcess()
{
    for (auto fg : fgAppProcessInfos_) {
        if (KillProcess(fg.pid_) != ERR_OK) {
            THERMAL_HILOGE(COMP_SVC, "failed to kill fg process");
        }
    }
}

void ActionApplicationProcess::KillAllAppProcess()
{
    for (auto all : allAppProcessInfos_) {
        if (KillProcess(all.pid_) != ERR_OK) {
            THERMAL_HILOGE(COMP_SVC, "failed to kill all process");
        }
    }
}

void ActionApplicationProcess::ProcessAppActionRequest(const uint32_t& value)
{
    THERMAL_HILOGD(COMP_SVC, "value: %{public}d", value);
    GetAllRunnningAppProcess();
    switch (value) {
        case KILL_FG_PROCESS_APP: {
            KillFgAppProcess();
            break;
        }
        case KILL_BG_PROCESS_APP: {
            KillBgAppProcess();
            break;
        }
        case KILL_ALL_PROCESS_APP: {
            KillAllAppProcess();
            break;
        }
        default: {
            break;
        }
    }
}

void ActionApplicationProcess::ProcessAppActionExecution(const uint32_t& value)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    int32_t ret = -1;
    char processBuf[MAX_PATH] = {0};
    ret = snprintf_s(processBuf, MAX_PATH, sizeof(processBuf) - 1, PROCESS_PATH);
    if (ret < EOK) {
        return;
    }
    std::string valueString = std::to_string(value) + "\n";
    ret = FileOperation::WriteFile(processBuf, valueString, valueString.length());
    if (ret != EOK) {
        return;
    }
}
} // namespace PowerMgr
} // namespace OHOS
