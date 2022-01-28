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

#include "thermal_shell_command.h"

#include <cerrno>
#include <fcntl.h>
#include <getopt.h>
#include <iostream>
#include <string_ex.h>
#include <unistd.h>

#include "iservice_registry.h"
#include "singleton.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace PowerMgr {
static const std::string HELP_MSG =
    "usage: thermal-shell\n"
    "command list:\n"
    "  dump    :    Dump thermal info. \n"
    "  help    :    Show this help menu. \n";

ThermalShellCommand::ThermalShellCommand(int argc, char *argv[]) : ShellCommand(argc, argv, "thermal-shell")
{}

ErrCode ThermalShellCommand::CreateCommandMap()
{
    commandMap_ = {
        {"help", std::bind(&ThermalShellCommand::RunAsHelpCommand, this)},
        {"dump", std::bind(&ThermalShellCommand::RunAsDumpCommand, this)},
    };

    return ERR_OK;
}

ErrCode ThermalShellCommand::CreateMessageMap()
{
    messageMap_ = {};

    return ERR_OK;
}

ErrCode ThermalShellCommand::init()
{
    return OHOS::ERR_OK;
}

ErrCode ThermalShellCommand::RunAsHelpCommand()
{
    resultReceiver_.clear();
    resultReceiver_.append(HELP_MSG);
    return ERR_OK;
}

ErrCode ThermalShellCommand::RunAsDumpCommand()
{
    resultReceiver_.clear();

    ThermalMgrClient &client = ThermalMgrClient::GetInstance();
    std::string ret = client.Dump(argList_);
    resultReceiver_.append("Thermal Dump result: \n");
    resultReceiver_.append(ret);

    return ERR_OK;
}

extern "C" void PrintDumpFileError(std::string& receiver, const char* path)
{
    receiver.append("Open Dump file (");
    receiver.append(path);
    receiver.append(") failed: ");
    receiver.append(std::to_string(errno));
    receiver.append("\n");
}
}  // namespace Powermgr
}  // namespace OHOS