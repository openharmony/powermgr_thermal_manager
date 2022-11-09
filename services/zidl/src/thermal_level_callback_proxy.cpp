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

#include "thermal_level_callback_proxy.h"
#include "thermal_common.h"
#include <ipc_types.h>
#include <message_parcel.h>

namespace OHOS {
namespace PowerMgr {
bool ThermalLevelCallbackProxy::GetThermalLevel(ThermalLevel level)
{
    sptr<IRemoteObject> remote = Remote();
    THERMAL_RETURN_IF_WITH_RET((remote == nullptr), false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(ThermalLevelCallbackProxy::GetDescriptor())) {
        THERMAL_HILOGE(COMP_FWK, "ThermalLevelCallbackProxy::GetThermalLevel write descriptor failed!");
        return false;
    }

    THERMAL_WRITE_PARCEL_WITH_RET(data, Int32, static_cast<int32_t>(level), false);

    int ret = remote->SendRequest(static_cast<int>(IThermalLevelCallback::GET_THERMAL_LEVEL), data, reply, option);
    if (ret != ERR_OK) {
        THERMAL_HILOGE(COMP_FWK, "SendRequest is failed, error code: %{public}d", ret);
        return false;
    }
    return true;
}
} // namespace PowerMgr
} // namespace OHOS
