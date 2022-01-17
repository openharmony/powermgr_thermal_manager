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

#include "thermal_srv_stub.h"

#include "message_parcel.h"
#include "string_ex.h"

#include "thermal_srv_proxy.h"
#include "ithermal_temp_callback.h"
#include "ithermal_level_callback.h"
#include "thermal_srv_sensor_info.h"
#include "thermal_common.h"

namespace OHOS {
namespace PowerMgr {
int ThermalSrvStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    THERMAL_HILOGD(MODULE_THERMALMGR_SERVICE, "ThermalSrvStub::OnRemoteRequest, cmd = %{public}d, \
        flags = %{public}d", code, option.GetFlags());
    std::u16string descriptor = ThermalSrvStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "ThermalSrvStub::OnRemoteRequest failed, descriptor is not matched!");
        return E_GET_THERMAL_SERVICE_FAILED;
    }

    switch (code) {
        case static_cast<int>(IThermalSrv::REG_THERMAL_TEMP_CALLBACK): {
            return SubscribeThermalTempCallbackStub(data);
        }
        case static_cast<int>(IThermalSrv::UNREG_THERMAL_TEMP_CALLBACK): {
            return UnSubscribeThermalTempCallbackStub(data);
        }
        case static_cast<int>(IThermalSrv::REG_THERMAL_LEVEL_CALLBACK): {
            return SubscribeThermalLevelCallbackStub(data);
        }
        case static_cast<int>(IThermalSrv::UNREG_THERMAL_LEVEL_CALLBACK): {
            return UnSubscribeThermalLevelCallbackStub(data);
        }
        case static_cast<int>(IThermalSrv::GET_SENSOR_INFO): {
            return GetThermalSrvSensorInfoStub(data, reply);
        }
        case static_cast<int>(IThermalSrv::GET_TEMP_LEVEL): {
            return GetThermalevelStub(reply);
        }
        default: {
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
    return ERR_OK;
}

int32_t ThermalSrvStub::SubscribeThermalTempCallbackStub(MessageParcel& data)
{
    THERMAL_HILOGD(MODULE_THERMALMGR_SERVICE, "SubscribeThermalTempCallbackStub Enter");
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    THERMAL_RETURN_IF_WITH_RET((obj == nullptr), E_READ_PARCEL_ERROR_THERMAL);
    sptr<IThermalTempCallback> callback = iface_cast<IThermalTempCallback>(obj);
    THERMAL_RETURN_IF_WITH_RET((callback == nullptr), E_READ_PARCEL_ERROR_THERMAL);
    std::vector<std::string> typeList;
    if (!data.ReadStringVector(&typeList)) {
        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s failed to read type list", __func__);
        return ERR_INVALID_VALUE;
    }
    SubscribeThermalTempCallback(typeList, callback);
    return ERR_OK;
}

int32_t ThermalSrvStub::UnSubscribeThermalTempCallbackStub(MessageParcel& data)
{
    THERMAL_HILOGD(MODULE_THERMALMGR_SERVICE, "UnSubscribeThermalTempCallbackStub Enter");
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    THERMAL_RETURN_IF_WITH_RET((obj == nullptr), E_READ_PARCEL_ERROR_THERMAL);
    sptr<IThermalTempCallback> callback = iface_cast<IThermalTempCallback>(obj);
    THERMAL_RETURN_IF_WITH_RET((callback == nullptr), E_READ_PARCEL_ERROR_THERMAL);
    UnSubscribeThermalTempCallback(callback);
    return ERR_OK;
}

int32_t ThermalSrvStub::SubscribeThermalLevelCallbackStub(MessageParcel &data)
{
    THERMAL_HILOGD(MODULE_THERMALMGR_SERVICE, "SubscribeThermalLevelCallbackStub Enter");
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    THERMAL_RETURN_IF_WITH_RET((obj == nullptr), E_READ_PARCEL_ERROR_THERMAL);
    sptr<IThermalLevelCallback> callback = iface_cast<IThermalLevelCallback>(obj);
    THERMAL_RETURN_IF_WITH_RET((callback == nullptr), E_READ_PARCEL_ERROR_THERMAL);
    SubscribeThermalLevelCallback(callback);
    return ERR_OK;
}

int32_t ThermalSrvStub::UnSubscribeThermalLevelCallbackStub(MessageParcel &data)
{
    THERMAL_HILOGD(MODULE_THERMALMGR_SERVICE, "UnSubscribeThermalLevelCallbackStub Enter");
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    THERMAL_RETURN_IF_WITH_RET((obj == nullptr), E_READ_PARCEL_ERROR_THERMAL);
    sptr<IThermalLevelCallback> callback = iface_cast<IThermalLevelCallback>(obj);
    THERMAL_RETURN_IF_WITH_RET((callback == nullptr), E_READ_PARCEL_ERROR_THERMAL);
    SubscribeThermalLevelCallback(callback);
    return ERR_OK;
}

int32_t ThermalSrvStub::GetThermalSrvSensorInfoStub(MessageParcel &data, MessageParcel &reply)
{
    ThermalSrvSensorInfo sensorInfo;
    uint32_t type = 0;
    THERMAL_HILOGD(MODULE_THERMALMGR_SERVICE, "GetThermalSrvSensorInfoStub: Enter");

    THERMAL_READ_PARCEL_WITH_RET(data, Uint32, type, E_READ_PARCEL_ERROR_THERMAL);
    THERMAL_HILOGD(MODULE_THERMALMGR_SERVICE, "GetThermalSrvSensorInfoStub: Enter %{public}d", type);
    bool ret = GetThermalSrvSensorInfo(static_cast<SensorType>(type), sensorInfo);
    if (ret) {
        if (!reply.WriteParcelable(&sensorInfo)) {
            THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "write failed");
            return -1;
        }
    }
    return ERR_OK;
}

int32_t ThermalSrvStub::GetThermalevelStub(MessageParcel& reply)
{
    THERMAL_HILOGD(MODULE_THERMALMGR_SERVICE, "GetThermalevelStub: Enter");
    ThermalLevel level;
    GetThermalLevel(level);
    THERMAL_WRITE_PARCEL_WITH_RET(reply, Uint32, static_cast<uint32_t>(level), ERR_OK);
    return ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS