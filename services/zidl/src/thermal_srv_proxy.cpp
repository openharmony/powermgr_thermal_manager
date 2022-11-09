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

#include "errors.h"
#include "ithermal_temp_callback.h"
#include "thermal_common.h"
#include "thermal_log.h"
#include <message_option.h>
#include <message_parcel.h>

namespace OHOS {
namespace PowerMgr {
bool ThermalSrvProxy::SubscribeThermalTempCallback(
    const std::vector<std::string>& typeList, const sptr<IThermalTempCallback>& callback)
{
    THERMAL_HILOGD(COMP_FWK, "Enter");
    sptr<IRemoteObject> remote = Remote();
    THERMAL_RETURN_IF_WITH_RET((remote == nullptr) || (callback == nullptr), false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(ThermalSrvProxy::GetDescriptor())) {
        THERMAL_HILOGE(COMP_FWK, "write descriptor failed!");
        return false;
    }

    THERMAL_WRITE_PARCEL_WITH_RET(data, RemoteObject, callback->AsObject(), false);
    THERMAL_WRITE_PARCEL_WITH_RET(data, StringVector, typeList, false);

    int ret = remote->SendRequest(static_cast<int>(IThermalSrv::REG_THERMAL_TEMP_CALLBACK), data, reply, option);
    if (ret != ERR_OK) {
        THERMAL_HILOGE(COMP_FWK, "SendRequest is failed, error code: %{public}d", ret);
        return false;
    }
    return true;
}

bool ThermalSrvProxy::UnSubscribeThermalTempCallback(const sptr<IThermalTempCallback>& callback)
{
    THERMAL_HILOGD(COMP_FWK, "Enter");
    sptr<IRemoteObject> remote = Remote();
    THERMAL_RETURN_IF_WITH_RET((remote == nullptr) || (callback == nullptr), false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(ThermalSrvProxy::GetDescriptor())) {
        THERMAL_HILOGE(COMP_FWK, "write descriptor failed!");
        return false;
    }

    THERMAL_WRITE_PARCEL_WITH_RET(data, RemoteObject, callback->AsObject(), false);

    int ret = remote->SendRequest(static_cast<int>(IThermalSrv::UNREG_THERMAL_TEMP_CALLBACK), data, reply, option);
    if (ret != ERR_OK) {
        THERMAL_HILOGE(COMP_FWK, "SendRequest is failed, error code: %{public}d", ret);
        return false;
    }
    return true;
}

bool ThermalSrvProxy::SubscribeThermalLevelCallback(const sptr<IThermalLevelCallback>& callback)
{
    THERMAL_HILOGD(COMP_FWK, "Enter");
    sptr<IRemoteObject> remote = Remote();
    THERMAL_RETURN_IF_WITH_RET((remote == nullptr) || (callback == nullptr), false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(ThermalSrvProxy::GetDescriptor())) {
        THERMAL_HILOGE(COMP_FWK, "write descriptor failed!");
        return false;
    }

    THERMAL_WRITE_PARCEL_WITH_RET(data, RemoteObject, callback->AsObject(), false);

    int ret = remote->SendRequest(static_cast<int>(IThermalSrv::REG_THERMAL_LEVEL_CALLBACK), data, reply, option);
    if (ret != ERR_OK) {
        THERMAL_HILOGE(COMP_FWK, "SendRequest is failed, error code: %{public}d", ret);
        return false;
    }
    return true;
}

bool ThermalSrvProxy::UnSubscribeThermalLevelCallback(const sptr<IThermalLevelCallback>& callback)
{
    THERMAL_HILOGD(COMP_FWK, "Enter");
    sptr<IRemoteObject> remote = Remote();
    THERMAL_RETURN_IF_WITH_RET((remote == nullptr) || (callback == nullptr), false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(ThermalSrvProxy::GetDescriptor())) {
        THERMAL_HILOGE(COMP_FWK, "write descriptor failed!");
        return false;
    }

    THERMAL_WRITE_PARCEL_WITH_RET(data, RemoteObject, callback->AsObject(), false);

    int ret = remote->SendRequest(static_cast<int>(IThermalSrv::UNREG_THERMAL_LEVEL_CALLBACK), data, reply, option);
    if (ret != ERR_OK) {
        THERMAL_HILOGE(COMP_FWK, "SendRequest is failed, error code: %{public}d", ret);
        return false;
    }
    return true;
}

bool ThermalSrvProxy::SubscribeThermalActionCallback(
    const std::vector<std::string>& actionList, const std::string& desc, const sptr<IThermalActionCallback>& callback)
{
    THERMAL_HILOGD(COMP_FWK, "Enter");
    sptr<IRemoteObject> remote = Remote();
    THERMAL_RETURN_IF_WITH_RET((remote == nullptr) || (callback == nullptr), false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(ThermalSrvProxy::GetDescriptor())) {
        THERMAL_HILOGE(COMP_FWK, "write descriptor failed!");
        return false;
    }

    THERMAL_WRITE_PARCEL_WITH_RET(data, RemoteObject, callback->AsObject(), false);
    THERMAL_WRITE_PARCEL_WITH_RET(data, StringVector, actionList, false);
    THERMAL_WRITE_PARCEL_WITH_RET(data, String, desc, false);

    int ret = remote->SendRequest(static_cast<int>(IThermalSrv::REG_THERMAL_ACTION_CALLBACK), data, reply, option);
    if (ret != ERR_OK) {
        THERMAL_HILOGE(COMP_FWK, "SendRequest is failed, error code: %{public}d", ret);
        return false;
    }
    return true;
}

bool ThermalSrvProxy::UnSubscribeThermalActionCallback(const sptr<IThermalActionCallback>& callback)
{
    THERMAL_HILOGD(COMP_FWK, "Enter");
    sptr<IRemoteObject> remote = Remote();
    THERMAL_RETURN_IF_WITH_RET((remote == nullptr) || (callback == nullptr), false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(ThermalSrvProxy::GetDescriptor())) {
        THERMAL_HILOGE(COMP_FWK, "write descriptor failed!");
        return false;
    }

    THERMAL_WRITE_PARCEL_WITH_RET(data, RemoteObject, callback->AsObject(), false);

    int ret = remote->SendRequest(static_cast<int>(IThermalSrv::UNREG_THERMAL_ACTION_CALLBACK), data, reply, option);
    if (ret != ERR_OK) {
        THERMAL_HILOGE(COMP_FWK, "SendRequest is failed, error code: %{public}d", ret);
        return false;
    }
    return true;
}

bool ThermalSrvProxy::GetThermalSrvSensorInfo(const SensorType& type, ThermalSrvSensorInfo& sensorInfo)
{
    THERMAL_HILOGD(COMP_FWK, "Enter");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        return false;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(ThermalSrvProxy::GetDescriptor())) {
        THERMAL_HILOGE(COMP_FWK, "write descriptor failed!");
        return false;
    }

    THERMAL_WRITE_PARCEL_WITH_RET(data, Uint32, static_cast<uint32_t>(type), false);

    int ret = remote->SendRequest(static_cast<int>(IThermalSrv::GET_SENSOR_INFO), data, reply, option);
    if (ret != ERR_OK) {
        THERMAL_HILOGE(COMP_FWK, "SendRequest is failed, error code: %{public}d", ret);
        return false;
    }

    std::unique_ptr<ThermalSrvSensorInfo> info(reply.ReadParcelable<ThermalSrvSensorInfo>());
    if (!info) {
        return false;
    }
    sensorInfo = *info;
    return true;
}

bool ThermalSrvProxy::GetThermalLevel(ThermalLevel& level)
{
    THERMAL_HILOGD(COMP_FWK, "Enter");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        return false;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(ThermalSrvProxy::GetDescriptor())) {
        THERMAL_HILOGE(COMP_FWK, "write descriptor failed!");
        return false;
    }

    int ret = remote->SendRequest(static_cast<int>(IThermalSrv::GET_TEMP_LEVEL), data, reply, option);
    if (ret != ERR_OK) {
        THERMAL_HILOGE(COMP_FWK, "SendRequest is failed, error code: %{public}d", ret);
        return false;
    }
    uint32_t thermalLevel;
    THERMAL_READ_PARCEL_WITH_RET(reply, Uint32, thermalLevel, false);
    level = static_cast<ThermalLevel>(thermalLevel);
    return true;
}

bool ThermalSrvProxy::GetThermalInfo()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        return false;
    }

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(ThermalSrvProxy::GetDescriptor())) {
        THERMAL_HILOGE(COMP_FWK, "write descriptor failed!");
        return result;
    }

    int ret = remote->SendRequest(static_cast<int>(IThermalSrv::GET_THERMAL_INFO), data, reply, option);
    if (ret != ERR_OK) {
        THERMAL_HILOGE(COMP_FWK, "SendRequest is failed, error code: %{public}d", ret);
        return result;
    }
    if (!reply.ReadBool(result)) {
        THERMAL_HILOGE(COMP_FWK, "ReadBool fail");
    }
    return true;
}

bool ThermalSrvProxy::SetScene(const std::string& scene)
{
    THERMAL_HILOGD(COMP_FWK, "Enter");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        return false;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(ThermalSrvProxy::GetDescriptor())) {
        THERMAL_HILOGE(COMP_FWK, "write descriptor failed!");
        return false;
    }

    THERMAL_WRITE_PARCEL_WITH_RET(data, String, scene, false);

    int ret = remote->SendRequest(static_cast<int>(IThermalSrv::SET_SCENE), data, reply, option);
    if (ret != ERR_OK) {
        THERMAL_HILOGE(COMP_FWK, "SendRequest is failed, error code: %{public}d", ret);
        return false;
    }
    return true;
}

std::string ThermalSrvProxy::ShellDump(const std::vector<std::string>& args, uint32_t argc)
{
    sptr<IRemoteObject> remote = Remote();
    std::string result = "remote error";
    THERMAL_RETURN_IF_WITH_RET(remote == nullptr, result);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(ThermalSrvProxy::GetDescriptor())) {
        THERMAL_HILOGE(COMP_FWK, "write descriptor failed!");
        return 0;
    }

    data.WriteUint32(argc);
    for (uint32_t i = 0; i < argc; i++) {
        data.WriteString(args[i]);
    }
    int ret = remote->SendRequest(static_cast<int>(IThermalSrv::SHELL_DUMP), data, reply, option);
    if (ret != ERR_OK) {
        THERMAL_HILOGE(COMP_FWK, "SendRequest is failed, error code: %{public}d", ret);
        return result;
    }
    result = reply.ReadString();
    return result;
}
} // namespace PowerMgr
} // namespace OHOS
