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

#ifndef THERMAL_SRV_PROXY_H
#define THERMAL_SRV_PROXY_H

#include "ithermal_srv.h"
#include "nocopyable.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace PowerMgr {
class ThermalSrvProxy : public IRemoteProxy<IThermalSrv> {
public:
    explicit ThermalSrvProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<IThermalSrv>(impl) {}
    ~ThermalSrvProxy() = default;
    DISALLOW_COPY_AND_MOVE(ThermalSrvProxy);

    virtual void SubscribeThermalTempCallback(const std::vector<std::string> &typeList,
        const sptr<IThermalTempCallback> &callback) override;
    virtual void UnSubscribeThermalTempCallback(const sptr<IThermalTempCallback> &callback) override;
    virtual void SubscribeThermalLevelCallback(const sptr<IThermalLevelCallback>& callback) override;
    virtual void UnSubscribeThermalLevelCallback(const sptr<IThermalLevelCallback>& callback) override;
    virtual bool GetThermalSrvSensorInfo(const SensorType &type, ThermalSrvSensorInfo& sensorInfo) override;
    virtual void GetThermalLevel(ThermalLevel& level) override;
private:
    static inline BrokerDelegator<ThermalSrvProxy> delegator_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_SRV_PROXY_H