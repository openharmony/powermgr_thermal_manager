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

#include "action_popup.h"

#include "ui_service_mgr_client.h"
#include "wm_common.h"
#include "constants.h"
#include "thermal_common.h"

namespace OHOS {
namespace PowerMgr {
bool ActionPopup::InitParams(const std::string &params)
{
    params_ = params;
    return true;
}

void ActionPopup::SetStrict(bool flag)
{
    flag_ = flag;
}

void ActionPopup::AddActionValue(std::string value)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, " %{public}s value=%{public}s", __func__, value.c_str());
    if (value.empty()) return;
    valueList_.push_back(atoi(value.c_str()));
}

void ActionPopup::Execute()
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
        HandlePopupEvent(value);
        lastValue_ = value;
    }
}

void ActionPopup::HandlePopupEvent(const int32_t value)
{
    bool ret = false;
    switch (value) {
        case LOWER_TEMP:
            ret = ShowDialog(THERMAL_LOWER_TEMP_PARAMS);
            if (!ret) {
                THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "%{public}s to failed to popup", __func__);
                return;
            }
            break;
        case HIGHER_TEMP:
            ret = ShowDialog(THERMAL_HIGH_TEMP_PARAMS);
            if (!ret) {
                THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "%{public}s to failed to popup", __func__);
                return;
            }
            break;
        default:
            break;
    }
}

bool ActionPopup::ShowDialog(const std::string &params)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "Handle thermal level is too higher or too lower");
    // show dialog
    const int POSTION_X = 150;
    const int POSTION_Y = 150;
    const int WIDTH = 150;
    const int HEIGHT = 150;
    if (params.empty()) {
        return false;
    }

    Ace::UIServiceMgrClient::GetInstance()->ShowDialog(
        "thermal_dialog",
        params,
        OHOS::Rosen::WindowType::WINDOW_TYPE_SYSTEM_ALARM_WINDOW,
        POSTION_X,
        POSTION_Y,
        WIDTH,
        HEIGHT,
        [this](int32_t id, const std::string& event, const std::string& params) {
            THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "Dialog callback: %{public}s, %{public}s",
                event.c_str(), params.c_str());
            if (event == "EVENT_CANCEL") {
                Ace::UIServiceMgrClient::GetInstance()->CancelDialog(id);
            }
        });
    return true;
}
} // namespace PowerMgr
} // namespace OHOS