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

#include <deque>
#include <memory>
#include <mutex>
#include <time.h>
#include "thermal_sensor_info.h"
#include "thermal_observer.h"

namespace OHOS {
namespace PowerMgr {
class ThermalServiceSubscriber {
public:
    ThermalServiceSubscriber();
    ~ThermalServiceSubscriber() = default;
    bool Init();
    void OnTemperatureChanged(TypeTempMap typeTempMap);
    void SetHistoryTypeTempMap(TypeTempMap typeTempMap);
    uint32_t GetRateCount() const
    {
        return rateCount_;
    }
    const std::map<std::string, std::deque<double>>& GetSensorsRate()
    {
        return sensorsRateMap_;
    }
    TypeTempMap& GetSubscriberInfo()
    {
        return typeTempMap_;
    }
private:
    double GetThermalRiseRate(const std::deque<int32_t> &tempQueue);

    TypeTempMap typeTempMap_;
    uint32_t historyCount_ {0};
    uint32_t rateCount_ {3};
    std::map<std::string, std::deque<int32_t>> typeHistoryMap_;
    std::map<std::string, std::deque<double>> sensorsRateMap_;
    std::mutex mutex_;
};
} // namespace PowerMgr
} // namespace OHOS

#endif //  THERMAL_SRV_SUBSCRIBER_H