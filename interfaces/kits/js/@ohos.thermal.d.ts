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

declare namespace thermal {
    /**
     * thermal level definitions
     */
    export enum ThermalLevel {
        /**
        * Thermal Level: COOL
        */
        COOL = 0,
        /**
         * Thermal Level: WARM
         */
        NORMAL = 1,
        /**
         * Thermal Level: WARM
         */

        WARM = 2,
        /**
         * Thermal Level: HOT
         */
        HOT = 3,
        /**
         * Thermal Level: OVERHEATED
         */
        OVERHEATED = 4,
        /**
         * Thermal Level: WARNING
         */
        WARNING = 5,
        /**
         * Thermal Level: EMERGENCY
         */
        EMERGENCY = 6,
    }
    /**
     * subscribe to the change notify of thermal level
     *
     * @devices phone
     * @since 6
     * @SysCap ces
     * @param callback Specified callback method.
     * @return -
     * @testapi
     */
    function subscribeThermalLevel(callback: AsyncCallback<ThermalLevel>): void;

    /**
    * unsubscribe to the change notify of thermal level
    *
    * @devices phone
    * @since 6
    * @SysCap ces
    * @param callback Specified callback method.
    * @return -
    * @testapi
    */
    function unsubscribeThermalLevel(callback?: AsyncCallback<void>): void;

    /**
    * directly obtain the temperature level
    *
    * @devices phone
    * @since 6
    * @SysCap ces
    * @param
    * @return - the thermal level value
    * @testapi
    */
    function getThermalLevel(): number;
}
export default thermal;
