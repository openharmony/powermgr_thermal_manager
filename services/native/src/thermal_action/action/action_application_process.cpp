/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include <sys/types.h>

#include "file_operation.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"
#include "if_system_ability_manager.h"
#include "iremote_object.h"
#include "singleton.h"
#include "securec.h"

#include "thermal_service.h"

using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace PowerMgr {
namespace {
const int32_t SIGNAL_KILL = 9;
auto g_service = DelayedSpSingleton<ThermalService>::GetInstance();
const std::string processPath = "/data/thermal/config/process_ctrl";
const int MAX_PATH = 256;
}
bool ActionApplicationProcess::Init()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "KillApplicationAction: Enter");
    if (appMgrClient_ == nullptr) {
        appMgrClient_ = std::make_unique<AppMgrClient>();
    }
    return true;
}

bool ActionApplicationProcess::InitParams(const std::string &params)
{
    params_ = params;
    return true;
}

void ActionApplicationProcess::SetStrict(bool flag)
{
    flag_ = flag;
}

void ActionApplicationProcess::AddActionValue(std::string value)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, " %{public}s value=%{public}s", __func__, value.c_str());
    if (value.empty()) return;
    valueList_.push_back(atoi(value.c_str()));
}

void ActionApplicationProcess::Execute()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, " %{public}s enter", __func__);
    int value = lastValue_;
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
        lastValue_ = value;
    }
}

ErrCode ActionApplicationProcess::KillApplicationAction(const std::string &bundleName)
{
    int result = ERR_OK;
    result = appMgrClient_->KillApplication(bundleName);
    if (result == ERR_OK) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "kill application:%{public}s successfully.", bundleName.c_str());
    } else {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "failed to kill application:%{public}s.", bundleName.c_str());
    }
    return result;
}

ErrCode ActionApplicationProcess::GetRunningProcessInfo(std::vector<RunningProcessInfo> &info)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "GetRunningProcessInfo: Enter");
    ErrCode result = ERR_OK;
    result = appMgrClient_->GetAllRunningProcesses(info);
    if (result == ERR_OK) {
        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "get running process info successfully.");
    } else {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "failed to get running process info.");
    }
    return result;
}

ErrCode ActionApplicationProcess::KillProcess(const pid_t pid)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "KillProcess: Enter");
    int32_t ret = -1;
    if (pid > 0) {
        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "KillProcess: kill pid %{public}d", pid);
        ret = kill(pid, SIGNAL_KILL);
        if (ret == ERR_OK) {
            THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "KillProcess: success kill");
        } else {
            THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "KillProcess: failed to kill");
        }
    }
    return ret;
}

RunningProcessInfo ActionApplicationProcess::GetAppProcessInfoByName(const std::string &processName)
{
    RunningProcessInfo appProcessInfo;
    appProcessInfo.pid_ = 0;
    if (ERR_OK == GetRunningProcessInfo(allAppProcessInfos_)) {
        for (const auto &info : allAppProcessInfos_) {
            if (processName == info.processName_) {
                appProcessInfo = info;
            }
        }
    }
    return appProcessInfo;
}


void ActionApplicationProcess::GetAllRunnningAppProcess()
{
    if (ERR_OK == GetRunningProcessInfo(allAppProcessInfos_)) {
        for (const auto &info : allAppProcessInfos_) {
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
            THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "KillBgAppProcess: failed to kill bg process");
        }
    }
}

void ActionApplicationProcess::KillFgAppProcess()
{
    for (auto fg : fgAppProcessInfos_) {
        if (KillProcess(fg.pid_) != ERR_OK) {
            THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "KillFgAppProcess: failed to kill fg process");
        }
    }
}

void ActionApplicationProcess::KillAllAppProcess()
{
    for (auto all : allAppProcessInfos_) {
        if (KillProcess(all.pid_) != ERR_OK) {
            THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "KillFgAppProcess: failed to kill all process");
        }
    }
}

void ActionApplicationProcess::KillService(const std::string &serviceName)
{
    system(("kill -9 $(pidof " + serviceName + ") > /dev/null 2>&1").c_str());
}

void ActionApplicationProcess::ProcessAppActionRequest(const uint32_t &value)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "ProcessAppActionRequest: action %{public}d", value);
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

void ActionApplicationProcess::ProcessAppActionExecution(const uint32_t &value)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "start to kill application");
    int32_t ret = -1;
    char processBuf[MAX_PATH] = {0};
    ret = snprintf_s(processBuf, PATH_MAX, sizeof(processBuf) - 1, processPath.c_str());
    if (ret < ERR_OK) {
        return;
    }
    std::string valueString = std::to_string(value) + "\n";
    ret = FileOperation::WriteFile(processBuf, valueString, valueString.length());
    if (ret != ERR_OK) {
        return;
    }
}
} // namespace PowerMgr
} // namespace OHOS