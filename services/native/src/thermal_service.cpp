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

#include <fcntl.h>
#include <ipc_skeleton.h>
#include <unistd.h>
#include "file_ex.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "securec.h"
#include "system_ability_definition.h"

#include "constants.h"
#include "thermal_mgr_dumper.h"
#include "thermal_srv_config_parser.h"
#include "thermal_common.h"

namespace OHOS {
namespace PowerMgr {
namespace {
std::string path = "/system/etc/thermal_config/thermal_service_config.xml";
constexpr const char *HDF_SERVICE_NAME = "thermal_interface_service";
const std::string THMERMAL_SERVICE_NAME = "ThermalService";
auto g_service = DelayedSpSingleton<ThermalService>::GetInstance();
constexpr int32_t STATUS_OK = 0;
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

    if (!CreateConfigModule()) {
        return false;
    }

    if (thermalInterface_ == nullptr) {
        thermalInterface_ = IThermalInterface::Get();
        if (thermalInterface_ == nullptr) {
            THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "failed to get thermal hdf interface");
            return false;
        }
    }

    if (!RigisterHdfStatusListener()) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "hdf status register fail");
        return false;
    }

    if (!InitModules()) {
        return false;
    }
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "Init success");
    return true;
}

bool ThermalService::CreateConfigModule()
{
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
    return true;
}

bool ThermalService::InitModules()
{
    if (!ThermalSrvConfigParser::GetInstance().ThermalSrvConfigInit(path)) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "thermal service config init fail");
        return false;
    }

    if (popup_ == nullptr) {
        popup_ = std::make_shared<ActionPopup>();
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

void ThermalService::OnStop()
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "stop service");
    if (!ready_) {
        return;
    }
    eventRunner_.reset();
    handler_.reset();
    ready_ = false;
    thermalInterface_->Unregister();
    servmgr_->UnregisterServiceStatusListener(hdfListener_);
}

void ThermalService::SubscribeThermalTempCallback(const std::vector<std::string> &typeList,
    const sptr<IThermalTempCallback> &callback)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "ThermalService::SubscribeThermalTempCallback Enter");
    auto uid = IPCSkeleton::GetCallingUid();
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s uid %{public}d ",
        __func__, uid);
    observer_->SubscribeThermalTempCallback(typeList, callback);
}

void ThermalService::UnSubscribeThermalTempCallback(const sptr<IThermalTempCallback> &callback)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "ThermalService::UnSubscribeThermalTempCallback Enter");
    auto uid = IPCSkeleton::GetCallingUid();
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s uid %{public}d ",
        __func__, uid);
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

void ThermalService::SubscribeThermalLevelCallback(const sptr<IThermalLevelCallback> &callback)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "ThermalService::SubscribeThermalLevelCallback Enter");
    auto uid = IPCSkeleton::GetCallingUid();
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s uid %{public}d ",
        __func__, uid);
    actionMgr_->SubscribeThermalLevelCallback(callback);
}

void ThermalService::UnSubscribeThermalLevelCallback(const sptr<IThermalLevelCallback> &callback)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "ThermalService::UnSubscribeThermalLevelCallback Enter");
    auto uid = IPCSkeleton::GetCallingUid();
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s uid %{public}d ",
        __func__, uid);
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
        case ThermalsrvEventHandler::SEND_GET_THERMAL_HDF_SERVICE_MSG: {
            int32_t ret = -1;
            if (serviceSubscriber_ == nullptr) {
                serviceSubscriber_ = std::make_shared<ThermalServiceSubscriber>();
                if (!(serviceSubscriber_->Init())) {
                    THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "thermal service suvscriber start fail");
                    return;
                }
            }
            sptr<IThermalCallback> g_callback = new ThermalCallbackImpl();
            ThermalCallbackImpl::ThermalEventCallback eventCb =
                std::bind(&ThermalService::HandleThermalCallbackEvent, this, std::placeholders::_1);
            ThermalCallbackImpl::RegisterThermalEvent(eventCb);
            ret = thermalInterface_->Register(g_callback);
            if (ret != ERR_OK) {
                THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "set hdf callback failed");
                return;
            }
            break;
        }
        default:
            break;
    }
}

bool ThermalService::RigisterHdfStatusListener()
{
    servmgr_  = IServiceManager::Get();
    if (servmgr_ == nullptr) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "hdf service manager is nullptr");
        return false;
    }

    hdfListener_ = new HdfServiceStatusListener(HdfServiceStatusListener::StatusCallback(
        [&](const OHOS::HDI::ServiceManager::V1_0::ServiceStatus &status) {
            if (status.serviceName != std::string(HDF_SERVICE_NAME)) {
                THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "service name mismatch");
                return;
            }

            if (status.deviceClass != DEVICE_CLASS_DEFAULT) {
                THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "deviceClass mismatch");
                return;
            }

            if (status.status == SERVIE_STATUS_START) {
                thermalInterface_ = IThermalInterface::Get();
                if (thermalInterface_ == nullptr) {
                    THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "failed to get thermal hdf interface");
                    return;
                }
                RemoveEvent(ThermalsrvEventHandler::SEND_GET_THERMAL_HDF_SERVICE_MSG);
                SendEvent(ThermalsrvEventHandler::SEND_GET_THERMAL_HDF_SERVICE_MSG, 0, 0);
            } else if (status.status == SERVIE_STATUS_STOP) {
                thermalInterface_->Unregister();
                return;
            }
        }));

    int status = servmgr_->RegisterServiceStatusListener(hdfListener_, DEVICE_CLASS_DEFAULT);
    if (status != STATUS_OK) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "register failed");
        return false;
    }
    return true;
}

int32_t ThermalService::HandleThermalCallbackEvent(const HdfThermalCallbackInfo& event)
{
    TypeTempMap typeTempMap;
    if (!event.info.empty()) {
        for (auto iter = event.info.begin(); iter != event.info.end(); iter++) {
            THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s: type: %{public}s", __func__, iter->type.c_str());
            THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s: temp: %{public}d", __func__, iter->temp);
            typeTempMap.insert(std::make_pair(iter->type, iter->temp));
        }
    }
    serviceSubscriber_->OnTemperatureChanged(typeTempMap);
    return ERR_OK;
}

std::string ThermalService::ShellDump(const std::vector<std::string>& args, uint32_t argc)
{
    auto uid = IPCSkeleton::GetCallingUid();
    if (uid >= APP_FIRST_UID) {
        THERMAL_HILOGE(MODULE_THERMAL_INNERKIT,
            "%{public}s Request failed, %{public}d permission check failed", __func__, uid);
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "PID: %{public}d Call %{public}s !", pid, __func__);
    std::string result;
    bool ret = ThermalMgrDumper::Dump(args, result);
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "ThermalMgrDumper :%{public}d", ret);
    return result;
}
} // namespace PowerMgr
} // namespace OHOS