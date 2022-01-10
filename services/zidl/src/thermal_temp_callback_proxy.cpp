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

#include "thermal_temp_callback_proxy.h"
#include <ipc_types.h>
#include <message_parcel.h>
#include "thermal_common.h"

namespace OHOS {
namespace PowerMgr {
void ThermalTempCallbackProxy::OnThermalTempChanged(TempCallbackMap &tempCbMap)
{
    sptr<IRemoteObject> remote = Remote();
    THERMAL_RETURN_IF(remote == nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(ThermalTempCallbackProxy::GetDescriptor())) {
        THERMAL_HILOGE(MODULE_THERMAL_INNERKIT, "ThermalTempCallbackProxy::%{public}s write descriptor failed!",
            __func__);
        return;
    }
    THERMAL_WRITE_PARCEL_NO_RET(data, Uint32, tempCbMap.size());
    for (auto iter : tempCbMap) {
        THERMAL_WRITE_PARCEL_NO_RET(data, String, iter.first);
        THERMAL_WRITE_PARCEL_NO_RET(data, Int32, iter.second);
    }

    int ret = remote->SendRequest(static_cast<int>(IThermalTempCallback::THERMAL_TEMPERATURE_CHANGD),
        data, reply, option);
    if (ret != ERR_OK) {
        THERMAL_HILOGE(MODULE_THERMAL_INNERKIT,
            "ThermalTempCallbackProxy::%{public}s SendRequest is failed, error code: %{public}d",
            __func__, ret);
    }
}
} // namespace PowerMgr
} // namespace OHOS