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

#include "thermal_zone_parser.h"

#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
#include <iostream>
#include <dirent.h>
#include <fcntl.h>
#include <climits>
#include <securec.h>
#include <unistd.h>
#include <sys/types.h>
#include "utils/hdf_log.h"
#include "osal/osal_mem.h"

#include "thermal_hdi_config_parser.h"

using namespace std;
namespace OHOS {
namespace HDI {
namespace THERMAL {
namespace V1_0 {
namespace {
const int32_t MAX_BUFF_SIZE = 128;
const int32_t MAX_SYSFS_SIZE = 128;
const int ZONE_15 = 15;
const int ZONE_16 = 16;
const std::string SIMULATION_TYPE_DIR = "/data/thermal/%s/type";
const std::string SIMULATION_TEMP_DIR = "/data/thermal/%s/temp";
const std::string THERMAL_SYSFS = "/sys/devices/virtual/thermal";
const std::string THERMAL_ZONE_DIR_NAME = "thermal_zone%d";
const std::string COOLING_DEVICE_DIR_NAME = "cooling_device%d";
const std::string THERMAL_ZONE_DIR_PATH = "/sys/class/thermal/%s";
const std::string THERMAL_TEMPERATURE_PATH = "/sys/class/thermal/%s/temp";
const std::string THEERMAL_TYPE_PATH = "/sys/class/thermal/%s/type";
const std::string CDEV_DIR_NAME = "cooling_device";
}

void ThermalZoneParser::FormatThermalPaths(char *path, size_t size, const char *format, const char* name)
{
    int32_t ret = snprintf_s(path, PATH_MAX, size - 1, format, name);
    if (ret == -1) {
        HDF_LOGW("%{public}s: failed to format path of %{public}s", __func__, name);
    }
}

void ThermalZoneParser::FormatThermalSysfsPaths(struct ThermalSysfsPathInfo *pTSysPathInfo)
{
    // Format Paths for thermal path
    FormatThermalPaths(pTSysPathInfo->thermalZonePath, sizeof(pTSysPathInfo->thermalZonePath),
        THERMAL_ZONE_DIR_PATH.c_str(), pTSysPathInfo->name);
    // Format paths for thermal zone node
    tzSysPathInfo_.name = pTSysPathInfo->name;
    FormatThermalPaths(tzSysPathInfo_.temperturePath, sizeof(tzSysPathInfo_.temperturePath),
        THERMAL_TEMPERATURE_PATH.c_str(), pTSysPathInfo->name);

    FormatThermalPaths(tzSysPathInfo_.typePath, sizeof(tzSysPathInfo_.typePath),
        THEERMAL_TYPE_PATH.c_str(), pTSysPathInfo->name);

    HDF_LOGD("%{public}s: temp path: %{public}s, type path: %{public}s ",
        __func__, tzSysPathInfo_.temperturePath, tzSysPathInfo_.typePath);

    lTzSysPathInfo_.push_back(tzSysPathInfo_);
}

int32_t ThermalZoneParser::InitThermalZoneSysfs()
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    int32_t index = 0;
    int32_t id = 0;

    dir = opendir(THERMAL_SYSFS.c_str());
    if (dir == NULL) {
        HDF_LOGE("%{public}s: cannot open thermal zone path", __func__);
        return HDF_ERR_IO;
    }

    while (true) {
        entry = readdir(dir);
        if (entry == NULL) {
            break;
        }

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        if (strncmp(entry->d_name, CDEV_DIR_NAME.c_str(), CDEV_DIR_NAME.size()) == 0) {
            continue;
        }

        if (entry->d_type == DT_DIR || entry->d_type == DT_LNK) {
            struct ThermalSysfsPathInfo sysfsInfo = {0};
            sysfsInfo.name = entry->d_name;
            HDF_LOGD("%{public}s: init sysfs info of %{public}s", __func__, sysfsInfo.name);
            int32_t ret = sscanf_s(sysfsInfo.name, THERMAL_ZONE_DIR_NAME.c_str(), &id);
            if (ret < HDF_SUCCESS) {
                return ret;
            }

            if (id == ZONE_15 || id == ZONE_16) {
                continue;
            }
            HDF_LOGD("%{public}s: Sensor %{public}s found at tz: %{public}d", __func__, sysfsInfo.name, id);
            if (index > MAX_SYSFS_SIZE) {
                HDF_LOGE("%{public}s: too many plugged types", __func__);
                break;
            }
            FormatThermalSysfsPaths(&sysfsInfo);
            index++;
        }
    }
    closedir(dir);
    return HDF_SUCCESS;
}

int32_t ThermalZoneParser::ReadThermalSysfsPath(const char *path, char *buf, size_t size)
{
    int32_t ret = -1;

    int32_t fd = open(path, O_RDONLY);
    if (fd < HDF_SUCCESS) {
        HDF_LOGE("%{public}s: failed to open %{public}s", __func__, path);
        return HDF_ERR_IO;
    }

    ret = read(fd, buf, size);
    if (ret < HDF_SUCCESS) {
        HDF_LOGE("%{public}s: failed to read %{public}s, %{public}d", __func__, path, fd);
        close(fd);
        return HDF_ERR_IO;
    }

    close(fd);
    buf[size - 1] = '\0';
    return HDF_SUCCESS;
}

int32_t ThermalZoneParser::ReadThermalSysfsToBuff(const char *path, char *buf, size_t size)
{
    int32_t ret = -1;
    if (flag_) {
        if (tzSysPathInfo_.name == NULL) {
            HDF_LOGW("%{public}s: thermal sysfs info is not exist", __func__);
            return HDF_ERR_INVALID_OBJECT;
        }
    }

    ret = ReadThermalSysfsPath(path, buf, size);
    if (ret != HDF_SUCCESS) {
        HDF_LOGW("%{public}s: read path %{public}s failed, ret: %{public}d", __func__, path, ret);
        return ret;
    }
    return HDF_SUCCESS;
}

int32_t ThermalZoneParser::ParserSimulationNode()
{
    HDF_LOGD("%{public}s: Enter", __func__);
    int32_t ret = -1;
    int32_t value = -1;
    char bufType[MAX_BUFF_SIZE] = {0};
    char bufTemp[MAX_BUFF_SIZE] = {0};
    char typeValue[MAX_BUFF_SIZE] = {0};
    char tempValue[MAX_BUFF_SIZE] = {0};
    ThermalZoneInfo thermalZoneInfo;
    std::vector<std::string> vType;
    vType.push_back("battery");
    vType.push_back("charger");
    vType.push_back("pa");
    vType.push_back("ambient");
    vType.push_back("ap");
    vType.push_back("cpu");
    vType.push_back("soc");
    vType.push_back("shell");

    ClearThermalZoneInfo();
    for (auto type : vType) {
        snprintf_s(bufType, PATH_MAX, sizeof(bufType) - 1, SIMULATION_TYPE_DIR.c_str(), type.c_str());
        snprintf_s(bufTemp, PATH_MAX, sizeof(bufTemp) - 1, SIMULATION_TEMP_DIR.c_str(), type.c_str());
        ret = ReadThermalSysfsPath(bufType, typeValue, sizeof(typeValue));
        if (ret != HDF_SUCCESS) {
            return ret;
        }
        std::string sensorType = typeValue;
        thermalZoneInfo.SetType(sensorType);
        HDF_LOGD("%{public}s: type: %{public}s", __func__, sensorType.c_str());

        ret = ReadThermalSysfsPath(bufTemp, tempValue, sizeof(tempValue));
        if (ret != HDF_SUCCESS) {
            return ret;
        }
        std::string temp = tempValue;
        value = ConvertInt(temp);
        HDF_LOGD("%{public}s: temp: %{public}d", __func__, value);
        thermalZoneInfo.SetTemp(value);
        lTzInfo_.push_back(thermalZoneInfo);
    }
    return HDF_SUCCESS;
}

int32_t ThermalZoneParser::ParseThermalZoneInfo()
{
    int32_t ret = -1;
    char bufType[MAX_BUFF_SIZE] = {0};
    char bufTemp[MAX_BUFF_SIZE] = {0};
    ThermalZoneInfo thermalZoneInfo;
    int32_t value;
    ClearThermalZoneInfo();
    {
        std::lock_guard<std::mutex> lock(mutex_);
        HDF_LOGD("%{public}s: Enter %{public}d", __func__, GetLTZPathInfo().size());
        for (auto it : GetLTZPathInfo()) {
            ret = ReadThermalSysfsToBuff(it.typePath, bufType, sizeof(bufType));
            if (ret != HDF_SUCCESS) {
                return ret;
            }
            HDF_LOGD("%{public}s: type %{public}s", __func__, bufType);
            std::string sensorType = bufType;
            thermalZoneInfo.SetType(sensorType);

            ret = ReadThermalSysfsToBuff(it.temperturePath, bufTemp, sizeof(bufTemp));
            if (ret != HDF_SUCCESS) {
                return ret;
            }
            std::string temp = bufTemp;
            value = ConvertInt(temp);
            HDF_LOGD("%{public}s: temp: %{public}d", __func__, value);
            thermalZoneInfo.SetTemp(value);
            lTzInfo_.push_back(thermalZoneInfo);
        }
    }
    return HDF_SUCCESS;
}

void ThermalZoneParser::ClearThermalZoneInfo()
{
    if (!lTzInfo_.empty()) {
        lTzInfo_.clear();
    } else {
        return;
    }
}
} // namespace V1_0
} // namaespace THERMAL
} // namespace HDI
} // namespace OHOS