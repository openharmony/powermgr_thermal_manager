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

#include "stdexcept"
#include "thermal_mgr_client.h"
#include "thermal_level_callback.h"
#include "thermal_log.h"
#include "refbase.h"

using namespace ohos::thermal;
using namespace OHOS;
using namespace OHOS::PowerMgr;

namespace {
thread_local auto& g_thermalMgrClient = ThermalMgrClient::GetInstance();
thread_local sptr<ThermalLevelCallback> g_thermalLevelCallback = new (std::nothrow) ThermalLevelCallback();

void RegisterThermalLevelCallback(callback_view<void(ohos::thermal::ThermalLevel)> thermalCb)
{
    THERMAL_HILOGD(COMP_FWK, "ets RegisterThermalLevelCallback interface");
    if (g_thermalLevelCallback == nullptr) {
        THERMAL_HILOGE(COMP_FWK, "g_thermalLevelCallback is nullptr");
        return;
    }
    g_thermalLevelCallback->UpdateCallback(thermalCb);
    g_thermalMgrClient.SubscribeThermalLevelCallback(g_thermalLevelCallback);
}

void UnregisterThermalLevelCallback(optional_view<callback<void(MyUndefined const&)>> thermalCb)
{
    THERMAL_HILOGD(COMP_FWK, "ets UnregisterThermalLevelCallback interface");
    if (g_thermalLevelCallback == nullptr) {
        THERMAL_HILOGE(COMP_FWK, "g_thermalLevelCallback is nullptr");
        return;
    }
    g_thermalMgrClient.UnSubscribeThermalLevelCallback(g_thermalLevelCallback);
    if (thermalCb) {
        THERMAL_HILOGI(COMP_FWK, "UnregisterThermalLevelCallback thermalCb start");
        (*thermalCb)(MyUndefined::make_Undefined());
    } else {
        THERMAL_HILOGI(COMP_FWK, "UnregisterThermalLevelCallback thermalCb is nullptr");
    }
}

ohos::thermal::ThermalLevel GetLevel()
{
    THERMAL_HILOGD(COMP_FWK, "ets GetLevel interface");
    OHOS::PowerMgr::ThermalLevel level = g_thermalMgrClient.GetThermalLevel();
    ohos::thermal::ThermalLevel levelValue = static_cast<ohos::thermal::ThermalLevel::key_t>(level);
    THERMAL_HILOGI(COMP_FWK, "level is %{public}d", static_cast<int32_t>(levelValue));
    return levelValue;
}
}  // namespace

// Since these macros are auto-generate, lint will cause false positive
// NOLINTBEGIN
TH_EXPORT_CPP_API_RegisterThermalLevelCallback(RegisterThermalLevelCallback);
TH_EXPORT_CPP_API_UnregisterThermalLevelCallback(UnregisterThermalLevelCallback);
TH_EXPORT_CPP_API_GetLevel(GetLevel);
// NOLINTEND