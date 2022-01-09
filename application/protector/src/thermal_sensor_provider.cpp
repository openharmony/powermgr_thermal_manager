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
    if (!GetFlag()) {
        ret = NodeInit();
        if (ret != ERR_OK) {
            THERMAL_HILOGE(MODULE_THERMAL_PROTECTOR, "%{public}s: failed to create file", __func__);
            return false;
        }
    } else {
        ret = InitThermalZoneSysfs();
        if (ret != ERR_OK) {
            THERMAL_HILOGE(MODULE_THERMAL_PROTECTOR, "%{public}s: failed to get path info.", __func__);
            return false;
        }
    }
    StartThread(this);
    return true;
}

void ThermalSensorProvider::GetThermalSensorInfo()
{
    THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s enter", __func__);
    int32_t ret = -1;

    if (!GetFlag()) {
        ret = ParserSimulationNode();
        if (ret != ERR_OK) {
            THERMAL_HILOGE(MODULE_THERMAL_PROTECTOR, "%{public}s failed to simulation node ", __func__);
            return;
        }
    } else {
        ret = ParseThermalZoneInfo();
        if (ret != ERR_OK) {
            THERMAL_HILOGE(MODULE_THERMAL_PROTECTOR, "%{public}s failed to thermal zone info", __func__);
            return;
        }
    }
    info_ = GetlTzInfo();

    THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s size = %{public}d", __func__, info_.size());

    for (auto type : intervalMap_) {
        THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s configType = %{public}s", __func__, type.first.c_str());
    }

    for (auto iter = info_.begin(); iter != info_.end(); iter++) {
        THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "type = %{public}s", iter->GetType().c_str());
        THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "temp = %{public}d", iter->GetTemp());
        for (auto intervalIter = intervalMap_.begin(); intervalIter != intervalMap_.end(); intervalIter++) {
            if (iter->GetType().find(intervalIter->first) != std::string::npos) {
                THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s success to find type", __func__);
                typeTempMap_.emplace(std::pair(intervalIter->first, iter->GetTemp()));
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
    THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s enter", __func__);
    GetThermalSensorInfo();
    for (auto info : typeTempMap_) {
        THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s type = %{public}s, temp = %{public}d",
            __func__, info.first.c_str(), info.second);
    }
    notify_(typeTempMap_);
}

int32_t ThermalSensorProvider::InitProviderTimer()
{
    THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s:  Enter", __func__);
    int32_t ret = -1;
    epFd_ = epoll_create1(EPOLL_CLOEXEC);

    ret = CreateProviderFd();
    if (ret != ERR_OK) {
        THERMAL_HILOGE(MODULE_THERMAL_PROTECTOR, "%{public}s failed to create polling fd", __func__);
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
        THERMAL_HILOGE(MODULE_THERMAL_PROTECTOR, "%{public}s epoll create failed, epFd_ is invalid", __func__);
        return ERR_INVALID_FD;
    }
    GetTypeInterval();
    THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s: %{public}d", __func__, interval_);
    SetTimerInterval(interval_, timerFd_);
    fcntl(timerFd_, F_SETFL, O_NONBLOCK);
    callbackHandler_.insert(std::make_pair(timerFd_, &ThermalSensorProvider::TimerProviderCallback));
    if (RegisterCallback(timerFd_, EVENT_TIMER_FD, epFd_)) {
        THERMAL_HILOGE(MODULE_THERMAL_PROTECTOR, "%{public}s register Timer event failed", __func__);
    }
    THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "return");
    return ERR_OK;
}

void ThermalSensorProvider::TimerProviderCallback(void *service)
{
    THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s enter", __func__);
    unsigned long long timers;

    if (read(timerFd_, &timers, sizeof(timers)) == -1) {
        THERMAL_HILOGE(MODULE_THERMAL_PROTECTOR, "%{public}s read timerFd_ failed", __func__);
        return;
    }
    NotifyPolicy();
    return;
}

void ThermalSensorProvider::SetTimerInterval(int interval, int32_t timerfd)
{
    THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s enter, start SetTimerInterval: %{public}d", __func__, timerfd);
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
        THERMAL_HILOGE(MODULE_THERMAL_PROTECTOR, "%{public}s: timer failed\n", __func__);
    }
    THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "return");
}

int32_t ThermalSensorProvider::RegisterCallback(const int fd, const EventType et, int32_t epfd)
{
    THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "Enter");
    struct epoll_event ev;

    ev.events = EPOLLIN;
    if (et == EVENT_TIMER_FD) {
        ev.events |= EPOLLWAKEUP;
    }
    THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}d, %{public}d", epfd, fd);
    ev.data.ptr = reinterpret_cast<void*>(this);
    ev.data.fd = fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        THERMAL_HILOGE(MODULE_THERMAL_PROTECTOR, "%{public}s: epoll_ctl failed, error num =%{public}d",
            __func__, errno);
        return -1;
    }
    THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "return");
    return ERR_OK;
}

int32_t ThermalSensorProvider::LoopingThreadEntry(void *arg, int32_t epfd)
{
    int nevents = 0;
    size_t eventct = callbackHandler_.size();
    struct epoll_event events[eventct];
    THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s: %{public}d, %{public}d", __func__, epfd, eventct);
    while (true) {
        nevents = epoll_wait(epfd, events, eventct, -1);
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
    THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s:  Enter", __func__);
    int ret = -1;
    ret = InitProviderTimer();
    if (ret != ERR_OK) {
        THERMAL_HILOGE(MODULE_THERMAL_PROTECTOR, "%{public}s init Timer failed, ret: %{public}d", __func__, ret);
        return;
    }
    Run(service, epFd_);
    while (true) {
        pause();
    }
    THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "return");
}
} // namespace PowerMgr
} // namespace OHOS