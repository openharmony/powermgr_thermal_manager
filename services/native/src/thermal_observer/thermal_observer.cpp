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
#include "ffrt_utils.h"

namespace OHOS {
namespace PowerMgr {
namespace {
auto g_service = DelayedSpSingleton<ThermalService>::GetInstance();
std::map<std::string, std::string> g_actionMap;
}
ThermalObserver::ThermalObserver(const wptr<ThermalService>& tms) : tms_(tms) {};
ThermalObserver::~ThermalObserver() {};

bool ThermalObserver::Init()
{
    if (sensorTempCBDeathRecipient_ == nullptr) {
        sensorTempCBDeathRecipient_ = new SensorTempCallbackDeathRecipient();
    }

    if (actionCBDeathRecipient_ == nullptr) {
        actionCBDeathRecipient_ = new ActionCallbackDeathRecipient();
    }

    InitSensorTypeMap();
    THERMAL_HILOGI(COMP_SVC, "ThermalObserver init succ");
    return true;
}

void ThermalObserver::InitSensorTypeMap()
{
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
            THERMAL_HILOGI(COMP_SVC, "sensorType = %{public}s", typeList[i].c_str());
            sensorType[i] = typeList[i];
        }
    }
    typeMap_.clear();
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
}

void ThermalObserver::SetRegisterCallback(Callback& callback)
{
    callback_ = callback;
}

void ThermalObserver::SubscribeThermalTempCallback(const std::vector<std::string>& typeList,
    const sptr<IThermalTempCallback>& callback)
{
    std::lock_guard<std::mutex> lock(mutexTempCallback_);
    THERMAL_RETURN_IF(callback == nullptr);
    auto object = callback->AsObject();
    THERMAL_RETURN_IF(object == nullptr);
    auto retIt = sensorTempListeners_.insert(callback);
    if (retIt.second) {
        object->AddDeathRecipient(sensorTempCBDeathRecipient_);
        callbackTypeMap_.insert(std::make_pair(callback, typeList));
        THERMAL_HILOGI(COMP_SVC, "add new temp listener, listeners.size=%{public}zu", sensorTempListeners_.size());
    } else {
        THERMAL_HILOGW(COMP_SVC, "subscribe failed, temp callback duplicate subscription!");
    }
}

void ThermalObserver::UnSubscribeThermalTempCallback(const sptr<IThermalTempCallback>& callback)
{
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
    THERMAL_HILOGI(COMP_SVC, "erase temp listener, listeners.size=%{public}zu, eraseNum=%{public}zu",
        sensorTempListeners_.size(), eraseNum);
}

void ThermalObserver::SubscribeThermalActionCallback(const std::vector<std::string>& actionList,
    const std::string& desc, const sptr<IThermalActionCallback>& callback)
{
    std::lock_guard<std::mutex> lock(mutexActionCallback_);
    THERMAL_RETURN_IF(callback == nullptr);
    auto object = callback->AsObject();
    THERMAL_RETURN_IF(object == nullptr);
    auto retIt = actionListeners_.insert(callback);
    if (retIt.second) {
        object->AddDeathRecipient(actionCBDeathRecipient_);
        callbackActionMap_.insert(std::make_pair(callback, actionList));
        THERMAL_HILOGI(COMP_SVC, "add new action listener, listeners.size=%{public}zu", actionListeners_.size());
    } else {
        THERMAL_HILOGW(COMP_SVC, "subscribe failed, action callback duplicate subscription!");
    }
}

void ThermalObserver::UnSubscribeThermalActionCallback(const sptr<IThermalActionCallback>& callback)
{
    std::lock_guard lock(mutexActionCallback_);
    THERMAL_RETURN_IF(callback == nullptr);
    auto object = callback->AsObject();
    THERMAL_RETURN_IF(object == nullptr);
    auto callbackIter = callbackActionMap_.find(callback);
    if (callbackIter != callbackActionMap_.end()) {
        callbackActionMap_.erase(callbackIter);
    }
    size_t eraseNum = actionListeners_.erase(callback);
    if (eraseNum != 0) {
        object->RemoveDeathRecipient(actionCBDeathRecipient_);
    }
    THERMAL_HILOGI(COMP_SVC, "erase action listener, listeners.size=%{public}zu, eraseNum=%{public}zu",
        actionListeners_.size(), eraseNum);
}

void ThermalObserver::PrintAction()
{
    std::string thermalActionLog;
    for (auto actionIter = g_actionMap.begin(); actionIter != g_actionMap.end(); ++actionIter) {
        thermalActionLog.append(actionIter->first).append("=").append(actionIter->second).append("|");
    }
    THERMAL_HILOGD(COMP_SVC, "execute action {listener:%{public}zu|%{public}zu} {%{public}s}.",
        sensorTempListeners_.size(), actionListeners_.size(), thermalActionLog.c_str());
}

void ThermalObserver::FindSubscribeActionValue()
{
    std::lock_guard lock(mutexActionCallback_);
    if (g_actionMap.empty()) {
        THERMAL_HILOGD(COMP_SVC, "no action");
        return;
    }
    PrintAction();
    if (actionListeners_.empty()) {
        THERMAL_HILOGD(COMP_SVC, "no subscribe");
        return;
    }

    IThermalActionCallback::ActionCallbackMap newActionCbMap;
    for (auto& listener : actionListeners_) {
        auto actionIter = callbackActionMap_.find(listener);
        if (actionIter != callbackActionMap_.end()) {
            THERMAL_HILOGD(COMP_SVC, "find callback.");
            DecisionActionValue(actionIter->second, newActionCbMap);
        }

        listener->OnThermalActionChanged(newActionCbMap);
    }
    g_actionMap.clear();
}

void ThermalObserver::DecisionActionValue(const std::vector<std::string>& actionList,
    IThermalActionCallback::ActionCallbackMap& newActionCbMap)
{
    std::lock_guard lock(mutexActionMap_);
    for (const auto& action : actionList) {
        THERMAL_HILOGD(COMP_SVC, "subscribe action is %{public}s.", action.c_str());
        for (auto actionIter = g_actionMap.begin(); actionIter != g_actionMap.end(); ++actionIter) {
            THERMAL_HILOGD(COMP_SVC, "xml action is %{public}s.", actionIter->first.c_str());
            if (action == actionIter->first) {
                newActionCbMap.insert(std::make_pair(action, actionIter->second));
            }
        }
    }
}

void ThermalObserver::SetDecisionValue(const std::string& actionName, const std::string& actionValue)
{
    std::lock_guard lock(mutexActionMap_);
    THERMAL_HILOGD(
        COMP_SVC, "actionName = %{public}s, actionValue = %{public}s", actionName.c_str(), actionValue.c_str());
    auto iter = g_actionMap.find(actionName);
    if (iter != g_actionMap.end()) {
        iter->second = actionValue;
    } else {
        g_actionMap.insert(std::make_pair(actionName, actionValue));
    }
}

void ThermalObserver::NotifySensorTempChanged(IThermalTempCallback::TempCallbackMap& tempCbMap)
{
    std::lock_guard lockTempCallback(mutexTempCallback_);
    static std::map<std::string, int32_t> preSensor;
    IThermalTempCallback::TempCallbackMap newTempCbMap;
    THERMAL_HILOGD(COMP_SVC,
        "listeners.size = %{public}zu, callbackTypeMap.size = %{public}zu",
        sensorTempListeners_.size(), callbackTypeMap_.size());
    if (sensorTempListeners_.empty()) {
        return;
    }
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

void ThermalObserver::OnReceivedSensorInfo(const TypeTempMap& info)
{
    {
        std::lock_guard lock(mutexCallbackInfo_);
        callbackinfo_ = info;
    }
    THERMAL_HILOGD(COMP_SVC, "callbackinfo_ size = %{public}zu", callbackinfo_.size());

    if (callback_ != nullptr) {
        callback_(callbackinfo_);
    }

    NotifySensorTempChanged(callbackinfo_);
}

bool ThermalObserver::GetThermalSrvSensorInfo(const SensorType& type, ThermalSrvSensorInfo& sensorInfo)
{
    for (auto iter : typeMap_) {
        THERMAL_HILOGD(COMP_SVC, "configType=%{public}s", iter.second.c_str());
    }
    THERMAL_HILOGI(COMP_SVC, "typeMap_=%{public}s", typeMap_[type].c_str());

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
        return false;
    }
    return false;
}

int32_t ThermalObserver::GetTemp(const SensorType& type)
{
    ThermalSrvSensorInfo info;
    GetThermalSrvSensorInfo(type, info);
    return info.GetTemp();
}

void ThermalObserver::SensorTempCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    if (remote == nullptr || remote.promote() == nullptr) {
        return;
    }
    THERMAL_HILOGI(COMP_SVC, "ThermalSensorTemp::OnRemoteDied remote");
    auto pms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (pms == nullptr) {
        return;
    }
    sptr<IThermalTempCallback> callback = iface_cast<IThermalTempCallback>(remote.promote());
    FFRTTask task = std::bind(&ThermalService::UnSubscribeThermalTempCallback, pms, callback);
    FFRTUtils::SubmitTask(task);
}

void ThermalObserver::ActionCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    if (remote == nullptr || remote.promote() == nullptr) {
        return;
    }
    THERMAL_HILOGI(COMP_SVC, "ThermalAction::OnRemoteDied remote");
    auto pms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (pms == nullptr) {
        return;
    }
    sptr<IThermalActionCallback> callback = iface_cast<IThermalActionCallback>(remote.promote());
    FFRTTask task = std::bind(&ThermalService::UnSubscribeThermalActionCallback, pms, callback);
    FFRTUtils::SubmitTask(task);
}
} // namespace PowerMgr
} // namespace OHOS
