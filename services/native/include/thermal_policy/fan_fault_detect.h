/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef FAN_FAULT_DETECT_H
#define FAN_FAULT_DETECT_H

#include <map>
#include <string>
#include <vector>

namespace OHOS {
namespace PowerMgr {

enum {
    FAN_FAULT_OK = 0,
    FAN_FAULT_TOO_SLOW,
    FAN_FAULT_TOO_FAST,
};

using FanSensorInfo = std::map<std::string, int32_t>;
using FanFaultInfoMap = std::map<int32_t, FanSensorInfo>;

class FanFaultDetect {
public:
    FanFaultDetect() = default;
    ~FanFaultDetect() = default;

    void OnFanSensorInfoChanged(const FanSensorInfo& report);
    void SetFaultInfoMap(FanFaultInfoMap& map);
    bool HasFanConfig();

private:
    void CheckFanFault(const FanSensorInfo& report);
    void CheckFanTooSlow(const FanSensorInfo& report, const FanSensorInfo& config);
    void CheckFanTooFast(const FanSensorInfo& report, const FanSensorInfo& config);
    bool CheckFanSensorInfo(const FanSensorInfo& report, const FanSensorInfo& config);
    int32_t GetSensorValue(const FanSensorInfo& map, const std::string& name);
    std::string FormatReportInfo(const FanSensorInfo& report, const FanSensorInfo& config);

    FanFaultInfoMap fanFaultInfoMap_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // FAN_FAULT_DETECT_H
