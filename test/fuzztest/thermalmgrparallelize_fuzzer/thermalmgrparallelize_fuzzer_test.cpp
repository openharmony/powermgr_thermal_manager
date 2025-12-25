/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

/* This files contains faultlog fuzzer test modules. */

#define FUZZ_PROJECT_NAME "thermalmgrparallelize_fuzzer"

#include <fuzzer/FuzzedDataProvider.h>
#include "ithermal_srv.h"
#include "thermal_action_callback_stub.h"
#include "thermal_level_callback_stub.h"
#include "thermal_service.h"
#include "thermal_temp_callback_stub.h"

using namespace OHOS::PowerMgr;
using namespace OHOS;

namespace OHOS {
constexpr int32_t MAX_STRING_LENGTH = 64;
sptr<ThermalService> g_service = nullptr;
class ThermalTempTestCallback : public ThermalTempCallbackStub {
public:
    bool OnThermalTempChanged(TempCallbackMap& tempCbMap) { return true; }
};
class ThermalLevelTestCallback : public ThermalLevelCallbackStub {
public:
    bool OnThermalTempChanged(ThermalLevel level) { return true; }
};
class ThermalActionTestCallback : public ThermalActionCallbackStub {
public:
    bool OnThermalActionChanged(ActionCallbackMap& actionCbMap) { return true; }
};

enum IpcCode : std::int16_t {
    SUBSCRIBE_THERMAL_TEMP = 0,
    UN_SUBSCRIBE_THERMAL_TEMP = 1,
    SUBSCRIBE_THERMAL_LEVEL = 2,
    UN_SUBSCRIBE_THERMAL_LEVEL = 3,
    SUBSCRIBE_THERMAL_ACTION = 4,
    UN_SUBSCRIBE_THERMAL_ACTION = 5,
    GET_THERMAL_SRV_SENSOR_INFO = 6,
    GET_THERMAL_LEVEL = 7,
    GET_THERMAL_INFO = 8,
    SET_SCENE = 9,
    UPDATE_THERMAL_STATE = 10,
    SHELL_DUMP = 11,
};
}

extern "C" int FuzzThermalMgrService(FuzzedDataProvider &provider)
{
    if (g_service == nullptr) {
        return 0;
    }

    static const int fuzzCodes[] = {
        IpcCode::SUBSCRIBE_THERMAL_TEMP, IpcCode::UN_SUBSCRIBE_THERMAL_TEMP,
        IpcCode::SUBSCRIBE_THERMAL_LEVEL, IpcCode::UN_SUBSCRIBE_THERMAL_LEVEL,
        IpcCode::SUBSCRIBE_THERMAL_ACTION, IpcCode::UN_SUBSCRIBE_THERMAL_ACTION,
        IpcCode::GET_THERMAL_SRV_SENSOR_INFO, IpcCode::GET_THERMAL_LEVEL,
        IpcCode::GET_THERMAL_INFO, IpcCode::SET_SCENE,
        IpcCode::UPDATE_THERMAL_STATE, IpcCode::SHELL_DUMP,
    };
    int code = provider.PickValueInArray(fuzzCodes);
    switch (code) {
        case IpcCode::SUBSCRIBE_THERMAL_TEMP: {
            std::vector<std::string> typeList;
            std::string type1 = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
            std::string type2 = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
            typeList.emplace_back(type1);
            typeList.emplace_back(type2);
            sptr<IThermalTempCallback> callback = new ThermalTempTestCallback();
            g_service->SubscribeThermalTempCallback(typeList, callback);
            break;
        }
        case IpcCode::UN_SUBSCRIBE_THERMAL_TEMP: {
            sptr<IThermalTempCallback> callback = new ThermalTempTestCallback();
            g_service->UnSubscribeThermalTempCallback(callback);
            break;
        }
        case IpcCode::SUBSCRIBE_THERMAL_LEVEL: {
            sptr<IThermalLevelCallback> callback = new ThermalLevelTestCallback();
            g_service->SubscribeThermalLevelCallback(callback);
            break;
        }
        case IpcCode::UN_SUBSCRIBE_THERMAL_LEVEL: {
            sptr<IThermalLevelCallback> callback = new ThermalLevelTestCallback();
            g_service->UnSubscribeThermalLevelCallback(callback);
            break;
        }
        case IpcCode::SUBSCRIBE_THERMAL_ACTION: {
            std::string action1 = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
            std::string action2 = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
            std::vector<std::string> actionList;
            actionList.emplace_back(action1);
            actionList.emplace_back(action2);
            std::string desc = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
            sptr<IThermalActionCallback> callback = new ThermalActionTestCallback();
            g_service->SubscribeThermalActionCallback(actionList, desc, callback);
            break;
        }
        case IpcCode::UN_SUBSCRIBE_THERMAL_ACTION: {
            sptr<IThermalActionCallback> callback = new ThermalActionTestCallback();
            g_service->UnSubscribeThermalActionCallback(callback);
            break;
        }
        case IpcCode::GET_THERMAL_SRV_SENSOR_INFO: {
            int32_t type = provider.ConsumeIntegral<int32_t>();
            ThermalSrvSensorInfo sensorInfo;
            bool sensorInfoRet;
            g_service->GetThermalSrvSensorInfo(type, sensorInfo, sensorInfoRet);
            break;
        }
        case IpcCode::GET_THERMAL_LEVEL: {
            int32_t level = provider.ConsumeIntegral<int32_t>();
            g_service->GetThermalLevel(level);
            break;
        }
        case IpcCode::GET_THERMAL_INFO: {
            g_service->GetThermalInfo();
            break;
        }
        case IpcCode::SET_SCENE: {
            std::string scene = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
            g_service->SetScene(scene);
            break;
        }
        case IpcCode::UPDATE_THERMAL_STATE: {
            std::string tag = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
            std::string val = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
            bool isImmed = provider.ConsumeBool();
            g_service->UpdateThermalState(tag, val, isImmed);
            break;
        }
        case IpcCode::SHELL_DUMP: {
            std::string arg1 = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
            std::string arg2 = provider.ConsumeRandomLengthString(MAX_STRING_LENGTH);
            std::vector<std::string> args;
            args.emplace_back(arg1);
            args.emplace_back(arg2);
            uint32_t argc = provider.ConsumeIntegral<uint32_t>();
            std::string result;
            g_service->ShellDump(args, argc, result);
            break;
        }
        default:
            break;
    }
    return 0;
}

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    g_service = ThermalService::GetInstance();
    g_service->OnStart();
    return 0;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    FuzzThermalMgrService(fdp);
    return 0;
}
