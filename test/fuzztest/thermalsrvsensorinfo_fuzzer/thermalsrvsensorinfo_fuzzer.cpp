/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#define FUZZ_PROJECT_NAME "thermalsrvsensorinfo_fuzzer"

#include <cstddef>
#include <cstdint>
#include <string>

#include "message_parcel.h"
#include "securec.h"
#include "thermal_srv_sensor_info.h"

using namespace OHOS::PowerMgr;
using namespace OHOS;

namespace {
const int32_t MIN_SIZE = 4;
const size_t MAX_TYPE_LENGTH = 32;
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < MIN_SIZE) {
        return false;
    }

    MessageParcel parcel;
    parcel.WriteBuffer(data, size);
    parcel.RewindRead(0);

    ThermalSrvSensorInfo sensorInfo;
    sensorInfo.ReadFromParcel(parcel);

    sensorInfo.GetType();
    sensorInfo.GetTemp();

    parcel.RewindRead(0);
    ThermalSrvSensorInfo* info = ThermalSrvSensorInfo::Unmarshalling(parcel);
    if (info != nullptr) {
        info->GetType();
        info->GetTemp();

        MessageParcel marshalParcel;
        info->Marshalling(marshalParcel);

        delete info;
        info = nullptr;
    }

    int32_t temp = static_cast<int32_t>(*(reinterpret_cast<const int32_t*>(data)));
    std::string type(reinterpret_cast<const char*>(data), size > MAX_TYPE_LENGTH ? MAX_TYPE_LENGTH : size);
    sensorInfo.SetType(type);
    sensorInfo.SetTemp(temp);

    sensorInfo.GetType();
    sensorInfo.GetTemp();

    MessageParcel outParcel;
    sensorInfo.Marshalling(outParcel);

    return true;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
