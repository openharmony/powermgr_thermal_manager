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

#include "thermal_napi_context.h"

#include <cstddef>
#include <cstdint>
#include "js_native_api.h"
#include "js_native_api_types.h"
#include "node_api.h"
#include "thermal_log.h"

using namespace OHOS::PowerMgr;

namespace {
static const int32_t CALLBACK_ARG_0 = 0;
static const int32_t CALLBACK_ARG_1 = 1;
static const int32_t CALLBACK_ARGS_COUNT = 2;
}

ThermalNapiContext::ThermalNapiContext(napi_env env, napi_value* args, uint32_t argc,
    int32_t callbackArg, napi_value object) : env_(env),
                                              object_(object),
                                              deferred_(nullptr),
                                              callbackRef_(nullptr),
                                              asyncWork_(nullptr),
                                              status_(AsyncStatus::PENDING)
{
    THERMAL_HILOGD(COMP_FWK, "Enter");
    if (callbackArg >= 0 && callbackArg < static_cast<int32_t>(argc)) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, args[callbackArg], &valueType);
        if (valueType == napi_function) {
            napi_create_reference(env, args[callbackArg], 1, &callbackRef_);
        }
    }

    if (callbackRef_ == nullptr) {
        napi_create_promise(env, &deferred_, &promise_);
    } else {
        napi_get_undefined(env, &promise_);
    }

    if (object_ == nullptr) {
        napi_get_global(env, &object_);
    }
    napi_get_undefined(env_, &outValue_);
    napi_get_undefined(env_, &outError_);
    Init(args, argc);
}

ThermalNapiContext::~ThermalNapiContext()
{
    if (deferred_) {
        deferred_ = NULL;
    }

    if (asyncWork_) {
        napi_delete_async_work(env_, asyncWork_);
    }
}

bool ThermalNapiContext::StartAsyncWork(const char* workName, ExecuteFunc exeFunc, FreeFunc freeFunc)
{
    napi_status status;

    napi_value resourceName;
    napi_create_string_latin1(this->env_, workName, NAPI_AUTO_LENGTH, &resourceName);

    exeFunc_ = exeFunc;
    freeFunc_ = freeFunc;

    status = napi_create_async_work(
        this->env_,
        nullptr,
        resourceName,
        ExecuteAsyncWork,
        CompleteAsyncWork,
        (void *)this,
        &asyncWork_);
    if (status != napi_ok) {
        THERMAL_HILOGE(COMP_FWK, "CreateAsyncWork failed: %d", status);
        return false;
    }
    napi_queue_async_work(this->env_, asyncWork_);

    return true;
}

void ThermalNapiContext::ExecuteAsyncWork(napi_env env, void *data)
{
    ThermalNapiContext* context = (ThermalNapiContext*)data;
    if (context->exeFunc_) {
        bool ret = context->exeFunc_();
        THERMAL_HILOGD(COMP_FWK, "execute work: %d", ret);
        if (ret) {
            context->status_ = AsyncStatus::RESOLVED;
        } else {
            context->status_ = AsyncStatus::REJECTED;
        }
    } else {
        THERMAL_HILOGW(COMP_FWK, "execute work: no exeFunc");
        context->status_ = AsyncStatus::REJECTED;
    }
}

void ThermalNapiContext::CompleteAsyncWork(napi_env env, napi_status status, void *data)
{
    ThermalNapiContext* context = (ThermalNapiContext*)data;

    if (context->deferred_) {
        if (context->status_ == AsyncStatus::RESOLVED) {
            THERMAL_HILOGD(COMP_FWK, "work complete: resolved");
            napi_resolve_deferred(env, context->deferred_, context->GetValue());
        } else {
            THERMAL_HILOGD(COMP_FWK, "work complete: rejected");
            napi_reject_deferred(env, context->deferred_, context->GetError());
        }
        context->deferred_ = nullptr;
    } else if (context->callbackRef_) {
        THERMAL_HILOGD(COMP_FWK, "work complete: callback");
        napi_value result = 0;
        napi_value callback = nullptr;
        napi_get_reference_value(env, context->callbackRef_, &callback);
        napi_value values[CALLBACK_ARGS_COUNT];

        values[CALLBACK_ARG_0] = context->GetError();
        values[CALLBACK_ARG_1] = context->GetValue();

        napi_call_function(env, context->object_, callback, context->cbParamCount_, values, &result);
        napi_delete_reference(env, context->callbackRef_);
        context->callbackRef_ = nullptr;
    } else {
        THERMAL_HILOGE(COMP_FWK, "work complete: nothing");
    }
    napi_delete_async_work(env, context->asyncWork_);
    context->asyncWork_ = nullptr;
    if (context->freeFunc_) {
        context->freeFunc_(context);
    }
}

void ThermalNapiContext::Init(napi_value* args, uint32_t argc)
{
    // do nothing in context class
}

napi_value ThermalNapiContext::GetValue()
{
    return outValue_;
}

napi_value ThermalNapiContext::GetError()
{
    return outError_;
}
