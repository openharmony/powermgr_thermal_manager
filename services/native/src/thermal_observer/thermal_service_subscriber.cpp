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

#include "thermal_service_subscriber.h"

#include <functional>

#include "string_operation.h"
#include "thermal_service.h"
#include "thermal_common.h"
#include "constants.h"
namespace OHOS {
namespace PowerMgr {
namespace {
const int32_t SEC_MIN_NUM = 60;
const int32_t THERMAL_UPDATE_PERIOD = -5;
}
ThermalServiceSubscriber::ThermalServiceSubscriber() { }

bool ThermalServiceSubscriber::Init()
{
    auto tms = ThermalService::GetInstance();
    historyCount_ = tms->GetBaseinfoObj()->GetHistoryTempCount();
    uint32_t riseRateCount = tms->GetBaseinfoObj()->GetRiseRateCount();
    rateCount_ = riseRateCount <= 0 ? rateCount_ : riseRateCount;
    return true;
}

void ThermalServiceSubscriber::OnTemperatureChanged(TypeTempMap typeTempMap)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (typeTempMap.empty()) {
        THERMAL_HILOGE(COMP_SVC, "failed to get sensor info: %{public}zu", typeTempMap.size());
        return;
    }

    if (!typeTempMap_.empty()) {
        typeTempMap_.clear();
    }

    for (auto it : typeTempMap) {
        typeTempMap_[it.first] = it.second;
    }
    auto tms = ThermalService::GetInstance();
    tms->GetSensorInfo()->SetTypeTempMap(typeTempMap_);
    tms->GetSensorInfo()->NotifyObserver();

    SetHistoryTypeTempMap(typeTempMap_);
    return;
}

void ThermalServiceSubscriber::SetHistoryTypeTempMap(TypeTempMap typeTempMap)
{
    THERMAL_HILOGD(COMP_SVC, "SetHistoryTypeTempMap: historyCount_=%{public}d, rateCount_=%{public}d",
        historyCount_, rateCount_);
    if (historyCount_ <= 1) {
        return;
    }

    for (auto itMap : typeTempMap) {
        auto iter = typeHistoryMap_.find(itMap.first);
        if (iter != typeHistoryMap_.end()) {
            if (iter->second.size() >= historyCount_) {
                iter->second.pop_back();
                iter->second.push_front(itMap.second);
            } else {
                iter->second.push_front(itMap.second);
            }
        } else {
            std::deque<int32_t> historyTempList;
            historyTempList.push_front(itMap.second);
            typeHistoryMap_.insert(std::make_pair(itMap.first, historyTempList));
        }
    }

    for (auto history : typeHistoryMap_) {
        const auto& item = history.second;
        double rate = GetThermalRiseRate(item);

        auto iter = sensorsRateMap_.find(history.first);
        if (iter != sensorsRateMap_.end()) {
            if (iter->second.size() >= rateCount_) {
                iter->second.pop_front();
                iter->second.push_back(rate);
            } else {
                iter->second.push_back(rate);
            }
        } else {
            std::deque<double> historyRateList;
            historyRateList.push_front(rate);
            sensorsRateMap_.insert(std::make_pair(history.first, historyRateList));
        }
    }
}

double ThermalServiceSubscriber::GetThermalRiseRate(const std::deque<int32_t> &tempQueue)
{
    uint32_t tempQueueSize = tempQueue.size();
    if (tempQueueSize < historyCount_) {
        return 0;
    }

    // yi: tempQueue[i]
    int32_t time = 0;                   // xi
    int32_t timeSum = 0;                // sum of xi
    int32_t tempSum = 0;                // sum of yi
    int32_t timeSquaredSum = 0;         // sum of xi * xi
    int32_t timeTempMultiplySum = 0;    // sum of xi * yi

    for (uint32_t timeIndex = 0; timeIndex < tempQueueSize; timeIndex++) {
        time = timeIndex * THERMAL_UPDATE_PERIOD;
        timeSum += time;
        tempSum += tempQueue.at(timeIndex);
        timeSquaredSum += time * time;
        timeTempMultiplySum += time * tempQueue.at(timeIndex);
        THERMAL_HILOGD(COMP_SVC, "GetThermalRiseRate: timeIndex=%{private}u, value=%{private}d", timeIndex,
            tempQueue.at(timeIndex));
    }
    int32_t calcTime = tempQueueSize * timeSquaredSum - timeSum * timeSum;  // (n * sum(xi * xi)) - (sum(xi) * sum(xi))
    if (calcTime == 0) {
        return 0;
    }
    // (n * sum(xi * yi) - (sum(xi) * sum(yi))) / (n * sum(xi * xi)) - (sum(xi) * sum(xi))
    double slopeRate = (double)(tempQueueSize * timeTempMultiplySum - timeSum * tempSum) / calcTime;

    return slopeRate * SEC_MIN_NUM;
}
} // namespace PowerMgr
} // namespace OHOS