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

#include "action_display.h"

#include "display_power_mgr_client.h"
#include "file_operation.h"
#include "thermal_service.h"
#include "securec.h"

using namespace OHOS::DisplayPowerMgr;
namespace OHOS {
namespace PowerMgr {
namespace {
auto g_service = DelayedSpSingleton<ThermalService>::GetInstance();
const std::string lcdPath = "/data/service/el0/thermal/config/lcd";
const int MAX_PATH = 256;
}
bool ActionDisplay::InitParams(const std::string &params)
{
    params_ = params;
    return true;
}

void ActionDisplay::SetStrict(bool flag)
{
    flag_ = flag;
}

void ActionDisplay::AddActionValue(std::string value)
{
    THERMAL_HILOGD(COMP_SVC, "value=%{public}s", value.c_str());
    if (value.empty()) return;
    valueList_.push_back(atoi(value.c_str()));
}

void ActionDisplay::Execute()
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
        if (!g_service->GetFlag()) {
            DisplayExecution(value);
        } else {
            DisplayRequest(value);
        }
        lastValue_ = value;
    }
}

uint32_t ActionDisplay::DisplayRequest(uint32_t brightness)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    uint32_t id = 0;
    if (!DisplayPowerMgrClient::GetInstance().SetBrightness(brightness, id)) {
        THERMAL_HILOGE(COMP_SVC, "failed to set brightness");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

uint32_t ActionDisplay::DisplayExecution(uint32_t brightness)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    int32_t ret = -1;
    char lcdBuf[MAX_PATH] = {0};
    ret = snprintf_s(lcdBuf, PATH_MAX, sizeof(lcdBuf) - 1, lcdPath.c_str());
    if (ret < ERR_OK) {
        return ret;
    }
    std::string valueString = std::to_string(brightness) + "\n";
    ret = FileOperation::WriteFile(lcdBuf, valueString, valueString.length());
    if (ret != ERR_OK) {
        return ret;
    }
    return ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS