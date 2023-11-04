/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef EXTEND_STATE_COLLECTION_H
#define EXTEND_STATE_COLLECTION_H

#include "istate_collection.h"
#include "thermal_timer.h"

namespace OHOS {
namespace PowerMgr {
class ExtendStateCollection : public IStateCollection {
public:
    bool Init() override;
    bool InitParam(std::string& params) override;
    std::string GetState() override;
    void SetState(const std::string& stateValue) override;
    bool DecideState(const std::string& value) override;
public:
    ExtendStateCollection(const std::string& stateName);
    ~ExtendStateCollection() = default;
private:
    std::string stateName_;
    std::string stateValue_ = "-1";
};
} // namespace PowerMgr
} // namespace OHOS
#endif // EXTEND_STATE_COLLECTION_H