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

#include "thermal_action_factory.h"

#include "action_application_process.h"
#include "action_charger.h"
#include "action_cpu_big.h"
#include "action_cpu_boost.h"
#include "action_cpu_isolate.h"
#include "action_cpu_lit.h"
#include "action_cpu_med.h"
#include "action_display.h"
#include "action_gpu.h"
#include "action_node.h"
#include "action_popup.h"
#include "action_shutdown.h"
#include "action_thermal_level.h"
#include "action_voltage.h"
#include "action_volume.h"
#include "constants.h"
#include "string_operation.h"
#include "thermal_common.h"

namespace OHOS {
namespace PowerMgr {
namespace {
std::map<std::string, ActionFunc> g_actionMap;
}
void ThermalActionFactory::InitFactory()
{
    g_actionMap = {
        std::make_pair(CPU_BIG_ACTION_NAME,
            [&](std::string actionName) { return std::make_shared<ActionCpuBig>(actionName); }),
        std::make_pair(CPU_MED_ACTION_NAME,
            [&](std::string actionName) { return std::make_shared<ActionCpuMed>(actionName); }),
        std::make_pair(CPU_LIT_ACTION_NAME,
            [&](std::string actionName) { return std::make_shared<ActionCpuLit>(actionName); }),
        std::make_pair(GPU_ACTION_NAME,
            [&](std::string actionName) { return std::make_shared<ActionGpu>(actionName); }),
        std::make_pair(LCD_ACTION_NAME,
            [&](std::string actionName) { return std::make_shared<ActionDisplay>(actionName); }),
        std::make_pair(VOLUME_ACTION_NAME,
            [&](std::string actionName) { return std::make_shared<ActionVolume>(actionName); }),
        std::make_pair(SHUTDOWN_ACTION_NAME,
            [&](std::string actionName) { return std::make_shared<ActionShutdown>(actionName); }),
        std::make_pair(THERMAL_LEVEL_NAME,
            [&](std::string actionName) { return std::make_shared<ActionThermalLevel>(actionName); }),
        std::make_pair(POPUP_ACTION_NAME,
            [&](std::string actionName) { return std::make_shared<ActionPopup>(actionName); }),
        std::make_pair(CURRENT_ACTION_NAME,
            [&](std::string actionName) { return std::make_shared<ActionCharger>(actionName); }),
        std::make_pair(VOLATAGE_ACTION_NAME,
            [&](std::string actionName) { return std::make_shared<ActionVoltage>(actionName); }),
        std::make_pair(CPU_BOOST_ACTION_NAME,
            [&](std::string actionName) { return std::make_shared<ActionCpuBoost>(actionName); }),
        std::make_pair(CPU_ISOLATE_ACTION_NAME,
            [&](std::string actionName) { return std::make_shared<ActionCpuIsolate>(actionName); }),
        std::make_pair(PROCESS_ACTION_NAME,
            [&](std::string actionName) { return std::make_shared<ActionApplicationProcess>(actionName); }),
        std::make_pair(NODE_ACTION_NAME,
            [&](std::string actionName) { return std::make_shared<ActionNode>(actionName); }),
    };
}

std::shared_ptr<IThermalAction> ThermalActionFactory::Create(const std::string& actionClass,
    const std::string& actionName)
{
    for (auto iter = g_actionMap.begin(); iter != g_actionMap.end(); ++iter) {
        if (StringOperation::Compare(actionClass, iter->first)) {
            return iter->second(actionName);
        }
    }

    THERMAL_HILOGD(COMP_SVC, "create factory failed");
    return nullptr;
}
} // namespace PowerMgr
} // namespace OHOS
