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

#include "fan_fault_detect.h"
#include "thermal_hisysevent.h"
#include "thermal_log.h"

namespace OHOS {
namespace PowerMgr {

const int32_t SENSOR_INVALID_VALUE = -1;
const std::string FAN = "fan";

void FanFaultDetect::OnFanSensorInfoChanged(const FanSensorInfo& report)
{
    CheckFanFault(report);
}

void FanFaultDetect::CheckFanFault(const FanSensorInfo& report)
{
    for (auto &faultInfo : fanFaultInfoMap_) {
        if (faultInfo.first == FAN_FAULT_TOO_SLOW) {
            CheckFanTooSlow(report, faultInfo.second);
        } else if (faultInfo.first == FAN_FAULT_TOO_FAST) {
            CheckFanTooFast(report, faultInfo.second);
        }
    }
}

void FanFaultDetect::CheckFanTooSlow(const FanSensorInfo& report, const FanSensorInfo& config)
{
    if (!CheckFanSensorInfo(report, config)) {
        return;
    }

    bool tempHigh = false;
    for (auto &sensorInfo : config) {
        if (sensorInfo.first == "fan") {
            continue;
        }
        if (report.at(sensorInfo.first) > sensorInfo.second) {
            tempHigh = true;
            break;
        }
    }
    if (report.at(FAN) < config.at(FAN) && tempHigh) {
        std::string reportInfo = FormatReportInfo(report, config);
        THERMAL_HILOGE(COMP_SVC, "fan is slow, %{public}s", reportInfo.c_str());
        WriteFanFaultEvent(FAN_FAULT_TOO_SLOW, reportInfo);
    }
}

void FanFaultDetect::CheckFanTooFast(const FanSensorInfo& report, const FanSensorInfo& config)
{
    if (!CheckFanSensorInfo(report, config)) {
        return;
    }

    bool tempLow = true;
    for (auto &sensorInfo : config) {
        if (sensorInfo.first == "fan") {
            continue;
        }
        if (report.at(sensorInfo.first) > sensorInfo.second) {
            tempLow = false;
            break;
        }
    }
    if ((report.at(FAN) > config.at(FAN)) && tempLow) {
        std::string reportInfo = FormatReportInfo(report, config);
        THERMAL_HILOGE(COMP_SVC, "fan is fast, %{public}s", reportInfo.c_str());
        WriteFanFaultEvent(FAN_FAULT_TOO_FAST, reportInfo);
    }
}

bool FanFaultDetect::CheckFanSensorInfo(const FanSensorInfo& report, const FanSensorInfo& config)
{
    // must config and report fan speed
    if (GetSensorValue(report, "fan") < 0 || GetSensorValue(config, "fan") < 0) {
        return false;
    }

    for (auto &sensorInfo : config) {
        if (sensorInfo.second < 0) {
            return false;
        }
        // config and report info must match
        if (GetSensorValue(report, sensorInfo.first) < 0) {
            return false;
        }
    }

    return true;
}

int32_t FanFaultDetect::GetSensorValue(const FanSensorInfo& map, const std::string& name)
{
    auto iter = map.find(name);
    if (iter == map.end()) {
        return SENSOR_INVALID_VALUE;
    }

    return iter->second;
}

std::string FanFaultDetect::FormatReportInfo(const FanSensorInfo& report, const FanSensorInfo& config)
{
    std::string reportInfo = "Report";
    for (auto &sensorInfo : config) {
        reportInfo += " ";
        reportInfo += sensorInfo.first;
        reportInfo += ":";
        reportInfo += std::to_string(report.at(sensorInfo.first));
    }
    return reportInfo;
}

bool FanFaultDetect::HasFanConfig()
{
    return !fanFaultInfoMap_.empty();
}

void FanFaultDetect::SetFaultInfoMap(FanFaultInfoMap& map)
{
    fanFaultInfoMap_ = map;
}
} // namespace PowerMgr
} // namespace OHOS
