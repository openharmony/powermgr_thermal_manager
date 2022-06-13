/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

import router from '@ohos.router'

export default {
    data: {
        temperature: router.getParams().temperature,
        dialogTitle: "",
        cancelButton: "Cancel",
    },
    onInit() {
        console.info('getParams: ' + router.getParams());
        if (this.temperature == "LowerTemp") {
            this.dialogTitle = this.$t("message.lowerTemperature");
        } else if (this.temperature == "HighTemp") {
            this.dialogTitle = this.$t("message.highTemperature");
        }
        this.cancelButton = this.$t("message.cancelButton");
    },
    onCancel() {
        console.info('click cancel');
        callNativeHandler("EVENT_CANCEL", "cancel");
    }
}
