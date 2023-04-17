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

#include "mock_thermal_mgr_client.h"
#include "thermal_common.h"
#include <datetime_ex.h>
#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <iservice_registry.h>
#include <string_ex.h>
#include <system_ability_definition.h>

namespace OHOS {
namespace PowerMgr {
MockThermalMgrClient::MockThermalMgrClient() {};
MockThermalMgrClient::~MockThermalMgrClient()
{
    if (thermalSrv_ != nullptr) {
        auto remoteObject = thermalSrv_->AsObject();
        if (remoteObject != nullptr) {
            remoteObject->RemoveDeathRecipient(deathRecipient_);
        }
    }
}

ErrCode MockThermalMgrClient::Connect()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (thermalSrv_ != nullptr) {
        return ERR_OK;
    }

    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        THERMAL_HILOGE(COMP_FWK, "Failed to get Registry!");
        return E_GET_SYSTEM_ABILITY_MANAGER_FAILED_THERMAL;
    }

    sptr<IRemoteObject> remoteObject_ = sam->CheckSystemAbility(POWER_MANAGER_THERMAL_SERVICE_ID);
    if (remoteObject_ == nullptr) {
        THERMAL_HILOGE(COMP_FWK, "GetSystemAbility failed!");
        return E_GET_THERMAL_SERVICE_FAILED;
    }

    deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new ThermalMgrDeathRecipient());
    if (deathRecipient_ == nullptr) {
        THERMAL_HILOGE(COMP_FWK, "Failed to create ThermalMgrDeathRecipient!");
        return ERR_NO_MEMORY;
    }

    if ((remoteObject_->IsProxyObject()) && (!remoteObject_->AddDeathRecipient(deathRecipient_))) {
        THERMAL_HILOGE(COMP_FWK, "Add death recipient to PowerMgr service failed.");
        return E_ADD_DEATH_RECIPIENT_FAILED_THERMAL;
    }

    thermalSrv_ = iface_cast<IThermalSrv>(remoteObject_);
    THERMAL_HILOGI(COMP_FWK, "Connecting ThermalMgrService success.");
    return ERR_OK;
}

void MockThermalMgrClient::ResetProxy(const wptr<IRemoteObject>& remote)
{
    std::lock_guard<std::mutex> lock(mutex_);
    THERMAL_RETURN_IF(thermalSrv_ == nullptr);

    auto serviceRemote = thermalSrv_->AsObject();
    if ((serviceRemote != nullptr) && (serviceRemote == remote.promote())) {
        serviceRemote->RemoveDeathRecipient(deathRecipient_);
        thermalSrv_ = nullptr;
    }
}

void MockThermalMgrClient::ThermalMgrDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    if (remote == nullptr) {
        THERMAL_HILOGE(COMP_FWK, "ThermalMgrDeathRecipient::OnRemoteDied failed, remote is nullptr.");
        return;
    }

    MockThermalMgrClient::GetInstance().ResetProxy(remote);
    THERMAL_HILOGI(COMP_FWK, "ThermalMgrDeathRecipient::Recv death notice.");
}

bool MockThermalMgrClient::GetThermalInfo()
{
    THERMAL_RETURN_IF_WITH_RET(Connect() != ERR_OK, false);
    bool ret = false;
    ret = thermalSrv_->GetThermalInfo();
    THERMAL_HILOGD(COMP_FWK, "Calling GetThermalInfo Success");
    return ret;
}
} // namespace PowerMgr
} // namespace OHOS
