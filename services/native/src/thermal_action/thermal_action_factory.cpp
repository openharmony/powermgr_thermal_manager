/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "action_cpu_med.h"
#include "action_cpu_lit.h"
#include "action_gpu.h"
#include "action_display.h"
#include "action_shutdown.h"
#include "action_thermal_level.h"
#include "action_popup.h"
#include "action_voltage.h"
#include "string_operation.h"
#include "constants.h"
#include "thermal_common.h"

namespace OHOS {
namespace PowerMgr {
std::shared_ptr<IThermalAction> ThermalActionFactory::Create(const std::string& actionName)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    if (StringOperation::Compare(actionName, CPU_BIG_ACTION_NAME)) {
        return std::make_shared<ActionCpuBig>();
    } else if (StringOperation::Compare(actionName, CPU_MED_ACTION_NAME)) {
        return std::make_shared<ActionCpuMed>();
    } else if (StringOperation::Compare(actionName, CPU_LIT_ACTION_NAME)) {
        return std::make_shared<ActionCpuLit>();
    } else if (StringOperation::Compare(actionName, GPU_ACTION_NAME)) {
        return std::make_shared<ActionGpu>();
    } else if (StringOperation::Compare(actionName, LCD_ACTION_NAME)) {
        return std::make_shared<ActionDisplay>();
    } else if (StringOperation::Compare(actionName, SHUTDOWN_ACTION_NAME)) {
        return std::make_shared<ActionShutdown>();
    } else if (StringOperation::Compare(actionName, PROCESS_ACTION_NAME)) {
        return std::make_shared<ActionApplicationProcess>();
    } else if (StringOperation::Compare(actionName, THERMAL_LEVEL_NAME)) {
        return std::make_shared<ActionThermalLevel>();
    }  else if (StringOperation::Compare(actionName, POPUP_ACTION_NAME)) {
        return std::make_shared<ActionPopup>();
    } else if (StringOperation::Compare(actionName, CURRENT_SC_ACTION_NAME)) {
        return std::make_shared<ActionCharger>();
    } else if (StringOperation::Compare(actionName, CURRENT_BUCK_ACTION_NAME)) {
        return std::make_shared<ActionCharger>();
    } else if (StringOperation::Compare(actionName, VOLATAGE_SC_ACTION_NAME)) {
        return std::make_shared<ActionVoltage>();
    } else if (StringOperation::Compare(actionName, VOLATAGE_BUCK_ACTION_NAME)) {
        return std::make_shared<ActionVoltage>();
    } else {
        return nullptr;
    }
    THERMAL_HILOGD(COMP_SVC, "Exit");
}
} // namespace PowerMgr
} // namespace OHOS
