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

#ifndef POWERMGR_THERMAL_MGR_DUMPER_H
#define POWERMGR_THERMAL_MGR_DUMPER_H

#include <string>
#include <vector>

#include "thermal_service.h"

namespace OHOS {
namespace PowerMgr {
class ThermalMgrDumper {
public:
    ThermalMgrDumper() = delete;
    ~ThermalMgrDumper() = delete;

    static bool Dump(const std::vector<std::string>& args, std::string& result);

private:
    static void SwitchTempReport(const std::vector<std::string>& args, std::string& result,
        sptr<ThermalService>& tms);
    static void EmulateTempReport(const std::vector<std::string>& args, std::string& result,
        sptr<ThermalService>& tms);
    static bool DumpPolicy(const std::vector<std::string>& args, std::string& result,
        sptr<ThermalService>& tms);
    static void ShowUsage(std::string& result);
    static void ShowThermalZoneInfo(std::string& result);
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWER_MGR_DUMPER_H