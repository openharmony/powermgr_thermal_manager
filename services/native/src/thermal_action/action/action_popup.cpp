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

#include "action_popup.h"

#include "display_manager.h"
#include "ui_service_mgr_client.h"
#include "wm_common.h"
#include "constants.h"
#include "thermal_common.h"

namespace OHOS {
namespace PowerMgr {
namespace {
constexpr int UI_DIALOG_POWER_WIDTH_NARROW = 400;
constexpr int UI_DIALOG_POWER_HEIGHT_NARROW = 240;
}
bool ActionPopup::InitParams(const std::string &params)
{
    params_ = params;
    return true;
}

void ActionPopup::SetStrict(bool flag)
{
    flag_ = flag;
}

void ActionPopup::SetProtocol(const std::string& protocol)
{
}

void ActionPopup::AddActionValue(std::string value)
{
    THERMAL_HILOGD(COMP_SVC, "value=%{public}s", value.c_str());
    if (value.empty()) return;
    valueList_.push_back(atoi(value.c_str()));
}

void ActionPopup::Execute()
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    uint32_t value;
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
        HandlePopupEvent(value);
        lastValue_ = value;
    }
}

void ActionPopup::HandlePopupEvent(const int32_t value)
{
    if (dialogId_ >= 0) {
        return;
    }
    bool ret = false;
    switch (value) {
        case LOWER_TEMP:
            ret = ShowDialog(THERMAL_LOWER_TEMP_PARAMS);
            if (!ret) {
                THERMAL_HILOGE(COMP_SVC, "failed to popup");
                return;
            }
            break;
        case HIGHER_TEMP:
            ret = ShowDialog(THERMAL_HIGH_TEMP_PARAMS);
            if (!ret) {
                THERMAL_HILOGE(COMP_SVC, "failed to popup");
                return;
            }
            break;
        default:
            break;
    }
}

bool ActionPopup::ShowDialog(const std::string &params)
{
    // show dialog
    int width;
    int height;

    GetDisplayPosition(width, height);

    if (params.empty()) {
        return false;
    }

    int32_t errCode = Ace::UIServiceMgrClient::GetInstance()->ShowDialog(
        "thermal_dialog",
        params,
        OHOS::Rosen::WindowType::WINDOW_TYPE_SYSTEM_ALARM_WINDOW,
        0,
        0,
        width,
        height,
        [this](int32_t id, const std::string& event, const std::string& params) {
            THERMAL_HILOGI(COMP_SVC, "Dialog callback: %{public}s, %{public}s",
                event.c_str(), params.c_str());
            if (event == "EVENT_CANCEL") {
                Ace::UIServiceMgrClient::GetInstance()->CancelDialog(id);
                dialogId_ = -1;
            }
        },
        &dialogId_);
    THERMAL_HILOGI(COMP_SVC, "Show dialog errCode %{public}d, dialogId=%{public}d", errCode, dialogId_);
    return true;
}

void ActionPopup::GetDisplayPosition(int32_t& width, int32_t& height)
{
    auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    if (display == nullptr) {
        THERMAL_HILOGE(COMP_SVC, "dialog GetDefaultDisplay fail, try again.");
        display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    }

    if (display != nullptr) {
        THERMAL_HILOGI(COMP_SVC, "display size: %{public}d x %{public}d",
            display->GetWidth(), display->GetHeight());
        width = display->GetWidth();
        height = display->GetHeight();
    } else {
        THERMAL_HILOGI(COMP_SVC, "dialog get display fail, use default wide.");
        width = UI_DIALOG_POWER_WIDTH_NARROW;
        height = UI_DIALOG_POWER_HEIGHT_NARROW;
    }
    THERMAL_HILOGI(COMP_SVC, "GetDisplayPosition: :w:%{public}d x h:%{public}d)", width, height);
}
} // namespace PowerMgr
} // namespace OHOS
