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

#ifndef MOCK_THERMAL_SERVICE_CLIENT_H
#define MOCK_THERMAL_SERVICE_CLIENT_H

#include <string>
#include <singleton.h>
#include "ithermal_srv.h"
#include "thermal_srv_sensor_info.h"

namespace OHOS {
namespace PowerMgr {
class MockThermalMgrClient final : public DelayedRefSingleton<MockThermalMgrClient> {
    DECLARE_DELAYED_REF_SINGLETON(MockThermalMgrClient)

public:
    DISALLOW_COPY_AND_MOVE(MockThermalMgrClient);

    bool GetThermalInfo();
private:
    class ThermalMgrDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        ThermalMgrDeathRecipient() = default;
        ~ThermalMgrDeathRecipient() = default;
        void OnRemoteDied(const wptr<IRemoteObject>& remote);
    private:
        DISALLOW_COPY_AND_MOVE(ThermalMgrDeathRecipient);
    };

private:
    ErrCode Connect();
private:
    sptr<IThermalSrv> thermalSrv_ {nullptr};
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ {nullptr};
    void ResetProxy(const wptr<IRemoteObject>& remote);
    std::mutex mutex_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // MOCK_THERMAL_SERVICE_CLIENT_H
