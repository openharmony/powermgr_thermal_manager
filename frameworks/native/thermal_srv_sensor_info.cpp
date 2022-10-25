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

#include "thermal_srv_sensor_info.h"

#include "string_ex.h"
#include "thermal_common.h"

namespace OHOS {
namespace PowerMgr {
ThermalSrvSensorInfo::ThermalSrvSensorInfo(std::string &type, int32_t &temp): type_(type), temp_(temp)
{}

/**
 * @brief read this Sequenceable object from a Parcel.
 *
 * @param inParcel Indicates the Parcel object into which the Sequenceable object has been marshaled.
 * @return Returns true if read successed; returns false otherwise.
 */
bool ThermalSrvSensorInfo::ReadFromParcel(Parcel &parcel)
{
    type_ = Str16ToStr8(parcel.ReadString16());
    temp_ = parcel.ReadInt32();
    return true;
}

/**
 * @brief Unmarshals this Sequenceable object from a Parcel.
 *
 * @param inParcel Indicates the Parcel object into which the Sequenceable object has been marshaled.
 */
ThermalSrvSensorInfo *ThermalSrvSensorInfo::Unmarshalling(Parcel &parcel)
{
    ThermalSrvSensorInfo *thermalSrvSensorInfo = new (std::nothrow) ThermalSrvSensorInfo();
    if (thermalSrvSensorInfo && !thermalSrvSensorInfo->ReadFromParcel(parcel)) {
        THERMAL_HILOGE(COMP_FWK, "ThermalSrvSensorInfo::Unmarshalling ReadFromParcel failed");
        delete thermalSrvSensorInfo;
        thermalSrvSensorInfo = nullptr;
    }
    return thermalSrvSensorInfo;
}

/**
 * @brief Marshals this Sequenceable object into a Parcel.
 *
 * @param outParcel Indicates the Parcel object to which the Sequenceable object will be marshaled.
 */

bool ThermalSrvSensorInfo::Marshalling(Parcel &parcel) const
{
    return (parcel.WriteString16(Str8ToStr16(type_)) && parcel.WriteInt32(temp_));
}
} // namespace PowerMgr
} // namespace OHOS
