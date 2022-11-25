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

#ifndef ACTION_APPLICATION_PROCESS_H
#define ACTION_APPLICATION_PROCESS_H

#include "ithermal_action.h"

#include "delayed_sp_singleton.h"
#include "app_mgr_service_inner.h"
#include "app_mgr_service.h"
#include "running_process_info.h"
#include "app_mgr_client.h"

namespace OHOS {
namespace PowerMgr {
class ActionApplicationProcess : public IThermalAction {
public:
    ActionApplicationProcess(const std::string& actionName);
    ~ActionApplicationProcess() = default;
    enum {
        KILL_FG_PROCESS_APP = 1,
        KILL_BG_PROCESS_APP = 2,
        KILL_ALL_PROCESS_APP = 3,
    };

    void InitParams(const std::string& params) override;
    virtual void SetStrict(bool flag) override;
    virtual void SetEnableEvent(bool enable) override;
    virtual void AddActionValue(std::string value) override;
    virtual void Execute() override;
    bool Init();
    ErrCode KillApplicationAction(const std::string& bundleName);
    ErrCode GetRunningProcessInfo(std::vector<AppExecFwk::RunningProcessInfo>& info);
    AppExecFwk::RunningProcessInfo GetAppProcessInfoByName(const std::string& processName);
    ErrCode KillProcess(const pid_t pid);
    void GetAllRunnningAppProcess();
    void KillFgAppProcess();
    void KillBgAppProcess();
    void KillAllAppProcess();
    void ProcessAppActionRequest(const uint32_t& value);

    /* The api is used to UT, MT, ST */
    void ProcessAppActionExecution(const uint32_t& value);
private:
    std::vector<uint32_t> valueList_;
    bool flag_;
    bool enableEvent_ = false;
    std::unique_ptr<AppExecFwk::AppMgrClient> appMgrClient_;
    std::vector<AppExecFwk::RunningProcessInfo> bgAppProcessInfos_;
    std::vector<AppExecFwk::RunningProcessInfo> fgAppProcessInfos_;
    std::vector<AppExecFwk::RunningProcessInfo> allAppProcessInfos_;
    uint32_t lastValue_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // ACTION_APPLICATION_PROCESS_H
