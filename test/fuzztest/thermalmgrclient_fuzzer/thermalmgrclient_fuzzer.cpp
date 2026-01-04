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

#include "thermalmgrclient_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <map>
#include <string>
#include <vector>

#ifndef THERMAL_SERVICE_DEATH_UT
#define THERMAL_SERVICE_DEATH_UT 1
#endif
#include "thermal_mgr_client.h"
#include "thermal_service.h"
#include "thermal_action_callback_stub.h"
#include "thermal_level_callback_stub.h"
#include "thermal_temp_callback_stub.h"
#include "thermal_policy.h"
#include "thermal_observer.h"
#include "thermal_action_manager.h"
#include "state_machine.h"
#include "thermal_config_base_info.h"
#include "thermal_sensor_info.h"
#include "thermal_service_subscriber.h"
#include "fan_fault_detect.h"
#include "v1_1/thermal_types.h"

#include "thermal_common.h"
#include "system_ability_definition.h"

using namespace OHOS;
using namespace OHOS::PowerMgr;
using namespace OHOS::HDI::Thermal::V1_1;

namespace {
constexpr size_t MAX_STR_LEN = 64;
constexpr size_t MAX_VEC_LEN = 16;
constexpr size_t SHORT_STR_LEN = 16;
constexpr size_t DUMP_STR_LEN = 32;
constexpr size_t MAX_THERMAL_ZONE_COUNT = 8;
constexpr size_t MAX_DUMP_OPTION_COUNT = 5;
constexpr size_t MAX_POLICY_MAP_SIZE = 6;
constexpr size_t MAX_SENSOR_MAP_SIZE = 6;
constexpr size_t MAX_STATE_MAP_SIZE = 4;
constexpr size_t MIN_CLIENT_DUMP_ARGS = 10;
constexpr size_t MAX_CLIENT_DUMP_ARGS = 20;
constexpr int32_t SENSOR_TYPE_MAX = 16;
constexpr int32_t MAX_TEMP = 100000;
constexpr int32_t MIN_TEMP = -50000;

sptr<ThermalService> g_service = nullptr;

std::string ConsumeSafeString(FuzzedDataProvider &fdp, size_t maxLen)
{
    auto s = fdp.ConsumeRandomLengthString(maxLen);
    for (auto &ch : s) {
        if (ch == '\0') {
            ch = 'a';
        }
    }
    return s;
}

std::vector<std::string> ConsumeStringVector(FuzzedDataProvider &fdp)
{
    size_t cnt = fdp.ConsumeIntegralInRange<size_t>(0, MAX_VEC_LEN);
    std::vector<std::string> out;
    out.reserve(cnt);
    for (size_t i = 0; i < cnt; ++i) {
        out.emplace_back(ConsumeSafeString(fdp, MAX_STR_LEN));
    }
    return out;
}

SensorType ConsumeSensorType(FuzzedDataProvider &fdp)
{
    int32_t v = fdp.ConsumeIntegralInRange<int32_t>(0, SENSOR_TYPE_MAX);
    return static_cast<SensorType>(v);
}

std::string ConsumeThermalZoneType(FuzzedDataProvider &fdp)
{
    static const std::vector<std::string> types = {
        "battery", "soc", "shell", "cpu", "charger", "ambient", "pa", "ap", "gpu", "usb", "wifi"
    };
    if (fdp.ConsumeBool()) {
        return types[fdp.ConsumeIntegralInRange<size_t>(0, types.size() - 1)];
    }
    return ConsumeSafeString(fdp, SHORT_STR_LEN);
}

class ThermalTempTestCallback : public ThermalTempCallbackStub {
public:
    bool OnThermalTempChanged(TempCallbackMap &tempCbMap) override
    {
        (void)tempCbMap;
        return true;
    }
};

class ThermalLevelTestCallback : public ThermalLevelCallbackStub {
public:
    bool OnThermalLevelChanged(ThermalLevel level) override
    {
        (void)level;
        return true;
    }
};

class ThermalActionTestCallback : public ThermalActionCallbackStub {
public:
    bool OnThermalActionChanged(ActionCallbackMap &actionCbMap) override
    {
        (void)actionCbMap;
        return true;
    }
};

void InjectService(ThermalMgrClient &client)
{
    if (g_service == nullptr) {
        return;
    }
    client.thermalSrv_ = g_service;
}

void FuzzThermalCallbackEvent(FuzzedDataProvider &fdp)
{
    if (g_service == nullptr) {
        return;
    }

    HdfThermalCallbackInfo event;
    size_t infoCount = fdp.ConsumeIntegralInRange<size_t>(0, MAX_THERMAL_ZONE_COUNT);
    for (size_t i = 0; i < infoCount; ++i) {
        ThermalZoneInfo info;
        info.type = ConsumeThermalZoneType(fdp);
        info.temp = fdp.ConsumeIntegralInRange<int32_t>(MIN_TEMP, MAX_TEMP);
        event.info.push_back(info);
    }

    g_service->HandleThermalCallbackEvent(event);

    if (fdp.ConsumeBool()) {
        g_service->HandleFanCallbackEvent(event);
    }
}

void FuzzTempEmulation(FuzzedDataProvider &fdp)
{
    if (g_service == nullptr) {
        return;
    }

    TypeTempMap tempMap;
    size_t mapSize = fdp.ConsumeIntegralInRange<size_t>(0, MAX_THERMAL_ZONE_COUNT);
    for (size_t i = 0; i < mapSize; ++i) {
        std::string type = ConsumeThermalZoneType(fdp);
        int32_t temp = fdp.ConsumeIntegralInRange<int32_t>(MIN_TEMP, MAX_TEMP);
        tempMap[type] = temp;
    }

    g_service->SetTempReportSwitch(fdp.ConsumeBool());
    g_service->HandleTempEmulation(tempMap);
}

void FuzzServiceDump(FuzzedDataProvider &fdp)
{
    if (g_service == nullptr) {
        return;
    }

    static const std::vector<std::string> dumpOptions = {
        "-h", "--help", "-t", "-temp", "-a", "-action", "-l", "-level", "-s", "-state",
        "-p", "-policy", "-c", "-config", "-d", "-debug", "-thermallevel", "-dialog"
    };

    std::vector<std::u16string> args;
    size_t argCount = fdp.ConsumeIntegralInRange<size_t>(0, MAX_DUMP_OPTION_COUNT);
    for (size_t i = 0; i < argCount; ++i) {
        std::string arg;
        if (fdp.ConsumeBool() && !dumpOptions.empty()) {
            arg = dumpOptions[fdp.ConsumeIntegralInRange<size_t>(0, dumpOptions.size() - 1)];
        } else {
            arg = ConsumeSafeString(fdp, DUMP_STR_LEN);
        }
        std::u16string u16Arg(arg.begin(), arg.end());
        args.push_back(u16Arg);
    }

    int fd = fdp.ConsumeIntegralInRange<int>(-1, 10);
    g_service->Dump(fd, args);
}

void FuzzShellDump(FuzzedDataProvider &fdp)
{
    if (g_service == nullptr) {
        return;
    }

    std::vector<std::string> args = ConsumeStringVector(fdp);
    uint32_t argc = fdp.ConsumeIntegral<uint32_t>();
    std::string result;
    g_service->ShellDump(args, argc, result);
}

void FuzzOnAddSystemAbility(FuzzedDataProvider &fdp)
{
    if (g_service == nullptr) {
        return;
    }

    static const int32_t saIds[] = {
        COMMON_EVENT_SERVICE_ID,
        SOC_PERF_SERVICE_SA_ID,
        POWER_MANAGER_BATT_SERVICE_ID,
        POWER_MANAGER_THERMAL_SERVICE_ID,
    };

    int32_t saId;
    if (fdp.ConsumeBool()) {
        constexpr size_t lastIndex = (sizeof(saIds) / sizeof(saIds[0])) - 1;
        saId = saIds[fdp.ConsumeIntegralInRange<size_t>(0, lastIndex)];
    } else {
        saId = fdp.ConsumeIntegral<int32_t>();
    }

    std::string deviceId = ConsumeSafeString(fdp, DUMP_STR_LEN);
    g_service->OnAddSystemAbility(saId, deviceId);
}

void FuzzThermalPolicy(FuzzedDataProvider &fdp)
{
    if (g_service == nullptr) {
        return;
    }

    auto policy = g_service->GetPolicy();
    if (policy == nullptr) {
        return;
    }

    TypeTempMap tempInfo;
    size_t mapSize = fdp.ConsumeIntegralInRange<size_t>(0, MAX_POLICY_MAP_SIZE);
    for (size_t i = 0; i < mapSize; ++i) {
        std::string type = ConsumeThermalZoneType(fdp);
        int32_t temp = fdp.ConsumeIntegralInRange<int32_t>(MIN_TEMP, MAX_TEMP);
        tempInfo[type] = temp;
    }

    if (fdp.ConsumeBool()) {
        policy->OnSensorInfoReported(tempInfo);
    }

    if (fdp.ConsumeBool()) {
        std::string result;
        policy->DumpLevel(result);
    }

    if (fdp.ConsumeBool()) {
        std::string result;
        policy->DumpPolicy(result);
    }

    if (fdp.ConsumeBool()) {
        policy->ExecutePolicy();
    }

    if (fdp.ConsumeBool()) {
        std::map<std::string, std::string> stateMap;
        size_t stateCount = fdp.ConsumeIntegralInRange<size_t>(0, MAX_STATE_MAP_SIZE);
        for (size_t i = 0; i < stateCount; ++i) {
            stateMap[ConsumeSafeString(fdp, SHORT_STR_LEN)] = ConsumeSafeString(fdp, SHORT_STR_LEN);
        }
        policy->StateMachineDecision(stateMap);
    }
}

void FuzzThermalObserver(FuzzedDataProvider &fdp)
{
    if (g_service == nullptr) {
        return;
    }

    auto observer = g_service->GetObserver();
    if (observer == nullptr) {
        return;
    }

    if (fdp.ConsumeBool()) {
        SensorType type = ConsumeSensorType(fdp);
        ThermalSrvSensorInfo info;
        observer->GetThermalSrvSensorInfo(type, info);
    }

    if (fdp.ConsumeBool()) {
        sptr<IThermalTempCallback> cb = fdp.ConsumeBool()
            ? sptr<IThermalTempCallback>(new ThermalTempTestCallback())
            : nullptr;
        std::vector<std::string> typeList = ConsumeStringVector(fdp);
        observer->SubscribeThermalTempCallback(typeList, cb);
    }

    if (fdp.ConsumeBool()) {
        sptr<IThermalTempCallback> cb = fdp.ConsumeBool()
            ? sptr<IThermalTempCallback>(new ThermalTempTestCallback())
            : nullptr;
        observer->UnSubscribeThermalTempCallback(cb);
    }

    if (fdp.ConsumeBool()) {
        sptr<IThermalActionCallback> cb = fdp.ConsumeBool()
            ? sptr<IThermalActionCallback>(new ThermalActionTestCallback())
            : nullptr;
        std::vector<std::string> actionList = ConsumeStringVector(fdp);
        std::string desc = ConsumeSafeString(fdp, DUMP_STR_LEN);
        observer->SubscribeThermalActionCallback(actionList, desc, cb);
    }

    if (fdp.ConsumeBool()) {
        sptr<IThermalActionCallback> cb = fdp.ConsumeBool()
            ? sptr<IThermalActionCallback>(new ThermalActionTestCallback())
            : nullptr;
        observer->UnSubscribeThermalActionCallback(cb);
    }
}

void FuzzThermalActionManager(FuzzedDataProvider &fdp)
{
    if (g_service == nullptr) {
        return;
    }

    auto actionMgr = g_service->GetActionManagerObj();
    if (actionMgr == nullptr) {
        return;
    }

    if (fdp.ConsumeBool()) {
        uint32_t level = actionMgr->GetThermalLevel();
        (void)level;
    }

    if (fdp.ConsumeBool()) {
        sptr<IThermalLevelCallback> cb = fdp.ConsumeBool()
            ? sptr<IThermalLevelCallback>(new ThermalLevelTestCallback())
            : nullptr;
        actionMgr->SubscribeThermalLevelCallback(cb);
    }

    if (fdp.ConsumeBool()) {
        sptr<IThermalLevelCallback> cb = fdp.ConsumeBool()
            ? sptr<IThermalLevelCallback>(new ThermalLevelTestCallback())
            : nullptr;
        actionMgr->UnSubscribeThermalLevelCallback(cb);
    }
}

void FuzzStateMachine(FuzzedDataProvider &fdp)
{
    if (g_service == nullptr) {
        return;
    }

    auto state = g_service->GetStateMachineObj();
    if (state == nullptr) {
        return;
    }

    if (fdp.ConsumeBool()) {
        std::string tag = ConsumeSafeString(fdp, DUMP_STR_LEN);
        std::string val = ConsumeSafeString(fdp, DUMP_STR_LEN);
        state->UpdateState(tag, val);
    }

    if (fdp.ConsumeBool()) {
        std::string result;
        state->DumpState(result);
    }
}

void FuzzServiceSubscriber(FuzzedDataProvider &fdp)
{
    if (g_service == nullptr) {
        return;
    }

    auto subscriber = g_service->GetSubscriber();
    if (subscriber == nullptr) {
        return;
    }

    TypeTempMap tempMap;
    size_t mapSize = fdp.ConsumeIntegralInRange<size_t>(0, MAX_SENSOR_MAP_SIZE);
    for (size_t i = 0; i < mapSize; ++i) {
        std::string type = ConsumeThermalZoneType(fdp);
        int32_t temp = fdp.ConsumeIntegralInRange<int32_t>(MIN_TEMP, MAX_TEMP);
        tempMap[type] = temp;
    }

    if (fdp.ConsumeBool()) {
        subscriber->OnTemperatureChanged(tempMap);
    }
}

void FuzzServiceMisc(FuzzedDataProvider &fdp)
{
    if (g_service == nullptr) {
        return;
    }

    if (fdp.ConsumeBool()) {
        std::string scene = ConsumeSafeString(fdp, DUMP_STR_LEN);
        g_service->SetScene(scene);
    }

    if (fdp.ConsumeBool()) {
        std::string tag = ConsumeSafeString(fdp, DUMP_STR_LEN);
        std::string val = ConsumeSafeString(fdp, DUMP_STR_LEN);
        bool isImmed = fdp.ConsumeBool();
        g_service->UpdateThermalState(tag, val, isImmed);
    }

    if (fdp.ConsumeBool()) {
        int32_t level;
        g_service->GetThermalLevel(level);
    }

    if (fdp.ConsumeBool()) {
        int32_t type = fdp.ConsumeIntegralInRange<int32_t>(0, SENSOR_TYPE_MAX);
        ThermalSrvSensorInfo info;
        bool ret;
        g_service->GetThermalSrvSensorInfo(type, info, ret);
    }

    if (fdp.ConsumeBool()) {
        g_service->GetThermalInfo();
    }

    if (fdp.ConsumeBool()) {
        bool isReady = g_service->IsServiceReady();
        (void)isReady;
    }

    if (fdp.ConsumeBool()) {
        std::string scene = ThermalService::GetScene();
        (void)scene;
    }

    if (fdp.ConsumeBool()) {
        g_service->SetTempReportSwitch(fdp.ConsumeBool());
    }
}

void FuzzSensorInfo(FuzzedDataProvider &fdp)
{
    if (g_service == nullptr) {
        return;
    }

    auto sensorInfo = g_service->GetSensorInfo();
    if (sensorInfo == nullptr) {
        return;
    }

    if (fdp.ConsumeBool()) {
        std::string type = ConsumeThermalZoneType(fdp);
        int32_t temp = sensorInfo->GetTemp(type);
        (void)temp;
    }

    if (fdp.ConsumeBool()) {
        TypeTempMap tempMap;
        size_t mapSize = fdp.ConsumeIntegralInRange<size_t>(0, MAX_SENSOR_MAP_SIZE);
        for (size_t i = 0; i < mapSize; ++i) {
            std::string type = ConsumeThermalZoneType(fdp);
            int32_t temp = fdp.ConsumeIntegralInRange<int32_t>(MIN_TEMP, MAX_TEMP);
            tempMap[type] = temp;
        }
        sensorInfo->SetTypeTempMap(tempMap);
    }
}

void FuzzBaseInfo(FuzzedDataProvider &fdp)
{
    if (g_service == nullptr) {
        return;
    }

    auto baseInfo = g_service->GetBaseinfoObj();
    if (baseInfo == nullptr) {
        return;
    }

    if (fdp.ConsumeBool()) {
        baseInfo->Dump();
    }
}

void SetupClientService(FuzzedDataProvider &fdp, ThermalMgrClient &client)
{
    if (fdp.ConsumeBool()) {
        client.thermalSrv_ = nullptr;
    } else {
        InjectService(client);
    }

    if (fdp.ConsumeBool()) {
        wptr<IRemoteObject> nullRemote;
        client.ResetProxy(nullRemote);
    }
    if (fdp.ConsumeBool() && g_service != nullptr) {
        sptr<IRemoteObject> ro = g_service->AsObject();
        wptr<IRemoteObject> wro = ro;
        client.ResetProxy(wro);
        InjectService(client);
    }
}

void FuzzClientSubscriptions(FuzzedDataProvider &fdp, ThermalMgrClient &client,
    const std::vector<std::string> &typeList, const std::vector<std::string> &actionList,
    const std::string &desc, const sptr<IThermalTempCallback> &tempCb,
    const sptr<IThermalLevelCallback> &levelCb, const sptr<IThermalActionCallback> &actionCb)
{
    if (fdp.ConsumeBool()) {
        (void)client.SubscribeThermalTempCallback(typeList, tempCb);
    }
    if (fdp.ConsumeBool()) {
        (void)client.UnSubscribeThermalTempCallback(tempCb);
    }
    if (fdp.ConsumeBool()) {
        (void)client.SubscribeThermalLevelCallback(levelCb);
    }
    if (fdp.ConsumeBool()) {
        (void)client.UnSubscribeThermalLevelCallback(levelCb);
    }
    if (fdp.ConsumeBool()) {
        (void)client.SubscribeThermalActionCallback(actionList, desc, actionCb);
    }
    if (fdp.ConsumeBool()) {
        (void)client.UnSubscribeThermalActionCallback(actionCb);
    }
}

void FuzzClientOperations(FuzzedDataProvider &fdp, ThermalMgrClient &client,
    const std::string &scene, const std::string &tag, const std::string &val, bool isImmed)
{
    if (fdp.ConsumeBool()) {
        (void)client.GetThermalSensorTemp(ConsumeSensorType(fdp));
    }
    if (fdp.ConsumeBool()) {
        (void)client.GetThermalLevel();
    }
    if (fdp.ConsumeBool()) {
        (void)client.SetScene(scene);
    }
    if (fdp.ConsumeBool()) {
        (void)client.UpdateThermalState(tag, val, isImmed);
    }
    if (fdp.ConsumeBool()) {
        (void)client.UpdateThermalPolicy();
    }
}

void FuzzClientDump(FuzzedDataProvider &fdp, ThermalMgrClient &client)
{
    if (fdp.ConsumeBool()) {
        std::vector<std::string> args = ConsumeStringVector(fdp);
        (void)client.Dump(args);
    }
    if (fdp.ConsumeBool()) {
        std::vector<std::string> args;
        size_t cnt = fdp.ConsumeIntegralInRange<size_t>(MIN_CLIENT_DUMP_ARGS, MAX_CLIENT_DUMP_ARGS);
        for (size_t i = 0; i < cnt; ++i) {
            args.emplace_back(ConsumeSafeString(fdp, MAX_STR_LEN));
        }
        (void)client.Dump(args);
    }
}

void FuzzClientOnce(FuzzedDataProvider &fdp)
{
    auto &client = ThermalMgrClient::GetInstance();

    SetupClientService(fdp, client);

    auto typeList = ConsumeStringVector(fdp);
    auto actionList = ConsumeStringVector(fdp);
    std::string desc = ConsumeSafeString(fdp, MAX_STR_LEN);
    std::string scene = ConsumeSafeString(fdp, MAX_STR_LEN);
    std::string tag = ConsumeSafeString(fdp, MAX_STR_LEN);
    std::string val = ConsumeSafeString(fdp, MAX_STR_LEN);
    bool isImmed = fdp.ConsumeBool();

    sptr<IThermalTempCallback> tempCb = fdp.ConsumeBool()
        ? sptr<IThermalTempCallback>(new ThermalTempTestCallback()) : nullptr;
    sptr<IThermalLevelCallback> levelCb = fdp.ConsumeBool()
        ? sptr<IThermalLevelCallback>(new ThermalLevelTestCallback()) : nullptr;
    sptr<IThermalActionCallback> actionCb = fdp.ConsumeBool()
        ? sptr<IThermalActionCallback>(new ThermalActionTestCallback()) : nullptr;

    FuzzClientSubscriptions(fdp, client, typeList, actionList, desc, tempCb, levelCb, actionCb);
    FuzzClientOperations(fdp, client, scene, tag, val, isImmed);
    FuzzClientDump(fdp, client);
}
} // namespace

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    (void)argc;
    (void)argv;
    g_service = ThermalService::GetInstance();
    if (g_service != nullptr) {
        g_service->InitSystemTestModules();
        g_service->OnStart();
        auto baseInfo = g_service->GetBaseinfoObj();
        if (baseInfo != nullptr) {
            baseInfo->Init();
        }
        auto observer = g_service->GetObserver();
        if (observer != nullptr) {
            observer->InitSensorTypeMap();
        }
    }
    return 0;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return 0;
    }

    FuzzedDataProvider fdp(data, size);

    FuzzClientOnce(fdp);

    if (fdp.ConsumeBool()) {
        FuzzThermalCallbackEvent(fdp);
    }
    if (fdp.ConsumeBool()) {
        FuzzTempEmulation(fdp);
    }
    if (fdp.ConsumeBool()) {
        FuzzServiceDump(fdp);
    }
    if (fdp.ConsumeBool()) {
        FuzzShellDump(fdp);
    }
    if (fdp.ConsumeBool()) {
        FuzzOnAddSystemAbility(fdp);
    }
    if (fdp.ConsumeBool()) {
        FuzzThermalPolicy(fdp);
    }
    if (fdp.ConsumeBool()) {
        FuzzThermalObserver(fdp);
    }
    if (fdp.ConsumeBool()) {
        FuzzThermalActionManager(fdp);
    }
    if (fdp.ConsumeBool()) {
        FuzzStateMachine(fdp);
    }
    if (fdp.ConsumeBool()) {
        FuzzServiceSubscriber(fdp);
    }
    if (fdp.ConsumeBool()) {
        FuzzServiceMisc(fdp);
    }
    if (fdp.ConsumeBool()) {
        FuzzSensorInfo(fdp);
    }
    if (fdp.ConsumeBool()) {
        FuzzBaseInfo(fdp);
    }

    return 0;
}
