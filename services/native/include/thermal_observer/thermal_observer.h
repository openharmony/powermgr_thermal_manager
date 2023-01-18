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

#ifndef THERMAL_OBSERVER_H
#define THERMAL_OBSERVER_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <set>
#include "delayed_sp_singleton.h"

#include "ithermal_action_callback.h"
#include "ithermal_temp_callback.h"
#include "thermal_srv_sensor_info.h"

namespace OHOS {
namespace PowerMgr {
using TypeTempMap = std::map<std::string, int32_t>;

class ThermalService;
class ThermalObserver {
public:
    using Callback = std::function<void(TypeTempMap)>;
    ThermalObserver() {};
    explicit ThermalObserver(const wptr<ThermalService>& tms);
    ~ThermalObserver();

    bool Init();
    void OnReceivedSensorInfo(const TypeTempMap& info);
    void SubscribeThermalTempCallback(const std::vector<std::string>& typeList,
        const sptr<IThermalTempCallback>& callback);
    void UnSubscribeThermalTempCallback(const sptr<IThermalTempCallback>& callback);
    void SubscribeThermalActionCallback(const std::vector<std::string>& actionList,
        const std::string& desc, const sptr<IThermalActionCallback>& callback);
    void UnSubscribeThermalActionCallback(const sptr<IThermalActionCallback>& callback);
    void FindSubscribeActionValue();
    void NotifyActionChanged(const sptr<IThermalActionCallback>& listener,
        IThermalActionCallback::ActionCallbackMap& newActionCbMap);
    void NotifySensorTempChanged(IThermalTempCallback::TempCallbackMap& tempCbMap);
    bool CompareActionCallbackMap(const IThermalActionCallback::ActionCallbackMap& map1,
        const IThermalActionCallback::ActionCallbackMap& map2);
    void DecisionActionValue(const std::vector<std::string>& actionList,
        IThermalActionCallback::ActionCallbackMap& newActionCbMap);
    bool GetThermalSrvSensorInfo(const SensorType& type, ThermalSrvSensorInfo& sensorInfo);
    void SetRegisterCallback(Callback& callback);
    void SetSensorTemp(const std::string& type, const int32_t& temp);
    void SetDecisionValue(const std::string& actionName, const std::string& actionValue);
    /**
     * Get related function
     */
    int32_t GetTemp(const SensorType& type);
    std::map<SensorType, std::string> GetSensorType()
    {
        return typeMap_;
    }
    class SensorTempCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        SensorTempCallbackDeathRecipient() = default;
        virtual void OnRemoteDied(const wptr<IRemoteObject>& remote);
        virtual ~SensorTempCallbackDeathRecipient() = default;
    };

    class ActionCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        ActionCallbackDeathRecipient() = default;
        virtual void OnRemoteDied(const wptr<IRemoteObject>& remote);
        virtual ~ActionCallbackDeathRecipient() = default;
    };
private:
    void InitSensorTypeMap();
    struct classcomp {
        bool operator() (const sptr<IThermalTempCallback>& l, const sptr<IThermalTempCallback>& r) const
        {
            return l->AsObject() < r->AsObject();
        }
    };

    struct ClassActionComp {
        bool operator() (const sptr<IThermalActionCallback>& l, const sptr<IThermalActionCallback>& r) const
        {
            return l->AsObject() < r->AsObject();
        }
    };

    const wptr<ThermalService> tms_;
    std::mutex mutexActionCallback_;
    std::mutex mutexTempCallback_;
    std::mutex mutexActionMap_;
    std::mutex mutexCallbackInfo_;
    TypeTempMap callbackinfo_;
    sptr<IRemoteObject::DeathRecipient> sensorTempCBDeathRecipient_;
    sptr<IRemoteObject::DeathRecipient> actionCBDeathRecipient_;
    std::set<const sptr<IThermalTempCallback>, classcomp> sensorTempListeners_;
    std::map<const sptr<IThermalTempCallback>, std::vector<std::string>> callbackTypeMap_;
    std::set<const sptr<IThermalActionCallback>, ClassActionComp> actionListeners_;
    std::map<const sptr<IThermalActionCallback>, std::vector<std::string>> callbackActionMap_;
    Callback callback_;
    std::map<SensorType, std::string> typeMap_;
};
} // namespace PowerMgr
} // namespace OHOS

#endif // THERMAL_OBSERVER_H
