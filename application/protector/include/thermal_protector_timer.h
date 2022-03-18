/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef THERMAL_PROTECTOR_TIMER_H
#define THERMAL_PROTECTOR_TIMER_H

#include <map>
#include <vector>
#include "thermal_sensor_provision.h"

namespace OHOS {
namespace PowerMgr {
enum EventType {
    EVENT_UEVENT_FD,
    EVENT_TIMER_FD,
};
class ThermalProtectorTimer {
public:
    using Callback = std::function<void(ThermalProtectorTimer*, void*)>;
    using NotifyTask = std::function<void(SensorsMap)>;

    ThermalProtectorTimer(const std::shared_ptr<ThermalSensorProvision>& provision);
    ~ThermalProtectorTimer() {}
    void RegisterTask(NotifyTask task);
    int32_t Init();
    void ReportThermalData();
    int32_t LoopingThreadEntry(void *arg, int32_t epfd);
    void Run(void *service, int32_t epfd);
    void StartThread(void *service);
    int32_t RegisterCallback(const int32_t fd, const EventType et, int32_t epfd);
    void SetTimerInterval(int32_t interval, int32_t timerfd);
    void ResetCount();
private:
    void NotifyPolicy();
    int32_t InitProvisionTimer();
    void TimerProvisionCallback(void *service);
    int32_t CreateProvisionFd();
    int32_t timeCount_ {0};
    int32_t epFd_ {-1};
    int32_t timerFd_ {-1};
    int32_t timerInterval_ {-1};
    std::map<int32_t, Callback> callbackHandler_;
    std::vector<int32_t> multipleList_;
    int32_t reportTime_;
    NotifyTask notify_;
    SensorsMap sensorMap_;
    std::shared_ptr<ThermalSensorProvision> provision_;
};
} // PowerMgr
} // OHOS
#endif // THERMAL_HDF_TIMER_H