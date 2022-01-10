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

#ifndef HILOG_WRAPPER_H
#define HILOG_WRAPPER_H

#define CONFIG_HILOG
#ifdef CONFIG_HILOG
#include "hilog/log.h"
namespace OHOS {
namespace PowerMgr {
#define __FILENAME__            (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)
#define __FORMATED(fmt, ...)    "[%{public}s] %{public}s# " fmt, __FILENAME__, __FUNCTION__, ##__VA_ARGS__

#ifdef THERMAL_HILOGF
#undef THERMAL_HILOGF
#endif

#ifdef THERMAL_HILOGE
#undef THERMAL_HILOGE
#endif

#ifdef THERMAL_HILOGW
#undef THERMAL_HILOGW
#endif

#ifdef THERMAL_HILOGI
#undef THERMAL_HILOGI
#endif

#ifdef THERMAL_HILOGD
#undef THERMAL_HILOGD
#endif

// param of log interface, such as THERMAL_HILOGF.
enum THERMALMgrSubModule {
    MODULE_THERMAL_INNERKIT = 0,
    MODULE_THERMALHDI_SERVICE,
    MODULE_THERMAL_JAVAKIT, // java kit, defined to avoid repeated use of domain.
    MODULE_THERMAL_JNI,
    MODULE_THERMAL_PROTECTOR,
    MODULE_THERMAL_JS_NAPI,
    MODULE_THERMALMGR_SERVICE,
    MODULE_THERMALHDI_CLIENT,
    MODULE_THERMAL_COMMON,
    THERMALMGR_MODULE_BUTT,
};

// 0xD002900: subsystem:THERMALMgr module:THERMALMgr, reserved 8 bit.
static constexpr unsigned int BASE_THERMALMGR_DOMAIN_ID = 0xD002900;

enum THERMALMgrDomainId {
    THERMALMGR_INNERKIT_DOMAIN = BASE_THERMALMGR_DOMAIN_ID + MODULE_THERMAL_INNERKIT,
    THERMALHDI_SERVICE_DOMAIN,
    THERMALMGR_JAVAKIT_DOMAIN,
    THERMALMGR_PROTECTOR_DOMAIN,
    THERMALMGR_JS_NAPI_DOMAIN,
    THERMALMGR_SERVICE_DOMAIN,
    THERMALHDI_CLIENT_DOMAIN,
    THERMAL_COMMON_DOMAIN,
    THERMALMGR_BUTT,
};

static constexpr OHOS::HiviewDFX::HiLogLabel THERMAL_MGR_LABEL[THERMALMGR_MODULE_BUTT] = {
    {LOG_CORE, THERMALMGR_INNERKIT_DOMAIN, "THERMALMGRClient"},
    {LOG_CORE, THERMALHDI_SERVICE_DOMAIN, "THERMALHDService"},
    {LOG_CORE, THERMALMGR_JAVAKIT_DOMAIN, "THERMALMGRJavaService"},
    {LOG_CORE, THERMALMGR_INNERKIT_DOMAIN, "THERMALMGRJni"},
    {LOG_CORE, THERMALMGR_PROTECTOR_DOMAIN, "THERMALMGRProtector"},
    {LOG_CORE, THERMALMGR_JS_NAPI_DOMAIN, "THERMALMGRJSNapi"},
    {LOG_CORE, THERMALMGR_SERVICE_DOMAIN, "THERMALMGRService"},
    {LOG_CORE, THERMALHDI_CLIENT_DOMAIN, "THERMALHDIClient"},
    {LOG_CORE, THERMAL_COMMON_DOMAIN, "THERMALCommon"},
};

// In order to improve performance, do not check the module range, module should less than THERMALMGR_MODULE_BUTT.
#define THERMAL_HILOGF(module, ...) \
    (void)OHOS::HiviewDFX::HiLog::Fatal(THERMAL_MGR_LABEL[module], __FORMATED(__VA_ARGS__))
#define THERMAL_HILOGE(module, ...) \
    (void)OHOS::HiviewDFX::HiLog::Error(THERMAL_MGR_LABEL[module], __FORMATED(__VA_ARGS__))
#define THERMAL_HILOGW(module, ...) \
    (void)OHOS::HiviewDFX::HiLog::Warn(THERMAL_MGR_LABEL[module], __FORMATED(__VA_ARGS__))
#define THERMAL_HILOGI(module, ...) \
    (void)OHOS::HiviewDFX::HiLog::Info(THERMAL_MGR_LABEL[module], __FORMATED(__VA_ARGS__))
#define THERMAL_HILOGD(module, ...) \
    (void)OHOS::HiviewDFX::HiLog::Debug(THERMAL_MGR_LABEL[module], __FORMATED(__VA_ARGS__))
} // namespace THERMALMgr
} // namespace OHOS

#else

#define THERMAL_HILOGF(...)
#define THERMAL_HILOGE(...)
#define THERMAL_HILOGW(...)
#define THERMAL_HILOGI(...)
#define THERMAL_HILOGD(...)

#endif // CONFIG_HILOG
#endif // HILOG_WRAPPER_H