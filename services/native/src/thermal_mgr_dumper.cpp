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

#include <cstdlib>

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
constexpr int32_t TEMP_EMUL_PARAM_NUM = 2;
}

bool ThermalMgrDumper::Dump(const std::vector<std::string>& args, std::string& result)
{
    result.clear();
    if ((args.size() == 0) || (args[0] == ARGS_HELP)) {
        ShowUsage(result);
        return true;
    }

    auto tms = ThermalService::GetInstance();
    if (tms == nullptr) {
        return false;
    }

    if (args[0] == ARGS_TEMP_REPORT) {
        SwitchTempReport(args, result, tms);
    } else if (args[0] == ARGS_TEMP_EMULATION) {
        EmulateTempReport(args, result, tms);
    } else if (args[0] == ARGS_THERMALINFO) {
        ShowThermalZoneInfo(result);
    } else {
        return DumpPolicy(args, result, tms);
    }
    return true;
}

void ThermalMgrDumper::SwitchTempReport(const std::vector<std::string>& args,
    std::string& result, sptr<ThermalService>& tms)
{
#ifndef THERMAL_USER_VERSION
    if (args.size() >= TEMP_EMUL_PARAM_NUM && args[1] == ARGS_STOP_TEMP_FLAG) {
        tms->SetTempReportSwitch(false);
        result.append("Temperature reporting has been stopped.\n");
    } else {
        tms->SetTempReportSwitch(true);
        result.append("Temperature reporting has been started.\n");
    }
#else
    result.append("[Failed] User version is not supported.\n");
#endif
}

void ThermalMgrDumper::EmulateTempReport(const std::vector<std::string>& args,
    std::string& result, sptr<ThermalService>& tms)
{
#ifndef THERMAL_USER_VERSION
    if (args.size() <= TEMP_EMUL_PARAM_NUM) {
        result.append("[Error] Insufficient input parameters!\n");
        return;
    }
    TypeTempMap tempMap;
    for (size_t i = 1; (TEMP_EMUL_PARAM_NUM * i) < args.size(); ++i) {
        tempMap.emplace(args[TEMP_EMUL_PARAM_NUM * i - 1], atoi(args[TEMP_EMUL_PARAM_NUM * i].c_str()));
        result.append("Report temperature [ ")
            .append(args[TEMP_EMUL_PARAM_NUM * i - 1])
            .append(" ]: ")
            .append(args[TEMP_EMUL_PARAM_NUM * i])
            .append("\n");
    }
    if (!tms->HandleTempEmulation(tempMap)) {
        result.append("[Error] Original temperature reporting has not been closed! You need to close it first.\n");
    }
#else
    result.append("[Failed] User version is not supported.\n");
#endif
}

bool ThermalMgrDumper::DumpPolicy(const std::vector<std::string>& args,
    std::string& result, sptr<ThermalService>& tms)
{
    if (!tms->GetSimulationXml()) {
        return false;
    }

    for (auto it = args.begin(); it != args.end(); it++) {
        if (*it == ARGS_DIALOG) {
            tms->GetActionPopup()->ShowThermalDialog(ActionPopup::TempStatus::HIGHER_TEMP);
        } else if (*it == ARGS_SCENE) {
            tms->GetStateMachineObj()->DumpState(result);
        } else if (*it == ARGS_LEVEL) {
            tms->GetPolicy()->DumpLevel(result);
        } else if (*it == ARGS_ACTION) {
            tms->GetActionManagerObj()->DumpAction(result);
        } else if (*it == ARGS_POLICY) {
            tms->GetPolicy()->DumpPolicy(result);
        } else if (*it == ARGS_IDLE) {
            tms->GetStateMachineObj()->DumpIdle(result);
        } else {
            break;
        }
    }
    return true;
}

void ThermalMgrDumper::ShowThermalZoneInfo(std::string& result)
{
    auto tms = ThermalService::GetInstance();
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
        .append("    -t: show thermal zone data.\n")
        .append("  only for engineer:\n")
        .append("    -st: switch temperature report, 0: stop, 1: start.\n")
        .append("    -te: temperature emulation.\n")
        .append("    -d: show thermal level dialog.\n")
        .append("    -s: show thermal scene data.\n")
        .append("    -l: show thermal level data.\n")
        .append("    -a: show thermal action data.\n")
        .append("    -p: show thermal policy data.\n")
        .append("    -i: show thermal charging idle state data.\n");
}
} // namespace PowerMgr
} // namespace OHOS
