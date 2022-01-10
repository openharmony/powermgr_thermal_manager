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

#ifndef THERMAL_ZONE_INFO_H
#define THERMAL_ZONE_INFO_H

#include <string>

namespace OHOS {
namespace HDI {
namespace THERMAL {
namespace V1_0 {
class ThermalZoneInfo {
public:
    ThermalZoneInfo() {};
    ~ThermalZoneInfo() {};

    int32_t GetTemp() const;
    std::string GetType() const;
    void SetTemp(const int32_t temp);
    void SetType(const std::string &type);

private:
    int32_t temp_;
    std::string type_;
};
} // namespace V1_0
} // namaespace THERMAL
} // namespace HDI
} // namespace OHOS
#endif // THERMAL_ZONE_INFO_H