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

import { AsyncCallback, Callback } from './basic';

/**
 * 提供系统热档位相关的回调及查询接口，可用于温控动作的输入指导。
 * {@link subscribeThermalLevel} 可用于订阅热档位回调，档位变化时能及时通知
 * {@link getThermalLevel} 可用于实时查询当前系统的热档位信息
 *
 * @SysCap SystemCapability.PowerMgr.ThermalManager
 * @since 8
 */
declare namespace thermal {
    /**
     * Enumerates the {@link ThermalLevel} types.
     *
     * @since 8
     */
    export enum ThermalLevel {
        /**
         * 表明设备处于凉爽的状态，业务执行不受限制。
         */
        COOL = 0,
        /**
         * 表明设备处于普通工作状态，但并不凉爽，需要注意临近发热状态。
         */
        NORMAL = 1,
        /**
         * 表明设备已经进入温热状态，部分无感知业务需要考虑停止或延迟执行。
         */
        WARM = 2,
        /**
         * 表明设备已经明显发热，无感知业务应全面停止，其他业务应考虑降规格及负载。
         */
        HOT = 3,
        /**
         * 表明设备已经发热严重，无感知业务应全面停止，主要业务需降低规格及负载。
         */
        OVERHEATED = 4,
        /**
         * 表明设备已经发热严重并且即将进入紧急状态，无感知业务应全面停止，主要业务应降低至最低规格。
         */
        WARNING = 5,
        /**
         * 表明设备已经进入紧急状态，所有业务应当全面停止工作，可保留部分紧急求助功能。
         */
        EMERGENCY = 6,
    }
    /**
     * 订阅热档位变化时的回调提醒。
     *
     * @param callback Specified callback method.
     * @return Return thermal level.
     * @since 8
     */
    function subscribeThermalLevel(callback: AsyncCallback<ThermalLevel>): void;

    /**
     * 取消热档位变化的订阅。
     *
     * @param callback Specified callback method.
     * @return Null.
     * @since 8
     */
    function unsubscribeThermalLevel(callback?: AsyncCallback<void>): void;

    /**
     * 获取当前的热档位信息。
     *
     * @return Return the thermal level value.
     * @since 8
     */
    function getThermalLevel(): ThermalLevel;
}
export default thermal;
