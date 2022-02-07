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
 * Provides interfaces to obtain thermal information.
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
         * Indicates the device is cool.
         */
        COOL = 0,
        /**
         * Indicates the device is in normal working condition, but not cool.
         */
        NORMAL = 1,
        /**
         * Indicates the device is in a warm state.
         */
        WARM = 2,
        /**
         * Indicates the device is in a hot state.
         */
        HOT = 3,
        /**
         * Indicates the device is in a state of severe heating.
         */
        OVERHEATED = 4,
        /**
         * Indicates the device is about to enter a state of emergency.
         */
        WARNING = 5,
        /**
         * Indicates the device is in a state of emergency
         * and must stop working immediately.
         */
        EMERGENCY = 6,
    }
    /**
     * Subscribe to thermal level change notification.
     *
     * @param callback Specified callback method.
     * @return Return thermal level.
     * @since 8
     */
    function subscribeThermalLevel(callback: AsyncCallback<ThermalLevel>): void;

    /**
     * Cancel thermal level change subscription.
     *
     * @param callback Specified callback method.
     * @return Null.
     * @since 8
     */
    function unsubscribeThermalLevel(callback?: AsyncCallback<void>): void;

    /**
     * Get the current thermal level information.
     *
     * @return Return the thermal level value.
     * @since 8
     */
    function getThermalLevel(): number;
}
export default thermal;
