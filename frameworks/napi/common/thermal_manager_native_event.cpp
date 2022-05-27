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

#include "thermal_manager_native_event.h"
#include "thermal_common.h"

using namespace OHOS::PowerMgr;

ThermalManagerNativeEvent::ThermalManagerNativeEvent(napi_env env, napi_value thisVar)
{
    env_ = env;
    thisVarRef_ = nullptr;
    napi_create_reference(env, thisVar, 1, &thisVarRef_);
}

ThermalManagerNativeEvent::~ThermalManagerNativeEvent()
{
    for (auto iter = eventMap_.begin(); iter != eventMap_.end(); iter++) {
        auto listener = iter->second;
        napi_delete_reference(env_, listener->handlerRef);
    }
    eventMap_.clear();
    napi_delete_reference(env_, thisVarRef_);
}

void ThermalManagerNativeEvent::On(const std::string &eventType, napi_value handler)
{
    THERMAL_HILOGD(COMP_FWK, "ThermalManagerNativeEvent On in for event: %{public}s", eventType.c_str());
    auto listener = std::make_shared<ThermalEventListener>();
    listener->eventType = eventType;
    napi_create_reference(env_, handler, 1, &listener->handlerRef);
    eventMap_[eventType] = listener;
}

void ThermalManagerNativeEvent::Off(const std::string &eventType)
{
    THERMAL_HILOGD(COMP_FWK, "ThermalManagerNativeEvent off in for event: %{public}s", eventType.c_str());
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env_, &scope);
    if (scope == nullptr) {
        THERMAL_HILOGE(COMP_FWK, "scope is nullptr");
        return;
    }

    auto iter = eventMap_.find(eventType);
    if (iter == eventMap_.end()) {
        THERMAL_HILOGE(COMP_FWK, "eventType %{public}s not find", eventType.c_str());
        return;
    }
    auto listener = iter->second;
    napi_delete_reference(env_, listener->handlerRef);
    eventMap_.erase(eventType);
    napi_close_handle_scope(env_, scope);
}

void ThermalManagerNativeEvent::OnEvent(const std::string &eventType, size_t argc, const napi_value* argv)
{
    THERMAL_HILOGD(COMP_FWK, "OnEvent for %{public}s", eventType.c_str());
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env_, &scope);
    if (scope == nullptr) {
        THERMAL_HILOGE(COMP_FWK, "scope is nullptr");
        return;
    }

    auto iter = eventMap_.find(eventType);
    if (iter == eventMap_.end()) {
        THERMAL_HILOGE(COMP_FWK, "OnEvent: eventType %{public}s not find", eventType.c_str());
        return;
    }
    auto listener = iter->second;
    napi_value thisVar = nullptr;
    napi_status status = napi_get_reference_value(env_, thisVarRef_, &thisVar);
    if (status != napi_ok) {
        THERMAL_HILOGE(COMP_FWK,
            "OnEvent napi_get_reference_value thisVar for %{public}s failed, status = %{public}d",
            eventType.c_str(), status);
        return;
    }
    napi_value handler = nullptr;
    status = napi_get_reference_value(env_, listener->handlerRef, &handler);
    if (status != napi_ok) {
        THERMAL_HILOGE(COMP_FWK,
            "OnEvent napi_get_reference_value handler for %{public}s failed, status = %{public}d",
            eventType.c_str(), status);
        return;
    }
    napi_value callResult = nullptr;
    status = napi_call_function(env_, thisVar, handler, argc, argv, &callResult);
    if (status != napi_ok) {
        THERMAL_HILOGE(COMP_FWK,
            "OnEvent: napi_call_function for %{public}s failed, status = %{public}d",
            eventType.c_str(), status);
        return;
    }
    napi_close_handle_scope(env_, scope);
    THERMAL_HILOGD(COMP_FWK, "Exit");
}
