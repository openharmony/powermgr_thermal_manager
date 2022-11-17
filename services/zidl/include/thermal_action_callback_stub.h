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

#ifndef THERMAL_ACTION_CALLBACK_STUB_H
#define THERMAL_ACTION_CALLBACK_STUB_H
#include "ithermal_action_callback.h"
#include <iremote_stub.h>
#include <nocopyable.h>

namespace OHOS {
namespace PowerMgr {
class ThermalActionCallbackStub : public IRemoteStub<IThermalActionCallback> {
public:
    DISALLOW_COPY_AND_MOVE(ThermalActionCallbackStub);
    ThermalActionCallbackStub() = default;
    virtual ~ThermalActionCallbackStub() = default;
    int OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    bool OnThermalActionChanged(ActionCallbackMap& actionCbMap) override
    {
        return true;
    }

private:
    int32_t OnThermalActionChangedStub(MessageParcel& data);
};
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_ACTION_CALLBACK_STUB_H
