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
#include "common_event_data.h"
#include "common_event_manager.h"
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
std::string chargePath = "/data/service/el0/thermal/state/charge";
IdleState g_idleStateConfig {};
IdleState g_cachedIdleState {};
bool g_isChargeIdle = true;
}
bool ChargerStateCollection::Init()
{
    g_idleStateConfig = g_service->GetStateMachineObj()->GetIdleStateConfig();
    if (!RegisterEvent()) {
        return false;
    }
    return true;
}

bool ChargerStateCollection::InitParam(std::string &params)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    params_ = params;
    return true;
}

std::string ChargerStateCollection::GetState()
{
    THERMAL_HILOGD(COMP_SVC, "charger state = %{public}s", mockState_.c_str());
    if (!g_service->GetFlag()) {
        return mockState_;
    } else {
        return state_;
    }
}

bool ChargerStateCollection::RegisterEvent()
{
    if (g_service == nullptr) {
        return false;
    }

    auto receiver = g_service->GetStateMachineObj()->GetCommonEventReceiver();
    if (receiver == nullptr) {
        return false;
    }
    bool ret;
    THERMAL_HILOGI(COMP_SVC, "start register battery change event");
    EventHandle batteryChangedHandler =
        std::bind(&ChargerStateCollection::HandleChangerStatusCompleted, this, std::placeholders::_1);
    ret = receiver->Start(CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED, batteryChangedHandler);
    if (!ret) {
        THERMAL_HILOGE(COMP_SVC, "fail to COMMON_EVENT_BATTERY_CHANGED");
        return false;
    }
    THERMAL_HILOGI(COMP_SVC, "start register thermal level change event");
    EventHandle thermalLevelHandler =
        std::bind(&ChargerStateCollection::HandleThermalLevelCompleted, this, std::placeholders::_1);
    ret = receiver->Start(CommonEventSupport::COMMON_EVENT_THERMAL_LEVEL_CHANGED, thermalLevelHandler);
    if (!ret) {
        THERMAL_HILOGE(COMP_SVC, "fail to COMMON_EVENT_THERMAL_LEVEL_CHANGED");
        return false;
    }
    return true;
}

void ChargerStateCollection::HandleChangerStatusCompleted(const CommonEventData &data)
{
    g_cachedIdleState.soc = data.GetWant().GetIntParam(ToString(BatteryInfo::COMMON_EVENT_CODE_CAPACITY), -1);
    g_cachedIdleState.current = data.GetWant().GetIntParam(
        ToString(BatteryInfo::COMMON_EVENT_CODE_PLUGGED_NOW_CURRENT), -1);
    g_cachedIdleState.charging = data.GetWant().GetIntParam(ToString(BatteryInfo::COMMON_EVENT_CODE_CHARGE_STATE), -1);
    HandleChargeIdleState();

    switch (g_cachedIdleState.charging) {
        case static_cast<int32_t>(BatteryChargeState::CHARGE_STATE_DISABLE): {
            state_ = ToString(DISABLE);
            break;
        }
        case static_cast<int32_t>(BatteryChargeState::CHARGE_STATE_ENABLE): {
            state_ = ToString(ENABLE);
            break;
        }
        case static_cast<int32_t>(BatteryChargeState::CHARGE_STATE_FULL): {
            state_ = ToString(FULL);
            break;
        }
        case static_cast<int32_t>(BatteryChargeState::CHARGE_STATE_NONE): {
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

void ChargerStateCollection::SetState()
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    char chargerBuf[MAX_PATH] = {0};
    char chagerValue[MAX_PATH] = {0};
    int32_t ret = -1;
    if (snprintf_s(chargerBuf, PATH_MAX, sizeof(chargerBuf) - 1, chargePath.c_str()) < ERR_OK) {
        return;
    }
    THERMAL_HILOGD(COMP_SVC, "read charge state");
    ret = FileOperation::ReadFile(chargerBuf, chagerValue, sizeof(chagerValue));
    if (ret != ERR_OK) {
        return;
    }
    mockState_ = chagerValue;
    THERMAL_HILOGI(COMP_SVC, "mockState_=%{public}s", mockState_.c_str());
}

bool ChargerStateCollection::DecideState(const std::string &value)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    SetState();
    THERMAL_HILOGI(COMP_SVC, "mockState_=%{public}s, value=%{public}s", mockState_.c_str(), value.c_str());
    return StringOperation::Compare(value, mockState_);
}

void ChargerStateCollection::HandleChargeIdleState()
{
    THERMAL_HILOGI(COMP_SVC, "soc=%{public}d, charging==%{public}d, current==%{public}d, level==%{public}d",
                   g_cachedIdleState.soc, g_cachedIdleState.charging, g_cachedIdleState.current,
                   g_cachedIdleState.level);
    bool isIdle = ((g_cachedIdleState.soc >= g_idleStateConfig.soc) &&
                   (g_cachedIdleState.charging == g_idleStateConfig.charging) &&
                   (g_cachedIdleState.current >= g_idleStateConfig.current) &&
                   (g_cachedIdleState.level <= g_idleStateConfig.level));
    if (isIdle != g_isChargeIdle) {
        THERMAL_HILOGI(COMP_SVC, "idle state changed, start to broadcast event");
        PublishIdleEvent(isIdle);
        g_isChargeIdle = isIdle;
    }
}
void ChargerStateCollection::PublishIdleEvent(bool isIdle)
{
    Want want;
    auto code = static_cast<uint32_t>(ChargeIdleEventCode::EVENT_CODE_CHARGE_IDLE_STATE);
    want.SetParam(ToString(code), isIdle);
    want.SetAction(CommonEventSupport::COMMON_EVENT_CHARGE_IDLE);
    CommonEventData commonData;
    commonData.SetWant(want);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(false);
    if (!CommonEventManager::PublishCommonEvent(commonData, publishInfo)) {
        THERMAL_HILOGW(COMP_SVC, "failed to publish charge idle event");
    }
}
void ChargerStateCollection::HandleThermalLevelCompleted(const CommonEventData& data)
{
    std::string key = ToString(static_cast<int32_t>(ThermalCommonEventCode::CODE_THERMAL_LEVEL_CHANGED));
    int32_t level = data.GetWant().GetIntParam(key, -1);
    g_cachedIdleState.level =level;
    HandleChargeIdleState();
}
} // namespace PowerMgr
} // namespace OHOS
