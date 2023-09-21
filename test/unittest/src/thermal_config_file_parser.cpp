/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "thermal_config_file_parser.h"

#include "string_ex.h"
#include "string_operation.h"

namespace OHOS {
namespace PowerMgr {
namespace {
static const std::string VENDOR_THERMAL_SRV_CONFIG_XML = "/vendor/etc/thermal_config/thermal_service_config.xml";
static const std::string SYSTEM_THERMAL_SRV_CONFIG_XML = "/system/etc/thermal_config/thermal_service_config.xml";
const std::string TRUE_STR = "1";
} // namespace
bool ThermalConfigFileParser::Init()
{
    if (!LoadThermalSrvConfigXml(VENDOR_THERMAL_SRV_CONFIG_XML)) {
        THERMAL_HILOGE(LABEL_TEST, "Failed to load vendor thermal config xml file");
        if (!LoadThermalSrvConfigXml(SYSTEM_THERMAL_SRV_CONFIG_XML)) {
            THERMAL_HILOGE(LABEL_TEST, "Failed to load system thermal config xml file");
            return false;
        }
    }
    return true;
}

bool ThermalConfigFileParser::GetActionEnableEvent(const std::string& actionName)
{
    for (auto iter : actionItem_) {
        if (iter.name.compare(actionName) == 0 ||
            actionName.find(iter.name) != std::string::npos) {
            return iter.enableEvent;
        }
    }
    return false;
}

bool ThermalConfigFileParser::GetActionStrict(const std::string& actionName)
{
    for (auto iter : actionItem_) {
        if (iter.name.compare(actionName) == 0 ||
            actionName.find(iter.name) != std::string::npos) {
            return iter.strict;
        }
    }
    return false;
}

bool ThermalConfigFileParser::GetActionPolicy(const std::string& name, uint32_t level,
    std::vector<PolicyAction>& policy)
{
    auto vPolicyCfg = policyConfigMap_.find(name);
    if (vPolicyCfg != policyConfigMap_.end()) {
        for (auto cfgIter : vPolicyCfg->second) {
            if (cfgIter.level == level) {
                policy = cfgIter.policyActionList;
                return true;
            }
        }
    }
    return false;
}

std::vector<LevelItem> ThermalConfigFileParser::GetLevelItems(const std::string& name, const std::string& sensor)
{
    auto sensorInfo = sensorInfoMap_.find(name);
    if (sensorInfo != sensorInfoMap_.end()) {
        auto levelInfo = sensorInfo->second.find(sensor);
        if (levelInfo != sensorInfo->second.end()) {
            return levelInfo->second;
        }
    }
    return std::vector<LevelItem>{};
}

std::vector<StateItem> ThermalConfigFileParser::GetStateItem()
{
    return stateItem_;
}

bool ThermalConfigFileParser::LoadThermalSrvConfigXml(const std::string& path)
{
    if (!ParseXmlFile(path)) {
        return false;
    }
    return true;
}

bool ThermalConfigFileParser::ParseXmlFile(const std::string& path)
{
    std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)> docPtr(
        xmlReadFile(path.c_str(), nullptr, XML_PARSE_NOBLANKS), xmlFreeDoc);
    if (docPtr == nullptr) {
        THERMAL_HILOGE(LABEL_TEST, "ParseXMLFile::Init failed, read file failed.");
        return false;
    }

    auto rootNode = xmlDocGetRootElement(docPtr.get());
    if (rootNode == nullptr) {
        THERMAL_HILOGE(LABEL_TEST, "ParseXMLFile::Get root node failed.");
        return false;
    }

    for (auto node = rootNode->children; node; node = node->next) {
        if (node == nullptr) {
            continue;
        }
        if (!xmlStrcmp(node->name, BAD_CAST"base")) {
            ParseBaseNode(node);
        } else if (!xmlStrcmp(node->name, BAD_CAST"level")) {
            ParseLevelNode(node);
        } else if (!xmlStrcmp(node->name, BAD_CAST"state")) {
            ParseStateNode(node);
        } else if (!xmlStrcmp(node->name, BAD_CAST"action")) {
            ParseActionNode(node);
        } else if (!xmlStrcmp(node->name, BAD_CAST"policy")) {
            ParsePolicyNode(node);
        } else if (!xmlStrcmp(node->name, BAD_CAST"idle")) {
            ParseIdleNode(node);
        }
    }
    return true;
}

void ThermalConfigFileParser::ParseBaseNode(xmlNodePtr node)
{
    auto curNode = node->xmlChildrenNode;
    while (curNode != nullptr) {
        BaseItem item;
        xmlChar* xmlTag = xmlGetProp(curNode, BAD_CAST"tag");
        if (xmlTag != nullptr) {
            item.tag = reinterpret_cast<char*>(xmlTag);
            xmlFree(xmlTag);
        }

        xmlChar* xmlValue = xmlGetProp(curNode, BAD_CAST"value");
        if (xmlValue != nullptr) {
            item.value = reinterpret_cast<char*>(xmlValue);
            xmlFree(xmlValue);
        }
        baseInfoMap_.emplace(item.tag, item.value);
        curNode = curNode->next;
        THERMAL_HILOGD(LABEL_TEST, "tag: %{public}s, value:%{public}s",
            item.tag.c_str(), item.value.c_str());
    }
}

void ThermalConfigFileParser::ParseLevelNode(xmlNodePtr node)
{
    auto curNode = node->xmlChildrenNode;
    while (curNode != nullptr) {
        std::string name;
        std::shared_ptr<ThermalConfigSensorCluster> sc = std::make_shared<ThermalConfigSensorCluster>();
        xmlChar* xmlName = xmlGetProp(curNode, BAD_CAST"name");
        if (xmlName != nullptr) {
            name = reinterpret_cast<char*>(xmlName);
            xmlFree(xmlName);
        }
        ParseAuxSensorInfo(name, curNode, sc);
        ParseSensorInfo(name, curNode, sc);
        xmlChar* desc = xmlGetProp(curNode, BAD_CAST("desc"));
        if (desc != nullptr) {
            std::string descValue = reinterpret_cast<char*>(desc);
            if (TrimStr(descValue) == TRUE_STR) {
                sc->SetDescFlag(true);
            }
            xmlFree(desc);
        }
        sensorClusterMap_.emplace(std::pair(name, sc));
        curNode = curNode->next;
    }
}

void ThermalConfigFileParser::ParseStateNode(xmlNodePtr node)
{
    auto curNode = node->xmlChildrenNode;
    while (curNode != nullptr) {
        StateItem si;
        xmlChar* xmlName = xmlGetProp(curNode, BAD_CAST"name");
        if (xmlName != nullptr) {
            si.name = reinterpret_cast<char*>(xmlName);
            xmlFree(xmlName);
        }

        xmlChar* param = xmlGetProp(curNode, BAD_CAST("param"));
        if (param != nullptr) {
            si.params = reinterpret_cast<char*>(param);
            si.isExistParam = true;
            xmlFree(param);
        }
        stateItem_.push_back(si);
        THERMAL_HILOGI(LABEL_TEST, "si.name: %{public}s, si.params %{public}s",
            si.name.c_str(), si.params.c_str());
        curNode = curNode->next;
    }
}

void ThermalConfigFileParser::ParseActionNode(xmlNodePtr node)
{
    auto curNode = node->xmlChildrenNode;
    while (curNode != nullptr) {
        if (!xmlStrcmp(curNode->name, BAD_CAST"item")) {
            ActionItem ai;
            xmlChar* xmlName = xmlGetProp(curNode, BAD_CAST"name");
            if (xmlName != nullptr) {
                ai.name = reinterpret_cast<char*>(xmlName);
                xmlFree(xmlName);
            }
            xmlChar* param = xmlGetProp(curNode, BAD_CAST("param"));
            if (param != nullptr) {
                ai.params = reinterpret_cast<char*>(param);
                xmlFree(param);
            }
            xmlChar* protocol = xmlGetProp(curNode, BAD_CAST("protocol"));
            if (protocol != nullptr) {
                ai.protocol = reinterpret_cast<char*>(protocol);
                xmlFree(protocol);
            }
            xmlChar* strict = xmlGetProp(curNode, BAD_CAST("strict"));
            if (strict != nullptr) {
                std::string strictValue = reinterpret_cast<char*>(strict);
                ai.strict = (TrimStr(strictValue) == TRUE_STR);
                xmlFree(strict);
            }
            xmlChar* event = xmlGetProp(curNode, BAD_CAST("event"));
            if (event != nullptr) {
                std::string eventValue = reinterpret_cast<char*>(event);
                ai.enableEvent = (TrimStr(eventValue) == TRUE_STR);
                xmlFree(event);
            }
            THERMAL_HILOGD(LABEL_TEST,
                "ai.name: %{public}s, ai.strict: %{public}d, ai.params: %{public}s, ai.strict: %{public}s, "    \
                "ai.enableEvent: %{public}d",
                ai.name.c_str(), ai.strict, ai.params.c_str(), ai.protocol.c_str(), ai.enableEvent);

            actionItem_.push_back(ai);
        }
        curNode = curNode->next;
    }
}

void ThermalConfigFileParser::ParsePolicyNode(xmlNodePtr node)
{
    auto curNode = node->xmlChildrenNode;
    while (curNode != nullptr) {
        PolicyConfig policyConfig;
        std::string clusterName;
        xmlChar* xmlName = xmlGetProp(curNode, BAD_CAST"name");
        if (xmlName != nullptr) {
            clusterName = reinterpret_cast<char*>(xmlName);
            xmlFree(xmlName);
        }

        xmlChar* xmlLevel = xmlGetProp(curNode, BAD_CAST"level");
        if (xmlLevel != nullptr) {
            uint32_t level = 0;
            StringOperation::StrToUint(reinterpret_cast<char*>(xmlLevel), level);
            policyConfig.level = level;
            THERMAL_HILOGD(LABEL_TEST, "policyConfig.name: %{public}s, policyConfig.level:%{public}d",
                clusterName.c_str(), level);
            xmlFree(xmlLevel);
        }

        ParsePolicySubnode(curNode, policyConfig);

        const auto& clusterIter = policyConfigMap_.find(clusterName);
        THERMAL_HILOGD(LABEL_TEST, "clusterName: %{public}s", clusterName.c_str());
        if (clusterIter == policyConfigMap_.end()) {
            std::vector<PolicyConfig> policyList;
            policyList.push_back(policyConfig);
            policyConfigMap_.emplace(clusterName, policyList);
        } else {
            clusterIter->second.push_back(policyConfig);
        }
        curNode = curNode->next;
    }
}

void ThermalConfigFileParser::ParseIdleNode(xmlNodePtr node)
{
    IdleState idleState;
    for (auto subNode = node->children; subNode != nullptr; subNode = subNode->next) {
        if (!xmlStrcmp(subNode->name, BAD_CAST"thermallevel")) {
            xmlChar* value = xmlNodeGetContent(subNode);
            if (value != nullptr) {
                StrToInt(reinterpret_cast<char*>(value), idleState.level);
                xmlFree(value);
            }
        } else if (!xmlStrcmp(subNode->name, BAD_CAST"soc")) {
            xmlChar* value = xmlNodeGetContent(subNode);
            if (value != nullptr) {
                StrToInt(reinterpret_cast<char*>(value), idleState.soc);
                xmlFree(value);
            }
        } else if (!xmlStrcmp(subNode->name, BAD_CAST"charging")) {
            xmlChar* value = xmlNodeGetContent(subNode);
            if (value != nullptr) {
                StrToInt(reinterpret_cast<char*>(value), idleState.charging);
                xmlFree(value);
            }
        } else if (!xmlStrcmp(subNode->name, BAD_CAST"current")) {
            xmlChar* value = xmlNodeGetContent(subNode);
            if (value != nullptr) {
                StrToInt(reinterpret_cast<char*>(value), idleState.current);
                xmlFree(value);
            }
        } else {
            THERMAL_HILOGD(LABEL_TEST, "not supported node, name=%{public}s", subNode->name);
        }
    }
    THERMAL_HILOGI(LABEL_TEST, "level=%{public}d, soc=%{public}d, charging=%{public}d, current=%{public}d",
                   idleState.level, idleState.soc, idleState.charging, idleState.current);
}

void ThermalConfigFileParser::ParseAuxSensorInfo(
    const std::string& name, const xmlNode* cur, std::shared_ptr<ThermalConfigSensorCluster>& sc)
{
    xmlChar* auxSensorInfo = xmlGetProp(cur, BAD_CAST"aux_sensor");
    if (auxSensorInfo != nullptr) {
        std::vector<std::string> auxSensorList;
        AuxSensorInfoMap auxSensorLevelInfo;
        std::string auxSensor = reinterpret_cast<char*>(auxSensorInfo);
        sc->SetAuxFlag(true);
        StringOperation::SplitString(auxSensor, auxSensorList, ",");
        for (uint32_t i = 0; i < auxSensorList.size(); i++) {
            std::string sensorType = auxSensorList[i];
            if (auxSensorList[i].empty()) {
                continue;
            }
            THERMAL_HILOGD(LABEL_TEST, "aux_sensor item: %{public}s", sensorType.c_str());
            auxSensorLevelInfo.emplace(sensorType, ParseAuxSensorSubnodeInfo(cur, auxSensorList, i));
        }
        auxSensorInfoMap_.emplace(name, auxSensorLevelInfo);
        sc->SetAuxSensorLevelInfo(auxSensorLevelInfo);
        xmlFree(auxSensorInfo);
    }
}

void ThermalConfigFileParser::ParseSensorInfo(const std::string& name, const xmlNode* cur,
    std::shared_ptr<ThermalConfigSensorCluster>& sc)
{
    SensorInfoMap sensorLevelInfo;
    std::vector<std::string> sensors;
    xmlChar* xmlSensor = xmlGetProp(cur, BAD_CAST"sensor");
    if (xmlSensor != nullptr) {
        StringOperation::SplitString(reinterpret_cast<char*>(xmlSensor), sensors, ",");
        for (uint32_t i = 0; i < sensors.size(); i++) {
            std::string sensorType = sensors.at(i);
            std::vector<LevelItem> vItem;
            ParseSensorSubnodeInfo(cur, vItem, sensors, i, sc);
            sensorLevelInfo.emplace(std::pair(sensorType, vItem));
        }
        sensorInfoMap_.insert(std::make_pair(name, sensorLevelInfo));
        sc->SetSensorLevelInfo(sensorLevelInfo);
        xmlFree(xmlSensor);
    }
}

std::vector<AuxLevelItem> ThermalConfigFileParser::ParseAuxSensorSubnodeInfo(const xmlNode* cur,
    std::vector<std::string>& auxSensorList, const uint32_t i)
{
    std::vector<AuxLevelItem> auxItems;
    for (auto subNode = cur->children; subNode != nullptr; subNode = subNode->next) {
        std::string tempRanges;
        AuxLevelItem auxlevelItem;
        if (ParseAuxSensorSubnodeInfoTrigerRange(subNode, auxSensorList, tempRanges, i) == false) {
            break;
        }

        std::vector<std::string> tempRiseRanges;
        StringOperation::SplitString(tempRanges, tempRiseRanges, "_");
        const int32_t INDEX0 = 0;
        const int32_t INDEX1 = 1;
        StrToInt(tempRiseRanges[INDEX0], auxlevelItem.lowerTemp);
        StrToInt(tempRiseRanges[INDEX1], auxlevelItem.upperTemp);
        xmlChar* xmlLevel = xmlGetProp(subNode, BAD_CAST("level"));
        if (xmlLevel != nullptr) {
            StrToInt(reinterpret_cast<char*>(xmlLevel), auxlevelItem.level);
            xmlFree(xmlLevel);
        }
        THERMAL_HILOGD(LABEL_TEST, "aux_trigger_range: %{public}s",
            tempRanges.c_str());
        THERMAL_HILOGD(LABEL_TEST, "lowerTemp: %{public}d, upperTemp: %{public}d",
            auxlevelItem.lowerTemp, auxlevelItem.upperTemp);
        auxItems.push_back(auxlevelItem);
    }
    return auxItems;
}

bool ThermalConfigFileParser::ParseAuxSensorSubnodeInfoTrigerRange(const xmlNode* subNode,
    std::vector<std::string>& auxSensorList, std::string& tempRanges, const uint32_t i)
{
    xmlChar* xmlTriggerRange = xmlGetProp(subNode, BAD_CAST("aux_trigger_range"));
    if (xmlTriggerRange != nullptr) {
        std::string auxTriggerRange = reinterpret_cast<char*>(xmlTriggerRange);
        if (!auxTriggerRange.empty()) {
            std::vector<std::string> auxTempranges;
            StringOperation::SplitString(auxTriggerRange, auxTempranges, ",");
            if (auxSensorList.size() > auxTempranges.size()) {
                THERMAL_HILOGI(LABEL_TEST, "The auxiliary sensor does not match the threshold range");
                xmlFree(xmlTriggerRange);
                return false;
            }
            tempRanges = auxTempranges[i];
        }
        xmlFree(xmlTriggerRange);
    }
    return true;
}

void ThermalConfigFileParser::ParseSensorSubnodeInfo(const xmlNode* cur, std::vector<LevelItem>& vItem,
    std::vector<std::string>& sensors, const uint32_t i, std::shared_ptr<ThermalConfigSensorCluster>& sc)
{
    for (auto subNode = cur->children; subNode; subNode = subNode->next) {
        if (subNode == nullptr) {
            continue;
        }
        LevelItem levelItem;
        std::vector<std::string> thresholds;
        std::vector<std::string> thresholdClrs;
        xmlChar* xmlThreshold = xmlGetProp(subNode, BAD_CAST("threshold"));
        if (xmlThreshold != nullptr) {
            StringOperation::SplitString(reinterpret_cast<char*>(xmlThreshold), thresholds, ",");
            xmlFree(xmlThreshold);
        }
        xmlChar* xmlThresholdClr = xmlGetProp(subNode, BAD_CAST("threshold_clr"));
        if (xmlThresholdClr != nullptr) {
            StringOperation::SplitString(reinterpret_cast<char*>(xmlThresholdClr), thresholdClrs, ",");
            xmlFree(xmlThresholdClr);
        }
        if (sensors.size() > thresholds.size() || sensors.size() > thresholdClrs.size()) {
            THERMAL_HILOGI(LABEL_TEST, "The sensor does not match the threshold range");
            break;
        }
        xmlChar* xmlLevel = xmlGetProp(subNode, BAD_CAST("level"));
        if (xmlLevel != nullptr) {
            StringOperation::StrToUint(reinterpret_cast<char*>(xmlLevel), levelItem.level);
            xmlFree(xmlLevel);
        }

        StrToInt(thresholds.at(i), levelItem.threshold);
        StrToInt(thresholdClrs.at(i), levelItem.thresholdClr);
        xmlChar* tempRiseRates = xmlGetProp(subNode, BAD_CAST("temp_rise_rate"));
        if (tempRiseRates != nullptr) {
            std::vector<std::string> rates;
            sc->SetRateFlag(true);
            StringOperation::SplitString(reinterpret_cast<char*>(tempRiseRates), rates, ",");
            if (sensors.size() > rates.size()) {
                break;
            }
            StringOperation::StrToDouble(rates.at(i), levelItem.tempRiseRate);
        }
        vItem.push_back(levelItem);
        xmlFree(tempRiseRates);
    }
}

void ThermalConfigFileParser::ParsePolicySubnode(const xmlNode* cur, PolicyConfig& policyConfig)
{
    for (auto subNode = cur->children; subNode != nullptr; subNode = subNode->next) {
        PolicyAction policyAction;
        policyAction.actionName = reinterpret_cast<const char*>(subNode->name);
        xmlChar* xmlActionValue = xmlNodeGetContent(subNode);
        if (xmlActionValue != nullptr) {
            policyAction.actionValue = reinterpret_cast<char*>(xmlActionValue);
            THERMAL_HILOGD(LABEL_TEST,
                "policyAction.actionNodeName: %{public}s, policyAction.value:%{public}s",
                policyAction.actionName.c_str(), policyAction.actionValue.c_str());
            xmlFree(xmlActionValue);
        }

        if (subNode->properties == nullptr) {
            THERMAL_HILOGD(LABEL_TEST, "action prop is nullptr");
            policyAction.isProp = false;
            policyConfig.policyActionList.push_back(policyAction);
            continue;
        }
        for (auto actionProp = subNode->properties; actionProp != nullptr; actionProp = actionProp->next) {
            std::string propName = reinterpret_cast<const char*>(actionProp->name);
            xmlChar* xmlPropValue = xmlGetProp(subNode, actionProp->name);
            if (xmlPropValue != nullptr) {
                std::string propValue = reinterpret_cast<char*>(xmlPropValue);
                THERMAL_HILOGD(LABEL_TEST, "propName.name: %{public}s, propValue:%{public}s",
                    propName.c_str(), propValue.c_str());
                policyAction.actionPropMap.emplace(std::pair(propName, propValue));
                xmlFree(xmlPropValue);
            }
            policyAction.isProp = true;
        }
        policyConfig.policyActionList.push_back(policyAction);
    }
}
} // namespace PowerMgr
} // namespace OHOS
