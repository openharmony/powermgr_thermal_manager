/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

import app from '@system.app'
import Context from '@ohos.napi_context'
import thermal from "@ohos.thermal"
import ThermalLevel from "@ohos.thermal"
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

describe('appInfoTest', function () {
    console.log("*************Thermal API Test Begin*************");
    const MSEC_1000 = 1000;
    const MSEC_10000 = 10000;
    /**
     * @tc.number thermal_manager_js_000
     * @tc.name Thermal_000
     * @tc.desc Thermal acquisition kit
     */
    it('Thermal_000', 0, async function (done) {
        console.info("enter");
        await new Promise((resolve, reject) => {
            setTimeout(() => {
                var cold = thermal.getThermalLevel();
                console.info("cold level is: " + cold);
                expect(cold >= 0 && cold <= 5).assertEqual('1');
                resolve();
            }, MSEC_10000);
        })
        done();
    })

   /**
     * @tc.number thermal_manager_js_001
     * @tc.name Thermal_001
     * @tc.desc Thermal acquisition kit
     */
    it('Thermal_001', 0, async function (done) {
        console.info("enter");
        await new Promise((resolve, reject) => {
            setTimeout(() => {
                var normal = thermal.getThermalLevel();
                console.info("normal level is: " + normal);
                expect(normal >= 0 && normal <= 5).assertEqual('1');
                resolve();
            }, MSEC_10000);
        })
        done();
    })

    /**
     * @tc.number thermal_manager_js_002
     * @tc.name Thermal_002
     * @tc.desc Thermal acquisition kit
     */
    it('Thermal_002', 0, async function (done) {
        await new Promise((resolve, reject) => {
            setTimeout(() => {
                var warm = thermal.getThermalLevel();
                console.info("warm level is: " + warm);
                expect(warm >= 0 && warm <= 5).assertEqual('1');
                resolve();
            }, MSEC_10000);
        })
        done();
    })

    /**
     * @tc.number thermal_manager_js_003
     * @tc.name Thermal_003
     * @tc.desc Thermal acquisition kit
     */
    it('Thermal_003', 0, async function (done) {
        await new Promise((resolve, reject) => {
            setTimeout(() => {
                var hot = thermal.getThermalLevel();
                console.info("hot level is: " + hot);
                expect(hot >= 0 && hot <= 5).assertEqual('1');
                resolve();
            }, MSEC_10000);
        })
        done();
    })

    /**
     * @tc.number thermal_manager_js_004
     * @tc.name Thermal_004
     * @tc.desc Thermal acquisition kit
     */
    it('Thermal_004', 0, async function (done) {
        await new Promise((resolve, reject) => {
            setTimeout(() => {
                var overheated = thermal.getThermalLevel();
                console.info("overheated level is: " + overheated);
                expect(overheated >= 0 && overheated <= 5).assertEqual('1');
                resolve();
            }, MSEC_10000);
        })
        done();
    })

    /**
     * @tc.number thermal_manager_js_005
     * @tc.name Thermal_005
     * @tc.desc Thermal acquisition kit
     */
    it('Thermal_005', 0, async function (done) {
        await new Promise((resolve, reject) => {
            setTimeout(() => {
                var warning = thermal.getThermalLevel();
                console.info("warning level is: " + warning);
                expect(warning >= 0 && warning <= 5).assertEqual('1');
                resolve();
            }, MSEC_10000);
        })
        done();
    })

    /**
     * @tc.number thermal_manager_js_006
     * @tc.name Thermal_006
     * @tc.desc Thermal acquisition kit
     */
    it('Thermal_006', 0, async function (done) {
        await new Promise((resolve, reject) => {
            setTimeout(() => {
                var emergency = thermal.getThermalLevel();
                console.info("emergency level is: " + emergency);
                expect(emergency >= 0 && emergency <= 5).assertEqual('1');
                resolve();
            }, MSEC_10000);
        })
        done();
    })

    /**
     * @tc.number thermal_manager_js_007
     * @tc.name Thermal_007
     * @tc.desc Thermal acquisition kit
     */
    it('Thermal_007', 0, async function (done) {
        var count = 0;
        thermal.subscribeThermalLevel((cool) => {
            console.info("cool level is: " + cool);
            count++;
            expect(cool >= 0 && cool <= 5).assertEqual('1');
        })
        await new Promise((resolve, reject) => {
            setTimeout(() => {
                thermal.unsubscribeThermalLevel();
                resolve();
            }, MSEC_10000);
        })
        expect(count >= 1).assertEqual('1');
        done();
    })

    /**
     * @tc.number thermal_manager_js_008
     * @tc.name Thermal_008
     * @tc.desc Thermal acquisition kit
     */
    it('Thermal_008', 0, async function (done) {
        var count = 0;
        thermal.subscribeThermalLevel((warm) => {
            console.info("cool level is: " + warm);
            count++;
            expect(warm >= 0 && warm <= 5).assertEqual('1');
        })
        await new Promise((resolve, reject) => {
            setTimeout(() => {
                thermal.unsubscribeThermalLevel();
                resolve();
            }, MSEC_10000);
        })
        expect(count >= 1).assertEqual('1');
        done();
    })

    /**
     * @tc.number thermal_manager_js_009
     * @tc.name Thermal_009
     * @tc.desc Thermal acquisition kit
     */
    it('Thermal_009', 0, async function (done) {
        var count = 0;
        thermal.subscribeThermalLevel((hot) => {
            console.info("hot level is: " + hot);
            count++;
            expect(hot >= 0 && hot <= 5).assertEqual('1');
        })
        await new Promise((resolve, reject) => {
            setTimeout(() => {
                thermal.unsubscribeThermalLevel();
                resolve();
            }, MSEC_10000);
        })
        expect(count >= 1).assertEqual('1');
        done();
    })

    /**
     * @tc.number thermal_manager_js_010
     * @tc.name Thermal_010
     * @tc.desc Thermal acquisition kit
     */
    it('Thermal_010', 0, async function (done) {
        var count = 0;
        thermal.subscribeThermalLevel((overheated) => {
            console.info("overheated level is: " + overheated);
            count++;
            expect(overheated >= 0 && overheated <= 5).assertEqual('1');
        })
        await new Promise((resolve, reject) => {
            setTimeout(() => {
                thermal.unsubscribeThermalLevel();
                resolve();
            }, MSEC_10000);
        })
        expect(count >= 1).assertEqual('1');
        done();
    })

    /**
     * @tc.number thermal_manager_js_011
     * @tc.name Thermal_011
     * @tc.desc Thermal acquisition kit
     */
    it('Thermal_011', 0, async function (done) {
        var count = 0;
        thermal.subscribeThermalLevel((warning) => {
            console.info("warning level is: " + warning);
            count++;
            expect(warning >= 0 && warning <= 5).assertEqual('1');
        })
        await new Promise((resolve, reject) => {
            setTimeout(() => {
                thermal.unsubscribeThermalLevel();
                resolve();
            }, MSEC_10000);
        })
        expect(count >= 1).assertEqual('1');
        done();
    })

    /**
     * @tc.number thermal_manager_js_012
     * @tc.name Thermal_012
     * @tc.desc Thermal acquisition kit
     */
    it('Thermal_012', 0, async function (done) {
        var count = 0;
        thermal.subscribeThermalLevel((emergency) => {
            console.info("emergency level is: " + emergency);
            count++;
            expect(emergency >= 0 && emergency <= 5).assertEqual('1');
        })
        await new Promise((resolve, reject) => {
            setTimeout(() => {
                thermal.unsubscribeThermalLevel();
                resolve();
            }, MSEC_10000);
        })
        expect(count >= 1).assertEqual('1');
        done();
    })

    /**
     * @tc.number thermal_manager_js_013
     * @tc.name Thermal_013
     * @tc.desc Thermal acquisition kit
     */
    it('Thermal_013', 0, async function (done) {
        var count = 0;
        thermal.subscribeThermalLevel((normal) => {
            console.info("normal level is: " + normal);
            count++;
            expect(normal >= 0 && normal <= 5).assertEqual('1');
        })
        await new Promise((resolve, reject) => {
            setTimeout(() => {
                thermal.unsubscribeThermalLevel();
                resolve();
            }, MSEC_10000);
        })
        expect(count >= 1).assertEqual('1');
        done();
    })

    /**
     * @tc.number thermal_manager_js_014
     * @tc.name Thermal_014
     * @tc.desc Thermal acquisition kit
     */
    it('Thermal_014', 0, async function (done) {
        var count = 0;
        thermal.subscribeThermalLevel((cool) => {
            console.info("cool level is: " + cool);
            count++;
            expect(cool >= 0 && cool <= 5).assertEqual('1');
        })
        await new Promise((resolve, reject) => {
            setTimeout(() => {
                thermal.unsubscribeThermalLevel();
                resolve();
            }, MSEC_10000 * 2);
        })
        expect(count >= 2).assertEqual('1');
        done();
    })
})
