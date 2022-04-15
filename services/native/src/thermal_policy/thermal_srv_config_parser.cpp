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

#include "thermal_srv_config_parser.h"

#include "string_operation.h"
#include "thermal_common.h"
#include "thermal_service.h"

namespace OHOS {
namespace PowerMgr {
namespace {
auto g_service = DelayedSpSingleton<ThermalService>::GetInstance();
AuxSensorInfoMap g_auxSensorLevelInfo;
std::vector<PolicyConfig> g_policyList;
ThermalPolicy::PolicyConfigMap g_clusterPolicyMap;
}

ThermalSrvConfigParser::ThermalSrvConfigParser() {};

ThermalSrvConfigParser &ThermalSrvConfigParser::GetInstance()
{
    static ThermalSrvConfigParser instance;
    return instance;
}

bool ThermalSrvConfigParser::ThermalSrvConfigInit(std::string &path)
{
    ParseXMLFile(path);
    return true;
}

void ThermalSrvConfigParser::ParseXMLFile(std::string &path)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "ParseXMLFile: Enter");

    std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)> docPtr(
        xmlReadFile(path.c_str(), nullptr, XML_PARSE_NOBLANKS), xmlFreeDoc);
    if (docPtr == nullptr) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "ParseXMLFile::Init failed, read file failed.");
        return;
    }

    auto rootNode = xmlDocGetRootElement(docPtr.get());
    if (rootNode == nullptr) {
        THERMAL_HILOGE(MODULE_THERMALMGR_SERVICE, "ParseXMLFile::Get root node failed.");
        return;
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
        }
    }
}

void ThermalSrvConfigParser::ParseBaseNode(xmlNodePtr node)
{
    BaseInfoMap baseinfoMap;
    auto cur = node->xmlChildrenNode;
    while (cur != nullptr) {
        BaseItem bi;
        xmlChar* xmlTag = xmlGetProp(cur, BAD_CAST"tag");
        if (xmlTag != nullptr) {
            bi.tag = (char *)xmlTag;
            xmlFree(xmlTag);
        }

        xmlChar* xmlValue = xmlGetProp(cur, BAD_CAST"value");
        if (xmlValue != nullptr) {
            bi.value = (char *)xmlValue;
            xmlFree(xmlValue);
        }
        baseinfoMap.emplace(bi.tag, bi.value);
        cur = cur->next;
        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "tag: %{public}s, value:%{public}s",
            bi.tag.c_str(), bi.value.c_str());
    }
    g_service->GetBaseinfoObj()->SetBaseInfo(baseinfoMap);
}

void ThermalSrvConfigParser::ParseAuxSensorInfo(const xmlNode *cur, std::shared_ptr<ThermalConfigSensorCluster> &sc)
{
    xmlChar* auxSensor = xmlGetProp(cur, BAD_CAST"aux_sensor");
    if (auxSensor != nullptr) {
        std::vector<std::string> auxsensors;
        std::string auxsensor = (char *)auxSensor;
        sc->SetAuxFlag(true);
        StringOperation::SplitString(auxsensor, auxsensors, ",");
        for (uint32_t i = 0; i < auxsensors.size(); i++) {
            std::string sensorType = auxsensors[i];
            if (auxsensors[i].empty()) {
                continue;
            }
            ParseAuxSensorSubnodeInfo(cur, auxsensors, sensorType, i);
        }
        sc->SetAuxSensorLevelInfo(g_auxSensorLevelInfo);
        xmlFree(auxSensor);
    }
}

void ThermalSrvConfigParser::ParseAuxSensorSubnodeInfo(const xmlNode *cur, std::vector<std::string>& auxsensors,
    std::string& sensorType, const uint32_t i)
{
    std::string tempRanges;
    std::vector<AuxLevelItem> auxItems;
    for (auto subNode = cur->children; subNode != nullptr; subNode = subNode->next) {
        if (subNode == nullptr) {
            continue;
        }
        AuxLevelItem auxlevelItem;
        if (ParseAuxSensorSubnodeInfoTrigerRange(subNode, auxsensors, tempRanges, i) == false) {
            break;
        }

        std::vector<std::string> tempRiseRanges;
        StringOperation::SplitString(tempRanges, tempRiseRanges, "_");
        auxlevelItem.lowerTemp = atoi(tempRiseRanges[0].c_str());
        auxlevelItem.upperTemp = atoi(tempRiseRanges[1].c_str());
        xmlChar* xmlLevel = xmlGetProp(subNode, BAD_CAST("level"));
        if (xmlLevel != nullptr) {
            auxlevelItem.level = atoi((char *)xmlLevel);
            xmlFree(xmlLevel);
        }
        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "sensorType: %{public}s, tempRanges: %{public}s",
            sensorType.c_str(), tempRanges.c_str());
        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "lowerTemp: %{public}d, upperTemp: %{public}d",
            auxlevelItem.lowerTemp, auxlevelItem.upperTemp);
        auxItems.push_back(auxlevelItem);
        g_auxSensorLevelInfo.emplace(std::pair(sensorType, auxItems));
    }
}

bool ThermalSrvConfigParser::ParseAuxSensorSubnodeInfoTrigerRange(const xmlNode *subNode,
    std::vector<std::string>& auxsensors, std::string& tempRanges, const uint32_t i)
{
    xmlChar* xmlTriggerRange = xmlGetProp(subNode, BAD_CAST("aux_trigger_range"));
    if (xmlTriggerRange != nullptr) {
        std::string auxTriggerRange = (char *)xmlTriggerRange;
        if (!auxTriggerRange.empty()) {
            std::vector<std::string> auxTempranges;
            StringOperation::SplitString(auxTriggerRange, auxTempranges, ",");
            if (auxsensors.size() > auxTempranges.size()) {
                THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "The auxiliary sensor does not match the \
                    threshold range");
                return false;
            }
            tempRanges = auxTempranges[i];
        }
        xmlFree(xmlTriggerRange);
    }
    return true;
}

void ThermalSrvConfigParser::ParseSensorInfo(const xmlNode *cur, std::shared_ptr<ThermalConfigSensorCluster> &sc)
{
    SensorInfoMap sensorLevelInfo;
    std::vector<std::string> sensors;
    xmlChar* xmlSensor = xmlGetProp(cur, BAD_CAST"sensor");
    if (xmlSensor != nullptr) {
        StringOperation::SplitString((char *)xmlSensor, sensors, ",");
        for (uint32_t i = 0; i < sensors.size(); i++) {
            std::string sensorType = sensors.at(i);
            std::vector<LevelItem> vItem;
            ParseSensorSubnodeInfo(cur, vItem, sensors, i, sc);
            sensorLevelInfo.emplace(std::pair(sensorType, vItem));
        }
        sc->SetSensorLevelInfo(sensorLevelInfo);
        xmlFree(xmlSensor);
    }
}

void ThermalSrvConfigParser::ParseSensorSubnodeInfo(const xmlNode *cur, std::vector<LevelItem>& vItem,
    std::vector<std::string>& sensors, const uint32_t i, std::shared_ptr<ThermalConfigSensorCluster> &sc)
{
    for (auto subNode = cur->children; subNode; subNode = subNode->next) {
        if (subNode == nullptr) continue;
        LevelItem levelItem;
        std::vector<std::string> thresholds;
        std::vector<std::string> thresholdClrs;
        std::vector<std::string> rates;
        xmlChar* xmlThreshold = xmlGetProp(subNode, BAD_CAST("threshold"));
        if (xmlThreshold != nullptr) {
            StringOperation::SplitString((char *)xmlThreshold, thresholds, ",");
            xmlFree(xmlThreshold);
        }
        xmlChar* xmlThresholdClr = xmlGetProp(subNode, BAD_CAST("threshold_clr"));
        if (xmlThresholdClr != nullptr) {
            StringOperation::SplitString((char *)xmlThresholdClr, thresholdClrs, ",");
            xmlFree(xmlThresholdClr);
        }
        if (sensors.size() > thresholds.size() || sensors.size() > thresholdClrs.size()) {
            THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "The sensor does not match the threshold range");
            break;
        }
        xmlChar* xmlLevel = xmlGetProp(subNode, BAD_CAST("level"));
        if (xmlLevel != nullptr) {
            levelItem.level = static_cast<uint32_t>(atoi(((char *)xmlLevel)));
            xmlFree(xmlLevel);
        }

        levelItem.threshold = atoi(thresholds.at(i).c_str());
        levelItem.thresholdClr = atoi(thresholdClrs.at(i).c_str());
        xmlChar* tempRiseRates = xmlGetProp(subNode, BAD_CAST("temp_rise_rate"));
        if (tempRiseRates != nullptr) {
            sc->SetRateFlag(true);
            StringOperation::SplitString((char *)tempRiseRates, rates, ",");
            if (sensors.size() > rates.size()) {
                break;
            }
            levelItem.tempRiseRate = atof(rates.at(i).c_str());
        }
        vItem.push_back(levelItem);
        xmlFree(tempRiseRates);
    }
}

void ThermalSrvConfigParser::ParseLevelNode(xmlNodePtr node)
{
    auto cur = node->xmlChildrenNode;
    std::map<std::string, std::shared_ptr<ThermalConfigSensorCluster>> msc;
    while (cur != nullptr) {
        std::string name;
        std::shared_ptr<ThermalConfigSensorCluster> sc = std::make_shared<ThermalConfigSensorCluster>();
        xmlChar* xmlName = xmlGetProp(cur, BAD_CAST"name");
        if (xmlName != nullptr) {
            name = (char *)xmlName;
            xmlFree(xmlName);
        }
        ParseAuxSensorInfo(cur, sc);
        ParseSensorInfo(cur, sc);
        xmlChar* desc = xmlGetProp(cur, BAD_CAST("desc"));
        if (desc != nullptr) {
            std::string descValue = (char *)desc;
            if (atoi(descValue.c_str()) == 1) {
                sc->SetDescFlag(true);
            }
            xmlFree(desc);
        }
        msc.emplace(std::pair(name, sc));
        cur = cur->next;
    }
    g_service->GetPolicy()->SetSensorClusterMap(msc);
}

void ThermalSrvConfigParser::ParseStateNode(xmlNodePtr node)
{
    auto cur = node->xmlChildrenNode;
    std::vector<StateItem> vstateItem;
    while (cur != nullptr) {
        StateItem si;
        xmlChar* xmlName = xmlGetProp(cur, BAD_CAST"name");
        if (xmlName != nullptr) {
            si.name = (char *)xmlName;
            xmlFree(xmlName);
        }

        xmlChar* param = xmlGetProp(cur, BAD_CAST("param"));
        if (param != nullptr) {
            si.params =(char *)param;
            si.isExistParam = true;
            xmlFree(param);
        }
        vstateItem.push_back(si);
        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "si.name: %{public}s, si.params %{public}s",
            si.name.c_str(), si.params.c_str());
        cur = cur->next;
    }
    g_service->GetStateMachineObj()->SetStateItem(vstateItem);
}

void ThermalSrvConfigParser::ParseActionNode(xmlNodePtr node)
{
    auto cur = node->xmlChildrenNode;
    std::vector<ActionItem> vActionItem;
    while (cur != nullptr) {
        if (!xmlStrcmp(cur->name, BAD_CAST"item")) {
            ActionItem ai;
            xmlChar* xmlName = xmlGetProp(cur, BAD_CAST"name");
            if (xmlName != nullptr) {
                ai.name = (char *)xmlName;
                xmlFree(xmlName);
            }

            xmlChar* param = xmlGetProp(cur, BAD_CAST("param"));
            if (param != nullptr) {
                ai.params = (char *) param;
                xmlFree(param);
            }
            xmlChar* strict = xmlGetProp(cur, BAD_CAST("strict"));
            if (strict != nullptr) {
                std::string strictValue = (char *)strict;
                if (atoi(strictValue.c_str()) == 1) {
                    ai.strict = true;
                }
                xmlFree(strict);
            }
            THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE,
                "ai.name: %{public}s, ai.strict: %{public}d, ai.params: %{public}s",
                ai.name.c_str(), ai.strict, ai.params.c_str());
            vActionItem.push_back(ai);
        }
        cur = cur->next;
    }
    g_service->GetActionManagerObj()->SetActionItem(vActionItem);
}

void ThermalSrvConfigParser::ParsePolicyNode(xmlNodePtr node)
{
    auto cur = node->xmlChildrenNode;
    std::string clusterName;
    std::string diffName;
    while (cur != nullptr) {
        PolicyConfig policyConfig;
        xmlChar* xmlName = xmlGetProp(cur, BAD_CAST"name");
        if (xmlName != nullptr) {
            clusterName = (char *)xmlName;
            xmlFree(xmlName);
        }

        xmlChar* xmlLevel = xmlGetProp(cur, BAD_CAST"level");
        if (xmlLevel != nullptr) {
            uint32_t level = static_cast<uint32_t>(atoi((char *)xmlLevel));
            policyConfig.level = level;
            THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "policyConfig.name: %{public}s, policyConfig.level:%{public}d",
                clusterName.c_str(), level);
        }

        ParsePolicySubnode(cur, policyConfig);

        if (diffName != clusterName) {
            THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "diffName: %{public}s", diffName.c_str());
            g_clusterPolicyMap.insert(std::make_pair(diffName, g_policyList));
            diffName = clusterName;
            g_policyList.clear();
        }
        g_policyList.push_back(policyConfig);

        cur = cur->next;
    }
    g_clusterPolicyMap.insert(std::make_pair(clusterName, g_policyList));
    g_service->GetPolicy()->SetPolicyMap(g_clusterPolicyMap);
}

void ThermalSrvConfigParser::ParsePolicySubnode(const xmlNode *cur, PolicyConfig& policyConfig)
{
    for (auto subNode = cur->children; subNode != nullptr; subNode = subNode->next) {
        if (subNode == nullptr) continue;
        PolicyAction policyAction;
        policyAction.actionName = (char *)subNode->name;
        xmlChar* xmlactionValue = xmlNodeGetContent(subNode);
        if (xmlactionValue != nullptr) {
        policyAction.actionValue = (char *)xmlactionValue;
        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE,
            "policyAction.actionNodeName: %{public}s, policyAction.value:%{public}s",
            policyAction.actionName.c_str(), policyAction.actionValue.c_str());
            xmlFree(xmlactionValue);
        }

        if (subNode->properties == nullptr) {
            THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "action prop is nullptr");
            policyAction.isProp = false;
            policyConfig.vPolicyAction.push_back(policyAction);
            continue;
        }
        for (auto actionProp = subNode->properties; actionProp != nullptr; actionProp = actionProp->next) {
            std::string propName = (char *)actionProp->name;
            xmlChar* xmlPropValue = xmlGetProp(subNode, actionProp->name);
            if (xmlPropValue != nullptr) {
                std::string propValue = (char *)xmlPropValue;
                THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "propName.name: %{public}s, propValue:%{public}s",
                    propName.c_str(), propValue.c_str());
                policyAction.mActionProp.emplace(std::pair(propName, propValue));
                xmlFree(xmlPropValue);
            }
            policyAction.isProp = true;
        }
        policyConfig.vPolicyAction.push_back(policyAction);
    }
}
} // namespace PowerMgr
} // namespace OHOS
