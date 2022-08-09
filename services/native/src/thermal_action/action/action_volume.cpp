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
#include "action_volume.h"

#include <algorithm>
#include "audio_system_manager.h"
#include "audio_stream_manager.h"
#include "file_operation.h"
#include "thermal_hisysevent.h"
#include "thermal_service.h"
#include "securec.h"
#include "string_operation.h"

using namespace OHOS::AudioStandard;
namespace OHOS {
namespace PowerMgr {
namespace {
auto g_service = DelayedSpSingleton<ThermalService>::GetInstance();
constexpr const char* VOLUME_PATH = "/data/service/el0/thermal/config/volume";
const int MAX_PATH = 256;
std::map<std::string, std::string> g_sceneMap;
std::vector<ActionItem> g_actionInfo;
}
ActionVolume::ActionVolume(const std::string& actionName)
{
    actionName_ = actionName;
}

void ActionVolume::InitParams(const std::string& params)
{
}

void ActionVolume::SetStrict(bool flag)
{
    flag_ = flag;
}

void ActionVolume::SetEnableEvent(bool enable)
{
    enableEvent_ = enable;
}

void ActionVolume::SetXmlScene(const std::string& scene, const std::string& value)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    for (auto iter = g_sceneMap.begin(); iter != g_sceneMap.end(); ++iter) {
        if (iter->first == scene) {
            if (iter->second != value) {
                iter->second = value;
            }
            return;
        }
    }

    g_sceneMap.insert(std::make_pair(scene, value));
}

void ActionVolume::AddActionValue(std::string value)
{
    if (value.empty()) {
        return;
    }
    valueList_.push_back(atof(value.c_str()));
}

void ActionVolume::Execute()
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    float value;
    if (valueList_.empty()) {
        value = 0.0;
    } else {
        if (flag_) {
            value = *max_element(valueList_.begin(), valueList_.end());
        } else {
            value = *min_element(valueList_.begin(), valueList_.end());
        }
        valueList_.clear();
    }
    if (value != lastValue_) {
        if (!g_service->GetSimulationXml()) {
            VolumeRequest(value);
        } else {
            VolumeExecution(value);
        }
        lastValue_ = value;
    }
}

int32_t ActionVolume::VolumeRequest(float volume)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    std::string uid;
    std::vector<std::string> uidList;
    int32_t ret = -1;
    g_actionInfo = g_service->GetActionManagerObj()->GetActionItem();
    for (auto aInfo : g_actionInfo) {
        if (aInfo.name == "volume") {
            uid = aInfo.uid;
        }
    }
    StringOperation::SplitString(uid, uidList, ",");
    std::vector<std::unique_ptr<AudioRendererChangeInfo>> audioInfos;
    auto instance = AudioStreamManager::GetInstance();
    if (instance == nullptr) {
        THERMAL_HILOGW(COMP_SVC, "instance is nullptr");
        return ret;
    }

    ret = instance->GetCurrentRendererChangeInfos(audioInfos);
    if (ret < ERR_OK) {
        return ret;
    }
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
    return ERR_OK;
}

int32_t ActionVolume::VolumeExecution(float volume)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
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