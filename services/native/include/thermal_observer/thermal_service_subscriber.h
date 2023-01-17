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

#ifndef THERMAL_SRV_SUBSCRIBER_H
#define THERMAL_SRV_SUBSCRIBER_H

#include <memory>
#include <mutex>
#include <time.h>
#include "thermal_sensor_info.h"
#include "thermal_observer.h"
#include "delayed_sp_singleton.h"

namespace OHOS {
namespace PowerMgr {
class ThermalServiceSubscriber {
public:
    ThermalServiceSubscriber();
    ~ThermalServiceSubscriber() = default;
    bool Init();
    void OnTemperatureChanged(TypeTempMap typeTempMap);
    void SetHistoryTypeTempMap(TypeTempMap typeTempMap);
    std::map<std::string, double> GetSensorsRate()
    {
        return sensorsRateMap_;
    }
    TypeTempMap GetSubscriberInfo()
    {
        return typeTempMap_;
    }
private:
    TypeTempMap typeTempMap_;
    ThermalSensorInfo::TypeHistoryMap typeHistoryMap_;
    uint32_t historyCount_;
    time_t startTime_;
    time_t endTime_;
    std::map<std::string, double> sensorsRateMap_;
    std::mutex mutex_;
    int32_t count_ {0};
    int32_t magnification_ {0};
};
} // namespace PowerMgr
} // namespace OHOS

#endif //  THERMAL_SRV_SUBSCRIBER_H