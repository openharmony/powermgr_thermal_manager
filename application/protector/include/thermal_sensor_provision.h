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

#ifndef THERMAL_SENSOR_PROVISION_H
#define THERMAL_SENSOR_PROVISION_H

#include <list>
#include <map>
#include <limits.h>

namespace OHOS {
namespace PowerMgr {
using SensorsMap = std::map<std::string, int32_t>;

struct ThermalZoneSysfsPathInfo {
    char* name;
    char tempPath[PATH_MAX];
    char typePath[PATH_MAX];
    int32_t fd;
};

struct ThermalSysfsPathInfo {
    char* name;
    char thermalZonePath[PATH_MAX];
    char coolingDevicePath[PATH_MAX];
    int32_t fd;
};

class ThermalSensorProvision {
public:
    ThermalSensorProvision() {};
    ~ThermalSensorProvision() {};

    bool InitProvision();
    SensorsMap GetSensorData()
    {
        return typeTempMap_;
    }
    int32_t GetMaxCd();
    void ReportThermalZoneData(int32_t reportTime, std::vector<int32_t> &multipleList);
private:
    int32_t ParseThermalZoneInfo();
    int32_t InitThermalZoneSysfs();
    int32_t ReadThermalSysfsToBuff(const char* path, char* buf, size_t size) const;
    int32_t ReadSysfsFile(const char* path, char* buf, size_t size) const;
    void FormatThermalSysfsPaths(struct ThermalSysfsPathInfo *pTSysPathInfo);
    void FormatThermalPaths(char *path, size_t size, const char *format, const char* name);
    void Trim(char* str) const;
    SensorsMap typeTempMap_;

    /* timer related */
    struct ThermalZoneSysfsPathInfo tzSysPathInfo_;
    std::list<ThermalZoneSysfsPathInfo> lTzSysPathInfo_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif
