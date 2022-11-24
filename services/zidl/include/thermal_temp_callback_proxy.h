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

#ifndef THERMAL_TEMP_CALLBACK_PROXY_H
#define THERMAL_TEMP_CALLBACK_PROXY_H

#include "iremote_broker.h"
#include "iremote_object.h"
#include "ithermal_temp_callback.h"
#include "refbase.h"
#include <functional>
#include <iremote_proxy.h>
#include <nocopyable.h>

namespace OHOS {
namespace PowerMgr {
class ThermalTempCallbackProxy : public IRemoteProxy<IThermalTempCallback> {
public:
    explicit ThermalTempCallbackProxy(const sptr<IRemoteObject>& impl) : IRemoteProxy<IThermalTempCallback>(impl) {}
    ~ThermalTempCallbackProxy() = default;
    DISALLOW_COPY_AND_MOVE(ThermalTempCallbackProxy);
    virtual bool OnThermalTempChanged(TempCallbackMap& tempCbMap) override;

private:
    static inline BrokerDelegator<ThermalTempCallbackProxy> delegator_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_TEMP_CALLBACK_PROXY_H
