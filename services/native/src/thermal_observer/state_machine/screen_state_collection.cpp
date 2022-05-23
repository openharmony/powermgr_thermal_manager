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

#include "screen_state_collection.h"

#include "common_event_support.h"
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
const uint32_t SCREEN_ON = 1;
const uint32_t SCREEN_OFF = 0;
const int MAX_PATH = 256;
std::string screenPath = "/data/thermal/state/screen";
auto g_service = DelayedSpSingleton<ThermalService>::GetInstance();
}
bool ScreenStateCollection::Init()
{
    if (!RegisterEvent()) {
        return false;
    }
    return true;
}
bool ScreenStateCollection::InitParam(std::string &params)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    params_ = params;
    return true;
}

std::string ScreenStateCollection::GetState()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s screen state = %{public}s", __func__, mockState_.c_str());
    if (!g_service->GetFlag()) {
        return mockState_;
    } else {
        return state_;
    }
}

bool ScreenStateCollection::RegisterEvent()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    if (g_service == nullptr) return false;
    auto receiver = g_service->GetStateMachineObj()->GetCommonEventReceiver();
    if (receiver == nullptr) return false;
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s register screen on event", __func__);
    EventHandle handlerOn = std::bind(&ScreenStateCollection::HandleScreenOnCompleted, this, std::placeholders::_1);
    bool on = receiver->Start(CommonEventSupport::COMMON_EVENT_SCREEN_ON, handlerOn);
    if (!on) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "%{public}s fail to COMMON_EVENT_SCREEN_ON", __func__);
        return false;
    }
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s register screen off event", __func__);
    EventHandle handlerOff = std::bind(&ScreenStateCollection::HandleScreenOffCompleted, this, std::placeholders::_1);
    bool off = receiver->Start(CommonEventSupport::COMMON_EVENT_SCREEN_OFF, handlerOff);
    if (!off) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "%{public}s fail to COMMON_EVENT_SCREEN_OFF", __func__);
        return false;
    }
    return true;
}

void ScreenStateCollection::HandleScreenOnCompleted(const CommonEventData &data __attribute__((__unused__)))
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    state_ = ToString(SCREEN_ON);
}

void ScreenStateCollection::HandleScreenOffCompleted(const CommonEventData &data __attribute__((__unused__)))
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    state_ = ToString(SCREEN_OFF);
}

void ScreenStateCollection::SetState()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    char screenBuf[MAX_PATH] = {0};
    char screenValue[MAX_PATH] = {0};
    int32_t ret = -1;

    if (snprintf_s(screenBuf, PATH_MAX, sizeof(screenBuf) - 1, screenPath.c_str()) < ERR_OK) {
        return;
    }
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s read screen state", __func__);
    ret = FileOperation::ReadFile(screenBuf, screenValue, sizeof(screenValue));
    if (ret != ERR_OK) {
        return;
    }
    mockState_ = TrimStr(screenValue, '\n');
}

bool ScreenStateCollection::DecideState(const std::string &value)
{
    SetState();
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s mockState_=%{public}s, value=%{public}s",
        __func__, mockState_.c_str(), value.c_str());
    return StringOperation::Compare(value, mockState_);
}
} // namespace PowerMgr
} // namespace OHOS