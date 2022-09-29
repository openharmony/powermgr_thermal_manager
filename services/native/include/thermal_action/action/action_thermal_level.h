/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef ACTION_THERMAL_LEVEL_H
#define ACTION_THERMAL_LEVEL_H


#include <functional>
#include <set>
#include <mutex>
#include <common_event_publish_info.h>
#include <want.h>

#include "ithermal_level_callback.h"
#include "thermal_level_info.h"
#include "ithermal_action.h"

namespace OHOS {
namespace PowerMgr {
class ThermalService;

class ActionThermalLevel : public IThermalAction {
public:
    ActionThermalLevel(const std::string& actionName);
    ~ActionThermalLevel();
    explicit ActionThermalLevel(const wptr<ThermalService>& tms);

    void InitParams(const std::string& params) override;
    virtual void SetStrict(bool flag) override;
    virtual void SetEnableEvent(bool enable) override;
    virtual void AddActionValue(std::string value) override;
    virtual void Execute() override;
    bool Init();
    void SubscribeThermalLevelCallback(const sptr<IThermalLevelCallback>& callback);
    void UnSubscribeThermalLevelCallback(const sptr<IThermalLevelCallback>& callback);
    int32_t GetThermalLevel();
    uint32_t LevelRequest(int32_t level);

    void NotifyThermalLevelChanged(int32_t level);
    bool PublishLevelChangedEvents(ThermalCommonEventCode code, int32_t level);
public:
    class ThermalLevelCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        ThermalLevelCallbackDeathRecipient() = default;
        virtual void OnRemoteDied(const wptr<IRemoteObject>& remote);
        virtual ~ThermalLevelCallbackDeathRecipient() = default;
    };
private:
    struct classcomp {
        bool operator()(const sptr<IThermalLevelCallback>& l, const sptr<IThermalLevelCallback>& r) const
        {
            return l->AsObject() < r->AsObject();
        }
    };
private:
    const wptr<ThermalService> tms_;
    std::mutex mutex_;
    sptr<IRemoteObject::DeathRecipient> thermalLevelCBDeathRecipient_;
    std::vector<uint32_t> valueList_;
    std::string params_;
    bool flag_;
    bool enableEvent_ = false;
    static std::set<const sptr<IThermalLevelCallback>, classcomp> thermalLevelListeners_;
    static int32_t level_;
    uint32_t laststValue_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // ACTION_APPLICATION_PROCESS_H
