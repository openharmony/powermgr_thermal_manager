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
}
bool ThermalKernelConfigFile::Init(const std::string &path)
{
    THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s: enter", __func__);
    if (!baseInfo_) {
        baseInfo_ = std::make_unique<ProtectorBaseInfo>();
    }

    ParseThermalKernelXML(path);
    return true;
}

void ThermalKernelConfigFile::ParseThermalKernelXML(const std::string &path)
{
    THERMAL_HILOGI(MODULE_THERMAL_PROTECTOR, "%{public}s: Enter", __func__);

    std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)> docPtr(
        xmlReadFile(path.c_str(), nullptr, XML_PARSE_NOBLANKS), xmlFreeDoc);
    if (docPtr == nullptr) {
        THERMAL_HILOGE(MODULE_THERMAL_PROTECTOR, "%{public}s: Init failed, read file failed.", __func__);
        return;
    }

    auto rootNode = xmlDocGetRootElement(docPtr.get());
    if (rootNode == nullptr) {
        THERMAL_HILOGE(MODULE_THERMAL_PROTECTOR, "%{public}s: Get root node failed.", __func__);
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
        baseItem.tag = (char *)xmlGetProp(cur, BAD_CAST"tag");
        baseItem.value = (char *)xmlGetProp(cur, BAD_CAST"value");
        vBase.push_back(baseItem);
        cur = cur->next;
    }
    baseInfo_->SetBaseItem(vBase);
}

void ThermalKernelConfigFile::ParseControlNode(xmlNodePtr node)
{
    auto cur = node->xmlChildrenNode;
    ThermalKernelPolicy::ThermalZoneMap tzInfoMap;
    std::vector<LevelAction> levelActionList;
    while (cur != nullptr) {
        LevelAction levelAction;
        std::shared_ptr<ProtectorThermalZoneInfo> tzinfo = std::make_shared<ProtectorThermalZoneInfo>();
        std::string type = (char *)xmlGetProp(cur, BAD_CAST"type");
        int32_t interval = atoi((char *)xmlGetProp(cur, BAD_CAST"interval"));
        tzinfo->SetInterval(interval);
        auto desc = xmlGetProp(cur, BAD_CAST"desc");
        if (desc != nullptr) {
            std::string value = (char *)desc;
            if (atoi(value.c_str()) == 1) {
                tzinfo->SetDesc(true);
            }
        }
        std::vector<ThermalZoneInfoItem> tzItemList;
        levelAction.name = type;
        for (auto subNode = cur->children; subNode; subNode = subNode->next) {
            if (subNode == nullptr) {
                continue;
            }
            if (!xmlStrcmp(subNode->name, BAD_CAST"item")) {
                ThermalZoneInfoItem tziItem;
                tziItem.threshold= atoi((char*)xmlGetProp(subNode, BAD_CAST"threshold"));
                tziItem.thresholdClr = atoi((char*)xmlGetProp(subNode, BAD_CAST"threshold_clr"));
                tziItem.level = static_cast<uint32_t>(atoi((char*)xmlGetProp(subNode, BAD_CAST"level")));
                levelAction.level = tziItem.level;
                for (auto subActionNode = subNode->children; subActionNode; subActionNode = subActionNode->next) {
                    ActionItem action;
                    action.name = (char *)subActionNode->name;
                    action.value = static_cast<uint32_t>(atoi((char *)xmlNodeGetContent(subActionNode)));
                    levelAction.vAction.push_back(action);
                }
                tzItemList.push_back(tziItem);
                levelActionList.push_back(levelAction);
            }
        }
        tzinfo->SetThermalZoneItem(tzItemList);
        tzInfoMap.emplace(std::pair(type, tzinfo));
        cur = cur->next;
    }
    service.GetPolicy()->SetThermalZoneMap(tzInfoMap);
    service.GetPolicy()->SetLevelAction(levelActionList);
}
} // namespace PowerMgr
} // namespace OHOS
