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

#include "thermal_srv_proxy.h"

#include <ipc_types.h>
#include <message_option.h>
#include <message_parcel.h>
#include <string_ex.h>

#include "ithermal_temp_callback.h"
#include "thermal_common.h"

namespace OHOS {
namespace PowerMgr {
void ThermalSrvProxy::SubscribeThermalTempCallback(const std::vector<std::string> &typeList,
    const sptr<IThermalTempCallback>& callback)
{
    THERMAL_HILOGD(MODULE_THERMAL_INNERKIT, "ThermalSrvProxy::SubscribeThermalTempCallback Enter");
    sptr<IRemoteObject> remote = Remote();
    THERMAL_RETURN_IF((remote == nullptr) || (callback == nullptr));

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(ThermalSrvProxy::GetDescriptor())) {
        THERMAL_HILOGE(MODULE_THERMAL_INNERKIT, "ThermalSrvProxy::%{public}s write descriptor failed!", __func__);
        return;
    }

    THERMAL_WRITE_PARCEL_NO_RET(data, RemoteObject, callback->AsObject());
    THERMAL_WRITE_PARCEL_NO_RET(data, StringVector, typeList);

    int ret = remote->SendRequest(static_cast<int>(IThermalSrv::REG_THERMAL_TEMP_CALLBACK), data, reply, option);
    if (ret != ERR_OK) {
        THERMAL_HILOGE(MODULE_THERMAL_INNERKIT,
            "ThermalSrvProxy::%{public}s SendRequest is failed, error code: %{public}d",
            __func__, ret);
        return;
    }
}

void ThermalSrvProxy::UnSubscribeThermalTempCallback(const sptr<IThermalTempCallback>& callback)
{
    THERMAL_HILOGD(MODULE_THERMAL_INNERKIT, "ThermalSrvProxy::UnSubscribeThermalTempCallback Enter");
    sptr<IRemoteObject> remote = Remote();
    THERMAL_RETURN_IF((remote == nullptr) || (callback == nullptr));

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(ThermalSrvProxy::GetDescriptor())) {
        THERMAL_HILOGE(MODULE_THERMAL_INNERKIT, "ThermalSrvProxy::%{public}s write descriptor failed!", __func__);
        return;
    }

    THERMAL_WRITE_PARCEL_NO_RET(data, RemoteObject, callback->AsObject());

    int ret = remote->SendRequest(static_cast<int>(IThermalSrv::UNREG_THERMAL_TEMP_CALLBACK), data, reply, option);
    if (ret != ERR_OK) {
        THERMAL_HILOGE(MODULE_THERMAL_INNERKIT,
            "ThermalSrvProxy::%{public}s SendRequest is failed, error code: %{public}d",
            __func__, ret);
        return;
    }
}

void ThermalSrvProxy::SubscribeThermalLevelCallback(const sptr<IThermalLevelCallback>& callback)
{
    THERMAL_HILOGD(MODULE_THERMAL_INNERKIT, "ThermalSrvProxy::SubscribeThermalLevelCallback Enter");
    sptr<IRemoteObject> remote = Remote();
    THERMAL_RETURN_IF((remote == nullptr) || (callback == nullptr));

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(ThermalSrvProxy::GetDescriptor())) {
        THERMAL_HILOGE(MODULE_THERMAL_INNERKIT, "ThermalSrvProxy::%{public}s write descriptor failed!", __func__);
        return;
    }

    THERMAL_WRITE_PARCEL_NO_RET(data, RemoteObject, callback->AsObject());

    int ret = remote->SendRequest(static_cast<int>(IThermalSrv::REG_THERMAL_LEVEL_CALLBACK), data, reply, option);
    if (ret != ERR_OK) {
        THERMAL_HILOGE(MODULE_THERMAL_INNERKIT,
            "ThermalSrvProxy::%{public}s SendRequest is failed, error code: %{public}d",
            __func__, ret);
        return;
    }
}

void ThermalSrvProxy::UnSubscribeThermalLevelCallback(const sptr<IThermalLevelCallback>& callback)
{
    THERMAL_HILOGD(MODULE_THERMAL_INNERKIT, "ThermalSrvProxy::UnSubscribeThermalLevelCallback Enter");
    sptr<IRemoteObject> remote = Remote();
    THERMAL_RETURN_IF((remote == nullptr) || (callback == nullptr));

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(ThermalSrvProxy::GetDescriptor())) {
        THERMAL_HILOGE(MODULE_THERMAL_INNERKIT, "ThermalSrvProxy::%{public}s write descriptor failed!", __func__);
        return;
    }

    THERMAL_WRITE_PARCEL_NO_RET(data, RemoteObject, callback->AsObject());

    int ret = remote->SendRequest(static_cast<int>(IThermalSrv::UNREG_THERMAL_TEMP_CALLBACK), data, reply, option);
    if (ret != ERR_OK) {
        THERMAL_HILOGE(MODULE_THERMAL_INNERKIT,
            "ThermalSrvProxy::%{public}s SendRequest is failed, error code: %{public}d",
            __func__, ret);
        return;
    }
}

bool ThermalSrvProxy::GetThermalSrvSensorInfo(const SensorType &type, ThermalSrvSensorInfo& sensorInfo)
{
    THERMAL_HILOGD(MODULE_THERMAL_INNERKIT, "ThermalSrvProxy::GetThermalSrvSensorInfo Enter");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        return false;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(ThermalSrvProxy::GetDescriptor())) {
        THERMAL_HILOGE(MODULE_THERMAL_INNERKIT, "ThermalSrvProxy::%{public}s write descriptor failed!", __func__);
        return false;
    }

    THERMAL_WRITE_PARCEL_WITH_RET(data, Uint32, static_cast<uint32_t>(type), false);

    int ret = remote->SendRequest(static_cast<int>(IThermalSrv::GET_SENSOR_INFO), data, reply, option);
    if (ret != ERR_OK) {
        THERMAL_HILOGE(MODULE_THERMAL_INNERKIT,
            "ThermalSrvProxy::%{public}s SendRequest is failed, error code: %{public}d",
            __func__, ret);
        return false;
    }

    std::unique_ptr<ThermalSrvSensorInfo> info(reply.ReadParcelable<ThermalSrvSensorInfo>());
    if (!info) {
        return false;
    }
    sensorInfo = *info;
    return true;
}

void ThermalSrvProxy::GetThermalLevel(ThermalLevel& level)
{
    THERMAL_HILOGD(MODULE_THERMAL_INNERKIT, "ThermalSrvProxy::GetThermalLevel Enter");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(ThermalSrvProxy::GetDescriptor())) {
        THERMAL_HILOGE(MODULE_THERMAL_INNERKIT, "ThermalSrvProxy::%{public}s write descriptor failed!", __func__);
        return;
    }

    int ret = remote->SendRequest(static_cast<int>(IThermalSrv::GET_TEMP_LEVEL), data, reply, option);
    if (ret != ERR_OK) {
        THERMAL_HILOGE(MODULE_THERMAL_INNERKIT,
            "ThermalSrvProxy::%{public}s SendRequest is failed, error code: %{public}d",
            __func__, ret);
        return;
    }
    uint32_t thermalLevel;
    THERMAL_READ_PARCEL_NO_RET(reply, Uint32, thermalLevel);
    level = static_cast<ThermalLevel>(thermalLevel);
    return;
}

void ThermalSrvProxy::SetSensorTemp(const std::string &type, const int32_t &temp)
{
    THERMAL_HILOGD(MODULE_THERMAL_INNERKIT, "ThermalSrvProxy::SetSensorTemp Enter");
    sptr<IRemoteObject> remote = Remote();
    THERMAL_RETURN_IF((remote == nullptr));

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(ThermalSrvProxy::GetDescriptor())) {
        THERMAL_HILOGE(MODULE_THERMAL_INNERKIT, "ThermalSrvProxy::%{public}s write descriptor failed!", __func__);
        return;
    }

    THERMAL_WRITE_PARCEL_NO_RET(data, String, type);
    THERMAL_WRITE_PARCEL_NO_RET(data, Int32, temp);

    int ret = remote->SendRequest(static_cast<int>(IThermalSrv::SET_SENSOR_TEMP), data, reply, option);
    if (ret != ERR_OK) {
        THERMAL_HILOGE(MODULE_THERMAL_INNERKIT,
            "ThermalSrvProxy::%{public}s SendRequest is failed, error code: %{public}d",
            __func__, ret);
        return;
    }
}
} // namespace PowerMgr
} // namespace OHOS