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

#ifndef THERMAL_HISYSEVENT_H
#define THERMAL_HISYSEVENT_H

#include <string>

namespace OHOS {
namespace PowerMgr {
void WriteLevelChangedHiSysEvent(bool enableEvent, int32_t level);
void WriteActionTriggeredHiSysEvent(bool enableEvent, const std::string& actionName, int32_t value);
void WriteActionTriggeredHiSysEventWithRatio(bool enableEvent, const std::string& actionName, float value);
} // namespace PowerMgr
} // namespace OHOS
#endif // THERMAL_HISYSEVENT_H
