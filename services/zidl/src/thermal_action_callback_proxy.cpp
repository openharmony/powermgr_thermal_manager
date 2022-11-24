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

#include "thermal_action_callback_proxy.h"

#include "errors.h"
#include "message_option.h"
#include "thermal_common.h"
#include "thermal_log.h"
#include <message_parcel.h>

namespace OHOS {
namespace PowerMgr {
bool ThermalActionCallbackProxy::OnThermalActionChanged(ActionCallbackMap& actionCbMap)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    sptr<IRemoteObject> remote = Remote();
    THERMAL_RETURN_IF_WITH_RET((remote == nullptr), false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(ThermalActionCallbackProxy::GetDescriptor())) {
        THERMAL_HILOGE(COMP_FWK, "write descriptor failed!");
        return false;
    }

    THERMAL_WRITE_PARCEL_WITH_RET(data, Uint32, actionCbMap.size(), false);
    for (auto iter : actionCbMap) {
        THERMAL_HILOGD(COMP_SVC, "proxy type=%{public}s", iter.first.c_str());
        THERMAL_HILOGD(COMP_SVC, "proxy temp=%{public}f", iter.second);
        THERMAL_WRITE_PARCEL_WITH_RET(data, String, iter.first, false);
        THERMAL_WRITE_PARCEL_WITH_RET(data, Float, iter.second, false);
    }

    int ret = remote->SendRequest(static_cast<int>(IThermalActionCallback::THERMAL_ACTION_CHANGD), data, reply, option);
    if (ret != ERR_OK) {
        THERMAL_HILOGE(COMP_FWK, "SendRequest is failed, error code: %{public}d", ret);
        return false;
    }
    return true;
}
} // namespace PowerMgr
} // namespace OHOS
