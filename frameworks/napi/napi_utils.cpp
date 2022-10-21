/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "napi_utils.h"
#include "thermal_log.h"

namespace OHOS {
namespace PowerMgr {
napi_value NapiUtils::GetCallbackInfo(napi_env& env, napi_callback_info& info, size_t& argc, napi_value argv[])
{
    napi_value thisVar = nullptr;
    void* data = nullptr;
    if (napi_ok != napi_get_cb_info(env, info, &argc, argv, &thisVar, &data)) {
        THERMAL_HILOGW(COMP_APP, "Failed to get the input parameter");
    }
    return thisVar;
}

bool NapiUtils::CheckValueType(napi_env& env, napi_value& value, napi_valuetype checkType)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType != checkType) {
        THERMAL_HILOGW(COMP_APP, "Parameter type error");
        return false;
    }
    return true;
}
} // namespace PowerMgr
} // namespace OHOS
