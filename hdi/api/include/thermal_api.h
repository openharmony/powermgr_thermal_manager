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

#ifndef THERMAL_API_H
#define THERMAL_API_H

enum ThermalCmd {
    CMD_BIND_THERMAL_SUBSCRIBER,
    CMD_UNBIND_THERMAL_SUBSCRIBER,
    CMD_NOTIFY_SUBSCRIBER_V1,
    CMD_NOTIFY_SUBSCRIBER_V2,
    CMD_GET_THERMALZONEINFO,
    CMD_SET_CPU_FREQ,
    CMD_SET_GPU_FREQ,
    CMD_SET_BATTERY_CURRENT,
    CMD_SET_LCD_BLACKLIGHT,
    CMD_SET_FLAG_CONTROL,
    CMD_SET_SENSOR_TEMP,
};

#endif // THERMAL_API_H