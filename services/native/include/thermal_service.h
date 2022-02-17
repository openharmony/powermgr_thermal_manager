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

#ifndef THERMAL_SERVICE_H
#define THERMAL_SERVICE_H

#include <memory>
#include "delayed_sp_singleton.h"
#include "system_ability.h"
#include "iremote_object.h"

#include "action_popup.h"
#include "thermal_srv_stub.h"
#include "ithermal_level_callback.h"
#include "ithermal_temp_callback.h"
#include "thermal_srv_sensor_info.h"
#include "thermalsrv_event_handler.h"

#include "thermal_observer.h"
#include "thermal_sensor_info.h"
#include "thermal_service_subscriber.h"
#include "thermal_policy.h"
#include "state_machine.h"
#include "thermal_action_manager.h"
#include "thermal_config_base_info.h"
#include "thermal_config_sensor_cluster.h"
#include "thermal_callback_impl.h"
#include "thermal_types.h"
#include "thermal_interface_proxy.h"


namespace OHOS {
namespace PowerMgr {
using TypeTempMap = std::map<std::string, int32_t>;
using namespace OHOS::HDI::Thermal::V1_0;
class ThermalService final : public SystemAbility, public ThermalSrvStub {
    DECLARE_SYSTEM_ABILITY(ThermalService);
    DECLARE_DELAYED_SP_SINGLETON(ThermalService);
public:
    virtual void OnStart() override;
    virtual void OnStop() override;

    void SubscribeThermalTempCallback(const std::vector<std::string> &typeList,
        const sptr<IThermalTempCallback> &callback) override;
    void UnSubscribeThermalTempCallback(const sptr<IThermalTempCallback> &callback) override;
    void SubscribeThermalLevelCallback(const sptr<IThermalLevelCallback> &callback) override;
    void UnSubscribeThermalLevelCallback(const sptr<IThermalLevelCallback> &callback) override;
    bool GetThermalSrvSensorInfo(const SensorType &type, ThermalSrvSensorInfo& sensorInfo) override;
    void GetThermalLevel(ThermalLevel& level) override;
    virtual std::string ShellDump(const std::vector<std::string>& args, uint32_t argc) override;

    void HandleEvent(int event);
    void SendEvent(int32_t event, int64_t param, int64_t delayTime);
    void RemoveEvent(int32_t event);
    int32_t HandleThermalCallbackEvent(const HdfThermalCallbackInfo& event);

    void SetFlag(bool flag)
    {
        flag_ = flag;
    }

    bool GetFlag()
    {
        return flag_;
    }

    bool IsServiceReady() const
    {
        return ready_;
    }

    std::shared_ptr<ThermalsrvEventHandler> GetHandler() const
    {
        return handler_;
    }
    std::shared_ptr<AppExecFwk::EventRunner> GetEventRunner() const
    {
        return eventRunner_;
    }
    std::shared_ptr<ThermalConfigBaseInfo> GetBaseinfoObj() const
    {
        return baseInfo_;
    }
    std::shared_ptr<StateMachine> GetStateMachineObj() const
    {
        return state_;
    }
    std::shared_ptr<ThermalActionManager> GetActionManagerObj() const
    {
        return actionMgr_;
    }
    std::shared_ptr<ThermalPolicy> GetPolicy() const
    {
        return policy_;
    }
    std::shared_ptr<ThermalObserver> GetObserver() const
    {
        return observer_;
    }
    std::shared_ptr<ThermalSensorInfo> GetSensorInfo() const
    {
        return info_;
    }
    std::shared_ptr<ThermalServiceSubscriber> GetSubscriber() const
    {
        return serviceSubscriber_;
    }
    std::shared_ptr<ThermalConfigSensorCluster> GetSensorCluster() const
    {
        return cluster_;
    }

    std::shared_ptr<ActionPopup> GetActionPopup() const
    {
        return popup_;
    }

    sptr<IThermalInterface> GetThermalInterface() const
    {
        return thermalInterface_;
    }
private:
    bool Init();
    bool InitThermalDriver();
    bool InitThermalObserver();
    bool InitThermalAction();
    bool InitThermalPolicy();
    bool InitBaseInfo();
    bool InitSensorCluser();
    bool InitActionManager();
    bool InitStateMachine();
    bool InitModules();
    bool CreateConfigModule();
    bool ready_ {false};
    std::mutex mutex_;
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner_ {nullptr};
    std::shared_ptr<ThermalsrvEventHandler> handler_ {nullptr};
    std::shared_ptr<ThermalServiceSubscriber> serviceSubscriber_ {nullptr};
    std::shared_ptr<ThermalObserver> observer_ {nullptr} ;
    std::shared_ptr<ThermalSensorInfo> info_ {nullptr};
    std::shared_ptr<ThermalPolicy> policy_ {nullptr};
    std::shared_ptr<ThermalConfigBaseInfo> baseInfo_ {nullptr};
    std::shared_ptr<StateMachine> state_ {nullptr};
    std::shared_ptr<ThermalActionManager> actionMgr_ {nullptr};
    std::shared_ptr<ThermalConfigSensorCluster> cluster_ {nullptr};
    bool flag_ {false};
    sptr<IThermalInterface> thermalInterface_ {nullptr};
    std::shared_ptr<ActionPopup> popup_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_SERVICE_H