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
const int32_t COUNT_MAX = 6;
const int32_t SEC_MIN_NUM = 60;
const int32_t THOUSAND_UNIT = 1000;
auto g_service = DelayedSpSingleton<ThermalService>::GetInstance();
}
ThermalServiceSubscriber::ThermalServiceSubscriber() { }

bool ThermalServiceSubscriber::Init()
{
    historyCount_ = g_service->GetBaseinfoObj()->GetHistoryTempCount();
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

    g_service->GetSensorInfo()->SetTypeTempMap(typeTempMap_);
    g_service->GetSensorInfo()->NotifyObserver();

    if (count_ == 0) {
        startTime_ = time(NULL);
    }

    if (count_ == COUNT_MAX) {
        endTime_ = time(NULL);
        magnification_ ++;
        count_ = 0;
    }

    SetHistoryTypeTempMap(typeTempMap_);
    count_ ++;
    return;
}

void ThermalServiceSubscriber::SetHistoryTypeTempMap(TypeTempMap typeTempMap)
{
    THERMAL_HILOGD(COMP_SVC, "SetHistoryTypeTempMap: historyCount_=%{public}d", historyCount_);

    for (auto itMap : typeTempMap) {
        auto iter = typeHistoryMap_.find(itMap.first);
        if (iter != typeHistoryMap_.end()) {
            iter->second.pop_front();
            iter->second.push_back(itMap.second);
        } else {
            std::list<int32_t> historyTempList(historyCount_);
            historyTempList.push_back(itMap.second);
            typeHistoryMap_.insert(std::make_pair(itMap.first, historyTempList));
        }
    }

    int32_t sec = 0;
    if (magnification_ != 0) {
        sec = static_cast<int32_t>(difftime(endTime_, startTime_)) / magnification_;
    }
    THERMAL_HILOGI(COMP_SVC, "SetHistoryTypeTempMap: sec=%{public}d", sec);
    for (auto history : typeHistoryMap_) {
        const auto& item = history.second;
        // The initial value of the sum is 0
        double sum = std::accumulate(item.begin(), item.end(), 0);

        double rate = 0;
        if (sec == SEC_MIN_NUM) {
            rate = (sum / THOUSAND_UNIT / sec) * SEC_MIN_NUM;
        }

        sensorsRateMap_.insert(std::make_pair(history.first, rate));
    }
}
} // namespace PowerMgr
} // namespace OHOS
