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

#include "thermal_manager_napi.h"
#include <uv.h>
#include "thermal_common.h"
#include "thermal_level_info.h"

using namespace OHOS::PowerMgr;
using namespace OHOS;

namespace {
const uint8_t ARG_0 = 0;
const uint8_t ARG_1 = 1;
thread_local auto &g_thermalMgrClient = ThermalMgrClient::GetInstance();
thread_local sptr<ThermalLevelCallback> g_thermalLevelCallback = new (std::nothrow) ThermalLevelCallback();
}

napi_ref ThermalManagerNapi::thermalLevelConstructor_ = nullptr;

ThermalLevelCallback::~ThermalLevelCallback()
{
    ReleaseCallback();
}

void ThermalLevelCallback::UpdateCallback(napi_env env, napi_value jsCallback)
{
    std::lock_guard lock(mutex_);
    if (napi_ok != napi_create_reference(env, jsCallback, 1, &callbackRef_)) {
        THERMAL_HILOGW(COMP_FWK, "Failed to create a JS callback reference");
        callbackRef_ = nullptr;
    }
    env_ = env;
}

void ThermalLevelCallback::ReleaseCallback()
{
    std::lock_guard lock(mutex_);
    if (callbackRef_ != nullptr) {
        napi_delete_reference(env_, callbackRef_);
    }
    callbackRef_ = nullptr;
    env_ = nullptr;
}

void ThermalLevelCallback::GetThermalLevel(ThermalLevel level)
{
    std::lock_guard lock(mutex_);
    level_ = level;
    THERMAL_RETURN_IF_WITH_LOG(env_ == nullptr, "env is nullptr");
    uv_loop_s* loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    THERMAL_RETURN_IF_WITH_LOG(loop == nullptr, "napi_get_uv_event_loop loop is null");
    uv_work_t* work = new (std::nothrow) uv_work_t;
    THERMAL_RETURN_IF_WITH_LOG(work == nullptr, "uv_work_t work is null");
    work->data = reinterpret_cast<void *>(this);

    int32_t ret = uv_queue_work(loop, work, [] (uv_work_t *work) {}, [] (uv_work_t *work, int status) {
        ThermalLevelCallback* callback = reinterpret_cast<ThermalLevelCallback*>(work->data);
        if (callback != nullptr) {
            callback->OnThermalLevel();
        }
        delete work;
        work = nullptr;
    });
    if (ret != ERR_OK) {
        delete work;
        work = nullptr;
        THERMAL_HILOGW(COMP_FWK, "uv_queue_work is failed");
    }
}

void ThermalLevelCallback::OnThermalLevel()
{
    THERMAL_HILOGD(COMP_FWK, "level is: %{public}d", static_cast<int32_t>(level_));
    THERMAL_RETURN_IF_WITH_LOG(callbackRef_ == nullptr || env_ == nullptr, "js callback ref or env is nullptr");

    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env_, &scope);
    if (scope == nullptr) {
        THERMAL_HILOGW(COMP_FWK, "scope is nullptr");
        return;
    }

    napi_value levelValue = nullptr;
    if (napi_ok != napi_create_int32(env_, static_cast<int32_t>(level_), &levelValue)) {
        THERMAL_HILOGW(COMP_FWK, "napi_create_int32 callback failed");
        return;
    }

    napi_value callback = nullptr;
    napi_status status = napi_get_reference_value(env_, callbackRef_, &callback);
    if (status != napi_ok) {
        THERMAL_HILOGE(COMP_FWK, "napi_get_reference_value callback failed, status = %{public}d", status);
        return;
    }

    napi_value callResult = nullptr;
    status = napi_call_function(env_, nullptr, callback, ARG_1, &levelValue, &callResult);
    if (status != napi_ok) {
        THERMAL_HILOGE(COMP_FWK, "napi_call_function callback failed, status = %{public}d", status);
    }
    napi_close_handle_scope(env_, scope);
}

napi_value ThermalManagerNapi::Init(napi_env env, napi_value exports)
{
    THERMAL_HILOGD(COMP_FWK, "Enter");
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_FUNCTION("subscribeThermalLevel", SubscribeThermalLevel),
        DECLARE_NAPI_STATIC_FUNCTION("unsubscribeThermalLevel", UnSubscribeThermalLevel),
        DECLARE_NAPI_STATIC_FUNCTION("getThermalLevel", GetThermalLevel),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    InitThermalLevel(env, exports);

    THERMAL_HILOGD(COMP_FWK, "Exit");
    return exports;
}

napi_value ThermalManagerNapi::InitThermalLevel(napi_env env, napi_value exports)
{
    THERMAL_HILOGD(COMP_FWK, "Enter");
    napi_value cool;
    napi_value normal;
    napi_value warm;
    napi_value hot;
    napi_value overheated;
    napi_value warning;
    napi_value emergency;
    int32_t refCount = 1;

    napi_create_uint32(env, static_cast<uint32_t>(ThermalLevel::COOL), &cool);
    napi_create_uint32(env, static_cast<uint32_t>(ThermalLevel::NORMAL), &normal);
    napi_create_uint32(env, static_cast<uint32_t>(ThermalLevel::WARM), &warm);
    napi_create_uint32(env, static_cast<uint32_t>(ThermalLevel::HOT), &hot);
    napi_create_uint32(env, static_cast<uint32_t>(ThermalLevel::OVERHEATED), &overheated);
    napi_create_uint32(env, static_cast<uint32_t>(ThermalLevel::WARNING), &warning);
    napi_create_uint32(env, static_cast<uint32_t>(ThermalLevel::EMERGENCY), &emergency);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("COOL", cool),
        DECLARE_NAPI_STATIC_PROPERTY("NORMAL", normal),
        DECLARE_NAPI_STATIC_PROPERTY("WARM", warm),
        DECLARE_NAPI_STATIC_PROPERTY("HOT", hot),
        DECLARE_NAPI_STATIC_PROPERTY("OVERHEATED", overheated),
        DECLARE_NAPI_STATIC_PROPERTY("WARNING", warning),
        DECLARE_NAPI_STATIC_PROPERTY("EMERGENCY", emergency),
    };

    napi_value result = nullptr;
    napi_define_class(env, "ThermalLevel", NAPI_AUTO_LENGTH, EnumThermalLevelConstructor,
        nullptr, sizeof(desc) / sizeof(*desc), desc, &result);
    napi_create_reference(env, result, refCount, &thermalLevelConstructor_);
    napi_set_named_property(env, exports, "ThermalLevel", result);
    THERMAL_HILOGD(COMP_FWK, "Exit");
    return exports;
}

napi_value ThermalManagerNapi::EnumThermalLevelConstructor(napi_env env, napi_callback_info info)
{
    THERMAL_HILOGD(COMP_FWK, "Enter");
    size_t argc = 0;
    napi_value args[ARG_1] = { 0 };
    napi_value jsthis = nullptr;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, &data);

    THERMAL_HILOGI(COMP_FWK, "EnumThermalLevelConstructor %{public}d", status);
    if (status != napi_ok) {
        return nullptr;
    }
    THERMAL_HILOGD(COMP_FWK, "Exit");
    return jsthis;
}

napi_value ThermalManagerNapi::GetThermalLevel(napi_env env, napi_callback_info info)
{
    THERMAL_HILOGD(COMP_FWK, "Enter");
    napi_value napiValue = nullptr;
    ThermalLevel level = g_thermalMgrClient.GetThermalLevel();
    int32_t levelValue = static_cast<int32_t>(level);
    NAPI_CALL(env, napi_create_int32(env, levelValue, &napiValue));

    THERMAL_HILOGI(COMP_FWK, "level is %{public}d", levelValue);
    THERMAL_HILOGD(COMP_FWK, "Exit");
    return napiValue;
}

napi_value ThermalManagerNapi::SubscribeThermalLevel(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_1;
    napi_value args[ARG_1] = {0};
    napi_value jsthis;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, &data);
    NAPI_ASSERT(env, (status == napi_ok) && (argc > 0), "Bad parameters");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[ARG_0], &valueType);
    NAPI_ASSERT(env, valueType == napi_function, "type mismatch for parameter 1");

    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    THERMAL_RETURN_IF_WITH_RET(g_thermalLevelCallback == nullptr, result);
    g_thermalLevelCallback->ReleaseCallback();
    g_thermalLevelCallback->UpdateCallback(env, args[ARG_0]);
    g_thermalMgrClient.SubscribeThermalLevelCallback(g_thermalLevelCallback);

    return result;
}

napi_value ThermalManagerNapi::UnSubscribeThermalLevel(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_1;
    napi_value args[ARG_1] = { 0 };
    napi_value jsthis;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, &data);
    NAPI_ASSERT(env, (status == napi_ok) && (argc >= 0), "Bad parameters");

    THERMAL_RETURN_IF_WITH_RET(g_thermalLevelCallback == nullptr, nullptr);
    g_thermalLevelCallback->ReleaseCallback();
    g_thermalMgrClient.UnSubscribeThermalLevelCallback(g_thermalLevelCallback);
    
    THERMAL_RETURN_IF_WITH_RET(argc == 0, nullptr);
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[ARG_0], &valueType);
    NAPI_ASSERT(env, valueType == napi_function, "type mismatch for parameter 1");

    napi_value handler = nullptr;
    napi_ref handlerRef = nullptr;
    napi_create_reference(env, args[ARG_0], 1, &handlerRef);
    napi_get_reference_value(env, handlerRef, &handler);
    napi_delete_reference(env, handlerRef);

    napi_value result = nullptr;
    if (handler == nullptr) {
        THERMAL_HILOGW(COMP_FWK, "Handler should not be nullptr");
        return result;
    }

    napi_get_undefined(env, &result);
    status = napi_call_function(env, nullptr, handler, ARG_0, nullptr, &result);
    if (status != napi_ok) {
        THERMAL_HILOGW(COMP_FWK, "status=%{public}d", status);
        return result;
    }
    return result;
}

EXTERN_C_START
/*
 * function for module exports
 */
static napi_value ThermalInit(napi_env env, napi_value exports)
{
    THERMAL_HILOGD(COMP_FWK, "Enter");

    napi_value ret = ThermalManagerNapi::Init(env, exports);

    THERMAL_HILOGD(COMP_FWK, "Exit");

    return ret;
}
EXTERN_C_END

/*
 * Module definition
 */
static napi_module g_module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = "thermal",
    .nm_register_func = ThermalInit,
    .nm_modname = "thermal",
    .nm_priv = ((void *)0),
    .reserved = {0}
};

/*
 * Module registration
 */
extern "C" __attribute__((constructor)) void RegisterModule()
{
    napi_module_register(&g_module);
}
