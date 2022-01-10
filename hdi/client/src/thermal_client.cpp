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

#include "thermal_client.h"
#include "thermal_api.h"
#include "iservmgr_hdi.h"
#include "thermal_common.h"

namespace OHOS {
namespace PowerMgr {
using OHOS::HDI::ServiceManager::V1_0::IServiceManager;
#define HDF_LOG_TAG thermal_client

namespace {
const std::string THERMAL_DRIVER_SERVICE = "thermal_driver";
}

sptr<IRemoteObject> ThermalClient::GetThermalDriverService()
{
    auto serviceManager = IServiceManager::Get();
    if (serviceManager == nullptr) {
        THERMAL_HILOGW(MODULE_THERMALHDI_CLIENT, "service manager is nullptr");
        return nullptr;
    }
    auto thermalDriverService = serviceManager->GetService(THERMAL_DRIVER_SERVICE.c_str());
    if (thermalDriverService == nullptr) {
        THERMAL_HILOGW(MODULE_THERMALHDI_CLIENT, "thermal driver service is nullptr");
        return nullptr;
    }
    return thermalDriverService;
}

ErrCode ThermalClient::DoDispatch(uint32_t cmd, MessageParcel &data, MessageParcel &reply,  MessageOption &option)
{
    THERMAL_HILOGD(MODULE_THERMALHDI_CLIENT, "Start to dispatch cmd: %{public}d", cmd);
    auto thermalDriver = GetThermalDriverService();
    if (thermalDriver == nullptr) {
        return ERR_NO_INIT;
    }

    auto ret = thermalDriver->SendRequest(cmd, data, reply, option);
    if (ret != ERR_OK) {
        THERMAL_HILOGE(MODULE_THERMALHDI_CLIENT, "failed to send request, cmd: %{public}d, ret: %{public}d", cmd, ret);
        return ret;
    }
    return ERR_OK;
}

std::map<std::string, int32_t> ThermalClient::GetThermalZoneInfo()
{
    THERMAL_HILOGD(MODULE_THERMALHDI_CLIENT, "GetThermalZoneInfo enter");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    ErrCode ret = DoDispatch(CMD_GET_THERMALZONEINFO, data, reply, option);
    if (FAILED(ret)) {
        THERMAL_HILOGW(MODULE_THERMALHDI_CLIENT, "GetThermalZoneInfo failed, return INVALID_THERMAL_INT_VALUE");
    }
    std::map<std::string, int32_t> mtz;
    uint32_t outSize = reply.ReadUint32();
    THERMAL_HILOGD(MODULE_THERMALHDI_CLIENT, "OutSize: %{public}d", outSize);
    for (uint32_t i = 0; i < outSize; i++) {
        std::string curTypeData = reply.ReadString();
        THERMAL_HILOGD(MODULE_THERMALHDI_CLIENT, "curTypeData: %{public}s", curTypeData.c_str());
        int32_t curTempData = reply.ReadInt32();
        THERMAL_HILOGD(MODULE_THERMALHDI_CLIENT, "curTempData: %{public}d", curTempData);
        mtz.insert(std::pair<std::string, int32_t>(curTypeData, curTempData));
    }
    return mtz;
}

ErrCode ThermalClient::BindThermalDriverSubscriber(const sptr<ThermalSubscriber> &subscriber)
{
    THERMAL_HILOGD(MODULE_THERMALHDI_CLIENT, "BindThermalDriverSubscriber enter");
    if (subscriber == nullptr) {
        THERMAL_HILOGW(MODULE_THERMALHDI_CLIENT, "subscriber is nullptr");
        return ERR_INVALID_VALUE;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteRemoteObject(subscriber);
    return DoDispatch(CMD_BIND_THERMAL_SUBSCRIBER, data, reply, option);
}

ErrCode ThermalClient::UnbindThermalDriverSubscriber()
{
    THERMAL_HILOGD(MODULE_THERMALHDI_CLIENT, "UnbindThermalDriverSubscriber enter");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    return DoDispatch(CMD_UNBIND_THERMAL_SUBSCRIBER, data, reply, option);
}

int32_t ThermalClient::SetCPUFreq(uint32_t freq)
{
    THERMAL_HILOGD(MODULE_THERMALHDI_CLIENT, "Set CPU freq");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteUint32(freq)) {
        THERMAL_HILOGW(MODULE_THERMALHDI_CLIENT, "write freq failed");
        return ERR_INVALID_VALUE;
    }

    ErrCode ret = DoDispatch(CMD_SET_CPU_FREQ, data, reply, option);
    if (FAILED(ret)) {
        THERMAL_HILOGW(MODULE_THERMALHDI_CLIENT, "Set freq failed, return INVALID_THERMAL_INT_VALUE");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t ThermalClient::SetGPUFreq(uint32_t freq)
{
    THERMAL_HILOGD(MODULE_THERMALHDI_CLIENT, "Set GPU freq");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteUint32(freq)) {
        THERMAL_HILOGW(MODULE_THERMALHDI_CLIENT, "write freq failed");
        return ERR_INVALID_VALUE;
    }

    ErrCode ret = DoDispatch(CMD_SET_GPU_FREQ, data, reply, option);
    if (FAILED(ret)) {
        THERMAL_HILOGW(MODULE_THERMALHDI_CLIENT, "Set freq failed, return INVALID_THERMAL_INT_VALUE");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t ThermalClient::SetBatteryCurrent(uint32_t current)
{
    THERMAL_HILOGD(MODULE_THERMALHDI_CLIENT, "Set Battery current");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteUint32(current)) {
        THERMAL_HILOGW(MODULE_THERMALHDI_CLIENT, "write current failed");
        return ERR_INVALID_VALUE;
    }

    ErrCode ret = DoDispatch(CMD_SET_BATTERY_CURRENT, data, reply, option);
    if (FAILED(ret)) {
        THERMAL_HILOGW(MODULE_THERMALHDI_CLIENT, "Set current failed, return INVALID_THERMAL_INT_VALUE");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t ThermalClient::SetHdiFlag(bool flag)
{
    THERMAL_HILOGD(MODULE_THERMALHDI_CLIENT, "Set Battery current");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteBool(flag)) {
        THERMAL_HILOGW(MODULE_THERMALHDI_CLIENT, "write flag failed");
        return ERR_INVALID_VALUE;
    }

    ErrCode ret = DoDispatch(CMD_SET_FLAG_CONTROL, data, reply, option);
    if (FAILED(ret)) {
        THERMAL_HILOGW(MODULE_THERMALHDI_CLIENT, "Set flag failed, return ERR_INVALID_VALUE");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t ThermalClient::SetSensorTemp(std::string type, int32_t temp)
{
    THERMAL_HILOGD(MODULE_THERMALHDI_CLIENT, "SetSensorTemp");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteString(type)) {
        THERMAL_HILOGW(MODULE_THERMALHDI_CLIENT, "write type failed");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteInt32(temp)) {
        THERMAL_HILOGW(MODULE_THERMALHDI_CLIENT, "write temp failed");
        return ERR_INVALID_VALUE;
    }

    ErrCode ret = DoDispatch(CMD_SET_SENSOR_TEMP, data, reply, option);
    if (FAILED(ret)) {
        THERMAL_HILOGW(MODULE_THERMALHDI_CLIENT, "Set flag failed, return ERR_INVALID_VALUE");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS