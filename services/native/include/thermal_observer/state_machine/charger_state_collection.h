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

#ifndef CHARGER_STATE_COLLECTION_H
#define CHARGER_STATE_COLLECTION_H

#include "istate_collection.h"
#include "thermal_common_event_receiver.h"

using IntentWant = OHOS::AAFwk::Want;
namespace OHOS {
namespace PowerMgr {
class ChargerStateCollection : public IStateCollection {
public:
    enum {
        /**
         * Battery is charging.
         */
        DISABLE = 0,

        /**
         * Battery is not charging.
         */
        ENABLE = 1,

        /**
         * Battery charge full.
         */
        FULL = 2,

        /**
         * Battery is discharge.
         */
        NONE = 3,
        /**
        * The bottom of the enum.
        */
        BUTT = 4,
    };
    ChargerStateCollection() = default;
    ~ChargerStateCollection() = default;
    bool Init() override;
    bool InitParam(std::string &params) override;
    std::string GetState() override;
    virtual bool DecideState(const std::string &value) override;
    void HandleChangerStatusCompleted(const EventFwk::CommonEventData &data);
    bool RegisterEvent();
public:
    virtual void SetState() override;
private:
    std::string state_;
    std::string params_;
    std::string mockState_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // CHARGER_STATE_COLLECTION_H