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

#ifndef THERMAL_ACTION_CALLBACK_PROXY_H
#define THERMAL_ACTION_CALLBACK_PROXY_H

#include "iremote_broker.h"
#include "iremote_object.h"
#include "ithermal_action_callback.h"
#include "refbase.h"
#include <functional>
#include <iremote_proxy.h>
#include <nocopyable.h>

namespace OHOS {
namespace PowerMgr {
class ThermalActionCallbackProxy : public IRemoteProxy<IThermalActionCallback> {
public:
    explicit ThermalActionCallbackProxy(const sptr<IRemoteObject>& impl) : IRemoteProxy<IThermalActionCallback>(impl) {}
    ~ThermalActionCallbackProxy() = default;
    DISALLOW_COPY_AND_MOVE(ThermalActionCallbackProxy);
    virtual bool OnThermalActionChanged(ActionCallbackMap& actionCbMap) override;

private:
    static inline BrokerDelegator<ThermalActionCallbackProxy> delegator_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_ACTION_CALLBACK_PROXY_H
