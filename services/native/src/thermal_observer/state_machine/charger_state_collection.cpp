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

#include "charger_state_collection.h"

#include "common_event_subscriber.h"
#include "common_event_support.h"
#include "battery_info.h"
#include "file_operation.h"
#include "securec.h"
#include "string_ex.h"
#include "string_operation.h"
#include "thermal_service.h"
#include "thermal_common.h"

using namespace OHOS::EventFwk;
namespace OHOS {
namespace PowerMgr {
namespace {
auto g_service = DelayedSpSingleton<ThermalService>::GetInstance();
const int MAX_PATH = 256;
std::string chargePath = "/data/thermal/state/charge";
}
bool ChargerStateCollection::Init()
{
    if (!RegisterEvent()) {
        return false;
    }
    return true;
}

bool ChargerStateCollection::InitParam(std::string &params)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    params_ = params;
    return true;
}

std::string ChargerStateCollection::GetState()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s charger state = %{public}s", __func__, mockState_.c_str());
    if (!g_service->GetFlag()) {
        return mockState_;
    } else {
        return state_;
    }
}

bool ChargerStateCollection::RegisterEvent()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    if (g_service == nullptr) return false;
    auto receiver = g_service->GetStateMachineObj()->GetCommonEventReceiver();
    if (receiver == nullptr) return false;
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s start register charger state change event", __func__);
    EventHandle handler = std::bind(&ChargerStateCollection::HandleChangerStatusCompleted, this, std::placeholders::_1);
    
    bool ret = receiver->Start(CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED, handler);
    if (!ret) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "%{public}s fail to COMMON_EVENT_BATTERY_CHANGED", __func__);
        return false;
    }
    return true;
}

void ChargerStateCollection::HandleChangerStatusCompleted(const CommonEventData &data __attribute__((__unused__)))
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    int code = data.GetCode();
    std::string chargerStatus = data.GetData();
    if (code == BatteryInfo::COMMON_EVENT_CODE_CHARGE_STATE) {
        switch (atoi(chargerStatus.c_str())) {
            case static_cast<int>(BatteryChargeState::CHARGE_STATE_DISABLE): {
                state_ = ToString(DISABLE);
                break;
            }
            case static_cast<int>(BatteryChargeState::CHARGE_STATE_ENABLE): {
                state_ = ToString(ENABLE);
                break;
            }
            case static_cast<int>(BatteryChargeState::CHARGE_STATE_FULL): {
                state_ = ToString(FULL);
                break;
            }
            case static_cast<int>(BatteryChargeState::CHARGE_STATE_NONE): {
                state_ = ToString(NONE);
                break;
            }
            case static_cast<int>(BatteryChargeState::CHARGE_STATE_BUTT): {
                state_ = ToString(BUTT);
                break;
            }
            default:
                break;
        }
    }
}

void ChargerStateCollection::SetState()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    char chargerBuf[MAX_PATH] = {0};
    char chagerValue[MAX_PATH] = {0};
    int32_t ret = -1;
    if (snprintf_s(chargerBuf, PATH_MAX, sizeof(chargerBuf) - 1, chargePath.c_str()) < ERR_OK) {
        return;
    }
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s read charge state", __func__);
    ret = FileOperation::ReadFile(chargerBuf, chagerValue, sizeof(chagerValue));
    if (ret != ERR_OK) {
        return;
    }
    mockState_ = chagerValue;
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s mockState_=%{public}s", __func__, mockState_.c_str());
}

bool ChargerStateCollection::DecideState(const std::string &value)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    SetState();
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s mockState_=%{public}s, value=%{public}s",
        __func__, mockState_.c_str(), value.c_str());
    return StringOperation::Compare(value, mockState_);
}
} // namespace PowerMgr
} // namespace OHOS