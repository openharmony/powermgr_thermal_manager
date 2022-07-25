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

#ifndef THERMAL_CONFIG_BASE_INFO_H
#define THERMAL_CONFIG_BASE_INFO_H

#include <string>
#include <vector>
#include <map>

namespace OHOS {
namespace PowerMgr {
namespace {
using BaseInfoMap = std::map<std::string, std::string>;
constexpr const char* HISTORY_TEMP_COUNT_TAG = "history_temp_count";
constexpr const char* SENSORS_TYPE_TAG = "temperature_query_enum";
}

struct BaseItem {
    std::string tag;
    std::string value;
};

class ThermalConfigBaseInfo {
public:
    bool Init();
    void Dump();

    uint32_t GetHistoryTempCount()
    {
        return historyTempCount_;
    }

    std::vector<std::string> GetSensorsType() const
    {
        return sensorsType_;
    }

    void SetBaseInfo(BaseInfoMap &baseinfo)
    {
        baseItems_ = baseinfo;
    }

private:
    void SetHistoryTempCount();
    void SetSensorsType();

    uint32_t historyTempCount_;
    std::string sensorType_;
    std::vector<std::string> sensorsType_;
    BaseInfoMap baseItems_;
};
} // namespace PowerMgr
} // namespace OHOS

#endif //  THERMAL_CONFIG_BASE_INFO_H