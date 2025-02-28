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

#include "action_application_process.h"

#include <map>
#include <sys/types.h>
#include <dlfcn.h>

#include "constants.h"
#include "file_operation.h"
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
constexpr const char* PROCESS_PATH = "/data/service/el0/thermal/config/process_ctrl";
const int MAX_PATH = 256;
const int ERR_FAILED = -1;
}

ActionApplicationProcess::ActionApplicationProcess(const std::string& actionName)
{
    actionName_ = actionName;
}

void ActionApplicationProcess::InitParams(const std::string& params)
{
    (void)params;
}

void ActionApplicationProcess::SetStrict(bool enable)
{
    isStrict_ = enable;
}

void ActionApplicationProcess::SetEnableEvent(bool enable)
{
    enableEvent_ = enable;
}

void ActionApplicationProcess::AddActionValue(uint32_t actionId, std::string value)
{
    if (value.empty()) {
        return;
    }
    if (actionId > 0) {
        auto iter = policyActionMap_.find(actionId);
        if (iter != policyActionMap_.end()) {
            iter->second.uintDelayValue = static_cast<uint32_t>(static_cast<uint32_t>(strtol(value.c_str(),
                nullptr, STRTOL_FORMART_DEC)));
        }
    } else {
        valueList_.push_back(static_cast<uint32_t>(strtol(value.c_str(), nullptr, STRTOL_FORMART_DEC)));
    }
}

void ActionApplicationProcess::ExecuteInner()
{
    auto tms = ThermalService::GetInstance();
    THERMAL_RETURN_IF (tms == nullptr);
    for (auto &policyAction : policyActionMap_) {
        if (policyAction.second.isCompleted) {
            valueList_.push_back(policyAction.second.uintDelayValue);
        }
    }

    uint32_t value = GetActionValue();
    if (value != lastValue_) {
        if (!tms->GetSimulationXml()) {
            ProcessAppActionRequest(value);
        } else {
            ProcessAppActionExecution(value);
        }
        WriteActionTriggeredHiSysEvent(enableEvent_, actionName_, value);
        tms->GetObserver()->SetDecisionValue(actionName_, std::to_string(value));
        lastValue_ = value;
    }
    valueList_.clear();
}

void ActionApplicationProcess::ResetActionValue()
{
    lastValue_ = 0;
}

uint32_t ActionApplicationProcess::GetActionValue()
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

ErrCode ActionApplicationProcess::KillApplicationAction(const std::string& bundleName)
{
    typedef ErrCode(*KillApplicationActionFunc)(const std::string& bundleName);
    void* handler = dlopen("libthermal_ability.z.so", RTLD_LAZY | RTLD_NODELETE);
    if (handler == nullptr) {
        THERMAL_HILOGE(COMP_SVC, "dlopen KillApplicationActionFunc failed, reason : %{public}s", dlerror());
        return ERR_FAILED;
    }
    KillApplicationActionFunc KillApplicationAction =
        reinterpret_cast<KillApplicationActionFunc>(dlsym(handler, "KillApplicationAction"));
    if (KillApplicationAction == nullptr) {
        THERMAL_HILOGE(COMP_SVC, "KillApplicationActionFunc is null, reason : %{public}s", dlerror());
#ifndef FUZZ_TEST
        dlclose(handler);
#endif
        handler = nullptr;
        return ERR_FAILED;
    }
    ErrCode ret = KillApplicationAction(bundleName);
#ifndef FUZZ_TEST
    dlclose(handler);
#endif
    handler = nullptr;
    return ret;
}

void ActionApplicationProcess::ProcessAppActionRequest(const uint32_t& value)
{
    typedef void(*ProcessAppActionRequestFunc)(const uint32_t& value);
    void* handler = dlopen("libthermal_ability.z.so", RTLD_LAZY | RTLD_NODELETE);
    if (handler == nullptr) {
        THERMAL_HILOGE(COMP_SVC, "dlopen ProcessAppActionRequestFunc failed, reason : %{public}s", dlerror());
        return;
    }
    ProcessAppActionRequestFunc ProcessAppActionRequest =
        reinterpret_cast<ProcessAppActionRequestFunc>(dlsym(handler, "ProcessAppActionRequest"));
    if (ProcessAppActionRequest == nullptr) {
        THERMAL_HILOGE(COMP_SVC, "ProcessAppActionRequestFunc is null, reason : %{public}s", dlerror());
#ifndef FUZZ_TEST
        dlclose(handler);
#endif
        handler = nullptr;
        return;
    }
    ProcessAppActionRequest(value);
#ifndef FUZZ_TEST
    dlclose(handler);
#endif
    handler = nullptr;
    return;
}

void ActionApplicationProcess::ProcessAppActionExecution(const uint32_t& value)
{
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
