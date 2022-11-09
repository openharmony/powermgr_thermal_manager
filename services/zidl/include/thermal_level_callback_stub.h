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

#ifndef THERMAL_LEVEL_CALLBACK_STUB_H
#define THERMAL_LEVEL_CALLBACK_STUB_H

#include "ithermal_level_callback.h"
#include "message_option.h"
#include "refbase.h"
#include "thermal_level_info.h"
#include <cstdint>
#include <iremote_stub.h>
#include <nocopyable.h>

namespace OHOS {
namespace PowerMgr {
class ThermalLevelCallbackStub : public IRemoteStub<IThermalLevelCallback> {
public:
    DISALLOW_COPY_AND_MOVE(ThermalLevelCallbackStub);
    ThermalLevelCallbackStub() = default;
    virtual ~ThermalLevelCallbackStub() = default;
    int OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    bool GetThermalLevel(ThermalLevel __attribute__((unused)) level) override
    {
        return true;
    }

private:
    int32_t GetThermalLevelStub(MessageParcel& data);
};
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_TEMP_CALLBACK_STUB_H
