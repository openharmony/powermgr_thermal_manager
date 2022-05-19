/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef THERMAL_SENSOR_PROVIDER_H
#define THERMAL_SENSOR_PROVIDER_H


#include <functional>
#include <map>
#include <list>

#include "thermal_zone_manager.h"
#include "thermal_simulation_node.h"
#include "v1_0/thermal_types.h"

namespace OHOS {
namespace PowerMgr {
using namespace OHOS::HDI::Thermal::V1_0;
using SensorsMap = std::map<std::string, int32_t>;
enum EventType {
    EVENT_UEVENT_FD,
    EVENT_TIMER_FD,
};

class ThermalSensorProvider : public ThermalZoneManager {
public:
    using Callback = std::function<void(ThermalSensorProvider*, void*)>;
    using IntervalMap = std::map<std::string, int32_t>;
    using NotifyTask = std::function<void(SensorsMap)>;

    ThermalSensorProvider() {};
    ~ThermalSensorProvider() {};

    bool InitProvider();
    void GetThermalSensorInfo();
    void RegisterTask(NotifyTask task);
    void NotifyPolicy();
    void GetTypeInterval();

    int32_t LoopingThreadEntry(void *arg, int32_t epfd);
    void Run(void *service, int32_t epfd);
    void StartThread(void *service);
    int32_t RegisterCallback(const int fd, const EventType et, int32_t epfd);
    void SetTimerInterval(int interval, int32_t timerfd);
    void SetIntervalMap(IntervalMap &intervalMap);
private:
    int32_t InitProviderTimer();
    void TimerProviderCallback(void *service);
    int32_t CreateProviderFd();

    std::vector<ThermalZoneInfo> tzInfoList_;
    SensorsMap typeTempMap_;
    IntervalMap intervalMap_;
    NotifyTask notify_;
    uint32_t interval_;
    /* timer related */
    int32_t epFd_ {-1};
    int32_t timerFd_ {-1};
    int32_t timerInterval_ {-1};
    std::map<int32_t, Callback> callbackHandler_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif
