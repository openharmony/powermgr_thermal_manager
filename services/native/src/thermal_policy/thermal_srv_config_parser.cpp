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
        bi.tag = (char *)xmlGetProp(cur, BAD_CAST"tag");
        bi.value = (char *)xmlGetProp(cur, BAD_CAST"value");
        baseinfoMap.emplace(bi.tag, bi.value);
        cur = cur->next;
        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "tag: %{public}s, value:%{public}s",
            bi.tag.c_str(), bi.value.c_str());
    }
    g_service->GetBaseinfoObj()->SetBaseInfo(baseinfoMap);
}

void ThermalSrvConfigParser::ParseAuxSensorInfo(const xmlNode *cur, std::shared_ptr<ThermalConfigSensorCluster> &sc)
{
    AuxSensorInfoMap auxSensorLevelInfo;
    std::string tempRanges;
    auto auxSensor = xmlGetProp(cur, BAD_CAST"aux_sensor");
    if (auxSensor != nullptr) {
        std::vector<std::string> auxsensors;
        std::string auxsensor = (char *)auxSensor;
        sc->SetAuxFlag(true);
        StringOperation::SplitString(auxsensor, auxsensors, ",");
        for (uint32_t i = 0; i < auxsensors.size(); i++) {
            std::string sensorType = auxsensors[i];
            std::vector<AuxLevelItem> auxItems;
            if (auxsensors[i].empty()) continue;
            for (auto subNode = cur->children; subNode != nullptr; subNode = subNode->next) {
                if (subNode == nullptr) continue;
                AuxLevelItem auxlevelItem;
                std::string auxTriggerRange = (char *)xmlGetProp(subNode, BAD_CAST("aux_trigger_range"));
                if (!auxTriggerRange.empty()) {
                    std::vector<std::string> auxTempranges;
                    StringOperation::SplitString(auxTriggerRange, auxTempranges, ",");
                    if (auxsensors.size() > auxTempranges.size()) {
                        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "The auxiliary sensor does not match the \
                            threshold range");
                        break;
                    }
                    tempRanges = auxTempranges[i];
                }
                std::vector<std::string> tempRiseRanges;
                StringOperation::SplitString(tempRanges, tempRiseRanges, "_");
                auxlevelItem.lowerTemp = atoi(tempRiseRanges[0].c_str());
                auxlevelItem.upperTemp = atoi(tempRiseRanges[1].c_str());
                auxlevelItem.level = atoi((char *)xmlGetProp(subNode, BAD_CAST("level")));
                THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "sensorType: %{public}s, tempRanges: %{public}s",
                    sensorType.c_str(), tempRanges.c_str());
                THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "lowerTemp: %{public}d, upperTemp: %{public}d",
                    auxlevelItem.lowerTemp, auxlevelItem.upperTemp);
                auxItems.push_back(auxlevelItem);
            }
            auxSensorLevelInfo.emplace(std::pair(sensorType, auxItems));
        }
        sc->SetAuxSensorLevelInfo(auxSensorLevelInfo);
    }
}

void ThermalSrvConfigParser::ParseSensorInfo(const xmlNode *cur, std::shared_ptr<ThermalConfigSensorCluster> &sc)
{
    SensorInfoMap sensorLevelInfo;
    std::vector<std::string> sensors;
    StringOperation::SplitString((char *)xmlGetProp(cur, BAD_CAST"sensor"), sensors, ",");
    for (uint32_t i = 0; i < sensors.size(); i++) {
        std::string sensorType = sensors.at(i);
        std::vector<LevelItem> vItem;
        for (auto subNode = cur->children; subNode; subNode = subNode->next) {
            if (subNode == nullptr) continue;
            LevelItem levelItem;
            std::vector<std::string> thresholds;
            std::vector<std::string> thresholdClrs;
            std::vector<std::string> rates;
            StringOperation::SplitString((char *)xmlGetProp(subNode, BAD_CAST("threshold")), thresholds, ",");
            StringOperation::SplitString((char *)xmlGetProp(subNode, BAD_CAST("threshold_clr")),
                thresholdClrs, ",");
            if (sensors.size() > thresholds.size() || sensors.size() > thresholdClrs.size()) {
                THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "The sensor does not match the threshold range");
                break;
            }
            levelItem.level = atoi(((char *)xmlGetProp(subNode, BAD_CAST("level"))));
            levelItem.threshold = atoi(thresholds.at(i).c_str());
            levelItem.thresholdClr = atoi(thresholdClrs.at(i).c_str());
            auto tempRiseRates = xmlGetProp(subNode, BAD_CAST("temp_rise_rate"));
            if (tempRiseRates != nullptr) {
                sc->SetRateFlag(true);
                StringOperation::SplitString((char *)tempRiseRates, rates, ",");
                if (sensors.size() > rates.size()) {
                    break;
                }
                levelItem.tempRiseRate = atof(rates.at(i).c_str());
            }
            vItem.push_back(levelItem);
        }
        sensorLevelInfo.emplace(std::pair(sensorType, vItem));
    }
    sc->SetSensorLevelInfo(sensorLevelInfo);
}

void ThermalSrvConfigParser::ParseLevelNode(xmlNodePtr node)
{
    auto cur = node->xmlChildrenNode;
    std::map<std::string, std::shared_ptr<ThermalConfigSensorCluster>> msc;
    while (cur != nullptr) {
        std::shared_ptr<ThermalConfigSensorCluster> sc = std::make_shared<ThermalConfigSensorCluster>();
        std::string name = (char *) xmlGetProp(cur, BAD_CAST"name");

        ParseAuxSensorInfo(cur, sc);
        ParseSensorInfo(cur, sc);
        auto desc = xmlGetProp(cur, BAD_CAST("desc"));
        if (desc != nullptr) {
            std::string descValue = (char *)desc;
            if (atoi(descValue.c_str()) == 1) {
                sc->SetDescFlag(true);
            }
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
        si.name = (char *)xmlGetProp(cur, BAD_CAST"name");
        auto param = xmlGetProp(cur, BAD_CAST("param"));
        if (param != nullptr) {
            si.params =(char *)param;
            si.isExistParam = true;
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
            ai.name = (char *)xmlGetProp(cur, BAD_CAST"name");
            auto param = xmlGetProp(cur, BAD_CAST("param"));
            if (param != nullptr) {
                ai.params = (char *) param;
            }
            auto strict = xmlGetProp(cur, BAD_CAST("strict"));
            if (strict != nullptr) {
                std::string strictValue = (char *)strict;
                if (atoi(strictValue.c_str()) == 1) {
                    ai.strict = true;
                }
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
    ThermalPolicy::PolicyConfigMap clusterPolicyMap;
    std::vector<PolicyConfig> policyList;
    std::string clusterName;
    std::string diffName;
    while (cur != nullptr) {
        PolicyConfig policyConfig;
        clusterName = (char *)xmlGetProp(cur, BAD_CAST"name");
        uint32_t level = atoi((char *)xmlGetProp(cur, BAD_CAST"level"));

        policyConfig.level = level;
        THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "policyConfig.name: %{public}s, policyConfig.level:%{public}d",
            clusterName.c_str(), level);
        for (auto subNode = cur->children; subNode != nullptr; subNode = subNode->next) {
            if (subNode == nullptr) continue;
            PolicyAction policyAction;
            policyAction.actionName = (char *)subNode->name;
            policyAction.actionValue = (char *)xmlNodeGetContent(subNode);
            THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE,
                "policyAction.actionNodeName: %{public}s, policyAction.value:%{public}s",
                policyAction.actionName.c_str(), policyAction.actionValue.c_str());
            if (subNode->properties == nullptr) {
                THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "action prop is nullptr");
                policyAction.isProp = false;
            } else {
                for (auto actionProp = subNode->properties; actionProp != nullptr; actionProp = actionProp->next) {
                    std::string propName = (char *)actionProp->name;
                    std::string propValue = (char *)xmlGetProp(subNode, actionProp->name);
                    policyAction.mActionProp.emplace(std::pair(propName, propValue));
                    policyAction.isProp = true;
                    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "propName.name: %{public}s, propValue:%{public}s",
                        propName.c_str(), propValue.c_str());
                }
            }
            policyConfig.vPolicyAction.push_back(policyAction);
        }

        if (diffName != clusterName) {
            THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "diffName: %{public}s", diffName.c_str());
            clusterPolicyMap.insert(std::make_pair(diffName, policyList));
            diffName = clusterName;
            policyList.clear();
        }
        policyList.push_back(policyConfig);

        cur = cur->next;
    }
    clusterPolicyMap.insert(std::make_pair(clusterName, policyList));
    g_service->GetPolicy()->SetPolicyMap(clusterPolicyMap);
}
} // namespace PowerMgr
} // namespace OHOS