/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#include "string_ex.h"

namespace OHOS {
namespace PowerMgr {
namespace {
constexpr uint32_t AUX_SENSOR_RANGE_LEN = 2;
const std::string TRUE_STR = "1";
}

ThermalSrvConfigParser::ThermalSrvConfigParser() {};

bool ThermalSrvConfigParser::ThermalSrvConfigInit(const std::string& path)
{
    if (ParseXmlFile(path)) {
        return true;
    }
    return false;
}

bool ThermalSrvConfigParser::ParseXmlFile(const std::string& path)
{
    std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)> docPtr(
        xmlReadFile(path.c_str(), nullptr, XML_PARSE_NOBLANKS), xmlFreeDoc);
    if (docPtr == nullptr) {
        THERMAL_HILOGE(COMP_SVC, "init failed, read file failed");
        return false;
    }

    auto rootNode = xmlDocGetRootElement(docPtr.get());
    if (rootNode == nullptr) {
        THERMAL_HILOGE(COMP_SVC, "get root node failed");
        return false;
    }

    for (xmlNodePtr node = rootNode->xmlChildrenNode; node != nullptr; node = node->next) {
        if (!ParseRootNode(node)) {
            return false;
        }
    }
    return true;
}

bool ThermalSrvConfigParser::ParseRootNode(const xmlNodePtr& node)
{
    bool ret = false;
    if (!xmlStrcmp(node->name, BAD_CAST"base")) {
        ret = ParseBaseNode(node);
    } else if (!xmlStrcmp(node->name, BAD_CAST"level")) {
        ret = ParseLevelNode(node);
    } else if (!xmlStrcmp(node->name, BAD_CAST"state")) {
        ret = ParseStateNode(node);
    } else if (!xmlStrcmp(node->name, BAD_CAST"action")) {
        ret = ParseActionNode(node);
    } else if (!xmlStrcmp(node->name, BAD_CAST"policy")) {
        ret = ParsePolicyNode(node);
    } else if (!xmlStrcmp(node->name, BAD_CAST"idle")) {
        ret = ParseIdleNode(node);
    } else if (!xmlStrcmp(node->name, BAD_CAST"fan")) {
        ret = ParseFanNode(node);
    } else if (!xmlStrcmp(node->name, BAD_CAST"comment")) {
        ret = true;
    } else {
        THERMAL_HILOGE(COMP_SVC, "unknown root node %{public}s",
            reinterpret_cast<const char*>(node->name));
    }
    return ret;
}

bool ThermalSrvConfigParser::ParseBaseNode(const xmlNodePtr& node)
{
    BaseInfoMap baseInfoMap;
    xmlNodePtr cur = node->xmlChildrenNode;
    while (cur != nullptr) {
        if (xmlStrcmp(cur->name, BAD_CAST"item")) {
            cur = cur->next;
            continue;
        }
        BaseItem bi;
        xmlChar* xmlTag = xmlGetProp(cur, BAD_CAST"tag");
        if (xmlTag != nullptr) {
            bi.tag = reinterpret_cast<char*>(xmlTag);
            xmlFree(xmlTag);
        } else {
            THERMAL_HILOGE(COMP_SVC, "base tag is null");
            return false;
        }

        xmlChar* xmlValue = xmlGetProp(cur, BAD_CAST"value");
        if (xmlValue != nullptr) {
            bi.value = reinterpret_cast<char*>(xmlValue);
            xmlFree(xmlValue);
        } else {
            THERMAL_HILOGE(COMP_SVC, "base value is null, tag: %{public}s", bi.tag.c_str());
            return false;
        }
        baseInfoMap.emplace(bi.tag, bi.value);
        cur = cur->next;
        THERMAL_HILOGD(COMP_SVC, "tag: %{public}s, value: %{public}s", bi.tag.c_str(), bi.value.c_str());
    }
    auto tms = ThermalService::GetInstance();
    tms->GetBaseinfoObj()->SetBaseInfo(baseInfoMap);
    auto iter = baseInfoMap.find("sim_tz");
    if (iter != baseInfoMap.end()) {
        if (iter->second == "") {
            tms->SetSimulationXml(false);
            return true;
        }
        if (iter->second == "0" || iter->second == "1") {
            tms->SetSimulationXml(static_cast<bool>(std::stoi(iter->second)));
            return true;
        }
        return false;
    }
    tms->SetSimulationXml(false);
    return true;
}

bool ThermalSrvConfigParser::ParseLevelNode(const xmlNodePtr& node)
{
    xmlNodePtr cur = node->xmlChildrenNode;
    SensorClusterMap msc;
    while (cur != nullptr) {
        if (xmlStrcmp(cur->name, BAD_CAST"sensor_cluster")) {
            cur = cur->next;
            continue;
        }
        std::string name;
        SensorClusterPtr sc = std::make_shared<ThermalConfigSensorCluster>();
        xmlChar* xmlName = xmlGetProp(cur, BAD_CAST"name");
        if (xmlName != nullptr) {
            name = reinterpret_cast<char*>(xmlName);
            xmlFree(xmlName);
        } else {
            THERMAL_HILOGE(COMP_SVC, "level cluster name is null");
            return false;
        }
        if (!ParseLevelState(cur, sc) || !ParseAuxSensorInfo(cur, sc) || !ParseSensorInfo(cur, sc)) {
            THERMAL_HILOGE(COMP_SVC, "parse sensor info failed, cluster name: %{public}s", name.c_str());
            return false;
        }
        xmlChar* desc = xmlGetProp(cur, BAD_CAST("desc"));
        if (desc != nullptr) {
            if (TrimStr(reinterpret_cast<char*>(desc)) == TRUE_STR) {
                sc->SetDescFlag(true);
                THERMAL_HILOGD(COMP_SVC, "cluster [%{public}s] is desc", name.c_str());
            }
            xmlFree(desc);
        }
        if (!sc->CheckStandard()) {
            THERMAL_HILOGE(COMP_SVC, "cluster [%{public}s] some config error", name.c_str());
            return false;
        }
        msc.emplace(std::pair(name, sc));
        cur = cur->next;
    }
    auto tms = ThermalService::GetInstance();
    tms->GetPolicy()->SetSensorClusterMap(msc);
    return true;
}

bool ThermalSrvConfigParser::ParseLevelState(const xmlNodePtr& cur, SensorClusterPtr& sc)
{
    for (xmlNodePtr subNode = cur->xmlChildrenNode; subNode != nullptr; subNode = subNode->next) {
        if (xmlStrcmp(subNode->name, BAD_CAST"state") || (subNode->properties == nullptr)) {
            continue;
        }
        for (auto levStateProp = subNode->properties; levStateProp != nullptr; levStateProp = levStateProp->next) {
            std::string propName = reinterpret_cast<const char*>(levStateProp->name);
            xmlChar* xmlPropValue = xmlGetProp(subNode, levStateProp->name);
            if (xmlPropValue != nullptr) {
                std::string propValue = reinterpret_cast<char*>(xmlPropValue);
                THERMAL_HILOGD(COMP_SVC, "level state: %{public}s, value: %{public}s",
                    propName.c_str(), propValue.c_str());
                sc->AddState(propName, propValue);
                xmlFree(xmlPropValue);
            } else {
                THERMAL_HILOGE(COMP_SVC, "level state prop [%{public}s] value is null", propName.c_str());
                return false;
            }
        }
    }
    return true;
}

bool ThermalSrvConfigParser::ParseAuxSensorInfo(const xmlNodePtr& cur, SensorClusterPtr& sc)
{
    xmlChar* auxSensor = xmlGetProp(cur, BAD_CAST"aux_sensor");
    if (auxSensor != nullptr) {
        std::vector<std::string> auxsensors;
        AuxSensorInfoMap auxSensorLevelInfo;
        std::string auxsensor = reinterpret_cast<char*>(auxSensor);
        sc->SetAuxFlag(true);
        StringOperation::SplitString(auxsensor, auxsensors, ",");
        for (uint32_t i = 0; i < auxsensors.size(); i++) {
            std::vector<AuxLevelItem> auxLevelItems;
            std::string sensorType = auxsensors[i];
            if (sensorType.empty()) {
                THERMAL_HILOGE(COMP_SVC, "aux sensor type is empty");
                return false;
            }
            THERMAL_HILOGD(COMP_SVC, "parse aux sensor [%{public}s] item:", sensorType.c_str());
            if (!ParseAuxSensorLevInfo(cur, auxsensors, i, auxLevelItems)) {
                THERMAL_HILOGE(COMP_SVC, "parse aux sensor [%{public}s] sub node failed", sensorType.c_str());
                return false;
            }
            if (auxLevelItems.empty()) {
                THERMAL_HILOGE(COMP_SVC, "aux sensor [%{public}s] level info is empty", sensorType.c_str());
                return false;
            }
            auxSensorLevelInfo.emplace(sensorType, auxLevelItems);
        }
        sc->SetAuxSensorLevelInfo(auxSensorLevelInfo);
        xmlFree(auxSensor);
    }
    return true;
}

bool ThermalSrvConfigParser::ParseAuxSensorLevInfo(const xmlNodePtr& cur,
    std::vector<std::string>& auxsensors, const uint32_t sensorIdx, std::vector<AuxLevelItem>& auxLevelItems)
{
    for (xmlNodePtr subNode = cur->xmlChildrenNode; subNode != nullptr; subNode = subNode->next) {
        if (xmlStrcmp(subNode->name, BAD_CAST"item")) {
            continue;
        }
        std::string tempRangeStr;
        AuxLevelItem auxlevelItem;
        if (!ParseAuxSensorTriggerRange(subNode, auxsensors, tempRangeStr, sensorIdx)) {
            THERMAL_HILOGE(COMP_SVC, "parse aux sensor range failed");
            return false;
        }

        std::vector<std::string> tempRanges;
        StringOperation::SplitString(tempRangeStr, tempRanges, "_");
        if (static_cast<uint32_t>(tempRanges.size()) < AUX_SENSOR_RANGE_LEN) {
            THERMAL_HILOGE(COMP_SVC, "aux sensor temp range split failed");
            return false;
        }
        const int32_t INDEX0 = 0;
        const int32_t INDEX1 = 1;
        StrToInt(tempRanges[INDEX0], auxlevelItem.lowerTemp);
        StrToInt(tempRanges[INDEX1], auxlevelItem.upperTemp);
        xmlChar* xmlLevel = xmlGetProp(subNode, BAD_CAST("level"));
        if (xmlLevel != nullptr) {
            StrToInt(reinterpret_cast<char*>(xmlLevel), auxlevelItem.level);
            xmlFree(xmlLevel);
        } else {
            THERMAL_HILOGE(COMP_SVC, "aux sensor level is null");
            return false;
        }
        THERMAL_HILOGD(COMP_SVC, "lowerTemp: %{public}d, upperTemp: %{public}d",
            auxlevelItem.lowerTemp, auxlevelItem.upperTemp);
        auxLevelItems.push_back(auxlevelItem);
    }
    return true;
}

bool ThermalSrvConfigParser::ParseAuxSensorTriggerRange(const xmlNodePtr& subNode,
    std::vector<std::string>& auxsensors, std::string& tempRangeStr, const uint32_t sensorIdx)
{
    xmlChar* xmlTriggerRange = xmlGetProp(subNode, BAD_CAST("aux_trigger_range"));
    if (xmlTriggerRange != nullptr) {
        std::string auxTriggerRange = reinterpret_cast<char*>(xmlTriggerRange);
        if (!auxTriggerRange.empty()) {
            std::vector<std::string> auxTempranges;
            StringOperation::SplitString(auxTriggerRange, auxTempranges, ",");
            if (auxsensors.size() > auxTempranges.size()) {
                THERMAL_HILOGE(COMP_SVC, "aux sensor size (%{public}zu) don't match range size (%{public}zu)",
                    auxsensors.size(), auxTempranges.size());
                xmlFree(xmlTriggerRange);
                return false;
            }
            tempRangeStr = auxTempranges[sensorIdx];
        } else {
            THERMAL_HILOGE(COMP_SVC, "aux sensor trigger range is empty");
            return false;
        }
        xmlFree(xmlTriggerRange);
    } else {
        THERMAL_HILOGE(COMP_SVC, "aux sensor trigger range is null");
        return false;
    }
    return true;
}

bool ThermalSrvConfigParser::ParseSensorInfo(const xmlNodePtr& cur, SensorClusterPtr& sc)
{
    SensorInfoMap sensorLevelInfo;
    std::vector<std::string> sensors;
    xmlChar* xmlSensor = xmlGetProp(cur, BAD_CAST"sensor");
    if (xmlSensor != nullptr) {
        StringOperation::SplitString(reinterpret_cast<char*>(xmlSensor), sensors, ",");
        if (sensors.empty()) {
            THERMAL_HILOGE(COMP_SVC, "sensor type is empty");
            return false;
        }
        for (uint32_t i = 0; i < sensors.size(); i++) {
            std::string sensorType = sensors.at(i);
            std::vector<LevelItem> levelItems;
            if (!ParseSensorLevelInfo(cur, levelItems, sensors, i, sc)) {
                THERMAL_HILOGE(COMP_SVC, "parse sensor [%{public}s] level failed", sensorType.c_str());
                return false;
            }
            if (levelItems.empty()) {
                THERMAL_HILOGE(COMP_SVC, "sensor [%{public}s] level info is empty", sensorType.c_str());
                return false;
            }
            sensorLevelInfo.emplace(std::pair(sensorType, levelItems));
        }
        sc->SetSensorLevelInfo(sensorLevelInfo);
        xmlFree(xmlSensor);
    } else {
        THERMAL_HILOGE(COMP_SVC, "sensor type is null");
        return false;
    }
    return true;
}

bool ThermalSrvConfigParser::ParseSensorLevelInfo(const xmlNodePtr& cur, std::vector<LevelItem>& levelItems,
    std::vector<std::string>& sensors, const uint32_t sensorIdx, SensorClusterPtr& sc)
{
    for (xmlNodePtr subNode = cur->xmlChildrenNode; subNode != nullptr; subNode = subNode->next) {
        if (xmlStrcmp(subNode->name, BAD_CAST"item")) {
            continue;
        }
        LevelItem levelItem;
        if (!ParseLevelThreshold(subNode, levelItem, sensors, sensorIdx)) {
            THERMAL_HILOGE(COMP_SVC, "parse level threshold failed");
            return false;
        }

        xmlChar* tempRiseRates = xmlGetProp(subNode, BAD_CAST("temp_rise_rate"));
        if (tempRiseRates != nullptr) {
            std::vector<std::string> rates;
            sc->SetRateFlag(true);
            StringOperation::SplitString(reinterpret_cast<char*>(tempRiseRates), rates, ",");
            if (sensors.size() != rates.size()) {
                THERMAL_HILOGE(COMP_SVC, "sensor size (%{public}zu) don't match rise rate (%{public}zu)",
                    sensors.size(), rates.size());
                xmlFree(tempRiseRates);
                return false;
            }
            StringOperation::StrToDouble(rates.at(sensorIdx), levelItem.tempRiseRate);
        }
        levelItems.push_back(levelItem);
        xmlFree(tempRiseRates);
    }
    return true;
}

bool ThermalSrvConfigParser::ParseLevelThreshold(const xmlNodePtr& subNode, LevelItem& levelItem,
    std::vector<std::string>& sensors, const uint32_t sensorIdx)
{
    std::vector<std::string> thresholds;
    std::vector<std::string> thresholdClrs;
    xmlChar* xmlThreshold = xmlGetProp(subNode, BAD_CAST("threshold"));
    if (xmlThreshold != nullptr) {
        StringOperation::SplitString(reinterpret_cast<char*>(xmlThreshold), thresholds, ",");
        xmlFree(xmlThreshold);
    } else {
        THERMAL_HILOGE(COMP_SVC, "threshold is null");
        return false;
    }
    xmlChar* xmlThresholdClr = xmlGetProp(subNode, BAD_CAST("threshold_clr"));
    if (xmlThresholdClr != nullptr) {
        StringOperation::SplitString(reinterpret_cast<char*>(xmlThresholdClr), thresholdClrs, ",");
        xmlFree(xmlThresholdClr);
    } else {
        THERMAL_HILOGE(COMP_SVC, "threshold_clr is null");
        return false;
    }
    if (sensors.size() != thresholds.size() || sensors.size() != thresholdClrs.size()) {
        THERMAL_HILOGE(COMP_SVC,
            "sensor size (%{public}zu) don't match threshold (%{public}zu) or clr (%{public}zu)",
            sensors.size(), thresholds.size(), thresholdClrs.size());
        return false;
    }
    xmlChar* xmlLevel = xmlGetProp(subNode, BAD_CAST("level"));
    if (xmlLevel != nullptr) {
        StringOperation::StrToUint(reinterpret_cast<char*>(xmlLevel), levelItem.level);
        xmlFree(xmlLevel);
    } else {
        THERMAL_HILOGE(COMP_SVC, "level is null");
        return false;
    }

    StrToInt(thresholds.at(sensorIdx), levelItem.threshold);
    StrToInt(thresholdClrs.at(sensorIdx), levelItem.thresholdClr);
    return true;
}

bool ThermalSrvConfigParser::ParseStateNode(const xmlNodePtr& node)
{
    auto cur = node->xmlChildrenNode;
    std::vector<StateItem> stateItems;
    while (cur != nullptr) {
        if (xmlStrcmp(cur->name, BAD_CAST"item")) {
            cur = cur->next;
            continue;
        }
        StateItem si;
        xmlChar* xmlName = xmlGetProp(cur, BAD_CAST"name");
        if (xmlName != nullptr) {
            si.name = reinterpret_cast<char*>(xmlName);
            xmlFree(xmlName);
        } else {
            THERMAL_HILOGE(COMP_SVC, "state name is null");
            return false;
        }

        xmlChar* param = xmlGetProp(cur, BAD_CAST("param"));
        if (param != nullptr) {
            si.params = reinterpret_cast<char*>(param);
            si.isExistParam = true;
            xmlFree(param);
        }
        stateItems.push_back(si);
        THERMAL_HILOGD(COMP_SVC, "state: %{public}s, params: %{public}s", si.name.c_str(), si.params.c_str());
        cur = cur->next;
    }
    auto tms = ThermalService::GetInstance();
    tms->GetStateMachineObj()->SetStateItem(stateItems);
    return true;
}

bool ThermalSrvConfigParser::ParseActionNode(const xmlNodePtr& node)
{
    auto cur = node->xmlChildrenNode;
    std::vector<ActionItem> actionItems;
    while (cur != nullptr) {
        if (xmlStrcmp(cur->name, BAD_CAST"item")) {
            cur = cur->next;
            continue;
        }

        ActionItem ai;
        xmlChar* xmlName = xmlGetProp(cur, BAD_CAST"name");
        if (xmlName != nullptr) {
            ai.name = reinterpret_cast<char*>(xmlName);
            xmlFree(xmlName);
        } else {
            THERMAL_HILOGE(COMP_SVC, "action name is null");
            return false;
        }

        ParseActionInfo(cur, ai);
        actionItems.push_back(ai);
        cur = cur->next;
    }
    auto tms = ThermalService::GetInstance();
    tms->GetActionManagerObj()->SetActionItem(actionItems);
    return true;
}

bool ThermalSrvConfigParser::ParseActionInfo(const xmlNodePtr& cur, ActionItem& ai)
{
    xmlChar* param = xmlGetProp(cur, BAD_CAST("param"));
    if (param != nullptr) {
        ai.params = reinterpret_cast<char*>(param);
        xmlFree(param);
    }
    xmlChar* uid = xmlGetProp(cur, BAD_CAST("uid"));
    if (uid != nullptr) {
        ai.uid = reinterpret_cast<char*>(uid);
        xmlFree(uid);
    }
    xmlChar* protocol = xmlGetProp(cur, BAD_CAST("protocol"));
    if (protocol != nullptr) {
        ai.protocol = reinterpret_cast<char*>(protocol);
        xmlFree(protocol);
    }
    xmlChar* strict = xmlGetProp(cur, BAD_CAST("strict"));
    if (strict != nullptr) {
        ai.strict = (TrimStr(reinterpret_cast<char*>(strict)) == TRUE_STR);
        xmlFree(strict);
    }
    xmlChar* event = xmlGetProp(cur, BAD_CAST("event"));
    if (event != nullptr) {
        ai.enableEvent = (TrimStr(reinterpret_cast<char*>(event)) == TRUE_STR);
        xmlFree(event);
    }
    THERMAL_HILOGD(COMP_SVC,
        "ai.name: %{public}s, ai.strict: %{public}d, ai.params: %{public}s, ai.uid: %{public}s,"    \
        "ai.strict: %{public}s, ai.enableEvent: %{public}d",
        ai.name.c_str(), ai.strict, ai.params.c_str(), ai.uid.c_str(), ai.protocol.c_str(), ai.enableEvent);
    return true;
}

bool ThermalSrvConfigParser::ParsePolicyNode(const xmlNodePtr& node)
{
    auto cur = node->xmlChildrenNode;
    ThermalPolicy::PolicyConfigMap clusterPolicyMap;
    while (cur != nullptr) {
        if (xmlStrcmp(cur->name, BAD_CAST"config")) {
            cur = cur->next;
            continue;
        }
        PolicyConfig policyConfig;
        std::string clusterName;
        xmlChar* xmlName = xmlGetProp(cur, BAD_CAST"name");
        if (xmlName != nullptr) {
            clusterName = reinterpret_cast<char*>(xmlName);
            xmlFree(xmlName);
        } else {
            THERMAL_HILOGE(COMP_SVC, "policy config name is null");
            return false;
        }

        xmlChar* xmlLevel = xmlGetProp(cur, BAD_CAST"level");
        if (xmlLevel != nullptr) {
            StringOperation::StrToUint(reinterpret_cast<char*>(xmlLevel), policyConfig.level);
            THERMAL_HILOGD(COMP_SVC, "policyConfig.name: %{public}s, policyConfig.level: %{public}u",
                clusterName.c_str(), policyConfig.level);
            xmlFree(xmlLevel);
        } else {
            THERMAL_HILOGE(COMP_SVC, "policy [%{public}s] level is null", clusterName.c_str());
            return false;
        }

        ParsePolicyActionInfo(cur, policyConfig);

        const auto& clusterIter = clusterPolicyMap.find(clusterName);
        if (clusterIter == clusterPolicyMap.end()) {
            std::vector<PolicyConfig> policyList;
            policyList.push_back(policyConfig);
            clusterPolicyMap.emplace(clusterName, policyList);
        } else {
            clusterIter->second.push_back(policyConfig);
        }
        cur = cur->next;
    }
    auto tms = ThermalService::GetInstance();
    tms->GetPolicy()->SetPolicyMap(clusterPolicyMap);
    return true;
}

bool ThermalSrvConfigParser::ParsePolicyActionInfo(const xmlNodePtr& cur, PolicyConfig& policyConfig)
{
    for (xmlNodePtr subNode = cur->xmlChildrenNode; subNode != nullptr; subNode = subNode->next) {
        if (!xmlStrcmp(subNode->name, BAD_CAST"comment")) {
            continue;
        }
        PolicyAction policyAction;
        policyAction.actionName = reinterpret_cast<const char*>(subNode->name);
        xmlChar* actionValue = xmlNodeGetContent(subNode);
        if (actionValue != nullptr) {
            policyAction.actionValue = reinterpret_cast<char*>(actionValue);
            THERMAL_HILOGD(COMP_SVC,
                "policyAction.actionNodeName: %{public}s, policyAction.value: %{public}s",
                policyAction.actionName.c_str(), policyAction.actionValue.c_str());
            xmlFree(actionValue);
        } else {
            THERMAL_HILOGE(COMP_SVC, "action [%{public}s] value is null", policyAction.actionName.c_str());
            return false;
        }

        if (subNode->properties == nullptr) {
            policyAction.isProp = false;
            policyConfig.policyActionList.push_back(policyAction);
            continue;
        }
        for (auto actionProp = subNode->properties; actionProp != nullptr; actionProp = actionProp->next) {
            std::string propName = reinterpret_cast<const char*>(actionProp->name);
            xmlChar* xmlPropValue = xmlGetProp(subNode, actionProp->name);
            if (xmlPropValue != nullptr) {
                std::string propValue = reinterpret_cast<char*>(xmlPropValue);
                THERMAL_HILOGD(COMP_SVC, "propName.name: %{public}s, propValue:%{public}s",
                    propName.c_str(), propValue.c_str());
                policyAction.actionPropMap.emplace(std::pair(propName, propValue));
                xmlFree(xmlPropValue);
            } else {
                THERMAL_HILOGE(COMP_SVC, "prop [%{public}s] value is null", propName.c_str());
                return false;
            }
            policyAction.isProp = true;
        }
        policyConfig.policyActionList.push_back(policyAction);
    }
    return true;
}

bool ThermalSrvConfigParser::ParseIdleNode(const xmlNodePtr& node)
{
    IdleState idleState;
    for (xmlNodePtr subNode = node->xmlChildrenNode; subNode != nullptr; subNode = subNode->next) {
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
            THERMAL_HILOGD(COMP_SVC, "not supported node, name=%{public}s", subNode->name);
        }
    }
    THERMAL_HILOGI(COMP_SVC, "level=%{public}d, soc=%{public}d, charging=%{public}d, current=%{public}d",
                   idleState.level, idleState.soc, idleState.charging, idleState.current);
    auto tms = ThermalService::GetInstance();
    tms->GetStateMachineObj()->SetIdleStateConfig(idleState);
    return true;
}

bool ThermalSrvConfigParser::ParseFanNode(const xmlNodePtr &node)
{
    FanFaultInfoMap fanFaultInfoMap;
    xmlNodePtr cur = node->xmlChildrenNode;

    while (cur != nullptr) {
        if (xmlStrcmp(cur->name, BAD_CAST"sensor_cluster")) {
            cur = cur->next;
            continue;
        }
        xmlChar* xmlSensor = xmlGetProp(cur, BAD_CAST"sensor");
        if (xmlSensor == nullptr) {
            return false;
        }
        std::vector<std::string> sensors;
        std::string sensorStr = reinterpret_cast<char*>(xmlSensor);
        StringOperation::SplitString(sensorStr, sensors, ",");
        xmlFree(xmlSensor);
        if (!ParseFanFaultInfo(cur, sensors, fanFaultInfoMap)) {
            THERMAL_HILOGE(COMP_SVC, "ParseFanFaultInfo failed");
            return false;
        }
        cur = cur->next;
    }

    auto tms = ThermalService::GetInstance();
    tms->GetFanFaultDetect()->SetFaultInfoMap(fanFaultInfoMap);
    return true;
}

bool ThermalSrvConfigParser::ParseFanFaultInfo(const xmlNodePtr& node,
    std::vector<std::string> &sensors, FanFaultInfoMap &fanFaultInfoMap)
{
    uint32_t sensorNum = sensors.size();
    xmlNodePtr cur = node->xmlChildrenNode;

    while (cur != nullptr) {
        if (xmlStrcmp(cur->name, BAD_CAST"item")) {
            cur = cur->next;
            continue;
        }
        FanSensorInfo fanSensorInfo;
        xmlChar* xmlFault = xmlGetProp(cur, BAD_CAST"fault");
        if (xmlFault == nullptr) {
            return false;
        }
        std::string faultStr = reinterpret_cast<char*>(xmlFault);
        xmlFree(xmlFault);
        int32_t faultNum;
        StrToInt(faultStr, faultNum);
        xmlChar* xmlThreshold = xmlGetProp(cur, BAD_CAST"threshold");
        if (xmlThreshold == nullptr) {
            return false;
        }
        std::string thresholdStr = reinterpret_cast<char*>(xmlThreshold);
        std::vector<std::string> thresholds;
        StringOperation::SplitString(thresholdStr, thresholds, ",");
        xmlFree(xmlThreshold);
        if (thresholds.size() != sensorNum) {
            return false;
        }
        for (uint32_t i = 0; i < sensorNum; i++) {
            int32_t value;
            StrToInt(thresholds[i], value);
            fanSensorInfo.insert(std::make_pair(sensors[i], value));
        }
        fanFaultInfoMap.insert(std::make_pair(faultNum, fanSensorInfo));
        cur = cur->next;
    }

    return true;
}
} // namespace PowerMgr
} // namespace OHOS
