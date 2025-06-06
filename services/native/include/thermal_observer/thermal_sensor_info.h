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

#ifndef THERMAL_SENSOR_INFO_H
#define THERMAL_SENSOR_INFO_H

#include <list>
#include "thermal_observer.h"

namespace OHOS {
namespace PowerMgr {
class ThermalSensorInfo {
public:
    using TypeHistoryMap = std::map<std::string, std::list<int32_t>>;
    ThermalSensorInfo() = default;
    ~ThermalSensorInfo() = default;

    void SetTypeTempMap(TypeTempMap &typeTempMap);
    TypeTempMap GetTypeTempMap();
    int32_t GetTemp(std::string type);
    std::list<int> GetHistoryTemperature(std::string type);
    void NotifyObserver();
private:
    TypeTempMap typeTempMap_;
    std::mutex mutex_;
    std::list<int> historyMap_;
    int temp_;
    TypeHistoryMap typeHistoryMap_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_SENSOR_INFO_H