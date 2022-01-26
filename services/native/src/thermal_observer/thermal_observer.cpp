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
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    if (sensorTempCBDeathRecipient_ == nullptr) {
        sensorTempCBDeathRecipient_ = new SensorTempCallbackDeathRecipient();
    }

    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s init sensor type", __func__);
    InitSensorTypeMap();
    return true;
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s success", __func__);
}

void ThermalObserver::InitSensorTypeMap()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    std::vector<std::string> sensorType(TYPE_MAX_SIZE);
    auto baseInfo = g_service->GetBaseinfoObj();
    if (baseInfo == nullptr) return;
    auto typeList = baseInfo->GetSensorsType();

    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s sensorType size = %{public}zu", __func__, typeList.size());
    if (typeList.size() <= TYPE_MAX_SIZE) {
        typeList.resize(TYPE_MAX_SIZE);
    } else {
        return;
    }

    if (!typeList.empty()) {
        for (uint32_t i = 0; i < typeList.size(); i++) {
            THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s sensorType = %{public}s", __func__,
                typeList[i].c_str());
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
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s return", __func__);
}

void ThermalObserver::SetRegisterCallback(Callback &callback)
{
    callback_ = callback;
}

void ThermalObserver::SubscribeThermalTempCallback(const std::vector<std::string> &typeList,
    const sptr<IThermalTempCallback>& callback)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    std::lock_guard<std::mutex> lock(mutex_);
    THERMAL_RETURN_IF(callback == nullptr);
    auto object = callback->AsObject();
    THERMAL_RETURN_IF(object == nullptr);
    auto retIt = sensorTempListeners_.insert(callback);
    if (retIt.second) {
        object->AddDeathRecipient(sensorTempCBDeathRecipient_);
    }
    callbackTypeMap_.insert(std::make_pair(callback, typeList));
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE,
        "%{public}s, object=%{public}p, callback=%{public}p, listeners.size=%{public}d, insertOk=%{public}d",
        __func__, object.GetRefPtr(), callback.GetRefPtr(),
        static_cast<unsigned int>(sensorTempListeners_.size()), retIt.second);
}

void ThermalObserver::UnSubscribeThermalTempCallback(const sptr<IThermalTempCallback>& callback)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    std::lock_guard lock(mutex_);
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
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE,
        "%{public}s, object=%{public}p, callback=%{public}p, listeners.size=%{public}d, eraseNum=%{public}zu",
        __func__, object.GetRefPtr(), callback.GetRefPtr(), static_cast<unsigned int>(sensorTempListeners_.size()),
        eraseNum);
}

void ThermalObserver::NotifySensorTempChanged(IThermalTempCallback::TempCallbackMap &tempCbMap)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    static std::map<std::string, int32_t> preSensor;
    IThermalTempCallback::TempCallbackMap newTempCbMap;
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE,
        "%{public}s listeners.size = %{public}d, callbackTypeMap.size = %{public}zu",
        __func__, static_cast<unsigned int>(sensorTempListeners_.size()), callbackTypeMap_.size());
    if (sensorTempListeners_.empty()) return;
    for (auto& listener : sensorTempListeners_) {
        auto callbackIter = callbackTypeMap_.find(listener);
        if (callbackIter != callbackTypeMap_.end()) {
            THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s find callback", __func__);
            for (auto type : callbackIter->second) {
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
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    callbackinfo_ = info;

    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s: callbackinfo_ size = %{public}zu",
        __func__, callbackinfo_.size());

    if (callback_ != nullptr) {
        callback_(callbackinfo_);
    }

    NotifySensorTempChanged(callbackinfo_);
}

bool ThermalObserver::GetThermalSrvSensorInfo(const SensorType &type, ThermalSrvSensorInfo& sensorInfo)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter type=%{public}d",
        __func__, static_cast<uint32_t>(type));

    for (auto iter : typeMap_) {
        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "configType=%{public}s", iter.second.c_str());
    }
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "typeMap_=%{public}s", typeMap_[type].c_str());

    auto iter = callbackinfo_.find(typeMap_[type]);
    if (iter != callbackinfo_.end()) {
            THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "set temp for sensor");
            sensorInfo.SetType(typeMap_[type]);
            if (iter->second == INVALID_TEMP) {
                return false;
            } else {
                sensorInfo.SetTemp(iter->second);
            }
            return true;
    } else {
            THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "set invalid temp for sensor");
            sensorInfo.SetType(typeMap_[type]);
            sensorInfo.SetTemp(INVALID_TEMP);
            return false;
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
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s enter", __func__);
    if (remote == nullptr || remote.promote() == nullptr) {
        return;
    }
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "ThermalSensorTemp::%{public}s remote = %{public}p",
        __func__, remote.promote().GetRefPtr());
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