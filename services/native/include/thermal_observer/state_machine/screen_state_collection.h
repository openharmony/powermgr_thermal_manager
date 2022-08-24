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

#ifndef SCREEN_STATE_COLLECTION_H
#define SCREEN_STATE_COLLECTION_H

#include <common_event_subscriber.h>
#include "istate_collection.h"

using IntentWant = OHOS::AAFwk::Want;
namespace OHOS {
namespace PowerMgr {
class ScreenStateCollection : public IStateCollection {
public:
    ScreenStateCollection() = default;
    ~ScreenStateCollection() = default;
    bool Init() override;
    bool InitParam(std::string& params) override;
    std::string GetState() override;
    virtual bool DecideState(const std::string& value) override;
    void HandleScreenOnCompleted(const EventFwk::CommonEventData& data);
    void HandleScreenOffCompleted(const EventFwk::CommonEventData& data);
    bool RegisterEvent();
    virtual void SetState() override;
private:
    std::string params_;
    std::string state_;
    std::string mockState_;
};
}
}
#endif // SCREEN_STATE_COLLECTION_H
