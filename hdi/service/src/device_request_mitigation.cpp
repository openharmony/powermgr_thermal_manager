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

#include "device_request_mitigation.h"

#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include "securec.h"
#include "utils/hdf_log.h"
#include "hdf_base.h"

namespace OHOS {
namespace HDI {
namespace THERMAL {
namespace V1_0 {
namespace {
const int32_t MAX_PATH = 256;
const int32_t MAX_BUF_PATH = 256;
const std::string CPU_FREQ_PATH = "/data/mitigation/cpu/freq";
const std::string GPU_FREQ_PATH = "/data/mitigation/gpu/freq";
const std::string BATTERY_CHARGER_CURRENT_PATH = "/data/mitigation/charger/current";
const std::string BATTERY_CURRENT_PATH = "/data/mitigation/battery/current";
const std::string BATTERY_VOLTAGE_PATH = "/data/mitigation/battery/voltage";
}
int32_t DeviceRequestMitigation::WriteSysfsFd(int32_t fd, std::string buf, size_t bytesSize)
{
    ssize_t pos = 0;
    ssize_t recever = 0;
    do {
        recever = write(fd, buf.c_str() + (size_t) pos, bytesSize - (size_t)pos);
        if (recever < HDF_SUCCESS) {
            return recever;
        }
        pos += recever;
    } while ((ssize_t)bytesSize > pos);

    return (int32_t)bytesSize;
}

int32_t DeviceRequestMitigation::OpenSysfsFile(std::string filePath, int32_t flags)
{
    int32_t ret = -1;

    if (filePath.empty()) {
        return HDF_ERR_INVALID_PARAM;
    }

    ret = open(filePath.c_str(), flags);
    if (ret < HDF_SUCCESS) {
        HDF_LOGE("%{public}s: failed to open file %{public}s", __func__, filePath.c_str());
        return ret;
    }
    return ret;
}

int32_t DeviceRequestMitigation::WriteSysfsFile(std::string filePath, std::string buf, size_t bytesSize)
{
    int32_t ret = -1;
    int32_t fd = -1;
    fd = OpenSysfsFile(filePath.c_str(), O_RDWR);
    if (fd < HDF_SUCCESS) {
        HDF_LOGE("%{public}s: failed to open %{public}s", __func__, filePath.c_str());
        return HDF_ERR_IO;
    }
    ret = WriteSysfsFd(fd, buf.c_str(), bytesSize);
    close(fd);
    return ret;
}

int32_t DeviceRequestMitigation::SetFlag(bool flag)
{
    flag_ = flag;
    return HDF_SUCCESS;
}

int32_t DeviceRequestMitigation::CpuRequest(uint32_t freq)
{
    HDF_LOGD("CpuRequest %{public}s: the freq is %{public}d", __func__, freq);
    int32_t ret = -1;
    char freqBuf[MAX_PATH] = {0};
    char nodeBuf[MAX_BUF_PATH] = {0};
    static uint32_t previous;

    std::lock_guard<std::mutex> lock(mutex_);
    if (!flag_) {
        if (freq != previous) {
            snprintf_s(nodeBuf, PATH_MAX, sizeof(nodeBuf) - 1, "%s", CPU_FREQ_PATH.c_str());
            snprintf_s(freqBuf, PATH_MAX, sizeof(freqBuf) - 1, "%d", freq);
            if (WriteSysfsFile(nodeBuf, freqBuf, strlen(freqBuf)) > HDF_SUCCESS) {
                HDF_LOGD("%{public}s: Set freq to %{public}d", __func__, freq);
                previous = freq;
                ret = 0;
            } else {
                HDF_LOGE("%{public}s: failed to set freq", __func__);
            }
        } else {
            HDF_LOGD("%{public}s: the freq mitigation is already at %{public}d", __func__, freq);
            ret = 0;
        }
    } else {
        HDF_LOGD("%{public}s: Write real mitigation device tuning nodes", __func__);
    }

    return ret;
}

int32_t DeviceRequestMitigation::GpuRequest(uint32_t freq)
{
    HDF_LOGD("GpuRequest %{public}s: the freq is %{public}d", __func__, freq);
    int32_t ret = -1;
    char freqBuf[MAX_PATH] = {0};
    char nodeBuf[MAX_BUF_PATH] = {0};
    static uint32_t previous;

    std::lock_guard<std::mutex> lock(mutex_);
    if (!flag_) {
        if (freq != previous) {
            snprintf_s(nodeBuf, PATH_MAX, sizeof(nodeBuf) - 1, "%s", GPU_FREQ_PATH.c_str());
            snprintf_s(freqBuf, PATH_MAX, sizeof(freqBuf) - 1, "%d", freq);
            if (WriteSysfsFile(nodeBuf, freqBuf, strlen(freqBuf)) > HDF_SUCCESS) {
                HDF_LOGD("%{public}s: Set freq to %{public}d", __func__, freq);
                previous = freq;
                ret = 0;
            } else {
                HDF_LOGE("%{public}s: failed to set freq", __func__);
            }
        } else {
            HDF_LOGD("%{public}s: the freq mitigation is already at %{public}d", __func__, freq);
            ret = 0;
        }
    } else {
        HDF_LOGD("%{public}s: Write real mitigation device tuning nodes", __func__);
    }
    return ret;
}

int32_t DeviceRequestMitigation::ChargerRequest(uint32_t current)
{
    HDF_LOGD("%{public}s: the current is %{public}d", __func__, current);
    int32_t ret = -1;
    char currentBuf[MAX_PATH] = {0};
    char nodeBuf[MAX_BUF_PATH] = {0};
    static uint32_t previous;

    std::lock_guard<std::mutex> lock(mutex_);
    if (!flag_) {
        if (current != previous) {
            snprintf_s(nodeBuf, PATH_MAX, sizeof(nodeBuf) - 1, "%s", BATTERY_CHARGER_CURRENT_PATH.c_str());
            snprintf_s(currentBuf, PATH_MAX, sizeof(currentBuf) - 1, "%d%s", current, "\n");
            if (WriteSysfsFile(nodeBuf, currentBuf, strlen(currentBuf)) > HDF_SUCCESS) {
                HDF_LOGD("%{public}s: Set current to %{public}d", __func__, current);
                previous = current;
                ret = 0;
            } else {
                HDF_LOGE("%{public}s: failed to set current", __func__);
            }
        } else {
            HDF_LOGD("%{public}s: the current mitigation is already at %{public}d", __func__, current);
            ret = 0;
        }
    } else {
        HDF_LOGD("%{public}s: Write real mitigation device tuning nodes", __func__);
    }
    return ret;
}

int32_t DeviceRequestMitigation::BatteryCurrentRequest(uint32_t current)
{
    HDF_LOGD("%{public}s: current %{public}d", __func__, current);
    int32_t ret = -1;
    char currentBuf[MAX_PATH] = {0};
    char nodeBuf[MAX_BUF_PATH] = {0};
    static uint32_t previous;

    std::lock_guard<std::mutex> lock(mutex_);
    if (!flag_) {
        if (current != previous) {
            snprintf_s(nodeBuf, PATH_MAX, sizeof(nodeBuf) - 1, "%s", BATTERY_CURRENT_PATH.c_str());
            snprintf_s(currentBuf, PATH_MAX, sizeof(currentBuf) - 1, "%d", current);
            if (WriteSysfsFile(nodeBuf, currentBuf, strlen(currentBuf)) > HDF_SUCCESS) {
                HDF_LOGD("%{public}s: Set current to %{public}d", __func__, current);
                previous = current;
                ret = 0;
            } else {
                HDF_LOGE("%{public}s: failed to set current", __func__);
            }
        } else {
            HDF_LOGD("%{public}s: the current mitigation is already at %{public}d", __func__, current);
            ret = 0;
        }
    } else {
        HDF_LOGD("%{public}s: Write real mitigation device tuning nodes", __func__);
    }
    return ret;
}

int32_t DeviceRequestMitigation::BatteryVoltageRequest(uint32_t voltage)
{
    HDF_LOGD("%{public}s: current %{public}d", __func__, voltage);
    int32_t ret = -1;
    char voltageBuf[MAX_PATH] = {0};
    char voltageNode[MAX_BUF_PATH] = {0};
    static uint32_t previousVoltage;

    std::lock_guard<std::mutex> lock(mutex_);
    if (!flag_) {
        if (voltage != previousVoltage) {
            snprintf_s(voltageNode, PATH_MAX, sizeof(voltageNode) - 1, "%s", BATTERY_VOLTAGE_PATH.c_str());
            snprintf_s(voltageBuf, PATH_MAX, sizeof(voltageBuf) - 1, "%d", voltage);
            if (WriteSysfsFile(voltageNode, voltageBuf, strlen(voltageBuf)) > HDF_SUCCESS) {
                HDF_LOGD("%{public}s: Set current to %{public}d", __func__, voltage);
                previousVoltage = voltage;
                ret = 0;
            } else {
                HDF_LOGE("%{public}s: failed to set current", __func__);
            }
        } else {
            HDF_LOGD("%{public}s: the current mitigation is already at %{public}d", __func__, voltage);
            ret = 0;
        }
    } else {
        HDF_LOGD("%{public}s: Write real mitigation device tuning nodes", __func__);
    }
    return ret;
}
} // namespace V1_0
} // namaespace THERMAL
} // namespace HDI
} // namespace OHOS