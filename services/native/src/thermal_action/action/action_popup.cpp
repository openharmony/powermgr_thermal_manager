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

#include <map>
#include <ipc_skeleton.h>
#include "ability_manager_client.h"
#include "constants.h"
#include "thermal_common.h"
#include "thermal_hisysevent.h"
#include "thermal_service.h"
#include "power_mgr_client.h"

using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace PowerMgr {
namespace {
auto g_service = DelayedSpSingleton<ThermalService>::GetInstance();
static PowerMgrClient& g_powerMgrClient = PowerMgrClient::GetInstance();
}

ActionPopup::ActionPopup(const std::string& actionName)
{
    actionName_ = actionName;
}

void ActionPopup::InitParams(const std::string& params) {}

void ActionPopup::SetStrict(bool flag)
{
    flag_ = flag;
}

void ActionPopup::SetEnableEvent(bool enable)
{
    enableEvent_ = enable;
}

void ActionPopup::AddActionValue(std::string value)
{
    THERMAL_HILOGD(COMP_SVC, "value=%{public}s", value.c_str());
    if (value.empty()) {
        return;
    }
    valueList_.push_back(atoi(value.c_str()));
}

void ActionPopup::Execute()
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    uint32_t value;
    THERMAL_RETURN_IF(g_service == nullptr);
    std::string scene = g_service->GetScene();
    auto iter = g_sceneMap.find(scene);
    if (iter != g_sceneMap.end()) {
        value = static_cast<uint32_t>(atoi(iter->second.c_str()));
        if (value != lastValue_) {
            HandlePopupEvent(value);
            WriteActionTriggeredHiSysEvent(enableEvent_, actionName_, value);
            g_service->GetObserver()->SetDecisionValue(actionName_, iter->second);
            lastValue_ = value;
            valueList_.clear();
        }
        return;
    }

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
        WriteActionTriggeredHiSysEvent(enableEvent_, actionName_, value);
        g_service->GetObserver()->SetDecisionValue(actionName_, std::to_string(value));
        lastValue_ = value;
    }
}

void ActionPopup::HandlePopupEvent(const int32_t value)
{
    switch (value) {
        case LOWER_TEMP:
            ShowThermalDialog(ActionPopup::TempStatus::LOWER_TEMP);
            g_powerMgrClient.RefreshActivity(UserActivityType::USER_ACTIVITY_TYPE_ATTENTION);
            break;
        case HIGHER_TEMP:
            ShowThermalDialog(ActionPopup::TempStatus::HIGHER_TEMP);
            g_powerMgrClient.RefreshActivity(UserActivityType::USER_ACTIVITY_TYPE_ATTENTION);
            break;
        default:
            break;
    }
}

bool ActionPopup::ShowThermalDialog(TempStatus value)
{
    THERMAL_HILOGD(COMP_SVC, "ShowThermalDialog start.");
    auto client = AbilityManagerClient::GetInstance();
    if (client == nullptr) {
        return false;
    }
    AAFwk::Want want;
    if (value == TempStatus::LOWER_TEMP) {
        want.SetElementName("com.ohos.powerdialog", "ThermalServiceExtAbility_low");
    } else {
        want.SetElementName("com.ohos.powerdialog", "ThermalServiceExtAbility_high");
    }
    int32_t result = client->StartAbility(want);
    if (result != 0) {
        THERMAL_HILOGE(COMP_SVC, "ShowThermalDialog failed, result = %{public}d", result);
        return false;
    }
    THERMAL_HILOGD(COMP_SVC, "ShowThermalDialog success.");
    return true;
}
} // namespace PowerMgr
} // namespace OHOS
