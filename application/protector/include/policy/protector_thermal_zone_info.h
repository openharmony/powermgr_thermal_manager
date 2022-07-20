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
#include <stdint.h>
#include <iosfwd>
#include "thermal_sensor_provision.h"

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
    void Dump();
    void UpdateThermalLevel(int32_t temp);
    uint32_t GetThermalLevel()
    {
        return latestLevel_;
    }
    void SetThermalZoneItem(std::vector<ThermalZoneInfoItem> &tzItemList);
    void SetDesc(bool desc);
    void SetInterval(int32_t interval);
    int32_t GetInterval() const;
    void SetMultiple(int32_t multiple);
    int32_t GetMultiple() const;
    std::string GetPath() const;
    void SetPath(const std::string &path);
private:
    void AscJudgment(int32_t curTemp, uint32_t &level);
    void DescJudgment(int32_t curTemp, uint32_t &level);
    void HandleDescNextUpTemp(uint32_t &level, int32_t curTemp);
    void HandleDescCurDownTemp(uint32_t &level, int32_t curTemp);
    void HandleDescMaxSizeTemp(uint32_t &level, int32_t curTemp);
    void HandleDescMinSizeTemp(uint32_t &level, int32_t curTemp);
    void HandleAscNextUpTemp(uint32_t &level, int32_t curTemp);
    void HandleAscCurDownTemp(uint32_t &level, int32_t curTemp);
    void HandleAscMaxSizeTemp(uint32_t &level, int32_t curTemp);
    void HandleAscMinSizeTemp(uint32_t &level, int32_t curTemp);
    bool desc_ {false};
    std::string path_;
    uint32_t latestLevel_ {0};
    int32_t interval_;
    int32_t multiple_;
    std::vector<ThermalZoneInfoItem> tzItemList_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // PROTECTOR_THERMAL_ZONE_INFO_H