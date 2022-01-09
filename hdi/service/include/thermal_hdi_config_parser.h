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

#ifndef  THERMAL_HDI_CONFIG_PARSER_H
#define  THERMAL_HDI_CONFIG_PARSER_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <libxml/xpath.h>
#include <libxml/tree.h>
#include <libxml/parser.h>

#include "config/base_info_config.h"
#include "config/sensor_info_config.h"

namespace OHOS {
namespace HDI {
namespace THERMAL {
namespace V1_0 {
struct XMLThermal {
    std::string version;
    std::string product;
};

class ThermalHDIConfigParser {
public:
    using ThermalTypeMap = std::map<std::string, std::shared_ptr<SensorInfoConfig>>;
    ThermalHDIConfigParser() {};
    ~ThermalHDIConfigParser() = default;
    ThermalHDIConfigParser(const ThermalHDIConfigParser&) = delete;
    ThermalHDIConfigParser& operator=(const ThermalHDIConfigParser&) = delete;
    static ThermalHDIConfigParser &GetInsance();

    int32_t ThermalHDIConfigInit(const std::string &path);
    int32_t ParseThermalHdiXMLConfig(const std::string &path);
    void ParseBaseNode(xmlNodePtr node);
    void ParsePollingNode(xmlNodePtr node);
    std::map<std::string, uint32_t> GetIntervalMap();
    ThermalTypeMap GetSensorTypeMap();
private:
    std::shared_ptr<BaseInfoConfig> vbaseConfig_;
    std::map<std::string, uint32_t> intervalMap_;
    ThermalTypeMap typesMap_;
    XMLThermal thermal_;
};
} // V1_0
} // THERMAL
} // HDI
} // OHOS

#endif // THERMAL_HDI_CONFIG_PARSER_H