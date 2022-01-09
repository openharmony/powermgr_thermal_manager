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

#include "thermal_driver.h"

#include <sys/socket.h>
#include <unistd.h>
#include <hdf_log.h>
#include <hdf_base.h>
#include <linux/netlink.h>
#include "osal/osal_mem.h"
#include "osal/osal_thread.h"
#include "osal/osal_timer.h"

#include "thermal_host_driver_stub.h"
#include "thermal_hdi_config_parser.h"
#include "thermal_zone_parser.h"

using namespace OHOS::HDI::THERMAL::V1_0;

static int32_t ThermalDriverDisPatch(struct HdfDeviceIoClient *client, int cmdId, struct HdfSBuf *data,
    struct HdfSBuf *reply)
{
    struct ThermalDriverService *service = CONTAINER_OF(client->device->service,
        struct ThermalDriverService, ioService);
    return ThermalHostServiceOnRemoteRequest(service->instance, cmdId, *data, *reply);
}

static int32_t ThermalDriverBind(struct HdfDeviceObject *deviceObject)
{
    HDF_LOGD("%{public}s enter", __func__);

    struct ThermalDriverService *service = nullptr;
    if (deviceObject == nullptr) {
        HDF_LOGW("%{public}s: device is nullptr", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }

    service = reinterpret_cast<ThermalDriverService*> (OsalMemCalloc(sizeof(ThermalDriverService)));
    if (service == nullptr) {
        HDF_LOGE("%{public}s: failed to allocate memory", __func__);
    }

    service->device = deviceObject;
    service->ioService.Dispatch = ThermalDriverDisPatch;
    service->ioService.Open = nullptr;
    service->ioService.Release = nullptr;
    service->instance = ThermalHostDriverStubInstance();

    deviceObject->service = &(service->ioService);
    return HDF_SUCCESS;
}

static void ThermalDriverRelease(struct HdfDeviceObject *deviceObject)
{
    HDF_LOGD("%{public}s: Enter", __func__);
    if (deviceObject == nullptr) {
        return;
    }
    struct ThermalDriverService *service = CONTAINER_OF(deviceObject->service, struct ThermalDriverService, ioService);
    if (service == nullptr) {
        HDF_LOGW("%{public}s: thermal service is null, no need to release", __func__);
        return;
    }
    DestoryThermalHostServiceStub(service->instance);
    OsalMemFree(service);
}

static int32_t ThermalDriverInit(struct HdfDeviceObject *deviceObject)
{
    HDF_LOGD("%{public}s: enter", __func__);

    if (deviceObject == nullptr || deviceObject->service == nullptr) {
        HDF_LOGW("%{public}s: device or device->service is nullptr", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }
    return HDF_SUCCESS;
}

struct HdfDriverEntry g_thermalDriverEntry = {
    .moduleVersion = 1,
    .moduleName = "thermal_driver",
    .Bind = ThermalDriverBind,
    .Init = ThermalDriverInit,
    .Release = ThermalDriverRelease,
};

#ifndef __cplusplus
extern "C" {
#endif

HDF_INIT(g_thermalDriverEntry);

#ifndef __cplusplus
}
#endif