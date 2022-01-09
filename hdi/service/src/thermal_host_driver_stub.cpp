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

#include "thermal_host_driver_stub.h"

#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <hdf_base.h>
#include <hdf_log.h>
#include <hdf_sbuf_ipc.h>
#include <linux/netlink.h>
#include "osal/osal_mem.h"
#include "hdf_remote_service.h"
#include "hdf_sbuf.h"
#include "utils/hdf_log.h"

#include "thermal_driver.h"

namespace OHOS {
namespace HDI {
namespace THERMAL {
namespace V1_0 {
namespace {
auto &xmlParser = ThermalHDIConfigParser::GetInsance();
const int ERR_INVALID_FD = -1;
const int32_t MS_PER_SECOND = 1000;
bool flag = false;
const std::string FILE_NAME = "/system/etc/thermal_config/thermal_hdi_config.xml";
const std::string POLLING_V1 = "v1";
const std::string POLLING_V2 = "v2";
}
int32_t ThermalHostDriverStub::Init()
{
    HDF_LOGD("%{public}s:  Enter", __func__);
    int32_t ret = -1;
    if (simulation_ == nullptr) {
        simulation_ = std::make_unique<ThermalSimulationNode>();
        ret = simulation_->NodeInit();
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%{public}s: failed to create file, ret: %{public}d", __func__, ret);
            return ret;
        }
    }

    if (mitigation_ == nullptr) {
        mitigation_ = std::make_unique<DeviceRequestMitigation>();
    }

    if (thermalZone_ == nullptr) {
        thermalZone_ = std::make_unique<ThermalZoneParser>();
        ret = thermalZone_->InitThermalZoneSysfs();
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%{public}s: failed to get path info.", __func__);
        }
    }

    ret = xmlParser.ThermalHDIConfigInit(FILE_NAME);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: failed to init XML, ret: %{public}d", __func__, ret);
        return ret;
    }
    StartThread(this);
    return HDF_SUCCESS;
}

int32_t ThermalHostDriverStub::DispatchBindThermalSuscriber(struct ThermalDriverService& service, HdfSBuf& data)
{
    HDF_LOGD("%{public}s: ThermalHostDriverStub Enter", __func__);
    struct ThermalSubscriber *subscriber = nullptr;
    struct HdfRemoteService *remoteService = HdfSBufReadRemoteService(&data);
    if (remoteService == nullptr) {
        HDF_LOGW("%{public}s: remoteService is NULL", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    subscriber = reinterpret_cast<struct ThermalSubscriber *>(OsalMemCalloc(sizeof(*subscriber)));
    if (subscriber == nullptr) {
        HDF_LOGW("%{public}s: subscriber calloc error", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }
    subscriber->remoteService = remoteService;
    service.subscriber = subscriber;
    subscriber_ = subscriber;
    if (subscriber_ == nullptr) {
        HDF_LOGW("%{public}s: subscriber_is NULL", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    HDF_LOGD("%{public}s: ThermalHostDriverStub Enter2", __func__);
    return HDF_SUCCESS;
}

int32_t ThermalHostDriverStub::DispatchUnBindThermalSubscriber(struct ThermalDriverService& service) const
{
    if (service.subscriber == nullptr) {
        HDF_LOGW("%{public}s subscriber is nullptr", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    OsalMemFree(service.subscriber);
    service.subscriber = nullptr;
    return HDF_SUCCESS;
}

int32_t ThermalHostDriverStub::DispatchGetThermalZoneInfo(MessageParcel& data, MessageParcel& reply,
    MessageOption& option) const
{
    int32_t ret = -1;
    if (!thermalZone_->GetFlag()) {
        ret = thermalZone_->ParserSimulationNode();
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%{public}s: failed to parser simulation thermal zone info. ret: %{public}d", __func__, ret);
            return ret;
        }
    } else {
        ret = thermalZone_->ParseThermalZoneInfo();
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%{public}s: failed to parser really thermal zone info. ret: %{public}d", __func__, ret);
            return ret;
        }
    }

    HDF_LOGD("%{public}s: DispatchGetThermalZoneInfo Enter %{public}d", __func__, thermalZone_->GetlTzInfo().size());
    if (!reply.WriteUint32(thermalZone_->GetlTzInfo().size())) {
        HDF_LOGE("%{public}s: write failed", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    for (auto it: thermalZone_->GetlTzInfo()) {
        if (!reply.WriteString(it.GetType())) {
            HDF_LOGE("%{public}s: write result failed", __func__);
            return HDF_ERR_INVALID_PARAM;
        }
        if (!reply.WriteInt32(it.GetTemp())) {
            HDF_LOGE("%{public}s: write result failed", __func__);
            return HDF_ERR_INVALID_PARAM;
        }
    }
    return HDF_SUCCESS;
}

int32_t ThermalHostDriverStub::DispatchSetCPUFreq(MessageParcel& data, MessageParcel& reply,
    MessageOption& option) const
{
    HDF_LOGD("DispatchSetCPUFreq: Enter");
    uint32_t freq = data.ReadUint32();
    int32_t ret = mitigation_->CpuRequest(freq);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: failed to set freq %{public}d", __func__, ret);
        return ret;
    }
    return HDF_SUCCESS;
}

int32_t ThermalHostDriverStub::DispatchSetGPUFreq(MessageParcel& data, MessageParcel& reply,
    MessageOption& option) const
{
    HDF_LOGD("DispatchSetGPUFreq: Enter");
    uint32_t freq = data.ReadUint32();
    int32_t ret = mitigation_->GpuRequest(freq);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: failed to set freq %{public}d", __func__, ret);
        return ret;
    }
    return HDF_SUCCESS;
}

int32_t ThermalHostDriverStub::DispatchSetBatteryCurrent(MessageParcel& data, MessageParcel& reply,
    MessageOption& option) const
{
    int32_t ret = -1;
    uint32_t current = data.ReadUint32();
    ret = mitigation_->ChargerRequest(current);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: failed to set freq %{public}d", __func__, ret);
        return ret;
    }
    return HDF_SUCCESS;
}

int32_t ThermalHostDriverStub::DispatchSetFlag(MessageParcel& data, MessageParcel& reply,
    MessageOption& option) const
{
    int32_t ret = -1;
    bool flag = data.ReadBool();
    ret = mitigation_->SetFlag(flag);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: failed to set flag %{public}d", __func__, ret);
        return ret;
    }
    thermalZone_->SetFlag(flag);
    return HDF_SUCCESS;
}

int32_t ThermalHostDriverStub::DispatchSetSensorTemp(MessageParcel& data, MessageParcel& reply,
    MessageOption& option) const
{
    HDF_LOGD("%{public}s: enter", __func__);
    int32_t ret = -1;
    int32_t temp = 0;
    std::string type;

    if (!data.ReadString(type)) {
        HDF_LOGE("%{public}s: failed to read type", __func__);
    }

    if (!data.ReadInt32(temp)) {
        HDF_LOGE("%{public}s: failed to read temp", __func__);
    }

    ret = simulation_->SetTempRequest(type, temp);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: failed to set freq %{public}d", __func__, ret);
        return ret;
    }
    return HDF_SUCCESS;
    HDF_LOGD("%{public}s: return", __func__);
}

void ThermalHostDriverStub::NotifySubscriber(const struct ThermalSubscriber *subscriber, uint32_t cmd)
{
    HDF_LOGD("%{public}s: NotifySubscriber enter", __func__);
    if (subscriber == nullptr) {
        HDF_LOGD("%{public}s: subscriber is NULL", __func__);
        return;
    }
    int ret;
    struct HdfRemoteService *service = subscriber->remoteService;
    struct HdfSBuf *data = HdfSBufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSBufTypedObtain(SBUF_IPC);
    if (data == nullptr || reply == nullptr) {
        HDF_LOGE("%{public}s failed to obtain hdf sbuf", __func__);
        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
        return;
    }

    ReportSensorInfo(data, cmd);

    ret = service->dispatcher->Dispatch(service, cmd, data, reply);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%{public}s failed to notify subscriber, ret: %{public}d", __func__, ret);
    } else {
        HDF_LOGD("%{public}s: succeed to notify subscriber", __func__);
    }
    HdfSBufRecycle(data);
    HdfSBufRecycle(reply);
}

void ThermalHostDriverStub::ReportSensorInfo(struct HdfSBuf *data, uint32_t cmd)
{
    switch (cmd) {
        case CMD_NOTIFY_SUBSCRIBER_V1: {
            HDF_LOGD("%{public}s: NotifySubscriber V1", __func__);
            if (!HdfSbufWriteUint32(data, typeTempMapV1_.size())) {
                HDF_LOGE("%{public}s: size write failed", __func__);
            }
            HDF_LOGD("%{public}s: size: %{public}d", __func__, typeTempMapV1_.size());
            for (auto it : typeTempMapV1_) {
                HDF_LOGD("%{public}s: sensor type: %{public}s", __func__, it.first.c_str());
                HDF_LOGD("%{public}s: sensor temp: %{public}d", __func__, it.second);
                if (!HdfSbufWriteString(data, it.first.c_str())) {
                    HDF_LOGE("%{public}s: sensor type write failed, %{public}s", __func__, it.first.c_str());
                }
                if (!HdfSbufWriteInt32(data, it.second)) {
                    HDF_LOGE("%{public}s: sensor temp write failed, %{public}d", __func__, it.second);
                }
            }
            break;
        }
        case CMD_NOTIFY_SUBSCRIBER_V2: {
            HDF_LOGD("%{public}s: NotifySubscriber V2", __func__);
            if (!HdfSbufWriteUint32(data, typeTempMapV2_.size())) {
                HDF_LOGE("%{public}s: size write failed", __func__);
            }
            HDF_LOGD("%{public}s: size: %{public}d", __func__, typeTempMapV2_.size());
            for (auto it : typeTempMapV2_) {
                HDF_LOGD("%{public}s: sensor type: %{public}s", __func__, it.first.c_str());
                HDF_LOGD("%{public}s: sensor temp: %{public}d", __func__, it.second);
                if (!HdfSbufWriteString(data, it.first.c_str())) {
                    HDF_LOGE("%{public}s: sensor type write failed, %{public}s", __func__, it.first.c_str());
                }
                if (!HdfSbufWriteInt32(data, it.second)) {
                    HDF_LOGE("%{public}s: sensor temp write failed, %{public}d", __func__, it.second);
                }
            }
            break;
        }
        default: {
            break;
        }
    }
}

void ThermalHostDriverStub::UpdatePollingInfo()
{
    int32_t ret = -1;
    if (!thermalZone_->GetFlag()) {
        ret = thermalZone_->ParserSimulationNode();
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%{public}s: failed to parser simulation thermal zone info. ret: %{public}d", __func__, ret);
            return;
        }
    } else {
        ret = thermalZone_->ParseThermalZoneInfo();
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%{public}s: failed to parser really thermal zone info. ret: %{public}d", __func__, ret);
            return;
        }
    }
    UpdateSensorInfo(POLLING_V1, typeTempMapV1_);
    UpdateSensorInfo(POLLING_V2, typeTempMapV2_);
}

void ThermalHostDriverStub::UpdateSensorInfo(const std::string &pollingName,
    std::map<std::string, int32_t> &typeTempMap)
{
    HDF_LOGD("%{public}s: size: %{public}d", __func__, thermalZone_->GetlTzInfo().size());
    auto tzInfo = thermalZone_->GetlTzInfo();
    ThermalHDIConfigParser::ThermalTypeMap sensorTypeMap = xmlParser.GetSensorTypeMap();
    auto pollingIter = sensorTypeMap.find(pollingName);
    if (pollingIter != sensorTypeMap.end()) {
        auto vXTzInfo = pollingIter->second->GetXMLThermalZoneInfo();
        auto vXTnInfo = pollingIter->second->GetXMLThermalNodeInfo();
        for (auto info = tzInfo.begin(); info != tzInfo.end(); info++) {
            HDF_LOGD("%{public}s: type: %{public}s", __func__, info->GetType().c_str());
            HDF_LOGD("%{public}s: temp: %{public}d", __func__, info->GetTemp());
            if (!vXTzInfo.empty()) {
                for (auto iter = vXTzInfo.begin(); iter != vXTzInfo.end(); iter++) {
                    if (iter->isReplace) {
                        if (info->GetType().find(iter->type) != std::string::npos) {
                            typeTempMap[iter->replace] = info->GetTemp();
                        }
                    } else {
                        if (info->GetType().find(iter->type) != std::string::npos) {
                            typeTempMap[iter->type] = info->GetTemp();
                        }
                    }
                }
            }
            if (!vXTnInfo.empty()) {
                for (auto iter = vXTnInfo.begin(); iter != vXTnInfo.end(); iter++) {
                    if (info->GetType().find(iter->type) != std::string::npos) {
                        typeTempMap[iter->type] = info->GetTemp();
                    }
                }
            }
        }
    }
}

void ThermalHostDriverStub::ThermalZoneInfoV1Notify()
{
    UpdatePollingInfo();
    if (subscriber_ == nullptr) {
        HDF_LOGE("%{public}s: check subscriber_ failed", __func__);
    } else {
        HDF_LOGD("%{public}s: Enter", __func__);
        NotifySubscriber(subscriber_, CMD_NOTIFY_SUBSCRIBER_V1);
    }
}

void ThermalHostDriverStub::ThermalZoneInfoV2Notify()
{
    if (subscriber_ == nullptr) {
        HDF_LOGE("%{public}s: check subscriber_ failed", __func__);
    } else {
        HDF_LOGD("%{public}s: Enter", __func__);
        NotifySubscriber(subscriber_, CMD_NOTIFY_SUBSCRIBER_V2);
    }
}

int ThermalHostDriverStub::RegisterCallback(const int fd, const EventType et, int32_t epfd)
{
    struct epoll_event ev;

    ev.events = EPOLLIN;
    if (et == EVENT_TIMER_FD) {
        ev.events |= EPOLLWAKEUP;
    }

    ev.data.ptr = reinterpret_cast<void*>(this);
    ev.data.fd = fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        HDF_LOGE("%{public}s: epoll_ctl failed, error num =%{public}d", __func__, errno);
        return -1;
    }

    return 0;
}

void ThermalHostDriverStub::SetTimerInterval(int interval, int32_t timerfd)
{
    HDF_LOGD("%{public}s enter, start SetTimerInterval", __func__);
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
        HDF_LOGE("%{public}s: timer failed\n", __func__);
    }
}

void ThermalHostDriverStub::TimerCallback(void *service)
{
    HDF_LOGD("%{public}s enter", __func__);
    unsigned long long timers;
    if (read(timerFd_, &timers, sizeof(timers)) == -1) {
        HDF_LOGE("%{public}s read timerFd_ failed", __func__);
        return;
    }
    ThermalZoneInfoV1Notify();
    if (flag) {
        ThermalZoneInfoV2Notify();
        flag = false;
        return;
    }
    flag = true;
    return;
}

void ThermalHostDriverStub::TimerSecCallback(void *service)
{
    HDF_LOGD("%{public}s enter", __func__);
    unsigned long long timers;
    if (read(timerFdSec_, &timers, sizeof(timers)) == -1) {
        HDF_LOGE("%{public}s read timerFdSec_ failed", __func__);
        return;
    }
    ThermalZoneInfoV2Notify();
    return;
}

int ThermalHostDriverStub::LoopingThreadEntry(void *arg, int32_t epfd)
{
    int nevents = 0;
    size_t eventct = callbackHandler_.size();
    struct epoll_event events[eventct];

    HDF_LOGD("%{public}s enter, start batteryd looping", __func__);
    while (true) {
        nevents = epoll_wait(epfd, events, eventct, -1);
        if (nevents == -1) {
            continue;
        }

        for (int n = 0; n < nevents; ++n) {
            if (events[n].data.ptr) {
                ThermalHostDriverStub *func = const_cast<ThermalHostDriverStub *>(this);
                (callbackHandler_.find(events[n].data.fd)->second)(func, arg);
            }
        }
    }
}

void ThermalHostDriverStub::Run(void *service, int32_t epfd)
{
    std::make_unique<std::thread>(&ThermalHostDriverStub::LoopingThreadEntry, this, service, epfd)->detach();
}

int32_t ThermalHostDriverStub::InitTimer()
{
    HDF_LOGD("%{public}s: Enter", __func__);
    int32_t ret = -1;
    epFd_ = epoll_create1(EPOLL_CLOEXEC);

    ret = CreateV1Fd();
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%{public}s failed to create polling v1 fd", __func__);
    }
    return HDF_SUCCESS;
}

int32_t ThermalHostDriverStub::InitTimerV2()
{
    HDF_LOGD("%{public}s:  Enter", __func__);
    int32_t ret = -1;
    epFdV2_ = epoll_create1(EPOLL_CLOEXEC);

    ret = CreateV2Fd();
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%{public}s failed to create polling v1 fd", __func__);
    }
    return HDF_SUCCESS;
}

int32_t ThermalHostDriverStub::CreateV1Fd()
{
    timerFd_ = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (timerFd_ == ERR_INVALID_FD) {
        HDF_LOGE("%{public}s epoll create failed, epFd_ is invalid", __func__);
        return HDF_ERR_BAD_FD;
    }

    auto intervalMap = xmlParser.GetIntervalMap();
    auto v1 = intervalMap.find(POLLING_V1);
    if (v1 != intervalMap.end()) {
        int thermalPollingV1 = v1->second;
        HDF_LOGD("%{public}s: %{public}d", __func__, thermalPollingV1);
        SetTimerInterval(thermalPollingV1, timerFd_);
        fcntl(timerFd_, F_SETFL, O_NONBLOCK);
        callbackHandler_.insert(std::make_pair(timerFd_, &ThermalHostDriverStub::TimerCallback));
        if (RegisterCallback(timerFd_, EVENT_TIMER_FD, epFd_)) {
            HDF_LOGE("%{public}s register Timer event failed", __func__);
        }
    }
    return HDF_SUCCESS;
}

int32_t ThermalHostDriverStub::CreateV2Fd()
{
    timerFdSec_ = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (timerFdSec_ == ERR_INVALID_FD) {
        HDF_LOGE("%{public}s epoll create failed, epFdV2_ is invalid", __func__);
        return HDF_ERR_BAD_FD;
    }
    auto intervalMap = xmlParser.GetIntervalMap();
    auto v2 = intervalMap.find(POLLING_V1);
    if (v2 != intervalMap.end()) {
        int thermalPollingV2 = v2->second;
        HDF_LOGD("%{public}s: %{public}d", __func__, thermalPollingV2);
        SetTimerInterval(thermalPollingV2, timerFdSec_);
        fcntl(timerFdSec_, F_SETFL, O_NONBLOCK);
        callbackHandler_.insert(std::make_pair(timerFdSec_, &ThermalHostDriverStub::TimerSecCallback));
        if (RegisterCallback(timerFdSec_, EVENT_TIMER_FD, epFdV2_)) {
            HDF_LOGE("%{public}s register Timer event failed", __func__);
        }
    }
    return HDF_SUCCESS;
}

void ThermalHostDriverStub::StartThread(void *service)
{
    int ret;
    HDF_LOGD("%{public}s:  Enter", __func__);
    ret = InitTimer();
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%{public}s init Timer failed, ret: %{public}d", __func__, ret);
        return;
    }
    Run(service, epFd_);
    return;
}
} // namespace V1_0
} // namespace THERMAL
} // namespace HDI
} // namespace OHOS

void *ThermalHostDriverStubInstance()
{
    using namespace OHOS::HDI::THERMAL::V1_0;
    ThermalHostDriverStub *stub = new ThermalHostDriverStub();
    if (stub->Init() != HDF_SUCCESS) {
        delete stub;
        stub = nullptr;
        return nullptr;
    }
    return reinterpret_cast<void *>(stub);
}

void DestoryThermalHostServiceStub(void *obj)
{
    using namespace OHOS::HDI::THERMAL::V1_0;
    delete reinterpret_cast<ThermalHostDriverStub *>(obj);
}

int32_t ThermalHostServiceOnRemoteRequest(void *stub, int cmdId, struct HdfSBuf& data, struct HdfSBuf& reply)
{
    using namespace OHOS::HDI::THERMAL::V1_0;
    ThermalHostDriverStub *serviceStub = reinterpret_cast<ThermalHostDriverStub *>(stub);
    struct ThermalDriverService *service = CONTAINER_OF(serviceStub, ThermalDriverService, instance);

    if (cmdId == CMD_BIND_THERMAL_SUBSCRIBER) {
        return serviceStub->DispatchBindThermalSuscriber(*service, data);
    } else if (cmdId == CMD_UNBIND_THERMAL_SUBSCRIBER) {
        return serviceStub->DispatchUnBindThermalSubscriber(*service);
    }

    OHOS::MessageParcel *dataParcel = nullptr;
    OHOS::MessageParcel *replyParcel = nullptr;
    OHOS::MessageOption option;

    if (SbufToParcel(&reply, &replyParcel) != HDF_SUCCESS) {
        HDF_LOGE("%{public}s:invalid reply sbuf object to dispatch", __func__);
        return HDF_ERR_INVALID_PARAM;
    } else if (SbufToParcel(&data, &dataParcel) != HDF_SUCCESS) {
        HDF_LOGE("%{public}s:invalid data sbuf object to dispatch", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    switch (cmdId) {
        case CMD_GET_THERMALZONEINFO: {
            return serviceStub->DispatchGetThermalZoneInfo(*dataParcel, *replyParcel, option);
        }
        case CMD_SET_CPU_FREQ: {
            return serviceStub->DispatchSetCPUFreq(*dataParcel, *replyParcel, option);
        }
        case CMD_SET_GPU_FREQ: {
            return serviceStub->DispatchSetGPUFreq(*dataParcel, *replyParcel, option);
        }
        case CMD_SET_BATTERY_CURRENT: {
            return serviceStub->DispatchSetBatteryCurrent(*dataParcel, *replyParcel, option);
        }
        case CMD_SET_FLAG_CONTROL: {
            return serviceStub->DispatchSetFlag(*dataParcel, *replyParcel, option);
        }
        case CMD_SET_SENSOR_TEMP: {
            return serviceStub->DispatchSetSensorTemp(*dataParcel, *replyParcel, option);
        }
        default: {
            HDF_LOGE("%s: not support cmd %d", __func__, cmdId);
            return HDF_FAILURE;
        }
    }
}