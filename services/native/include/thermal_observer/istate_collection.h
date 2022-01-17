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

#ifndef ISTATE_COLLECTION_H
#define ISTATE_COLLECTION_H

#include <string>
#include <vector>
namespace OHOS {
namespace PowerMgr {
class IStateCollection {
public:
    IStateCollection() = default;
    virtual ~IStateCollection() = default;
    virtual bool Init();
    virtual bool InitParam(std::string &params) = 0;
    virtual std::string GetState() = 0;
    virtual bool DecideState(const std::string &value);
    /* Test */
    virtual void SetState() = 0;
};
}
}
#endif // ISTATE_COLLECTION_H