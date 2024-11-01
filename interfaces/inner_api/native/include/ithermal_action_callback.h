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

#ifndef ITHERMAL_ACTION_CALLBACK_H
#define ITHERMAL_ACTION_CALLBACK_H

#include <iremote_broker.h>
#include <iremote_object.h>
#include <map>

namespace OHOS {
namespace PowerMgr {
class IThermalActionCallback : public IRemoteBroker {
public:
    using ActionCallbackMap = std::map<std::string, std::string>;

    virtual bool OnThermalActionChanged(ActionCallbackMap& actionCbMap) = 0;

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.powermgr.IThermalActionCallback");
};
} // namespace PowerMgr
} // namespace OHOS
#endif //  ITHERMAL_ACTION_CALLBACK_H
