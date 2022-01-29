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

#include "thermal_action_factory.h"

#include "action_application_process.h"
#include "action_charger.h"
#include "action_cpu.h"
#include "action_gpu.h"
#include "action_display.h"
#include "action_shutdown.h"
#include "action_thermal_level.h"
#include "action_popup.h"
#include "string_operation.h"
#include "constants.h"
#include "thermal_common.h"

namespace OHOS {
namespace PowerMgr {
std::shared_ptr<IThermalAction> ThermalActionFactory::Create(std::string &actionName)
{
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s start", __func__);
    if (StringOperation::Compare(actionName, CPU_ACTION_NAME)) {
        return std::make_shared<ActionCpu>();
    } else if (StringOperation::Compare(actionName, GPU_ACTION_NAME)) {
        return std::make_shared<ActionGpu>();
    } else if (StringOperation::Compare(actionName, LCD_ACTION_NAME)) {
        return std::make_shared<ActionDisplay>();
    } else if (StringOperation::Compare(actionName, CHARGER_ACTION_NAME)) {
        return std::make_shared<ActionCharger>();
    } else if (StringOperation::Compare(actionName, SHUTDOWN_ACTION_NAME)) {
        return std::make_shared<ActionShutdown>();
    } else if (StringOperation::Compare(actionName, PROCESS_ACTION_NAME)) {
        return std::make_shared<ActionApplicationProcess>();
    } else if (StringOperation::Compare(actionName, THERMAL_LEVEL_NAME)) {
        return std::make_shared<ActionThermalLevel>();
    }  else if (StringOperation::Compare(actionName, POPUP_ACTION_NAME)) {
        return std::make_shared<ActionPopup>();
    } else {
        return nullptr;
    }
    THERMAL_HILOGI(MODULE_THERMALMGR_SERVICE, "%{public}s end", __func__);
}
} // namespace PowerMgr
} // namespace OHOS
