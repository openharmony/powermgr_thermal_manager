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

#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "ithermal_action_callback.h"
#include "ithermal_level_callback.h"
#include "ithermal_srv.h"
#include "nocopyable.h"
#include "refbase.h"
#include "thermal_level_info.h"
#include "thermal_srv_sensor_info.h"
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <vector>

namespace OHOS {
namespace PowerMgr {
class ThermalSrvProxy : public IRemoteProxy<IThermalSrv> {
public:
    explicit ThermalSrvProxy(const sptr<IRemoteObject>& impl) : IRemoteProxy<IThermalSrv>(impl) {}
    ~ThermalSrvProxy() = default;
    DISALLOW_COPY_AND_MOVE(ThermalSrvProxy);

    virtual bool SubscribeThermalTempCallback(
        const std::vector<std::string>& typeList, const sptr<IThermalTempCallback>& callback) override;
    virtual bool UnSubscribeThermalTempCallback(const sptr<IThermalTempCallback>& callback) override;
    virtual bool SubscribeThermalLevelCallback(const sptr<IThermalLevelCallback>& callback) override;
    virtual bool UnSubscribeThermalLevelCallback(const sptr<IThermalLevelCallback>& callback) override;
    virtual bool SubscribeThermalActionCallback(const std::vector<std::string>& actionList, const std::string& desc,
        const sptr<IThermalActionCallback>& callback) override;
    virtual bool UnSubscribeThermalActionCallback(const sptr<IThermalActionCallback>& callback) override;
    virtual bool GetThermalSrvSensorInfo(const SensorType& type, ThermalSrvSensorInfo& sensorInfo) override;
    virtual bool GetThermalLevel(ThermalLevel& level) override;
    virtual bool GetThermalInfo() override;
    virtual bool SetScene(const std::string& scene) override;
    virtual std::string ShellDump(const std::vector<std::string>& args, uint32_t argc) override;

private:
    static inline BrokerDelegator<ThermalSrvProxy> delegator_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_SRV_PROXY_H
