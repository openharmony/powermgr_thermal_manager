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

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "thermal_mgr_client.h"
#include "thermal_level_callback_stub.h"
#include "ithermal_level_callback.h"
#include "thermal_manager_native_event.h"

namespace OHOS {
namespace PowerMgr {
class ThermalLevelCallback : public ThermalLevelCallbackStub {
public:
    explicit ThermalLevelCallback() : level_(ThermalLevel::INVALID_LEVEL) {};
    virtual ~ThermalLevelCallback() {};
    void GetThermalLevel(ThermalLevel level) override;
    uint32_t GetLevel()
    {
        return static_cast<uint32_t>(level_);
    }

private:
    ThermalLevel level_;
};

class ThermalManagerNapi : public ThermalManagerNativeEvent {
public:
    explicit ThermalManagerNapi(napi_env env, napi_value thisVar);
    virtual ~ThermalManagerNapi();

    static napi_value Init(napi_env env, napi_value exports);
    static napi_value InitThermalLevel(napi_env env, napi_value exports);
    static napi_value SubscribeThermalLevel(napi_env env, napi_callback_info info);
    static napi_value UnSubscribeThermalLevel(napi_env env, napi_callback_info info);
    static napi_value GetThermalLevel(napi_env env, napi_callback_info info);

    static void RegisterCallback(const std::string &eventType);
    static napi_value EnumThermalLevelConstructor(napi_env env, napi_callback_info info);
    void OnThermalLevelSucceed(uint32_t level);
    static ThermalManagerNapi *GetThermalManagerNapi();
    static sptr<IThermalLevelCallback> callback_;
    static napi_ref thermalLevelConstructor_;

private:
    napi_ref callbackRef_;
    napi_env env_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_MANAGER_NAPI_H