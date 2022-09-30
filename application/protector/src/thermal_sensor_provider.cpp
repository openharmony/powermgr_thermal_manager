/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "thermal_sensor_provider.h"

#include <cerrno>
#include <thread>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <linux/netlink.h>

#include "thermal_kernel_config_file.h"
#include "thermal_common.h"

namespace OHOS {
namespace PowerMgr {
namespace {
const int ERR_INVALID_FD = -1;
const int32_t MS_PER_SECOND = 1000;
}

bool ThermalSensorProvider::InitProvider()
{
    int32_t ret = -1;

    ret = InitThermalZoneSysfs();
    if (ret != ERR_OK) {
        THERMAL_HILOGE(FEATURE_PROTECTOR, "failed to get path info.");
        return false;
    }

    StartThread(this);
    return true;
}

void ThermalSensorProvider::GetThermalSensorInfo()
{
    THERMAL_HILOGD(FEATURE_PROTECTOR, "Enter");
    int32_t ret = -1;

    ret = ParseThermalZoneInfo();
    if (ret != ERR_OK) {
        THERMAL_HILOGE(FEATURE_PROTECTOR, "failed to thermal zone info");
        return;
    }
    tzInfoList_ = tzInfoAcaualEvent_.info;

    THERMAL_HILOGI(FEATURE_PROTECTOR, "size = %{public}zu", tzInfoList_.size());

    for (auto type : intervalMap_) {
        THERMAL_HILOGI(FEATURE_PROTECTOR, "configType = %{public}s", type.first.c_str());
    }

    for (auto iter = tzInfoList_.begin(); iter != tzInfoList_.end(); iter++) {
        THERMAL_HILOGD(FEATURE_PROTECTOR, "type = %{public}s", iter->type.c_str());
        THERMAL_HILOGD(FEATURE_PROTECTOR, "temp = %{public}d", iter->temp);
        for (auto intervalIter = intervalMap_.begin(); intervalIter != intervalMap_.end(); intervalIter++) {
            if (iter->type.find(intervalIter->first) != std::string::npos) {
                THERMAL_HILOGI(FEATURE_PROTECTOR, "success to find type");
                typeTempMap_.insert(std::make_pair(intervalIter->first, iter->temp));
            }
        }
    }
}

void ThermalSensorProvider::SetIntervalMap(IntervalMap &intervalMap)
{
    intervalMap_ = intervalMap;
}

void ThermalSensorProvider::RegisterTask(NotifyTask task)
{
    notify_ = task;
}

void ThermalSensorProvider::NotifyPolicy()
{
    THERMAL_HILOGD(FEATURE_PROTECTOR, "Enter");
    GetThermalSensorInfo();
    for (auto info : typeTempMap_) {
        THERMAL_HILOGD(FEATURE_PROTECTOR, "type = %{public}s, temp = %{public}d",
            info.first.c_str(), info.second);
    }
    notify_(typeTempMap_);
}

int32_t ThermalSensorProvider::InitProviderTimer()
{
    THERMAL_HILOGD(FEATURE_PROTECTOR, "Enter");
    int32_t ret = -1;
    epFd_ = epoll_create1(EPOLL_CLOEXEC);

    ret = CreateProviderFd();
    if (ret != ERR_OK) {
        THERMAL_HILOGE(FEATURE_PROTECTOR, "failed to create polling fd");
        return ret;
    }
    return ERR_OK;
}

void ThermalSensorProvider::GetTypeInterval()
{
    for (auto iter : intervalMap_) {
        interval_ = iter.second;
    }
}

int32_t ThermalSensorProvider::CreateProviderFd()
{
    timerFd_ = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (timerFd_ == ERR_INVALID_FD) {
        THERMAL_HILOGE(FEATURE_PROTECTOR, "epoll create failed, epFd_ is invalid");
        return ERR_INVALID_FD;
    }
    GetTypeInterval();
    THERMAL_HILOGD(FEATURE_PROTECTOR, "interval_ is %{public}d", interval_);
    SetTimerInterval(interval_, timerFd_);
    fcntl(timerFd_, F_SETFL, O_NONBLOCK);
    callbackHandler_.insert(std::make_pair(timerFd_, &ThermalSensorProvider::TimerProviderCallback));
    if (RegisterCallback(timerFd_, EVENT_TIMER_FD, epFd_)) {
        THERMAL_HILOGE(FEATURE_PROTECTOR, "register Timer event failed");
    }
    THERMAL_HILOGD(FEATURE_PROTECTOR, "Exit");
    return ERR_OK;
}

void ThermalSensorProvider::TimerProviderCallback(void *service)
{
    THERMAL_HILOGD(FEATURE_PROTECTOR, "Enter");
    unsigned long long timers;

    if (read(timerFd_, &timers, sizeof(timers)) == -1) {
        THERMAL_HILOGE(FEATURE_PROTECTOR, "read timerFd_ failed");
        return;
    }
    NotifyPolicy();
    return;
}

void ThermalSensorProvider::SetTimerInterval(int interval, int32_t timerfd)
{
    THERMAL_HILOGD(FEATURE_PROTECTOR, "start SetTimerInterval: %{public}d", timerfd);
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
        THERMAL_HILOGE(FEATURE_PROTECTOR, "timer failed.");
    }
    THERMAL_HILOGD(FEATURE_PROTECTOR, "Exit");
}

int32_t ThermalSensorProvider::RegisterCallback(const int fd, const EventType et, int32_t epfd)
{
    THERMAL_HILOGD(FEATURE_PROTECTOR, "Enter");
    struct epoll_event ev;

    ev.events = EPOLLIN;
    if (et == EVENT_TIMER_FD) {
        ev.events |= EPOLLWAKEUP;
    }
    THERMAL_HILOGI(FEATURE_PROTECTOR, "epfd: %{public}d, fd: %{public}d", epfd, fd);
    ev.data.ptr = reinterpret_cast<void*>(this);
    ev.data.fd = fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        THERMAL_HILOGE(FEATURE_PROTECTOR, "epoll_ctl failed, error num =%{public}d", errno);
        return -1;
    }
    THERMAL_HILOGD(FEATURE_PROTECTOR, "Exit");
    return ERR_OK;
}

int32_t ThermalSensorProvider::LoopingThreadEntry(void *arg, int32_t epfd)
{
    size_t eventct = callbackHandler_.size();
    struct epoll_event events[eventct];
    THERMAL_HILOGD(FEATURE_PROTECTOR, "epfd: %{public}d, eventct: %{public}zu", epfd, eventct);
    while (true) {
        int32_t nevents = epoll_wait(epfd, events, eventct, -1);
        if (nevents == -1) {
            continue;
        }
        for (int n = 0; n < nevents; ++n) {
            if (events[n].data.ptr) {
                ThermalSensorProvider *func = const_cast<ThermalSensorProvider *>(this);
                (callbackHandler_.find(events[n].data.fd)->second)(func, arg);
            }
        }
    }
}

void ThermalSensorProvider::Run(void *service, int32_t epfd)
{
    std::make_unique<std::thread>(&ThermalSensorProvider::LoopingThreadEntry, this, service, epfd)->join();
}

void ThermalSensorProvider::StartThread(void *service)
{
    THERMAL_HILOGD(FEATURE_PROTECTOR, "Enter");
    int ret = -1;
    ret = InitProviderTimer();
    if (ret != ERR_OK) {
        THERMAL_HILOGE(FEATURE_PROTECTOR, "init Timer failed, ret: %{public}d", ret);
        return;
    }
    Run(service, epFd_);
    while (true) {
        pause();
    }
    THERMAL_HILOGD(FEATURE_PROTECTOR, "Exit");
}
} // namespace PowerMgr
} // namespace OHOS
