/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "action_node.h"

#include "file_operation.h"
#include "string_operation.h"
#include "thermal_common.h"
#include "thermal_service.h"

namespace OHOS {
namespace PowerMgr {
namespace {
constexpr int32_t PATH_IDX = 0;
constexpr int32_t FALLBACK_IDX = 1;
constexpr size_t MIN_PATH_LENGTH = 9;
}

ActionNode::ActionNode(const std::string& actionName)
{
    actionName_ = actionName;
}

void ActionNode::InitParams(const std::string& params)
{
    std::vector<std::string> paramList;
    StringOperation::SplitString(params, paramList, "|");
    int32_t paramNum = static_cast<int32_t>(paramList.size());
    if (paramNum > FALLBACK_IDX) {
        nodePath_ = paramList[PATH_IDX];
        fallbackValue_ = atoi(paramList[FALLBACK_IDX].c_str());
    } else if (paramNum > PATH_IDX) {
        nodePath_ = paramList[PATH_IDX];
    }
}

void ActionNode::SetStrict(bool enable)
{
    isStrict_ = enable;
}

void ActionNode::SetEnableEvent(bool enable)
{
    enableEvent_ = enable;
}

void ActionNode::AddActionValue(std::string value)
{
    if (value.empty()) {
        return;
    }
    valueList_.push_back(atoi(value.c_str()));
}

void ActionNode::Execute()
{
    int32_t value = fallbackValue_;
    if (!valueList_.empty()) {
        if (isStrict_) {
            value = *min_element(valueList_.begin(), valueList_.end());
        } else {
            value = *max_element(valueList_.begin(), valueList_.end());
        }
    }
    if (value != lastValue_) {
        std::string valStr = std::to_string(value);
        if (nodePath_.size() > MIN_PATH_LENGTH) {
            FileOperation::WriteFile(nodePath_, valStr, valStr.length());
        }
        auto tms = ThermalService::GetInstance();
        tms->GetObserver()->SetDecisionValue(actionName_, valStr);
        lastValue_ = value;
        THERMAL_HILOGD(COMP_SVC, "action execute: {%{public}s = %{public}d}", actionName_.c_str(), lastValue_);
    }
    valueList_.clear();
}
} // namespace PowerMgr
} // namespace OHOS
