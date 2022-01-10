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

#include <string>

#include "thermal_common.h"
#include "window_option.h"

namespace OHOS {
namespace PowerMgr {
namespace {
const std::string WINDOW_DIALOG_DOUBLE_BUTTON = "pages/dialog/index.js";
const std::string PARAM = "device temp is too lower";
const uint32_t SIZE = 200;
}

bool ActionPopup::ShowDialogAction()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s start", __func__);
    int width = 100;
    int height = 120;
    auto csurface = Surface::CreateSurfaceAsConsumer();
    csurface->SetDefaultWidthAndHeight(width, height);
    sptr<Window> window = CreateWindow(WINDOW_TYPE_ALARM_SCREEN, csurface);
    OHOS::Ace::DialogCallback lowTempCallback = std::bind(&ActionPopup::WarningLowTemp, this,
        std::placeholders::_1, std::placeholders::_2);
    OHOS::Ace::showDialog(window, WINDOW_DIALOG_DOUBLE_BUTTON, PARAM, lowTempCallback);
    return true;
}

void ActionPopup::WarningLowTemp(std::string event, std::string param)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s start", __func__);
}

sptr<OHOS::Window> ActionPopup::CreateWindow(OHOS::WindowType type, sptr<OHOS::Surface> csurface)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s start", __func__);
    auto wm = OHOS::WindowManager::GetInstance();
    if (wm == nullptr) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "%{public}s wm is nullptr", __func__);
        return nullptr;
    }
    wm->Init();
    auto option = OHOS::WindowOption::Get();
    if (option == nullptr) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "%{public}s option is nullptr", __func__);
        return nullptr;
    }

    sptr<OHOS::Window> window;
    option->SetWindowType(type);
    if (csurface == nullptr) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE,
            "%{public}s create surface is nullptr", __func__);
        return nullptr;
    }
    option->SetConsumerSurface(csurface);
    option->SetHeight(SIZE);
    option->SetWidth(SIZE);

    wm->CreateWindow(window, option);
    if (window == nullptr) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE,
            "%{public}s CreateWindow return nullptr", __func__);
        return nullptr;
    }
    return window;
}
} // namespace PowerMgr
} // namespace OHOS