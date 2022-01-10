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

#ifndef THERMAL_SUBSCRIBER_H
#define THERMAL_SUBSCRIBER_H

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "ipc_object_stub.h"
#include "thermal_api.h"

namespace OHOS {
namespace PowerMgr {
class ThermalSubscriber : public OHOS::IPCObjectStub {
typedef std::function<void(std::map<std::string, int32_t>)> Callback;
public:
    explicit ThermalSubscriber() : IPCObjectStub(u"ohos.powermgr.IThermalSubscriber")
    {
    };
    virtual ~ThermalSubscriber() = default;

    enum {
        NOTIFY_V1 = 0,
        NOTIFY_V2 = 1,
    };

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    Callback GetCallback()
    {
        return callback_;
    }
    void RegisterTempChangedCallback(Callback& callback);

private:
    void ParserThermalSensorInfo(MessageParcel &data, MessageParcel &reply, MessageOption &option);

    Callback callback_;
    std::map<std::string, int32_t> typeTempMap_;
};
} // namespace PowerMgr
} // namesapce OHOS

#endif // THERMAL_SUBSCRIBER_H