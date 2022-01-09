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

#include "thermal_service.h"

#include <unistd.h>
#include <ipc_skeleton.h>
#include "file_ex.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "constants.h"
#include "thermal_srv_config_parser.h"
#include "thermal_common.h"

namespace OHOS {
namespace PowerMgr {
namespace {
std::string path = "/system/etc/thermal_config/thermal_service_config.xml";
const std::string THMERMAL_SERVICE_NAME = "ThermalService";
auto g_service = DelayedSpSingleton<ThermalService>::GetInstance();
const bool G_REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(g_service.GetRefPtr());
}
ThermalService::ThermalService() : SystemAbility(POWER_MANAGER_THERMAL_SERVICE_ID, true) {}

ThermalService::~ThermalService() {}

void ThermalService::OnStart()
{
    int time = 100;
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "OnStart Enter");
    if (ready_) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "OnStart is ready, nothing to do");
        return;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(time));

    if (!(Init())) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "OnStart call init fail");
        return;
    }

    RemoveEvent(ThermalsrvEventHandler::SEND_LOAD_THERMALHDF_MSG);
    SendEvent(ThermalsrvEventHandler::SEND_LOAD_THERMALHDF_MSG, 0, 0);
    if (!Publish(DelayedSpSingleton<ThermalService>::GetInstance())) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "OnStart register to system ability manager failed.");
        return;
    }
    ready_ = true;
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "OnStart and add system ability success");
}

bool ThermalService::Init()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "Init start");
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_TO_SLEEP));

    if (!eventRunner_) {
        eventRunner_ = AppExecFwk::EventRunner::Create(THMERMAL_SERVICE_NAME);
        if (eventRunner_ == nullptr) {
            THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "Init failed due to create EventRunner");
            return false;
        }
    }

    if (!handler_) {
        handler_ = std::make_shared<ThermalsrvEventHandler>(eventRunner_, g_service);
        if (handler_ == nullptr) {
            THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "Init failed due to create handler error");
            return false;
        }
    }
    
    if (!baseInfo_) {
        baseInfo_ = std::make_shared<ThermalConfigBaseInfo>();
        if (baseInfo_ == nullptr) {
            THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "failed to create base info");
            return false;
        }
    }

    if (!state_) {
        state_ = std::make_shared<StateMachine>();
        if (state_ == nullptr) {
            THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "failed to create state machine");
            return false;
        }
    }

    if (!actionMgr_) {
        actionMgr_ = std::make_shared<ThermalActionManager>();
        if (actionMgr_ == nullptr) {
            THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "failed to create action manager");
            return false;
        }
    }

    if (cluster_ == nullptr) {
        cluster_ = std::make_shared<ThermalConfigSensorCluster>();
        if (cluster_ == nullptr) {
            THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "failed to create cluster");
            return false;
        }
    }

    if (!policy_) {
        policy_ = std::make_shared<ThermalPolicy>();
        if (policy_ == nullptr) {
            THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "failed to create thermal policy");
            return false;
        }
    }

    if (!InitModules()) {
        return false;
    }

    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "Init success");
    return true;
}

bool ThermalService::InitModules()
{
    if (!ThermalSrvConfigParser::GetInstance().ThermalSrvConfigInit(path)) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "thermal service config init fail");
        return false;
    }

    if (!InitThermalObserver()) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "thermal observer start fail");
        return false;
    }

    if (!InitStateMachine()) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "state machine init fail");
        return false;
    }

    if (!InitActionManager()) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "actiom manager init fail");
        return false;
    }

    if (!InitThermalPolicy()) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "thermal policy start fail");
        return false;
    }
    return true;
}
bool ThermalService::InitThermalObserver()
{
    if (!InitBaseInfo()) {
        return false;
    }

    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "InitThermalObserver: Init Enter");
    if (observer_ == nullptr) {
        observer_ = std::make_shared<ThermalObserver>(g_service);
        if (!(observer_->Init())) {
            THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "InitThermalObserver: thermal observer start fail");
            return false;
        }
    }
    if (info_ == nullptr) {
        info_ = std::make_shared<ThermalSensorInfo>();
    }
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "InitThermalObserver: Init Success");
    return true;
}

bool ThermalService::InitBaseInfo()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "InitBaseInfo: Init Enter");
    if (!baseInfo_->Init()) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "InitBaseInfo: base info init failed");
        return false;
    }
    return true;
}

bool ThermalService::InitStateMachine()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "InitStateMachine: Init Enter");
    if (!state_->Init()) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "InitStateMachine: state machine init failed");
        return false;
    }
    return true;
}

bool ThermalService::InitActionManager()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "InitActionManager: Init Enter");
    if (!actionMgr_->Init()) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "InitActionManager: action manager init failed");
        return false;
    }
    return true;
}

bool ThermalService::InitThermalPolicy()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "InitThermalPolicy: Init Enter");
    if (!policy_->Init()) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "InitThermalPolicy: policy init failed");
        return false;
    }
    return true;
}

bool ThermalService::InitThermalDriver()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "Thermal Driver service Enter");
    if (serviceSubscriber_ == nullptr) {
        serviceSubscriber_ = std::make_shared<ThermalServiceSubscriber>();
        if (!(serviceSubscriber_->Init())) {
            THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "thermal service suvscriber start fail");
            return false;
        }
    }
    serviceSubscriber_->RegisterTempChanged();
    sptr<ThermalSubscriber> thermalClientSubscriber = serviceSubscriber_->GetThermalSubscriber();
    ErrCode ret = ThermalClient::BindThermalDriverSubscriber(thermalClientSubscriber);
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "InitThermalDriver ret: %{public}d", ret);
    return SUCCEEDED(ret);
}

void ThermalService::OnStop()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "stop service");
    if (!ready_) {
        return;
    }
    eventRunner_.reset();
    handler_.reset();
    ready_ = false;
    ThermalClient::UnbindThermalDriverSubscriber();
}

bool ThermalService::SetHdiFlag(bool flag)
{
    ErrCode ret = ThermalClient::SetHdiFlag(flag);
    return SUCCEEDED(ret);
}

void ThermalService::SubscribeThermalTempCallback(const std::vector<std::string> &typeList,
    const sptr<IThermalTempCallback> &callback)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "ThermalService::SubscribeThermalTempCallback Enter");
    auto uid = IPCSkeleton::GetCallingUid();
    if (uid > APP_FIRST_UID) {
        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s Request failed, %{public}d permission check fail",
            __func__, uid);
        return;
    }
    observer_->SubscribeThermalTempCallback(typeList, callback);
}

void ThermalService::UnSubscribeThermalTempCallback(const sptr<IThermalTempCallback> &callback)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "ThermalService::UnSubscribeThermalTempCallback Enter");
    auto uid = IPCSkeleton::GetCallingUid();
    if (uid > APP_FIRST_UID) {
        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s Request failed, %{public}d permission check fail",
            __func__, uid);
        return;
    }
    observer_->UnSubscribeThermalTempCallback(callback);
}

bool ThermalService::GetThermalSrvSensorInfo(const SensorType &type, ThermalSrvSensorInfo& sensorInfo)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "ThermalService::GetThermalSrvSensorInfo Enter");
    if (!(observer_->GetThermalSrvSensorInfo(type, sensorInfo))) {
            THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "failed to get temp for sensor type");
            return false;
    }
    return true;
}

void ThermalService::SetSensorTemp(const std::string &type, const int32_t &temp)
{
    observer_->SetSensorTemp(type, temp);
}

void ThermalService::SubscribeThermalLevelCallback(const sptr<IThermalLevelCallback> &callback)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "ThermalService::SubscribeThermalLevelCallback Enter");
    auto uid = IPCSkeleton::GetCallingUid();
    if (uid > APP_FIRST_UID) {
        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s Request failed, %{public}d permission check fail",
            __func__, uid);
        return;
    }
    actionMgr_->SubscribeThermalLevelCallback(callback);
}

void ThermalService::UnSubscribeThermalLevelCallback(const sptr<IThermalLevelCallback> &callback)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "ThermalService::UnSubscribeThermalLevelCallback Enter");
    auto uid = IPCSkeleton::GetCallingUid();
    if (uid > APP_FIRST_UID) {
        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s Request failed, %{public}d permission check fail",
            __func__, uid);
        return;
    }
    actionMgr_->UnSubscribeThermalLevelCallback(callback);
}

void ThermalService::GetThermalLevel(ThermalLevel& level)
{
    uint32_t levelValue =  actionMgr_->GetThermalLevel();
    level = static_cast<ThermalLevel>(levelValue);
}

void ThermalService::SendEvent(int32_t event, int64_t param, int64_t delayTime)
{
    handler_->SendEvent(event, param, delayTime);
}

void ThermalService::RemoveEvent(int32_t event)
{
    handler_->RemoveEvent(event);
}

void ThermalService::HandleEvent(int event)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s start", __func__);
    switch (event) {
        case ThermalsrvEventHandler::SEND_LOAD_THERMALHDF_MSG: {
            THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "InitThermalObserver: Init thermal hdf");
            if (!(InitThermalDriver())) {
                THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "OnStart call initThermalDriver fail");
                return;
            }
            break;
        }
        default:
            break;
    }
}
} // namespace PowerMgr
} // namespace OHOS