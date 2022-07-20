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

#include "thermal_protector_util.h"

#include <cstdio>
#include <cstdint>
#include <iosfwd>
#include <unistd.h>
#include "errors.h"
#include "thermal_log.h"

namespace OHOS {
namespace PowerMgr {
int32_t ThermalProtectorUtil::WriteFile(std::string path, std::string buf, size_t size)
{
    FILE *stream = fopen(path.c_str(), "w+");
    if (stream == nullptr) {
        return ERR_INVALID_VALUE;
    }
    size_t ret = fwrite(buf.c_str(), strlen(buf.c_str()), 1, stream);
    if (ret == ERR_OK) {
        THERMAL_HILOGE(FEATURE_PROTECTOR, "ret=%{public}zu", ret);
    }
    int32_t state = fseek(stream, 0, SEEK_SET);
    if (state != ERR_OK) {
        fclose(stream);
        return state;
    }
    state = fclose(stream);
    if (state != ERR_OK) {
        return state;
    }
    return ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS
