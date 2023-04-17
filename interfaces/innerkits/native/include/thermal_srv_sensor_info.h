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

#ifndef THERMAL__SRV_SENSOR_INFO_H
#define THERMAL__SRV_SENSOR_INFO_H

#include <string>
#include <parcel.h>
#include <map>
#include "ability_info.h"

namespace OHOS {
namespace PowerMgr {
class ThermalSrvSensorInfo : public Parcelable {
public:
    ThermalSrvSensorInfo() = default;
    explicit ThermalSrvSensorInfo(std::string &type, int32_t &temp);
    virtual ~ThermalSrvSensorInfo() = default;

    inline const std::string &GetType() const
    {
        return type_;
    }

    inline const int32_t &GetTemp() const
    {
        return temp_;
    }

    inline void SetType(const std::string &type)
    {
        type_ = type;
    }

    inline void SetTemp(const int32_t &temp)
    {
        temp_ = temp;
    }
    /**
     * @brief read this Sequenceable object from a Parcel.
     *
     * @param inParcel Indicates the Parcel object into which the Sequenceable object has been marshaled.
     * @return Returns true if read successed; returns false otherwise.
     */
    bool ReadFromParcel(Parcel &parcel);

    /**
     * @brief Marshals this Sequenceable object into a Parcel.
     *
     * @param outParcel Indicates the Parcel object to which the Sequenceable object will be marshaled.
     */
    virtual bool Marshalling(Parcel &parcel) const override;

    /**
     * @brief Unmarshals this Sequenceable object from a Parcel.
     *
     * @param inParcel Indicates the Parcel object into which the Sequenceable object has been marshaled.
     */
    static ThermalSrvSensorInfo *Unmarshalling(Parcel &parcel);

private:
    std::string type_;
    int32_t temp_;
};

enum class SensorType : uint32_t {
    SOC = 0,
    BATTERY,
    SHELL,
    SENSOR1,
    SENSOR2,
    SENSOR3,
    SENSOR4,
    SENSOR5,
    SENSOR6,
    SENSOR7
};
} // PowerMgr
} // OHOS

#endif // THERMAL__SRV_SENSOR_INFO_H