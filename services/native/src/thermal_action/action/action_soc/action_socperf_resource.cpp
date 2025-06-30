/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
 
#include "action_socperf_resource.h"
 
#include "string_operation.h"
#include "thermal_hisysevent.h"
#include "thermal_service.h"
 
namespace OHOS {
namespace PowerMgr {
namespace {
constexpr int32_t RESID_IDX = 0;
constexpr int32_t FALLBACK_IDX = 1;
}
ActionSocPerfResource::ActionSocPerfResource(const std::string& actionName)
{
    actionName_ = actionName;
    SocActionBase::SocSet.insert(actionName_);
}
 
void ActionSocPerfResource::InitParams(const std::string& params)
{
    std::vector<std::string> paramList;
    StringOperation::SplitString(params, paramList, "|");
    int32_t paramNum = static_cast<int32_t>(paramList.size());
    if (paramNum > FALLBACK_IDX) {
        StrToInt(paramList[RESID_IDX], resId_);
        fallbackValue_ = atoi(paramList[FALLBACK_IDX].c_str());
    } else if (paramNum > RESID_IDX) {
        StrToInt(paramList[RESID_IDX], resId_);
    }
}
 
void ActionSocPerfResource::SetStrict(bool enable)
{
    isStrict_ = enable;
}
 
void ActionSocPerfResource::SetEnableEvent(bool enable)
{
    enableEvent_ = enable;
}
 
void ActionSocPerfResource::AddActionValue(uint32_t actionId, std::string value)
{
    if (value.empty()) {
        THERMAL_HILOGD(COMP_SVC, "value empty");
        return;
    }
    uint32_t newValue = 0;
    if (!StringOperation::StrToUint(value, newValue)) {
        return;
    }
    if (actionId > 0) {
        auto iter = policyActionMap_.find(actionId);
        if (iter != policyActionMap_.end()) {
            iter->second.uintDelayValue = newValue;
        }
    } else {
        valueList_.push_back(newValue);
    }
}
 
void ActionSocPerfResource::ExecuteInner()
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
        SocLimitRequest(resId_, value);
        WriteActionTriggeredHiSysEvent(enableEvent_, actionName_, value);
        if (tms->GetObserver() != nullptr) {
            tms->GetObserver()->SetDecisionValue(actionName_, std::to_string(value));
        }
        lastValue_ = value;
        THERMAL_HILOGD(COMP_SVC, "action execute: {%{public}s = %{public}u}", actionName_.c_str(), lastValue_);
    }
    valueList_.clear();
}
 
void ActionSocPerfResource::ResetActionValue()
{
    lastValue_ = 0;
}
 
uint32_t ActionSocPerfResource::GetActionValue()
{
    uint32_t value = fallbackValue_;
    if (!valueList_.empty()) {
        if (isStrict_) {
            value = *min_element(valueList_.begin(), valueList_.end());
        } else {
            value = *max_element(valueList_.begin(), valueList_.end());
        }
    }
    return value;
}
} // namespace PowerMgr
} // namespace OHOS