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


#ifndef STATE_COLLECTION_FACTORY_H
#define STATE_COLLECTION_FACTORY_H

#include "istate_collection.h"

namespace OHOS {
namespace PowerMgr {
class StateCollectionFactory {
public:
    static std::shared_ptr<IStateCollection> Create(std::string &stateName);
};
} // namespace PowerMgr
} // namespace OHOS
#endif // STATE_COLLECTION_FACTORY_H