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

#ifndef ITHERMAL_LEVEL_CALLBACK_H
#define ITHERMAL_LEVEL_CALLBACK_H

#include "thermal_level_info.h"
#include <iremote_broker.h>
#include <iremote_object.h>

namespace OHOS {
namespace PowerMgr {
class IThermalLevelCallback : public IRemoteBroker {
public:
    virtual bool OnThermalLevelChanged(ThermalLevel level) = 0;
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.powermgr.IThermalLevelCallback");
};
} // namespace PowerMgr
} // namespace OHOS
#endif //  ITHERMAL_LEVEL_CALLBACK_H
