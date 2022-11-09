/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "delayed_sp_singleton.h"
#include "iremote_object.h"
#include "system_ability.h"
#include <memory>

#include "action_popup.h"
#include "ithermal_level_callback.h"
#include "ithermal_temp_callback.h"
#include "thermal_srv_sensor_info.h"
#include "thermal_srv_stub.h"
#include "thermalsrv_event_handler.h"

#include "hdi_service_status_listener.h"
#include "state_machine.h"
#include "thermal_action_manager.h"
#include "thermal_callback.h"
#include "thermal_config_base_info.h"
#include "thermal_config_sensor_cluster.h"
#include "thermal_observer.h"
#include "thermal_policy.h"
#include "thermal_sensor_info.h"
#include "thermal_service_subscriber.h"
#include "v1_0/ithermal_interface.h"
#include "v1_0/thermal_types.h"

namespace OHOS {
namespace PowerMgr {
using TypeTempMap = std::map<std::string, int32_t>;
using namespace OHOS::HDI::Thermal::V1_0;
using namespace OHOS::HDI::ServiceManager::V1_0;
class ThermalService final : public SystemAbility, public ThermalSrvStub {
    DECLARE_SYSTEM_ABILITY(ThermalService);
    DECLARE_DELAYED_SP_SINGLETON(ThermalService);

public:
    virtual void OnStart() override;
    virtual void OnStop() override;
    virtual void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    virtual int32_t Dump(int fd, const std::vector<std::u16string>& args) override;
    bool SubscribeThermalTempCallback(
        const std::vector<std::string>& typeList, const sptr<IThermalTempCallback>& callback) override;
    bool UnSubscribeThermalTempCallback(const sptr<IThermalTempCallback>& callback) override;
    bool SubscribeThermalLevelCallback(const sptr<IThermalLevelCallback>& callback) override;
    bool UnSubscribeThermalLevelCallback(const sptr<IThermalLevelCallback>& callback) override;
    bool SubscribeThermalActionCallback(const std::vector<std::string>& actionList, const std::string& desc,
        const sptr<IThermalActionCallback>& callback) override;
    bool UnSubscribeThermalActionCallback(const sptr<IThermalActionCallback>& callback) override;
    bool GetThermalSrvSensorInfo(const SensorType& type, ThermalSrvSensorInfo& sensorInfo) override;
    bool GetThermalLevel(ThermalLevel& level) override;
    bool GetThermalInfo() override;
    bool SetScene(const std::string& scene) override;
    virtual std::string ShellDump(const std::vector<std::string>& args, uint32_t argc) override;

    void HandleEvent(int event);
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

    bool GetSimulationXml()
    {
        return isSimulation_;
    }

    std::string GetScene()
    {
        return scene_;
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
    void RegisterHdiStatusListener();
    void RegisterThermalHdiCallback();
    void SendEvent(int32_t event, int64_t delayTime);
    bool ready_ {false};
    bool isSimulation_ {false};
    std::mutex mutex_;
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner_ {nullptr};
    std::shared_ptr<ThermalsrvEventHandler> handler_ {nullptr};
    std::shared_ptr<ThermalServiceSubscriber> serviceSubscriber_ {nullptr};
    std::shared_ptr<ThermalObserver> observer_ {nullptr};
    std::shared_ptr<ThermalSensorInfo> info_ {nullptr};
    std::shared_ptr<ThermalPolicy> policy_ {nullptr};
    std::shared_ptr<ThermalConfigBaseInfo> baseInfo_ {nullptr};
    std::shared_ptr<StateMachine> state_ {nullptr};
    std::shared_ptr<ThermalActionManager> actionMgr_ {nullptr};
    std::shared_ptr<ThermalConfigSensorCluster> cluster_ {nullptr};
    bool flag_ {false};
    sptr<IThermalInterface> thermalInterface_ {nullptr};
    sptr<IServiceManager> hdiServiceMgr_ {nullptr};
    sptr<HdiServiceStatusListener::IServStatListener> hdiServStatListener_ {nullptr};
    std::shared_ptr<ActionPopup> popup_;
    std::string scene_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_SERVICE_H
