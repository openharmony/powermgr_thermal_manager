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

#include "cj_thermal_manager.h"
#include "thermal_common.h"
#include "thermal_level_info.h"
#include "cj_lambda.h"
#include "thermal_log.h"

using namespace OHOS;
using namespace OHOS::PowerMgr;
CjThermalLevelCallback::~CjThermalLevelCallback()
{
    ReleaseCallback();
}

void CjThermalLevelCallback::UpdateCallback(int64_t id)
{
    auto callback = reinterpret_cast<void(*)(int32_t)>(id);
    callbackFunc_ = CJLambda::Create(callback);
}

void CjThermalLevelCallback::ReleaseCallback()
{
    callbackFunc_ = nullptr;
}

bool CjThermalLevelCallback::OnThermalLevelChanged(ThermalLevel level)
{
    if (callbackFunc_) {
        callbackFunc_(static_cast<int32_t>(level));
    }
    return true;
}


CjThermalManager& CjThermalManager::GetInstance()
{
    thread_local CjThermalManager thermalMgr;
    return thermalMgr;
}

int32_t CjThermalManager::GetThermalLevel()
{
    ThermalLevel level = thermalMgrClient_.GetThermalLevel();
    return static_cast<int32_t>(level);
}

void CjThermalManager::SubscribeThermalLevel(int64_t id)
{
    thermalLevelCallback_->UpdateCallback(id);
    thermalMgrClient_.SubscribeThermalLevelCallback(thermalLevelCallback_);
}

void CjThermalManager::UnSubscribeThermalLevel(int64_t id)
{
    thermalLevelCallback_->ReleaseCallback();
    thermalMgrClient_.UnSubscribeThermalLevelCallback(thermalLevelCallback_);
    if (id != 0) {
        auto callback = reinterpret_cast<void(*)()>(id);
        CJLambda::Create(callback)();
    }
}