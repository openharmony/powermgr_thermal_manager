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

#ifndef THERMAL_CLIENT_H
#define THERMAL_CLIENT_H
#include <map>
#include <string>
#include "delayed_sp_singleton.h"
#include "thermal_subscriber.h"

namespace OHOS {
namespace PowerMgr {
class ThermalClient {
public:
    static ErrCode BindThermalDriverSubscriber(const sptr<ThermalSubscriber> &subscriber);
    static ErrCode UnbindThermalDriverSubscriber();
    static int32_t SetCPUFreq(uint32_t freq);
    static int32_t SetGPUFreq(uint32_t freq);
    static int32_t SetBatteryCurrent(uint32_t current);
    static std::map<std::string, int32_t> GetThermalZoneInfo();
public:
    /* The api is used to test */
    static int32_t SetHdiFlag(bool flag);
    static int32_t SetSensorTemp(std::string type, int32_t temp);
private:
    static sptr<IRemoteObject> GetThermalDriverService();
    static ErrCode DoDispatch(uint32_t cmd, MessageParcel &data, MessageParcel &reply,  MessageOption &option);
};
} // namespace PowerMgr
} // namespce OHOS
#endif // THERMAL_CLIENT_H