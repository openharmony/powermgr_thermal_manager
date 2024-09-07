/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include "action_airplane.h"

#include "file_operation.h"
#include "thermal_hisysevent.h"
#include "thermal_service.h"
#include "securec.h"
#include "constants.h"
#ifdef HAS_THERMAL_AIRPLANE_MANAGER_PART
#include "net_conn_client.h"
#endif

namespace OHOS {
namespace PowerMgr {
namespace {
constexpr const char* AIRPLANE_PATH = "/data/service/el0/thermal/config/airplane";
const int MAX_PATH = 256;
}

ActionAirplane::ActionAirplane(const std::string& actionName)
{
    actionName_ = actionName;
}

void ActionAirplane::InitParams(const std::string& params)
{
    (void)params;
}

void ActionAirplane::SetStrict(bool enable)
{
    isStrict_ = enable;
}

void ActionAirplane::SetEnableEvent(bool enable)
{
    enableEvent_ = enable;
}

void ActionAirplane::AddActionValue(std::string value)
{
    if (value.empty()) {
        return;
    }
    char *endptr;
    long int result = strtol(value.c_str(), &endptr, STRTOL_FORMART_DEC);
    if (*endptr != '\0') {
        THERMAL_HILOGE(COMP_SVC, "parse airplane value failed");
        return;
    }
    valueList_.push_back(static_cast<uint32_t>(result));
}

void ActionAirplane::Execute()
{
    auto tms = ThermalService::GetInstance();
    THERMAL_RETURN_IF (tms == nullptr);
    uint32_t value = GetActionValue();
    if (value != lastValue_) {
        if (!tms->GetSimulationXml()) {
            AirplaneRequest(value);
        } else {
            AirplaneExecution(value);
        }
        WriteActionTriggeredHiSysEventWithRatio(enableEvent_, actionName_, value);
        tms->GetObserver()->SetDecisionValue(actionName_, std::to_string(value));
        lastValue_ = value;
        THERMAL_HILOGD(COMP_SVC, "action execute: {%{public}s = %{public}u}", actionName_.c_str(), lastValue_);
    }
    valueList_.clear();
}

uint32_t ActionAirplane::GetActionValue()
{
    uint32_t value = FALLBACK_VALUE_UINT_ZERO;
    if (!valueList_.empty()) {
        if (isStrict_) {
            value = *min_element(valueList_.begin(), valueList_.end());
        } else {
            value = *max_element(valueList_.begin(), valueList_.end());
        }
    }
    return value;
}

int32_t ActionAirplane::AirplaneRequest(const uint32_t& value)
{
#ifdef HAS_THERMAL_AIRPLANE_MANAGER_PART
    switch (value) {
        case ActionAirplane::TempStatus::LOWER_TEMP:
            if (!ThermalService::userAirplaneState_) {
                THERMAL_HILOGD(COMP_SVC, "device exit Airplane mode");
                ThermalService::isThermalAirplane_ = true;
                NetManagerStandard::NetConnClient::GetInstance().SetAirplaneMode(false);
            } else {
                THERMAL_HILOGD(COMP_SVC, "device keep Airplane mode");
            }
            break;
        case ActionAirplane::TempStatus::HIGHER_TEMP:
            if (!ThermalService::userAirplaneState_) {
                THERMAL_HILOGD(COMP_SVC, "device start Airplane mode");
                ThermalService::isThermalAirplane_ = true;
                NetManagerStandard::NetConnClient::GetInstance().SetAirplaneMode(true);
            } else {
                THERMAL_HILOGD(COMP_SVC, "device already in Airplane mode");
            }
            break;
        default:
            break;
    }
#endif
    return ERR_OK;
}

int32_t ActionAirplane::AirplaneExecution(const uint32_t& value)
{
    int32_t ret = -1;
    char buf[MAX_PATH] = {0};
    ret = snprintf_s(buf, MAX_PATH, sizeof(buf) - 1, AIRPLANE_PATH);
    if (ret < ERR_OK) {
        THERMAL_HILOGE(COMP_SVC, "snprintf_s airplane value failed");
        return ret;
    }
    std::string valueString = std::to_string(value) + "\n";
    ret = FileOperation::WriteFile(buf, valueString, valueString.length());
    if (ret != ERR_OK) {
        THERMAL_HILOGE(COMP_SVC, "write airplane value failed");
        return ret;
    }
    return ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS