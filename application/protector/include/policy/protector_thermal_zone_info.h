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

#ifndef PROTECTOR_THERMAL_ZONE_INFO_H
#define PROTECTOR_THERMAL_ZONE_INFO_H

#include <string>
#include <vector>
#include "thermal_sensor_provider.h"

namespace OHOS {
namespace PowerMgr {
struct ThermalZoneInfoItem {
    uint32_t level;
    int32_t threshold;
    int32_t thresholdClr;
};

class ProtectorThermalZoneInfo {
public:
    ProtectorThermalZoneInfo() = default;
    ~ProtectorThermalZoneInfo() = default;
    bool Init();
    void Dump();
    void UpdateThermalLevel(int32_t temp);
    void AscJudgment(int32_t curTemp, uint32_t &level);
    void DescJudgment(int32_t curTemp, uint32_t &level);

    uint32_t GetThermlLevel()
    {
        return latestLevel_;
    }
    void SetThermalZoneItem(std::vector<ThermalZoneInfoItem> &vtzi);
    void SetDesc(bool desc);
private:
    bool desc_ {false};
    uint32_t latestLevel_ {0};
    std::vector<ThermalZoneInfoItem> vtzi_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // PROTECTOR_THERMAL_ZONE_INFO_H