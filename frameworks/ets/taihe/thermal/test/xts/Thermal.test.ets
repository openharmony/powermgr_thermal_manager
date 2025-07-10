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

import { describe, it, expect, TestType, Size, Level, beforeAll} from "../../index";
import { BusinessError } from '@ohos.base'
import hilog from '@ohos.hilog'
import Utils from './Util.test';
import thermal from '@ohos.thermal'

let domain: number = 0x0000;
let tag: string = 'testTag';
const ERROR_CODE = 201;

export default function ThermalTest() {

  describe("ThermalTest", (): void => {
    hilog.info(domain, tag, '%{public}s', 'describe start');
    it("ThermalTest0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'ThermalTest0100 start');
        let TAG = 'ThermalTest0100';
        try {
          thermal.registerThermalLevelCallback((level: thermal.ThermalLevel) => {
            console.info(TAG + ' registerThermalLevelCallback level ', level);
            expect(new Number(level)).assertLargerOrEqual(new Number(thermal.ThermalLevel.COOL));
            expect(new Number(level)).assertLessOrEqual(new Number(thermal.ThermalLevel.ESCAPE));
          });
          thermal.unregisterThermalLevelCallback();
          console.info(TAG + ' end');
          done();
        } catch (e) {
          hilog.error(domain, tag, '%{public}s', JSON.stringify(e));
          done();
        }
      })
    it("ThermalTest0200", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'ThermalTest0200 start');
        let TAG = 'ThermalTest0200';
        try {
          thermal.unregisterThermalLevelCallback(() => {
            console.info(TAG + ' unregisterThermalLevelCallback success');
          });
          console.info(TAG + ' end');
          done();
        } catch (e) {
          hilog.error(domain, tag, '%{public}s', JSON.stringify(e));
          expect().assertFail();
          done();
        }
      })
    it("ThermalTest0300", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'ThermalTest0300 start');
        let TAG = 'ThermalTest0300';
        try {
          let level = thermal.getLevel();
          expect(new Number(level)).assertLargerOrEqual(new Number(thermal.ThermalLevel.COOL));
          expect(new Number(level)).assertLessOrEqual(new Number(thermal.ThermalLevel.ESCAPE));
          console.info(TAG + ' end');
          done();
        } catch (e) {
          hilog.error(domain, tag, '%{public}s', JSON.stringify(e));
          done();
        }
      })
    it("ThermalTest0400", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'ThermalTest0400 start');
        let TAG = 'ThermalTest0400';
        try {
          let level = thermal.ThermalLevel.COOL;
          expect(level === 0).assertTrue();
          console.info(TAG + ' end');
          done();
        } catch (e) {
          hilog.error(domain, tag, '%{public}s', JSON.stringify(e));
          done();
        }
      })
    it("ThermalTest0500", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'ThermalTest0500 start');
        let TAG = 'ThermalTest0500';
        try {
          let level = thermal.ThermalLevel.NORMAL;
          expect(level === 1).assertTrue();
          console.info(TAG + ' end');
          done();
        } catch (e) {
          hilog.error(domain, tag, '%{public}s', JSON.stringify(e));
          done();
        }
      })
    it("ThermalTest0600", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'ThermalTest0600 start');
        let TAG = 'ThermalTest0600';
        try {
          let level = thermal.ThermalLevel.WARM;
          expect(level === 2).assertTrue();
          console.info(TAG + ' end');
          done();
        } catch (e) {
          hilog.error(domain, tag, '%{public}s', JSON.stringify(e));
          done();
        }
      })
    it("ThermalTest0700", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'ThermalTest0700 start');
        let TAG = 'ThermalTest0700';
        try {
          let level = thermal.ThermalLevel.HOT;
          expect(level === 3).assertTrue();
          console.info(TAG + ' end');
          done();
        } catch (e) {
          hilog.error(domain, tag, '%{public}s', JSON.stringify(e));
          done();
        }
      })
    it("ThermalTest0800", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'ThermalTest0800 start');
        let TAG = 'ThermalTest0800';
        try {
          let level = thermal.ThermalLevel.OVERHEATED;
          expect(level === 4).assertTrue();
          console.info(TAG + ' end');
          done();
        } catch (e) {
          hilog.error(domain, tag, '%{public}s', JSON.stringify(e));
          done();
        }
      })
    it("ThermalTest0900", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'ThermalTest0900 start');
        let TAG = 'ThermalTest0900';
        try {
          let level = thermal.ThermalLevel.WARNING;
          expect(level === 5).assertTrue();
          console.info(TAG + ' end');
          done();
        } catch (e) {
          hilog.error(domain, tag, '%{public}s', JSON.stringify(e));
          done();
        }
      })
    it("ThermalTest1000", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'ThermalTest1000 start');
        let TAG = 'ThermalTest1000';
        try {
          let level = thermal.ThermalLevel.EMERGENCY;
          expect(level === 6).assertTrue();
          console.info(TAG + ' end');
          done();
        } catch (e) {
          hilog.error(domain, tag, '%{public}s', JSON.stringify(e));
          done();
        }
      })
    it("ThermalTest1100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'ThermalTest1100 start');
        let TAG = 'ThermalTest1100';
        try {
          let level = thermal.ThermalLevel.ESCAPE;
          expect(level === 7).assertTrue();
          console.info(TAG + ' end');
          done();
        } catch (e) {
          hilog.error(domain, tag, '%{public}s', JSON.stringify(e));
          done();
        }
      })
  })
  hilog.info(domain, tag, '%{public}s', 'ThermalTest end');
}