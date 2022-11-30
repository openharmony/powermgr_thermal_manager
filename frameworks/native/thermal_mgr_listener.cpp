/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "thermal_mgr_listener.h"
#include "thermal_common.h"
#include "thermal_mgr_client.h"

namespace OHOS {
namespace PowerMgr {
bool ThermalMgrListener::ThermalLevelCallback::GetThermalLevel(ThermalLevel level)
{
    std::shared_ptr<ThermalMgrListener> listener = listener_.lock();
    if (listener == nullptr) {
        THERMAL_HILOGI(COMP_FWK, "listener is nullptr");
        return false;
    }
    listener->levelEvent_->OnThermalLevelResult(level);
    return true;
}

void ThermalMgrListener::RegisterServiceEvent()
{
    callback_ = new ThermalLevelCallback(shared_from_this());
    THERMAL_HILOGD(COMP_FWK, "start register");
    ThermalMgrClient::GetInstance().SubscribeThermalLevelCallback(callback_);
}

void ThermalMgrListener::UnRegisterServiceEvent()
{
    THERMAL_HILOGD(COMP_FWK, "Enter");
    ThermalMgrClient::GetInstance().UnSubscribeThermalLevelCallback(callback_);
}

int32_t ThermalMgrListener::SubscribeLevelEvent(const std::shared_ptr<ThermalLevelEvent>& levelEvent)
{
    THERMAL_HILOGD(COMP_FWK, "Enter");
    if (levelEvent == nullptr) {
        return ERR_INVALID_VALUE;
    }
    levelEvent_ = levelEvent;
    RegisterServiceEvent();
    return ERR_OK;
}

int32_t ThermalMgrListener::UnSubscribeLevelEvent()
{
    UnRegisterServiceEvent();
    return ERR_OK;
}

ThermalLevel ThermalMgrListener::GetThermalLevel()
{
    return ThermalMgrClient::GetInstance().GetThermalLevel();
}
} // namespace PowerMgr
} // namespace OHOS
