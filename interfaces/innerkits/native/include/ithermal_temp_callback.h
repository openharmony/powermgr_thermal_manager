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

#ifndef PowerMgr_ITHERMAL_TEMPERATURE_CALLBACK_H
#define PowerMgr_ITHERMAL_TEMPERATURE_CALLBACK_H

#include <iremote_broker.h>
#include <iremote_object.h>
#include <map>

namespace OHOS {
namespace PowerMgr {
class IThermalTempCallback : public IRemoteBroker {
public:
    using TempCallbackMap = std::map<std::string, int32_t>;
    enum {
        THERMAL_TEMPERATURE_CHANGD = 0,
    };

    virtual bool OnThermalTempChanged(TempCallbackMap& tempCbMap) = 0;

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.powermgr.IThermalTempCallback");
};
} // namespace PowerMgr
} // namespace OHOS
#endif // PowerMgr_ITHERMAL_TEMPERATURE_CALLBACK_H
