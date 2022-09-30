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

#include "thermal_protector_timer.h"
#include <cerrno>
#include <thread>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <linux/netlink.h>
#include "thermal_common.h"

namespace OHOS {
namespace PowerMgr {
namespace {
constexpr int32_t ERR_INVALID_FD = -1;
constexpr int32_t MS_PER_SECOND = 1000;
constexpr int32_t END_TIME = 2;
}
ThermalProtectorTimer::ThermalProtectorTimer(const std::shared_ptr<ThermalSensorProvision>& provision)
{
    provision_ = provision;
}

void ThermalProtectorTimer::RegisterTask(NotifyTask task)
{
    notify_ = task;
}

void ThermalProtectorTimer::NotifyPolicy()
{
    THERMAL_HILOGD(FEATURE_PROTECTOR, "Enter");
    provision_->ReportThermalZoneData(reportTime_, multipleList_);
    auto tzMap = provision_->GetSensorData();
    notify_(tzMap);
}

int32_t ThermalProtectorTimer::CreateProvisionFd()
{
    timerFd_ = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (timerFd_ == ERR_INVALID_FD) {
        THERMAL_HILOGE(FEATURE_PROTECTOR, "epoll create failed, epFd_ is invalid");
        return ERR_INVALID_VALUE;
    }
    int interval = provision_->GetMaxCd();
    SetTimerInterval(interval, timerFd_);
    fcntl(timerFd_, F_SETFL, O_NONBLOCK);
    callbackHandler_.insert(std::make_pair(timerFd_, &ThermalProtectorTimer::TimerProvisionCallback));
    if (RegisterCallback(timerFd_, EVENT_TIMER_FD, epFd_)) {
        THERMAL_HILOGI(FEATURE_PROTECTOR, "register Timer event failed");
    }

    THERMAL_HILOGD(FEATURE_PROTECTOR, "Exit");
    return ERR_OK;
}

int32_t ThermalProtectorTimer::RegisterCallback(const int32_t fd, const EventType et, int32_t epfd)
{
    struct epoll_event ev;

    ev.events = EPOLLIN;
    if (et == EVENT_TIMER_FD) {
        ev.events |= EPOLLWAKEUP;
    }
    THERMAL_HILOGI(FEATURE_PROTECTOR, "%{public}d, %{public}d", epfd, fd);
    ev.data.ptr = reinterpret_cast<void*>(this);
    ev.data.fd = fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        THERMAL_HILOGE(FEATURE_PROTECTOR, "epoll_ctl failed, error num =%{public}d", errno);
        return -1;
    }
    THERMAL_HILOGD(FEATURE_PROTECTOR, "Exit");
    return ERR_OK;
}

void ThermalProtectorTimer::TimerProvisionCallback(void *service)
{
    unsigned long long timers;

    if (read(timerFd_, &timers, sizeof(timers)) == -1) {
        THERMAL_HILOGE(FEATURE_PROTECTOR, "read timerFd_ failed");
        return;
    }
    reportTime_ = reportTime_ + 1;
    NotifyPolicy();
    ResetCount();
    return;
}

void ThermalProtectorTimer::SetTimerInterval(int32_t interval, int32_t timerfd)
{
    struct itimerspec itval;

    if (timerfd == ERR_INVALID_FD) {
        return;
    }

    timerInterval_ = interval;

    if (interval < 0) {
        interval = 0;
    }

    itval.it_interval.tv_sec = interval / MS_PER_SECOND;
    itval.it_interval.tv_nsec = 0;
    itval.it_value.tv_sec = interval / MS_PER_SECOND;
    itval.it_value.tv_nsec = 0;
    if (timerfd_settime(timerfd, 0, &itval, nullptr) == -1) {
        THERMAL_HILOGE(FEATURE_PROTECTOR, "timer failed\n");
    }
    THERMAL_HILOGD(FEATURE_PROTECTOR, "Exit");
}

int32_t ThermalProtectorTimer::InitProvisionTimer()
{
    int32_t ret;
    epFd_ = epoll_create1(EPOLL_CLOEXEC);

    ret = CreateProvisionFd();
    if (ret != ERR_OK) {
        THERMAL_HILOGE(FEATURE_PROTECTOR, "failed to create polling fd");
        return ret;
    }
    return ERR_OK;
}

int32_t ThermalProtectorTimer::LoopingThreadEntry(void *arg, int32_t epfd)
{
    size_t eventct = callbackHandler_.size();
    struct epoll_event events[eventct];
    THERMAL_HILOGI(FEATURE_PROTECTOR, "%{public}d, %{public}zu", epfd, eventct);
    while (true) {
        if (timeCount_ == END_TIME) {
            return ERR_OK;
        }
        int32_t nevents = epoll_wait(epfd, events, eventct, -1);
        if (nevents == -1) {
            continue;
        }
        for (int32_t n = 0; n < nevents; ++n) {
            if (events[n].data.ptr) {
                ThermalProtectorTimer *func = const_cast<ThermalProtectorTimer *>(this);
                (callbackHandler_.find(events[n].data.fd)->second)(func, arg);
            }
        }
        timeCount_ = timeCount_ + 1;
    }
}

void ThermalProtectorTimer::Run(void *service, int32_t epfd)
{
    std::make_unique<std::thread>(&ThermalProtectorTimer::LoopingThreadEntry, this, service, epfd)->join();
}

void ThermalProtectorTimer::StartThread(void *service)
{
    int32_t ret = InitProvisionTimer();
    if (ret != ERR_OK) {
        THERMAL_HILOGE(FEATURE_PROTECTOR, "init Timer failed, ret: %{public}d", ret);
        return;
    }
    Run(service, epFd_);
}

int32_t ThermalProtectorTimer::Init()
{
    StartThread(this);
    return ERR_OK;
}

void ThermalProtectorTimer::ResetCount()
{
    THERMAL_HILOGD(FEATURE_PROTECTOR, "multipleList_:%{public}zu", multipleList_.size());
    if (multipleList_.empty()) return;

    int32_t maxValue = *(std::max_element(multipleList_.begin(), multipleList_.end()));
    if (reportTime_ == maxValue) {
        THERMAL_HILOGI(FEATURE_PROTECTOR, "reportTime:%{public}d", reportTime_);
        reportTime_ = 0;
    }
}
} // PowerMgr
} // OHOS
