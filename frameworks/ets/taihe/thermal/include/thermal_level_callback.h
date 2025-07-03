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

#ifndef THERMAL_LEVEL_CALLBACK_H
#define THERMAL_LEVEL_CALLBACK_H

#include "ohos.thermal.proj.hpp"
#include "ohos.thermal.impl.hpp"
#include "taihe/runtime.hpp"
#include "thermal_level_callback_stub.h"

namespace OHOS {
namespace PowerMgr {
using namespace taihe;
class ThermalLevelCallback : public ThermalLevelCallbackStub {
public:
    ThermalLevelCallback() = default;
    virtual ~ThermalLevelCallback();
    void UpdateCallback(callback_view<void(ohos::thermal::ThermalLevel)> thermalCb);
    bool OnThermalLevelChanged(OHOS::PowerMgr::ThermalLevel level) override;
    void OnThermalLevel();

private:
    OHOS::PowerMgr::ThermalLevel level_ {OHOS::PowerMgr::ThermalLevel::COOL};
    optional<callback<void(ohos::thermal::ThermalLevel)>> callback_;
    std::mutex mutex_;
};
} // namespace PowerMgr
} // namespace OHOS

#endif // THERMAL_LEVEL_CALLBACK_H