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

#ifndef THERMAL_COMMON_H
#define THERMAL_COMMON_H

#include <cstdint>
#include <type_traits>

#include "thermal_log.h"
#include "thermal_mgr_errors.h"

namespace OHOS {
namespace PowerMgr {
#define THERMAL_RETURN_IF_WITH_RET(cond, retval) \
    if (cond) { return (retval); }
#define THERMAL_RETURN_IF(cond) if (cond) {return;}
#define THERMAL_RETURN_IF_WITH_LOG(cond, loginfo)                                                   \
    do {                                                                                \
        if (cond) {                                                                     \
            THERMAL_HILOGE(COMP_FWK, #loginfo);                                         \
            return;                                                                     \
        }                                                                               \
    } while (0)                                                                         \

#define THERMAL_READ_PARCEL_NO_RET(parcel, type, out)                                           \
    do {                                                                                \
        if (!(parcel).Read##type(out)) {                                                \
            THERMAL_HILOGE(COMP_FWK, "read "#out" failed");                          \
            return;                                                                     \
        }                                                                               \
    } while (0)                                                                         \

#define THERMAL_WRITE_PARCEL_NO_RET(parcel, type, data)                                         \
    do {                                                                                \
        if (!(parcel).Write##type(data)) {                                              \
            THERMAL_HILOGE(COMP_FWK, "write "#data" failed");                        \
            return;                                                                     \
        }                                                                               \
    } while (0)                                                                         \

#define THERMAL_READ_PARCEL_WITH_RET(parcel, type, out, retval)                                \
    do {                                                                               \
        if (!(parcel).Read##type(out)) {                                               \
            THERMAL_HILOGE(COMP_FWK, "read "#out" failed");                         \
            return (retval);                                                           \
        }                                                                              \
    } while (0)                                                                        \

#define THERMAL_WRITE_PARCEL_WITH_RET(parcel, type, data, retval)                              \
    do {                                                                               \
        if (!(parcel).Write##type(data)) {                                             \
            THERMAL_HILOGE(COMP_FWK, "write "#data" failed");                       \
            return (retval);                                                           \
        }                                                                              \
    } while (0)

template<typename E>
constexpr auto ThermalToUnderlying(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_COMMON_H
