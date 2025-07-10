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

import thermal from "@ohos.thermal";

function main() {
    try {
        console.info('registerThermalLevelCallback start');
        thermal.registerThermalLevelCallback((level: thermal.ThermalLevel) => {
            console.info('registerThermalLevelCallback level ', level);
        });
        console.info('registerThermalLevelCallback end');
        console.info('unregisterThermalLevelCallback start');
        thermal.unregisterThermalLevelCallback();
        console.info('unregisterThermalLevelCallback end');
        console.info('unregisterThermalLevelCallback fun start');
        thermal.unregisterThermalLevelCallback(() => {
            console.info('unregisterThermalLevelCallback success');
        });
        console.info('unregisterThermalLevelCallback fun end');
        console.info('getLevel start');
        let level = thermal.getLevel();
        console.info('getLevel ', level);
        console.info('getLevel end');
    } catch (err) {
        console.error("test thermal err: " + err)
    }
}