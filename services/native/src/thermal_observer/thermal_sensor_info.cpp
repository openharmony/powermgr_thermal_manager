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

#include "thermal_sensor_info.h"
#include "thermal_common.h"
#include "thermal_service.h"

namespace OHOS {
namespace PowerMgr {
namespace {
auto g_service = DelayedSpSingleton<ThermalService>::GetInstance();
}

TypeTempMap ThermalSensorInfo::GetTypeTempMap()
{
    return this->typeTempMap_;
}

void ThermalSensorInfo::SetTypeTempMap(TypeTempMap &typeTempMap)
{
    this->typeTempMap_ = typeTempMap;
}

int32_t ThermalSensorInfo::GetTemp(std::string type)
{
    TypeTempMap::iterator iter = this->typeTempMap_.find(type);
    if (iter == this->typeTempMap_.end()) {
        THERMAL_HILOGW(COMP_SVC, "failed to find sensor info");
        return ERR_INVALID_VALUE;
    }
    temp_ = (int32_t) this->typeTempMap_[type];
    return temp_;
}

std::list<int> ThermalSensorInfo::GetHistoryTemperature(std::string type)
{
    historyMap_ = typeHistoryMap_[type];
    return historyMap_;
}

void ThermalSensorInfo::NotifyObserver()
{
    g_service->GetObserver()->OnReceivedSensorInfo(GetTypeTempMap());
}
} // PowerMgr
} // OHOS