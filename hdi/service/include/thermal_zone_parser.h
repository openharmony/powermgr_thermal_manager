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

#ifndef THERMAL_ZONE_PARSER_H
#define THERMAL_ZONE_PARSER_H

#include <list>
#include <map>
#include <mutex>
#include <string>

#include "thermal_api.h"
#include "thermal_hdi_config_parser.h"
#include "thermal_zone_info.h"

namespace OHOS {
namespace HDI {
namespace THERMAL {
namespace V1_0 {
struct ThermalZoneSysfsPathInfo {
    char* name;
    char temperturePath[PATH_MAX];
    char typePath[PATH_MAX];
};

struct ThermalSysfsPathInfo {
    char* name;
    char thermalZonePath[PATH_MAX];
    char coolingDevicePath[PATH_MAX];
};

class ThermalZoneParser {
public:
    ThermalZoneParser() {};
    ~ThermalZoneParser() {};

    ThermalZoneSysfsPathInfo GetTzPathInfo()
    {
        return tzSysPathInfo_;
    };

    void SetTzPathInfo(ThermalZoneSysfsPathInfo tzSysPathInfo)
    {
        tzSysPathInfo_ = tzSysPathInfo;
    }

    std::list<ThermalZoneSysfsPathInfo> GetLTZPathInfo()
    {
        return lTzSysPathInfo_;
    }

    std::list<ThermalZoneInfo> GetlTzInfo()
    {
        return lTzInfo_;
    }

    inline int32_t ConvertInt(std::string value)
    {
        return std::stoi(value.c_str());
    }

    int32_t InitThermalZoneSysfs();
    int32_t ParseThermalZoneInfo();
    int32_t ParserSimulationNode();
    int32_t ReadThermalSysfsToBuff(const char *path, char *buf, size_t size);
    int32_t ReadThermalSysfsPath(const char *path, char *buf, size_t size);
    void FormatThermalSysfsPaths(struct ThermalSysfsPathInfo *pTSysPathInfo);
    void FormatThermalPaths(char *path, size_t size, const char *format, const char* name);
    void ClearThermalZoneInfo();
    void SetFlag(bool flag)
    {
        flag_ = flag;
    }
    bool GetFlag()
    {
        return flag_;
    }
private:
    struct ThermalZoneSysfsPathInfo tzSysPathInfo_;
    std::list<ThermalZoneSysfsPathInfo> lTzSysPathInfo_;
    std::list<ThermalZoneInfo> lTzInfo_;
    std::mutex mutex_;
    bool flag_ {false};
};
} // namespace V1_0
} // namaespace THERMAL
} // namespace HDI
} // namespace OHOS
#endif // THERMAL_ZONE_PARSER_H