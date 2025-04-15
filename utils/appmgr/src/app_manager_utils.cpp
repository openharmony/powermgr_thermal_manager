/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "app_manager_utils.h"

#include <csignal>
#include "thermal_log.h"

using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace PowerMgr {
namespace {
const int32_t SIGNAL_KILL = 9;
}

AppManagerUtils::AppManagerUtils()
{
    if (appMgrClient_ == nullptr) {
        appMgrClient_ = std::make_unique<AppMgrClient>();
    }
}

ErrCode AppManagerUtils::KillApplicationAction(const std::string& bundleName)
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

ErrCode AppManagerUtils::GetRunningProcessInfo(std::vector<RunningProcessInfo>& info)
{
    ErrCode result = ERR_OK;
    result = appMgrClient_->GetAllRunningProcesses(info);
    if (result == ERR_OK) {
        THERMAL_HILOGI(COMP_SVC, "get running process info successfully.");
    } else {
        THERMAL_HILOGE(COMP_SVC, "failed to get running process info.");
    }
    return result;
}

ErrCode AppManagerUtils::KillProcess(const pid_t pid)
{
    int32_t ret = -1;
    if (pid > 0) {
        ret = kill(pid, SIGNAL_KILL);
        if (ret == ERR_OK) {
            THERMAL_HILOGI(COMP_SVC, "KillProcess: success kill, pid=%{public}d", pid);
        } else {
            THERMAL_HILOGE(COMP_SVC, "KillProcess: failed to kill, pid=%{public}d", pid);
        }
    }
    return ret;
}

RunningProcessInfo AppManagerUtils::GetAppProcessInfoByName(const std::string& processName)
{
    RunningProcessInfo appProcessInfo;
    appProcessInfo.pid_ = 0;
    if (ERR_OK == GetRunningProcessInfo(allAppProcessInfos_)) {
        const auto& it = std::find_if(allAppProcessInfos_.begin(), allAppProcessInfos_.end(), [&](const auto& info) {
            return processName == info.processName_;
        });
        appProcessInfo = (it != allAppProcessInfos_.end()) ? *it : appProcessInfo;
    }
    return appProcessInfo;
}

void AppManagerUtils::GetAllRunnningAppProcess()
{
    allAppProcessInfos_.clear();
    bgAppProcessInfos_.clear();
    fgAppProcessInfos_.clear();
    if (ERR_OK == GetRunningProcessInfo(allAppProcessInfos_)) {
        for (const auto& info : allAppProcessInfos_) {
            if (info.state_ ==  AppProcessState::APP_STATE_BACKGROUND) {
                bgAppProcessInfos_.push_back(info);
            } else if (info.state_ == AppProcessState::APP_STATE_FOREGROUND) {
                fgAppProcessInfos_.push_back(info);
            }
        }
    }
    THERMAL_HILOGD(COMP_SVC, "GetAllRunnningAppProcess allAppProcessInfos_ size = %{public}d, bgAppProcessInfos_ "
        "size = %{public}d, fgAppProcessInfos_ size = %{public}d", static_cast<int>(allAppProcessInfos_.size()),
        static_cast<int>(bgAppProcessInfos_.size()), static_cast<int>(fgAppProcessInfos_.size()));
}

void AppManagerUtils::KillBgAppProcess()
{
    for (auto bg : bgAppProcessInfos_) {
        if (KillProcess(bg.pid_) != ERR_OK) {
            THERMAL_HILOGE(COMP_SVC, "failed to kill bg process");
        }
    }
}

void AppManagerUtils::KillFgAppProcess()
{
    for (auto fg : fgAppProcessInfos_) {
        if (KillProcess(fg.pid_) != ERR_OK) {
            THERMAL_HILOGE(COMP_SVC, "failed to kill fg process");
        }
    }
}

void AppManagerUtils::KillAllAppProcess()
{
    for (auto all : allAppProcessInfos_) {
        if (KillProcess(all.pid_) != ERR_OK) {
            THERMAL_HILOGE(COMP_SVC, "failed to kill all process");
        }
    }
}

void AppManagerUtils::ProcessAppActionRequest(const uint32_t& value)
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

extern "C" API ErrCode KillApplicationAction(const std::string& bundleName)
{
    return AppManagerUtils::GetInstance().KillApplicationAction(bundleName);
}

extern "C" API void ProcessAppActionRequest(const uint32_t& value)
{
    AppManagerUtils::GetInstance().ProcessAppActionRequest(value);
}
} // namespace PowerMgr
} // namespace OHOS