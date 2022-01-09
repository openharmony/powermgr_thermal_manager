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

#ifndef THERMAL_HOST_DRIVER_STUB_H
#define THERMAL_HOST_DRIVER_STUB_H

#include  <functional>
#include <map>
#include <string>
#include <mutex>
#include <thread>
#include <memory>
#include <future>
#include <functional>
#include <list>
#include <message_parcel.h>
#include <message_option.h>
#include <refbase.h>
#include "core/hdf_device_desc.h"
#include "hdf_remote_service.h"

#include "thermal_simulation_node.h"
#include "thermal_zone_parser.h"
#include "device_request_mitigation.h"
#include "thermal_hdi_config_parser.h"

namespace OHOS {
namespace HDI {
namespace THERMAL {
namespace V1_0 {
struct ThermalSubscriber {
    struct HdfRemoteService *remoteService;
};

enum EventType {
    EVENT_UEVENT_FD,
    EVENT_TIMER_FD,
};

class ThermalHostDriverStub {
public:
    typedef std::function<void(ThermalHostDriverStub*, void*)> Callback;
    ThermalHostDriverStub() {};
    virtual ~ThermalHostDriverStub() {};
    static ThermalHostDriverStub &GetInstance()
    {
        static ThermalHostDriverStub instance;
        return instance;
    }
    enum {
        NOTIFY_V1 = 0,
        NOTIFY_V2 = 1,
    };

    int32_t Init();
    int32_t DispatchBindThermalSuscriber(struct ThermalDriverService &service, struct HdfSBuf &);
    int32_t DispatchUnBindThermalSubscriber(struct ThermalDriverService& service) const;
    int32_t DispatchGetThermalZoneInfo(MessageParcel& data, MessageParcel& reply, MessageOption& option) const;
    int32_t DispatchSetCPUFreq(MessageParcel& data, MessageParcel& reply, MessageOption& option) const;
    int32_t DispatchSetGPUFreq(MessageParcel& data, MessageParcel& reply, MessageOption& option) const;
    int32_t DispatchSetBatteryCurrent(MessageParcel& data, MessageParcel& reply,
        MessageOption& option) const;
    int32_t DispatchSetFlag(MessageParcel& data, MessageParcel& reply, MessageOption& option) const;
    int32_t DispatchSetSensorTemp(MessageParcel& data, MessageParcel& reply, MessageOption& option) const;

    void NotifySubscriber(const struct ThermalSubscriber *subscriber, uint32_t cmd);
    void ReportSensorInfo(struct HdfSBuf *data, uint32_t cmd);
    /* Timer related function */
    void ThermalZoneInfoV1Notify();
    void ThermalZoneInfoV2Notify();
    int LoopingThreadEntry(void *arg, int32_t epfd);
    virtual void Run(void *service, int32_t epfd);
    void StartThread(void *service);
    int RegisterCallback(const int fd, const EventType et, int32_t epfd);
    void SetTimerInterval(int interval, int32_t timerfd);
    void UpdateSensorInfo(const std::string &pollingName, std::map<std::string, int32_t> &typeTempMap);
    void UpdatePollingInfo();

private:
    void TimerCallback(void *service);
    void TimerSecCallback(void *service);
    int32_t CreateV1Fd();
    int32_t CreateV2Fd();
    int32_t InitTimer();
    int32_t InitTimerV2();

    std::mutex mutex_;
    int32_t timerFd_ {-1};
    int32_t timerFdSec_ {-1};
    int timerInterval_ {-1};
    int32_t epFd_ {-1};
    int32_t epFdV2_ {-1};
    Callback callback_;
    std::map<int32_t, Callback> callbackHandler_;
    struct ThermalSubscriber* subscriber_ {nullptr};
    std::unique_ptr<ThermalZoneParser> thermalZone_ {nullptr};
    std::unique_ptr<DeviceRequestMitigation> mitigation_ {nullptr};
    std::unique_ptr<ThermalSimulationNode> simulation_ {nullptr};
    std::map<std::string, int32_t> typeTempMapV1_;
    std::map<std::string, int32_t> typeTempMapV2_;
};
} // V1_0
} // THERMAL
} // HDI
} // OHOS

void *ThermalHostDriverStubInstance();
void DestoryThermalHostServiceStub(void *obj);
int32_t ThermalHostServiceOnRemoteRequest(void *stub, int cmdId, struct HdfSBuf &data, struct HdfSBuf &reply);

#endif // THERMAL_HOST_DRIVER_STUB_H