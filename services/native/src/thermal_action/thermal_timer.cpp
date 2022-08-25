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

#include "thermal_timer.h"

#include "thermal_common.h"

using namespace OHOS::MiscServices;
namespace OHOS {
namespace PowerMgr {
ThermalTimerInfo::ThermalTimerInfo()
{
}

ThermalTimerInfo::~ThermalTimerInfo()
{
}

void ThermalTimerInfo::OnTrigger()
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    if (callback_ != nullptr) {
        THERMAL_HILOGI(COMP_SVC, "call back");
        callback_();
    }
    THERMAL_HILOGD(COMP_SVC, "Exit");
}

void ThermalTimerInfo::SetCallbackInfo(const TimerInfoCallback &callBack)
{
    callback_ = callBack;
}

void ThermalTimerInfo::SetType(const int &_type)
{
    type = _type;
}

void ThermalTimerInfo::SetRepeat(bool _repeat)
{
    repeat = _repeat;
}

void ThermalTimerInfo::SetInterval(const uint64_t &_interval)
{
    interval = _interval;
}

void ThermalTimerInfo::SetWantAgent(std::shared_ptr<OHOS::AbilityRuntime::WantAgent::WantAgent> _wantAgent)
{
    wantAgent = _wantAgent;
}

uint64_t ThermalTimer::CreateTimer(std::shared_ptr<ThermalTimerInfo>& timerInfo)
{
    uint64_t timerId = 0;
    timerId = TimeServiceClient::GetInstance()->CreateTimer(timerInfo);
    if (timerId <= 0) {
        THERMAL_HILOGE(COMP_SVC, "failed to create timer id");
        return timerId;
    }
    return timerId;
}

bool ThermalTimer::StartTimer(uint64_t timerId, uint64_t triggerTime)
{
    int delayTime = 6000;
    auto ret = TimeServiceClient::GetInstance()->StartTimer(timerId, triggerTime);
    std::this_thread::sleep_for(std::chrono::milliseconds(delayTime));
    if (!ret) {
        THERMAL_HILOGE(COMP_SVC, "failed to start timer");
        return ret;
    }
    return true;
}

bool ThermalTimer::StopTimer(uint64_t timerId)
{
    auto ret = TimeServiceClient::GetInstance()->StopTimer(timerId);
    if (!ret) {
        THERMAL_HILOGE(COMP_SVC, "failed to stop timer");
        return ret;
    }
    return true;
}

bool ThermalTimer::DestroyTimer(uint64_t timerId)
{
    auto ret = TimeServiceClient::GetInstance()->DestroyTimer(timerId);
    if (!ret) {
        THERMAL_HILOGE(COMP_SVC, "failed to destroy timer");
        return ret;
    }
    return true;
}

void ThermalTimerUtils::Start(int interval, NotifyTask &task)
{
    // is started, do not start again
    if (expired_ == false)
        return;

    // start async timer, launch thread and wait in that thread
    expired_ = false;
    std::thread([this, interval, task]() {
        while (!tryToExpire_) {
            // sleep every interval and do the task again and again until times up
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            task();
        }

        {
            // timer be stopped, update the condition variable expired and wake main thread
            std::lock_guard<std::mutex> locker(mutex_);
            expired_ = true;
            expiredCond_.notify_one();
        }
    }).detach();
}

void ThermalTimerUtils::StartOnce(int delay, NotifyTask &task)
{
    std::thread([delay, task]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        task();
    }).detach();
}

void ThermalTimerUtils::Stop()
{
    // do not stop again
    if (expired_)
        return;

    if (tryToExpire_)
        return;

    // wait until timer
    tryToExpire_ = true; // change this bool value to make timer while loop stop
    {
        std::unique_lock<std::mutex> locker(mutex_);
        expiredCond_.wait(locker, [this] {
            return expired_ == true;
        });

        // reset the timer
        if (expired_ == true)
            tryToExpire_ = false;
    }
}
} // namespace PowerMgr
} // namespace OHOS
