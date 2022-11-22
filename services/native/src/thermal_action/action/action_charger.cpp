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

#include "action_charger.h"

#include <map>
#include "constants.h"
#include "file_operation.h"
#include "securec.h"
#include "thermal_hisysevent.h"
#include "thermal_service.h"
#include "v1_1/ibattery_interface.h"

namespace OHOS {
namespace PowerMgr {
namespace {
auto g_service = DelayedSpSingleton<ThermalService>::GetInstance();
constexpr const char* SC_CURRENT_PATH = "/data/service/el0/thermal/config/sc_current";
constexpr const char* BUCK_CURRENT_PATH = "/data/service/el0/thermal/config/buck_current";
const int MAX_PATH = 256;
}
ActionCharger::ActionCharger(const std::string& actionName)
{
    actionName_ = actionName;
}

std::vector<ChargingLimit> ActionCharger::chargeLimitList_;

void ActionCharger::InitParams(const std::string& protocol)
{
    protocol_ = protocol;
}

void ActionCharger::SetStrict(bool flag)
{
    flag_ = flag;
}

void ActionCharger::SetEnableEvent(bool enable)
{
    enableEvent_ = enable;
}

void ActionCharger::AddActionValue(std::string value)
{
    THERMAL_HILOGD(COMP_SVC, "value=%{public}s", value.c_str());
    if (value.empty()) return;
    valueList_.push_back(atoi(value.c_str()));
}

void ActionCharger::Execute()
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    uint32_t value;
    THERMAL_RETURN_IF (g_service == nullptr);
    std::string scene = g_service->GetScene();
    auto iter = g_sceneMap.find(scene);
    if (iter != g_sceneMap.end()) {
        THERMAL_HILOGD(COMP_SVC, "g_service->GetScene()=%{public}s", g_service->GetScene().c_str());
        value = static_cast<uint32_t>(atoi(iter->second.c_str()));
        if (value != lastValue_) {
            ChargerRequest(value);
            WriteSimValue(value);
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
        ChargerRequest(value);
        WriteSimValue(value);
        WriteActionTriggeredHiSysEvent(enableEvent_, actionName_, value);
        g_service->GetObserver()->SetDecisionValue(actionName_, std::to_string(value));
        lastValue_ = value;
    }
}

int32_t ActionCharger::ChargerRequest(int32_t current)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    ChargingLimit chargingLimit;
    chargingLimit.type = TYPE_CURRENT;
    chargingLimit.protocol = protocol_;
    chargingLimit.value = current;
    chargeLimitList_.push_back(chargingLimit);

    auto thermalInterface = g_service->GetThermalInterface();
    if (thermalInterface != nullptr) {
        int32_t ret = thermalInterface->SetBatteryCurrent(current);
        if (ret != ERR_OK) {
            THERMAL_HILOGE(COMP_SVC, "failed to set charger current to thermal hdf");
            return ret;
        }
    }
    return ERR_OK;
}

void ActionCharger::ExecuteCurrentLimit()
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    if (chargeLimitList_.empty()) {
        return;
    }
    sptr<IBatteryInterface> iBatteryInterface = IBatteryInterface::Get();
    if (iBatteryInterface == nullptr) {
        THERMAL_HILOGE(COMP_SVC, "iBatteryInterface_ is nullptr");
        return;
    }
    int32_t result = iBatteryInterface->SetChargingLimit(chargeLimitList_);
    if (result != ERR_OK) {
        THERMAL_HILOGE(COMP_SVC, "failed to set charge limit");
        return;
    }
    chargeLimitList_.clear();
}

int32_t ActionCharger::WriteSimValue(int32_t simValue)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    int32_t ret = -1;
    char buf[MAX_PATH] = {0};
    if (protocol_ == SC_PROTOCOL) {
        ret = snprintf_s(buf, MAX_PATH, sizeof(buf) - 1, SC_CURRENT_PATH);
        if (ret < EOK) {
            return ret;
        }
    } else if (protocol_ == BUCK_PROTOCOL) {
        ret = snprintf_s(buf, MAX_PATH, sizeof(buf) - 1, BUCK_CURRENT_PATH);
        if (ret < EOK) {
            return ret;
        }
    }
    std::string valueString = std::to_string(simValue) + "\n";
    ret = FileOperation::WriteFile(buf, valueString, valueString.length());
    if (ret != ERR_OK) {
        return ret;
    }
    return ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS
