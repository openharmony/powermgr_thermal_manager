/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "thermal_sensor_provision.h"

#include <climits>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <vector>
#include <thread>
#include <securec.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include "thermal_kernel_config_file.h"
#include "thermal_common.h"
#include "thermal_kernel_service.h"

namespace OHOS {
namespace PowerMgr {
namespace {
constexpr int32_t MAX_BUFF_SIZE = 128;
constexpr int32_t MAX_SYSFS_SIZE = 128;
constexpr uint32_t ARG_0 = 0;
constexpr int32_t NUM_ZERO = 0;
constexpr const char* THERMAL_SYSFS = "/sys/devices/virtual/thermal";
constexpr const char* THERMAL_ZONE_DIR_NAME = "thermal_zone%d";
constexpr const char* THERMAL_ZONE_DIR_PATH = "/sys/class/thermal/%s";
constexpr const char* THERMAL_TEMPERATURE_PATH = "/sys/class/thermal/%s/temp";
constexpr const char* THEERMAL_TYPE_PATH = "/sys/class/thermal/%s/type";
constexpr const char* CDEV_DIR_NAME = "cooling_device";
auto &g_service = ThermalKernelService::GetInstance();
}

bool ThermalSensorProvision::InitProvision()
{
    int32_t ret = ParseThermalZoneInfo();
    if (ret != NUM_ZERO) {
        return false;
    }
    return true;
}

int32_t ThermalSensorProvision::ReadThermalSysfsToBuff(const char* path, char* buf, size_t size) const
{
    int32_t ret = ReadSysfsFile(path, buf, size);
    if (ret != ERR_OK) {
        THERMAL_HILOGW(FEATURE_PROTECTOR, "read path failed, ret: %{public}d", ret);
        return ret;
    }

    return ERR_OK;
}

int32_t ThermalSensorProvision::ReadSysfsFile(const char* path, char* buf, size_t size) const
{
    int32_t readSize;
    int32_t fd = open(path, O_RDONLY, S_IRUSR | S_IRGRP | S_IROTH);
    if (fd < NUM_ZERO) {
        THERMAL_HILOGE(FEATURE_PROTECTOR, "failed to open file");
        return ERR_INVALID_OPERATION;
    }
    readSize = read(fd, buf, size - 1);
    if (readSize < NUM_ZERO) {
        THERMAL_HILOGE(FEATURE_PROTECTOR, "failed to read file");
        close(fd);
        return ERR_INVALID_OPERATION;
    }

    buf[readSize] = '\0';
    Trim(buf);
    close(fd);

    return ERR_OK;
}

inline void ThermalSensorProvision::Trim(char* str) const
{
    if (str == nullptr) {
        return;
    }

    str[strcspn(str, "\n")] = 0;
}

void ThermalSensorProvision::FormatThermalSysfsPaths(struct ThermalSysfsPathInfo *pTSysPathInfo)
{
    // Format Paths for thermal path
    FormatThermalPaths(pTSysPathInfo->thermalZonePath, sizeof(pTSysPathInfo->thermalZonePath),
        THERMAL_ZONE_DIR_PATH, pTSysPathInfo->name);
    // Format paths for thermal zone node
    tzSysPathInfo_.name = pTSysPathInfo->name;
    FormatThermalPaths(tzSysPathInfo_.tempPath, sizeof(tzSysPathInfo_.tempPath),
        THERMAL_TEMPERATURE_PATH, pTSysPathInfo->name);

    FormatThermalPaths(tzSysPathInfo_.typePath, sizeof(tzSysPathInfo_.typePath),
        THEERMAL_TYPE_PATH, pTSysPathInfo->name);

    tzSysPathInfo_.fd = pTSysPathInfo->fd;
    lTzSysPathInfo_.push_back(tzSysPathInfo_);
}

void ThermalSensorProvision::FormatThermalPaths(char *path, size_t size, const char *format, const char* name)
{
    if (snprintf_s(path, size, size - 1, format, name) < EOK) {
        THERMAL_HILOGW(FEATURE_PROTECTOR, "failed to format path of %{public}s", name);
    }
}

int32_t ThermalSensorProvision::InitThermalZoneSysfs()
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    int32_t index = 0;
    int32_t id = 0;

    dir = opendir(THERMAL_SYSFS);
    if (dir == nullptr) {
        THERMAL_HILOGE(FEATURE_PROTECTOR, "cannot open thermal zone path");
        return ERR_INVALID_VALUE;
    }

    while (true) {
        entry = readdir(dir);
        if (entry == nullptr) {
            break;
        }

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        if (strncmp(entry->d_name, CDEV_DIR_NAME, strlen(CDEV_DIR_NAME)) == 0) {
            continue;
        }

        if (entry->d_type == DT_DIR || entry->d_type == DT_LNK) {
            struct ThermalSysfsPathInfo sysfsInfo = {0};
            sysfsInfo.name = entry->d_name;
            THERMAL_HILOGI(FEATURE_PROTECTOR, "init sysfs info of %{public}s", sysfsInfo.name);
            int32_t ret = sscanf_s(sysfsInfo.name, THERMAL_ZONE_DIR_NAME, &id);
            if (ret < ARG_0) {
                closedir(dir);
                return ret;
            }

            THERMAL_HILOGI(FEATURE_PROTECTOR, "Sensor %{public}s found at tz: %{public}d", sysfsInfo.name, id);
            sysfsInfo.fd = id;
            if (index > MAX_SYSFS_SIZE) {
                THERMAL_HILOGE(FEATURE_PROTECTOR, "too many plugged types");
                break;
            }

            FormatThermalSysfsPaths(&sysfsInfo);
            index++;
        }
    }
    closedir(dir);
    return ERR_OK;
}

int32_t ThermalSensorProvision::ParseThermalZoneInfo()
{
    char bufType[MAX_BUFF_SIZE] = {0};
    int32_t ret = InitThermalZoneSysfs();
    if (ret != ERR_OK) {
        THERMAL_HILOGE(FEATURE_PROTECTOR, "failed to get path info.");
        return false;
    }

    std::map<std::string, std::string> tzPathMap;
    if (!lTzSysPathInfo_.empty()) {
        for (auto iter = lTzSysPathInfo_.begin(); iter != lTzSysPathInfo_.end(); iter++) {
            ret = ReadThermalSysfsToBuff(iter->typePath, bufType, sizeof(bufType));
            if (ret != NUM_ZERO) {
                THERMAL_HILOGE(FEATURE_PROTECTOR, "failed to read thermal zone type");
                return ret;
            }
            std::string tzType = bufType;
            tzPathMap.insert(std::make_pair(tzType, iter->tempPath));
        }
    }
    auto tzInfoMap = g_service.GetPolicy()->GetThermalZoneMap();
    if (tzInfoMap.empty()) {
        return ERR_INVALID_VALUE;
    }
    for (auto tzIter : tzInfoMap) {
        auto typeIter = tzPathMap.find(tzIter.first);
        if (typeIter != tzPathMap.end()) {
            tzIter.second->SetPath(typeIter->second);
        }
    }
    return ERR_OK;
}

void ThermalSensorProvision::ReportThermalZoneData(int32_t reportTime, std::vector<int32_t> &multipleList)
{
    char tempBuf[MAX_BUFF_SIZE] = {0};
    typeTempMap_.clear();
    multipleList.clear();

    auto tzInfoMap = g_service.GetPolicy()->GetThermalZoneMap();
    if (tzInfoMap.empty()) {
        return;
    }

    for (auto sensorIter : tzInfoMap) {
        int multiple = sensorIter.second->GetMultiple();
        if (multiple == 0) {
            continue;
        }
        int result = reportTime % multiple;
        multipleList.push_back(multiple);
        if (result == NUM_ZERO) {
            if (sensorIter.second->GetPath().empty()) {
                continue;
            }

            int32_t ret = ReadThermalSysfsToBuff(sensorIter.second->GetPath().c_str(), tempBuf, sizeof(tempBuf));
            if (ret != NUM_ZERO) {
                continue;
            }
            int32_t temp = std::stoi(tempBuf);
            typeTempMap_.insert(std::make_pair(sensorIter.first, temp));
        }
    }
}

int32_t ThermalSensorProvision::GetMaxCd()
{
    return g_service.GetPolicy()->GetMaxCd();
}
} // namespace PowerMgr
} // namespace OHOS
