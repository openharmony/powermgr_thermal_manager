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

#include "thermal_observer.h"

#include <datetime_ex.h>

#include "ithermal_temp_callback.h"
#include "constants.h"
#include "string_operation.h"
#include "thermal_config_base_info.h"
#include "thermal_common.h"
#include "thermal_service.h"

namespace OHOS {
namespace PowerMgr {
namespace {
auto g_service = DelayedSpSingleton<ThermalService>::GetInstance();
const std::string TASK_UNREG_SENSOR_TEMP_CALLBACK = "SensorTemp_UnRegSensorTempCB";
}
ThermalObserver::ThermalObserver(const wptr<ThermalService>& tms) : tms_(tms) {};
ThermalObserver::~ThermalObserver() {};

bool ThermalObserver::Init()
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    if (sensorTempCBDeathRecipient_ == nullptr) {
        sensorTempCBDeathRecipient_ = new SensorTempCallbackDeathRecipient();
    }

    InitSensorTypeMap();
    THERMAL_HILOGD(COMP_SVC, "Exit");
    return true;
}

void ThermalObserver::InitSensorTypeMap()
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    std::vector<std::string> sensorType(TYPE_MAX_SIZE);
    auto baseInfo = g_service->GetBaseinfoObj();
    if (baseInfo == nullptr) return;
    auto typeList = baseInfo->GetSensorsType();

    THERMAL_HILOGD(COMP_SVC, "sensorType size = %{public}zu", typeList.size());
    if (typeList.size() <= TYPE_MAX_SIZE) {
        typeList.resize(TYPE_MAX_SIZE);
    } else {
        return;
    }

    if (!typeList.empty()) {
        for (uint32_t i = 0; i < typeList.size(); i++) {
            THERMAL_HILOGD(COMP_SVC, "sensorType = %{public}s", typeList[i].c_str());
            sensorType[i] = typeList[i];
        }
    }
    typeMap_.insert(std::make_pair(SensorType::SOC, sensorType[ARG_0]));
    typeMap_.insert(std::make_pair(SensorType::BATTERY, sensorType[ARG_1]));
    typeMap_.insert(std::make_pair(SensorType::SHELL, sensorType[ARG_2]));
    typeMap_.insert(std::make_pair(SensorType::SENSOR1, sensorType[ARG_3]));
    typeMap_.insert(std::make_pair(SensorType::SENSOR2, sensorType[ARG_4]));
    typeMap_.insert(std::make_pair(SensorType::SENSOR3, sensorType[ARG_5]));
    typeMap_.insert(std::make_pair(SensorType::SENSOR4, sensorType[ARG_6]));
    typeMap_.insert(std::make_pair(SensorType::SENSOR5, sensorType[ARG_7]));
    typeMap_.insert(std::make_pair(SensorType::SENSOR6, sensorType[ARG_8]));
    typeMap_.insert(std::make_pair(SensorType::SENSOR7, sensorType[ARG_9]));
    THERMAL_HILOGD(COMP_SVC, "Exit");
}

void ThermalObserver::SetRegisterCallback(Callback &callback)
{
    callback_ = callback;
}

void ThermalObserver::SubscribeThermalTempCallback(const std::vector<std::string> &typeList,
    const sptr<IThermalTempCallback>& callback)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    std::lock_guard<std::mutex> lock(mutexTempCallback_);
    THERMAL_RETURN_IF(callback == nullptr);
    auto object = callback->AsObject();
    THERMAL_RETURN_IF(object == nullptr);
    auto retIt = sensorTempListeners_.insert(callback);
    if (retIt.second) {
        object->AddDeathRecipient(sensorTempCBDeathRecipient_);
    }
    callbackTypeMap_.insert(std::make_pair(callback, typeList));
    THERMAL_HILOGD(COMP_SVC, "listeners.size=%{public}d, insertOk=%{public}d",
        static_cast<unsigned int>(sensorTempListeners_.size()), retIt.second);
}

void ThermalObserver::UnSubscribeThermalTempCallback(const sptr<IThermalTempCallback>& callback)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    std::lock_guard lock(mutexTempCallback_);
    THERMAL_RETURN_IF(callback == nullptr);
    auto object = callback->AsObject();
    THERMAL_RETURN_IF(object == nullptr);
    auto callbackIter = callbackTypeMap_.find(callback);
    if (callbackIter != callbackTypeMap_.end()) {
        callbackTypeMap_.erase(callbackIter);
    }
    size_t eraseNum = sensorTempListeners_.erase(callback);
    if (eraseNum != 0) {
        object->RemoveDeathRecipient(sensorTempCBDeathRecipient_);
    }
    THERMAL_HILOGD(COMP_SVC, "listeners.size=%{public}d, eraseNum=%{public}zu",
        static_cast<unsigned int>(sensorTempListeners_.size()), eraseNum);
}

void ThermalObserver::NotifySensorTempChanged(IThermalTempCallback::TempCallbackMap &tempCbMap)
{
    std::lock_guard lockTempCallback(mutexTempCallback_);
    static std::map<std::string, int32_t> preSensor;
    IThermalTempCallback::TempCallbackMap newTempCbMap;
    if (sensorTempListeners_.empty()) return;
    for (auto& listener : sensorTempListeners_) {
        auto callbackIter = callbackTypeMap_.find(listener);
        if (callbackIter != callbackTypeMap_.end()) {
            THERMAL_HILOGD(COMP_SVC, "find callback");
            for (auto type : callbackIter->second) {
                std::lock_guard lockCallbackInfo(mutexCallbackInfo_);
                if (preSensor[type] != tempCbMap[type]) {
                    newTempCbMap.insert(std::make_pair(type, tempCbMap[type]));
                    preSensor[type] = tempCbMap[type];
                }
            }
        }
        listener->OnThermalTempChanged(newTempCbMap);
    }
}

void ThermalObserver::OnReceivedSensorInfo(const TypeTempMap &info)
{
    {
        std::lock_guard lock(mutexCallbackInfo_);
        callbackinfo_ = info;
    }

    if (callback_ != nullptr) {
        callback_(callbackinfo_);
    }

    NotifySensorTempChanged(callbackinfo_);
}

bool ThermalObserver::GetThermalSrvSensorInfo(const SensorType &type, ThermalSrvSensorInfo& sensorInfo)
{
    std::lock_guard lock(mutexCallbackInfo_);
    auto iter = callbackinfo_.find(typeMap_[type]);
    if (iter != callbackinfo_.end()) {
        THERMAL_HILOGD(COMP_SVC, "set temp for sensor");
        sensorInfo.SetType(typeMap_[type]);
        if (iter->second == INVALID_TEMP) {
            return false;
        } else {
            sensorInfo.SetTemp(iter->second);
        }
        return true;
    } else {
        THERMAL_HILOGD(COMP_SVC, "set invalid temp for sensor");
        sensorInfo.SetType(typeMap_[type]);
        sensorInfo.SetTemp(INVALID_TEMP);
    }
    return false;
}

int32_t ThermalObserver::GetTemp(const SensorType &type)
{
    ThermalSrvSensorInfo info;
    GetThermalSrvSensorInfo(type, info);
    return info.GetTemp();
}

void ThermalObserver::SensorTempCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    if (remote == nullptr || remote.promote() == nullptr) {
        return;
    }
    auto pms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (pms == nullptr) {
        return;
    }

    auto handler = pms->GetHandler();
    if (handler == nullptr) {
        return;
    }
    sptr<IThermalTempCallback> callback = iface_cast<IThermalTempCallback>(remote.promote());
    std::function<void ()> unRegFunc = std::bind(&ThermalService::UnSubscribeThermalTempCallback, pms, callback);
    handler->PostTask(unRegFunc, TASK_UNREG_SENSOR_TEMP_CALLBACK);
}
} // namespace PowerMgr
} // namespace OHOS
