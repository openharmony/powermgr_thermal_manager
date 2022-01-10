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

#include "thermal_subscriber.h"

#include "iservmgr_hdi.h"
#include "thermal_common.h"

namespace OHOS {
namespace PowerMgr {
void ThermalSubscriber::RegisterTempChangedCallback(Callback &callback)
{
    callback_ = callback;
}

int32_t ThermalSubscriber::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    switch (code) {
        case CMD_NOTIFY_SUBSCRIBER_V1:
        case CMD_NOTIFY_SUBSCRIBER_V2: {
            ParserThermalSensorInfo(data, reply, option);
            break;
        }
        default: {
            THERMAL_HILOGE(MODULE_THERMALHDI_CLIENT, "no specific cmd for code: %{public}d", code);
            return ERR_INVALID_OPERATION;
        }
    }
    THERMAL_HILOGD(MODULE_THERMALHDI_CLIENT, "OnRemoteRequest: OutSize: %{public}d", typeTempMap_.size());
    callback_(typeTempMap_);
    typeTempMap_.clear();
    return ERR_OK;
}

void ThermalSubscriber::ParserThermalSensorInfo(MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    uint32_t outSize = data.ReadUint32();
    for (uint32_t i = 0; i < outSize; i++) {
        std::string curTypeData = data.ReadCString();
        int32_t curTempData = data.ReadInt32();
        THERMAL_HILOGD(MODULE_THERMALHDI_CLIENT, "OutSize=%{public}d, curTypeData=%{public}s, curTempData=%{public}d",
            outSize, curTypeData.c_str(), curTempData);
        typeTempMap_[curTypeData] = curTempData;
    }
}
} // namespace PowerMgr
} // namespace OHOS