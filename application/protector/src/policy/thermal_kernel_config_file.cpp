/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "thermal_kernel_config_file.h"
#include "thermal_common.h"
#include "thermal_kernel_service.h"

namespace OHOS {
namespace PowerMgr {
namespace {
auto &service = ThermalKernelService::GetInstance();
std::vector<LevelAction> g_levelActionList;
}
bool ThermalKernelConfigFile::Init(const std::string &path)
{
    THERMAL_HILOGD(FEATURE_PROTECTOR, "Enter");
    if (!baseInfo_) {
        baseInfo_ = std::make_unique<ProtectorBaseInfo>();
    }

    ParseThermalKernelXML(path);
    return true;
}

void ThermalKernelConfigFile::ParseThermalKernelXML(const std::string &path)
{
    THERMAL_HILOGD(FEATURE_PROTECTOR, "Enter");

    std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)> docPtr(
        xmlReadFile(path.c_str(), nullptr, XML_PARSE_NOBLANKS), xmlFreeDoc);
    if (docPtr == nullptr) {
        THERMAL_HILOGE(FEATURE_PROTECTOR, "Init failed, read file failed.");
        return;
    }

    auto rootNode = xmlDocGetRootElement(docPtr.get());
    if (rootNode == nullptr) {
        THERMAL_HILOGE(FEATURE_PROTECTOR, "Get root node failed.");
        return;
    }

    for (auto node = rootNode->children; node; node = node->next) {
        if (node == nullptr) continue;
        if (!xmlStrcmp(node->name, BAD_CAST"base")) {
            ParserBaseNode(node);
        } else if (!xmlStrcmp(node->name, BAD_CAST"control")) {
            ParseControlNode(node);
        }
    }
}

void ThermalKernelConfigFile::ParserBaseNode(xmlNodePtr node)
{
    auto cur = node->xmlChildrenNode;
    std::vector<BaseItem> vBase;
    while (cur != nullptr) {
        BaseItem baseItem;
        xmlChar* xmlTag = xmlGetProp(cur, BAD_CAST"tag");
        xmlChar* xmlValue = xmlGetProp(cur, BAD_CAST"value");
        if (xmlTag != nullptr) {
            baseItem.tag = (char *)xmlTag;
            xmlFree(xmlTag);
        }
        if (xmlValue != nullptr) {
            baseItem.value = (char *)xmlValue;
            xmlFree(xmlValue);
        }
        vBase.push_back(baseItem);
        cur = cur->next;
    }
    baseInfo_->SetBaseItem(vBase);
}

void ThermalKernelConfigFile::ParseControlNode(xmlNodePtr node)
{
    auto cur = node->xmlChildrenNode;
    ThermalKernelPolicy::ThermalZoneMap tzInfoMap;
    while (cur != nullptr) {
        LevelAction levelAction;
        std::string type;
        std::shared_ptr<ProtectorThermalZoneInfo> tzinfo = std::make_shared<ProtectorThermalZoneInfo>();
        xmlChar* xmlType = xmlGetProp(cur, BAD_CAST"type");
        if (xmlType != nullptr) {
            type = (char *)xmlType;
            xmlFree(xmlType);
        }
        xmlChar* xmlInterval = xmlGetProp(cur, BAD_CAST"interval");
        if (xmlInterval != nullptr) {
            int32_t interval = atoi((char *)xmlInterval);
            tzinfo->SetInterval(interval);
            xmlFree(xmlInterval);
        }

        xmlChar* desc = xmlGetProp(cur, BAD_CAST"desc");
        if (desc != nullptr) {
            std::string value = (char *)desc;
            if (atoi(value.c_str()) == 1) {
                tzinfo->SetDesc(true);
            }
            xmlFree(desc);
        }

        std::vector<ThermalZoneInfoItem> tzItemList;
        ParseSubNode(cur, tzItemList, type);

        tzinfo->SetThermalZoneItem(tzItemList);
        tzInfoMap.emplace(std::pair(type, tzinfo));
        cur = cur->next;
    }
    service.GetPolicy()->SetThermalZoneMap(tzInfoMap);
    service.GetPolicy()->SetLevelAction(g_levelActionList);
}

void ThermalKernelConfigFile::ParseSubNode(xmlNodePtr cur, std::vector<ThermalZoneInfoItem>& tzItemList,
    std::string& type)
{
    THERMAL_HILOGD(FEATURE_PROTECTOR, "Enter");
    LevelAction levelAction;
    levelAction.name = type;

    for (auto subNode = cur->children; subNode; subNode = subNode->next) {
        if (subNode == nullptr) {
            continue;
        }
        if (xmlStrcmp(subNode->name, BAD_CAST"item") != 0) {
            continue;
        }

        ThermalZoneInfoItem tziItem;
        xmlChar* xmlThreshold = xmlGetProp(subNode, BAD_CAST"threshold");
        if (xmlThreshold != nullptr) {
            tziItem.threshold= atoi((char*)xmlThreshold);
            xmlFree(xmlThreshold);
        }
        xmlChar* xmlThresholdClr = xmlGetProp(subNode, BAD_CAST"threshold_clr");
        if (xmlThresholdClr != nullptr) {
            tziItem.thresholdClr = atoi((char*)xmlThresholdClr);
            xmlFree(xmlThresholdClr);
        }
        xmlChar* xmlLevel = xmlGetProp(subNode, BAD_CAST"level");
        if (xmlLevel != nullptr) {
            tziItem.level = static_cast<uint32_t>(atoi((char*)xmlLevel));
            levelAction.level = tziItem.level;
            xmlFree(xmlLevel);
        }
        for (auto subActionNode = subNode->children; subActionNode; subActionNode = subActionNode->next) {
            ActionItem action;
            action.name = (char *)subActionNode->name;
            xmlChar* xmlValue = xmlNodeGetContent(subActionNode);
            if (xmlValue != nullptr) {
                action.value = static_cast<uint32_t>(atoi((char *)xmlValue));
                xmlFree(xmlValue);
            }
            levelAction.vAction.push_back(action);
        }
        tzItemList.push_back(tziItem);
        g_levelActionList.push_back(levelAction);
    }
}
} // namespace PowerMgr
} // namespace OHOS
