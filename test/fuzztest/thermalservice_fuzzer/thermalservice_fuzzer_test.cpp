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

#define FUZZ_PROJECT_NAME "thermalservice_fuzzer"
#define private   public
#define protected public

#include "thermal_fuzzer_test.h"
#include "ithermal_srv.h"
#include "thermal_service.h"

using namespace OHOS::PowerMgr;
using namespace OHOS;

namespace {
sptr<ThermalService> g_service = nullptr;
void ThermalServiceFuzzTest001(const uint8_t* data, size_t size)
{
    g_service = ThermalService::GetInstance();
    g_service->InitSystemTestModules();
    g_service->GetBaseinfoObj()->Init();
    g_service->InitThermalObserver();
    g_service->GetObserver()->InitSensorTypeMap();
    g_service->OnStart();
    std::string deviceId = "";
    g_service->OnAddSystemAbility(COMMON_EVENT_SERVICE_ID, deviceId);
    g_service->OnAddSystemAbility(SOC_PERF_SERVICE_SA_ID, deviceId);
    g_service->OnAddSystemAbility(POWER_MANAGER_BATT_SERVICE_ID, deviceId);
    g_service->RegisterBootCompletedCallback();
    std::vector<std::string> typeList;
    sptr<IThermalTempCallback> tempCallback = nullptr;
    g_service->SubscribeThermalTempCallback(typeList, tempCallback);
    g_service->UnSubscribeThermalTempCallback(tempCallback);
    sptr<IThermalLevelCallback> levelCallback = nullptr;
    g_service->SubscribeThermalLevelCallback(levelCallback);
    g_service->UnSubscribeThermalLevelCallback(levelCallback);
    std::vector<std::string> actionList;
    actionList.push_back("cpu_big");
    std::string desc = "";
    sptr<IThermalActionCallback> actionCallback = nullptr;
    g_service->SubscribeThermalActionCallback(actionList, desc, actionCallback);
    g_service->UnSubscribeThermalActionCallback(actionCallback);
    g_service->RegisterThermalHdiCallback();
    g_service->UnRegisterThermalHdiCallback();
    HdfThermalCallbackInfo event;
    ThermalZoneInfo info;
    info.type = "battery";
    info.temp = 0;
    event.info.push_back(info);
    g_service->HandleThermalCallbackEvent(event);
    TypeTempMap tempMap;
    g_service->isTempReport_ = false;
    g_service->HandleTempEmulation(tempMap);
    g_service->RegisterFanHdiCallback();
    g_service->HandleFanCallbackEvent(event);
    int fd = 0;
    std::vector<std::u16string> args;
    args.push_back(u"-h");
    g_service->Dump(fd, args);
    g_service->isBootCompleted_ = true;
    g_service->Dump(fd, args);
    g_service->ready_ = false;
    g_service->OnStop();
    g_service->EnableMock("", nullptr);
    g_service->DestroyInstance();
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    ThermalServiceFuzzTest001(data, size);
    return 0;
}
