/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef CJ_THERMAL_MANAGER_H
#define CJ_THERMAL_MANAGER_H

#include <memory>
#include <mutex>
#include "ithermal_level_callback.h"
#include "thermal_level_callback_stub.h"
#include "thermal_mgr_client.h"

namespace OHOS {
namespace PowerMgr {
class CjThermalLevelCallback : public ThermalLevelCallbackStub {
public:
    CjThermalLevelCallback() = default;
    virtual ~CjThermalLevelCallback();
    void UpdateCallback(int64_t id);
    void ReleaseCallback();
    bool OnThermalLevelChanged(ThermalLevel level) override;

private:
    std::function<void(int32_t)> callbackFunc_ {nullptr};
};

class CjThermalManager {
public:
    static CjThermalManager& GetThreadLocalInstance();
    void SubscribeThermalLevel(int64_t id);
    void UnSubscribeThermalLevel(int64_t id);
    int32_t GetThermalLevel();

private:
    CjThermalManager() = default;
    virtual ~CjThermalManager() = default;
    ThermalMgrClient& thermalMgrClient_ = ThermalMgrClient::GetInstance();
    sptr<CjThermalLevelCallback> thermalLevelCallback_ = new CjThermalLevelCallback();
};
} // PowerMgr
} // OHOS
#endif
