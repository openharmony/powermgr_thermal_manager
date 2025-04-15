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

#ifndef APP_MANAGER_UTILS_H
#define APP_MANAGER_UTILS_H

#define API __attribute__((visibility("default")))

#include "running_process_info.h"
#include "app_mgr_client.h"

namespace OHOS {
namespace PowerMgr {
class API AppManagerUtils {
public:
    enum {
        KILL_FG_PROCESS_APP = 1,
        KILL_BG_PROCESS_APP = 2,
        KILL_ALL_PROCESS_APP = 3,
    };

    static AppManagerUtils& GetInstance()
    {
        static AppManagerUtils appManagerUtils;
        return appManagerUtils;
    }

    ErrCode KillApplicationAction(const std::string& bundleName);
    ErrCode GetRunningProcessInfo(std::vector<AppExecFwk::RunningProcessInfo>& info);
    AppExecFwk::RunningProcessInfo GetAppProcessInfoByName(const std::string& processName);
    ErrCode KillProcess(const pid_t pid);
    void GetAllRunnningAppProcess();
    void KillFgAppProcess();
    void KillBgAppProcess();
    void KillAllAppProcess();
    void ProcessAppActionRequest(const uint32_t& value);

private:
    AppManagerUtils();
    virtual ~AppManagerUtils() = default;
    std::unique_ptr<AppExecFwk::AppMgrClient> appMgrClient_;
    std::vector<AppExecFwk::RunningProcessInfo> bgAppProcessInfos_;
    std::vector<AppExecFwk::RunningProcessInfo> fgAppProcessInfos_;
    std::vector<AppExecFwk::RunningProcessInfo> allAppProcessInfos_;
};

extern "C" API ErrCode KillApplicationAction(const std::string& bundleName);
extern "C" API void ProcessAppActionRequest(const uint32_t& value);
} // namespace PowerMgr
} // namespace OHOS
#endif // APP_MANAGER_UTILS_H