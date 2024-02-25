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

#include "thermal_mgr_dumper.h"

#include <stdlib.h>

#include "constants.h"
#include "thermal_common.h"
#include "thermal_service.h"
#include "action_popup.h"

namespace OHOS {
namespace PowerMgr {
namespace {
constexpr const char* ARGS_HELP = "-h";
constexpr const char* ARGS_DIALOG = "-d";
constexpr const char* ARGS_THERMALINFO = "-t";
constexpr const char* ARGS_SCENE = "-s";
constexpr const char* ARGS_LEVEL = "-l";
constexpr const char* ARGS_ACTION = "-a";
constexpr const char* ARGS_POLICY = "-p";
constexpr const char* ARGS_IDLE = "-i";
constexpr const char* ARGS_TEMP_REPORT = "-st";
constexpr const char* ARGS_STOP_TEMP_FLAG = "0";
constexpr const char* ARGS_TEMP_EMULATION = "-te";
}

bool ThermalMgrDumper::Dump(const std::vector<std::string>& args, std::string& result)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    result.clear();
    auto argc = args.size();
    if ((argc == 0) || (args[0] == ARGS_HELP)) {
        ShowUsage(result);
        return true;
    }

    auto tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        return false;
    }

    if (args[0] == ARGS_TEMP_REPORT) {
        SwitchTempReport(result, args);
        return true;
    } else if (args[0] == ARGS_TEMP_EMULATION) {
        return EmulateTempReport(result, args);
    }

    if (!tms->GetSimulationXml()) {
        return true;
    }

    if (args[0] == ARGS_DIALOG) {
        tms->GetActionPopup()->ShowThermalDialog(ActionPopup::TempStatus::HIGHER_TEMP);
        return true;
    }

    for (auto it = args.begin(); it != args.end(); it++) {
        if (*it == ARGS_THERMALINFO) {
            ShowThermalZoneInfo(result);
        } else if (*it == ARGS_SCENE) {
            std::shared_ptr<StateMachine> state = tms->GetStateMachineObj();
            state->DumpState(result);
        } else if (*it == ARGS_LEVEL) {
            std::shared_ptr<ThermalPolicy> policy = tms->GetPolicy();
            policy->DumpLevel(result);
        } else if (*it == ARGS_ACTION) {
            std::shared_ptr<ThermalActionManager> action = tms->GetActionManagerObj();
            action->DumpAction(result);
        } else if (*it == ARGS_POLICY) {
            std::shared_ptr<ThermalPolicy> policy = tms->GetPolicy();
            policy->DumpPolicy(result);
        } else if (*it == ARGS_IDLE) {
            std::shared_ptr<StateMachine> state = tms->GetStateMachineObj();
            state->DumpIdle(result);
        } else {
            break;
        }
    }
    return true;
}

void ThermalMgrDumper::SwitchTempReport(std::string& result, const std::vector<std::string>& args)
{
    if (args.size() < 2) {
        tms->SetTempReportSwitch(true);
        result.append("Temperature reporting has been started.\n");
        return;
    }
    if (args[1] == ARGS_STOP_TEMP_FLAG) {
        tms->SetTempReportSwitch(false);
        result.append("Temperature reporting has been stopped.\n");
        return;
    }
    tms->SetTempReportSwitch(true);
    result.append("Temperature reporting has been started.\n");
}

bool ThermalMgrDumper::EmulateTempReport(std::string& result, const std::vector<std::string>& args)
{
    if (args.size() < 3) {
        result.append("[Error] Insufficient input parameters!\n");
        return false;
    }
    TypeTempMap tempMap;
    for (size_t i = 1; (2 * i) < args.size(); ++i) {
        tempMap.emplace(args[2 * i - 1], atoi(args[2 * i].c_str()));
        result.append("Report temperature [ ")
            .append(args[2 * i - 1])
            .append(" ]: ")
            .append(args[2 * i]);
    }
    tms->HandleTempEmulation(tempMap);
    return true;
}

void ThermalMgrDumper::ShowThermalZoneInfo(std::string& result)
{
    auto tms = DelayedSpSingleton<ThermalService>::GetInstance();
    if (tms == nullptr) {
        THERMAL_HILOGE(COMP_SVC, "thermal service is nullptr");
        return;
    }
    auto tzMap = tms->GetSubscriber()->GetSubscriberInfo();
    for (auto& iter : tzMap) {
        result.append("Type: ")
            .append(iter.first)
            .append("\n")
            .append("Temperature: ")
            .append(ToString(iter.second))
            .append("\n");
    }
}

void ThermalMgrDumper::ShowUsage(std::string& result)
{
    result.append("Thermal manager dump options:\n")
        .append("  [-h] \n")
        .append("  description of the cmd option:\n")
        .append("    -h: show this help.\n")
        .append("    -d: show thermal level dialog.\n")
        .append("    -t: show thermal zone data.\n")
        .append("    -s: show thermal scene data.\n")
        .append("    -l: show thermal level data.\n")
        .append("    -a: show thermal action data.\n")
        .append("    -p: show thermal policy data.\n")
        .append("    -i: show thermal charging idle state data.\n");
}
} // namespace PowerMgr
} // namespace OHOS
