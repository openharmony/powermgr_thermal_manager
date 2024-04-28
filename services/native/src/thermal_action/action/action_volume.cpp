/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "action_volume.h"

#include <algorithm>
#ifdef HAS_THERMAL_AUDIO_FRAMEWORK_PART
#include "audio_system_manager.h"
#include "audio_stream_manager.h"
#endif
#include "file_operation.h"
#include "thermal_hisysevent.h"
#include "thermal_service.h"
#include "securec.h"
#include "string_operation.h"

#ifdef HAS_THERMAL_AUDIO_FRAMEWORK_PART
using namespace OHOS::AudioStandard;
#endif
namespace OHOS {
namespace PowerMgr {
namespace {
constexpr const char* VOLUME_PATH = "/data/service/el0/thermal/config/volume";
const int MAX_PATH = 256;
std::vector<ActionItem> g_actionInfo;
}

ActionVolume::ActionVolume(const std::string& actionName)
{
    actionName_ = actionName;
}

void ActionVolume::InitParams(const std::string& params)
{
    (void)params;
}

void ActionVolume::SetStrict(bool enable)
{
    isStrict_ = enable;
}

void ActionVolume::SetEnableEvent(bool enable)
{
    enableEvent_ = enable;
}

void ActionVolume::AddActionValue(std::string value)
{
    if (value.empty()) {
        return;
    }
    valueList_.push_back(static_cast<float>(strtof(value.c_str(), nullptr)));
}

void ActionVolume::Execute()
{
    auto tms = ThermalService::GetInstance();
    THERMAL_RETURN_IF (tms == nullptr);
    float value = GetActionValue();
    if (fabs(value - lastValue_) > FLOAT_ACCURACY) {
        if (!tms->GetSimulationXml()) {
            VolumeRequest(value);
        } else {
            VolumeExecution(value);
        }
        WriteActionTriggeredHiSysEventWithRatio(enableEvent_, actionName_, value);
        tms->GetObserver()->SetDecisionValue(actionName_, std::to_string(value));
        lastValue_ = value;
        THERMAL_HILOGD(COMP_SVC, "action execute: {%{public}s = %{public}f}", actionName_.c_str(), lastValue_);
    }
    valueList_.clear();
}

float ActionVolume::GetActionValue()
{
    float value = FALLBACK_VALUE_FLOAT;
    if (!valueList_.empty()) {
        if (isStrict_) {
            value = *min_element(valueList_.begin(), valueList_.end());
        } else {
            value = *max_element(valueList_.begin(), valueList_.end());
        }
    }
    return value;
}

int32_t ActionVolume::VolumeRequest(float volume)
{
    auto tms = ThermalService::GetInstance();
    std::string uid;
    std::vector<std::string> uidList;
    g_actionInfo = tms->GetActionManagerObj()->GetActionItem();
    const auto& item = std::find_if(g_actionInfo.begin(), g_actionInfo.end(), [](const auto& info) {
        return info.name == "volume";
    });
    uid = (item != g_actionInfo.end()) ? item->uid : uid;

    StringOperation::SplitString(uid, uidList, ",");
#ifdef HAS_THERMAL_AUDIO_FRAMEWORK_PART
    std::vector<std::unique_ptr<AudioRendererChangeInfo>> audioInfos;
    auto instance = AudioStreamManager::GetInstance();
#endif
    int32_t ret = -1;
#ifdef HAS_THERMAL_AUDIO_FRAMEWORK_PART
    if (instance == nullptr) {
        THERMAL_HILOGW(COMP_SVC, "instance is nullptr");
        return ret;
    }

    ret = instance->GetCurrentRendererChangeInfos(audioInfos);
#endif
    if (ret < ERR_OK) {
        return ret;
    }
#ifdef HAS_THERMAL_AUDIO_FRAMEWORK_PART
    if (audioInfos.size() <= 0) {
        THERMAL_HILOGD(COMP_SVC, "audioRendererChangeInfos: No Active Streams");
        return ERR_OK;
    }
    for (auto info = audioInfos.begin(); info != audioInfos.end(); ++info) {
        std::vector<std::string>::iterator it = find(uidList.begin(), uidList.end(),
            std::to_string(info->get()->clientUID));
        if (it != uidList.end()) {
            int32_t streamId = info->get()->sessionId;
            ret = AudioSystemManager::GetInstance()->SetLowPowerVolume(streamId, volume);
            if (ret < ERR_OK) {
                return ret;
            }
        }
    }
#endif
    return ERR_OK;
}

int32_t ActionVolume::VolumeExecution(float volume)
{
    int32_t ret = -1;
    char buf[MAX_PATH] = {0};
    ret = snprintf_s(buf, MAX_PATH, sizeof(buf) - 1, VOLUME_PATH);
    if (ret < ERR_OK) {
        return ret;
    }
    std::string valueString = std::to_string(volume) + "\n";
    ret = FileOperation::WriteFile(buf, valueString, valueString.length());
    if (ret != ERR_OK) {
        return ret;
    }
    return ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS
