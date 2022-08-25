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

#ifndef THERMAL_TIMER_H
#define THERMAL_TIMER_H

#include <string>
#include <chrono>
#include <atomic>
#include <memory>
#include <mutex>
#include <functional>
#include <cstdint>
#include <vector>
#include <thread>
#include <condition_variable>
#include <inttypes.h>
#include <sys/time.h>
#include "time_service_client.h"

namespace OHOS {
namespace PowerMgr {
class ThermalTimerInfo : public MiscServices::ITimerInfo {
public:
    using TimerInfoCallback = std::function<void()>;
    ThermalTimerInfo();
    virtual ~ThermalTimerInfo();
    virtual void OnTrigger() override;
    virtual void SetType(const int &type) override;
    virtual void SetRepeat(bool repeat) override;
    virtual void SetInterval(const uint64_t &interval) override;
    virtual void SetWantAgent(std::shared_ptr<OHOS::AbilityRuntime::WantAgent::WantAgent> wantAgent) override;
    void SetCallbackInfo(const TimerInfoCallback &callBack);
private:
    TimerInfoCallback callback_ {nullptr};
};


class ThermalTimer {
public:
    ThermalTimer() = default;
    ~ThermalTimer() = default;
    uint64_t CreateTimer(std::shared_ptr<ThermalTimerInfo>& timerInfo);
    bool StartTimer(uint64_t timerId, uint64_t triggerTime);
    bool StopTimer(uint64_t timerId);
    bool DestroyTimer(uint64_t timerId);
};

class ThermalTimerUtils {
public:
    using NotifyTask = std::function<void()>;
    ThermalTimerUtils() : expired_(true), tryToExpire_(false)
    {
    }
    ThermalTimerUtils(const ThermalTimerUtils& timerUtils)
    {
        expired_ = timerUtils.expired_.load();
        tryToExpire_ = timerUtils.tryToExpire_.load();
    }
    ~ThermalTimerUtils()
    {
        Stop();
    }
    void Start(int interval, NotifyTask &task);
    void StartOnce(int delay, NotifyTask &task);
    void Stop();
private:
    std::atomic<bool> expired_; // timer stopped status
    std::atomic<bool> tryToExpire_; // timer is in stop process
    std::mutex mutex_;
    std::condition_variable expiredCond_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_TIMER_H
