/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef THERMAL_MANAGER_NAPI_H
#define THERMAL_MANAGER_NAPI_H

#include <mutex>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "ithermal_level_callback.h"
#include "thermal_level_callback_stub.h"
#include "thermal_mgr_client.h"

namespace OHOS {
namespace PowerMgr {
class ThermalLevelCallback : public ThermalLevelCallbackStub {
public:
    ThermalLevelCallback() = default;
    virtual ~ThermalLevelCallback();
    void UpdateCallback(napi_env env, napi_value jsCallback);
    void ReleaseCallback();
    bool GetThermalLevel(ThermalLevel level) override;
    void OnThermalLevel();

private:
    ThermalLevel level_ {ThermalLevel::COOL};
    napi_ref callbackRef_ {nullptr};
    napi_env env_ {nullptr};
    std::mutex mutex_;
};

class ThermalManagerNapi {
public:
    ThermalManagerNapi() = default;
    virtual ~ThermalManagerNapi() = default;

    static napi_value Init(napi_env env, napi_value exports);
    static napi_value InitThermalLevel(napi_env env, napi_value exports);
    static napi_value SubscribeThermalLevel(napi_env env, napi_callback_info info);
    static napi_value UnSubscribeThermalLevel(napi_env env, napi_callback_info info);
    static napi_value GetThermalLevel(napi_env env, napi_callback_info info);
    static napi_value EnumThermalLevelConstructor(napi_env env, napi_callback_info info);
};
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_MANAGER_NAPI_H
