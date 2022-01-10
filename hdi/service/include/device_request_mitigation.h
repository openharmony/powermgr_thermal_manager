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

#ifndef DEVICE_REQUEST_MITIGATION_H
#define DEVICE_REQUEST_MITIGATION_H

#include <string>
#include <mutex>

namespace OHOS {
namespace HDI {
namespace THERMAL {
namespace V1_0 {
class DeviceRequestMitigation {
public:
    DeviceRequestMitigation() {};
    ~DeviceRequestMitigation() {};

    int32_t WriteSysfsFd(int32_t fd, std::string buf, size_t bytesSize);
    int32_t OpenSysfsFile(std::string filePath, int32_t flags);
    int32_t WriteSysfsFile(std::string filePath, std::string buf, size_t bytesSize);
    int32_t CpuRequest(uint32_t freq);
    int32_t GpuRequest(uint32_t freq);
    int32_t ChargerRequest(uint32_t current);
    int32_t BatteryCurrentRequest(uint32_t current);
    int32_t BatteryVoltageRequest(uint32_t voltage);
public:
    int32_t SetFlag(bool flag);
private:
    std::mutex mutex_;
    bool flag_ {false};
};
} // namespace V1_0
} // namaespace THERMAL
} // namespace HDI
} // namespace OHOS

#endif //  DEVICE_REQUEST_MITIGATION_H