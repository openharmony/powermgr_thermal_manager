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

#ifndef THERMAL_LEVEL_CALLBACK_PROXY_H
#define THERMAL_LEVEL_CALLBACK_PROXY_H

#include "ithermal_level_callback.h"
#include <iremote_proxy.h>
#include <nocopyable.h>

namespace OHOS {
namespace PowerMgr {
class ThermalLevelCallbackProxy : public IRemoteProxy<IThermalLevelCallback> {
public:
    explicit ThermalLevelCallbackProxy(const sptr<IRemoteObject>& impl) : IRemoteProxy<IThermalLevelCallback>(impl) {}
    ~ThermalLevelCallbackProxy() = default;
    DISALLOW_COPY_AND_MOVE(ThermalLevelCallbackProxy);
    virtual bool OnThermalLevelChanged(ThermalLevel level) override;

private:
    static inline BrokerDelegator<ThermalLevelCallbackProxy> delegator_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_LEVEL_CALLBACK_PROXY_H
