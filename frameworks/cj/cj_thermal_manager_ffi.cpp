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

#include <cstdint>
#include "cj_common_ffi.h"
#include "cj_thermal_manager.h"
#include "ffi_remote_data.h"

namespace OHOS {
namespace PowerMgr {

extern "C" {
FFI_EXPORT int32_t FfiOHOSThermalGetLevel()
{
    return CjThermalManager::GetThreadLocalInstance().GetThermalLevel();
}
FFI_EXPORT void FfiOHOSRegisterThermalLevelCallback(int64_t id)
{
    CjThermalManager::GetThreadLocalInstance().SubscribeThermalLevel(id);
}
FFI_EXPORT void FfiOHOSUnregisterThermalLevelCallback(int64_t id)
{
    CjThermalManager::GetThreadLocalInstance().UnSubscribeThermalLevel(id);
}
}
} // PowerMgr
} // OHOS