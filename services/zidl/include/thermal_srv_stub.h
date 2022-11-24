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

#ifndef THERMAL_MGR_SRV_STUB_H
#define THERMAL_MGR_SRV_STUB_H

#include "ipc_object_stub.h"
#include "ithermal_srv.h"
#include <iremote_stub.h>
#include <nocopyable.h>

namespace OHOS {
namespace PowerMgr {
class ThermalSrvStub : public IRemoteStub<IThermalSrv> {
public:
    DISALLOW_COPY_AND_MOVE(ThermalSrvStub);
    ThermalSrvStub() = default;
    virtual ~ThermalSrvStub() = default;
    int OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;

private:
    int32_t SubscribeThermalTempCallbackStub(MessageParcel& data);
    int32_t UnSubscribeThermalTempCallbackStub(MessageParcel& data);
    int32_t SubscribeThermalLevelCallbackStub(MessageParcel& data);
    int32_t UnSubscribeThermalLevelCallbackStub(MessageParcel& data);
    int32_t SubscribeThermalActionCallbackStub(MessageParcel& data);
    int32_t UnSubscribeThermalActionCallbackStub(MessageParcel& data);
    int32_t GetThermalSrvSensorInfoStub(MessageParcel& data, MessageParcel& reply);
    int32_t GetThermalevelStub(MessageParcel& reply);
    int32_t GetThermalInfoStub(MessageParcel& reply);
    int32_t SetSceneStub(MessageParcel& data);
    int32_t ShellDumpStub(MessageParcel& data, MessageParcel& reply);
    int32_t CheckRequestCode(const uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option);
};
} // namespace PowerMgr
} // namespace OHOS
#endif //  THERMAL_MGR_SRV_STUB_H
