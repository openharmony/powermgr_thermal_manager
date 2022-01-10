/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PROTECTOR_BASE_IFNO_H
#define PROTECTOR_BASE_IFNO_H

#include <string>
#include <vector>

namespace OHOS {
namespace PowerMgr {
struct BaseItem {
    std::string tag;
    std::string value;
};

class ProtectorBaseInfo {
public:
    ProtectorBaseInfo() = default;
    ~ProtectorBaseInfo() = default;
    bool Init();
    void SetBaseItem(std::vector<BaseItem> &vBase)
    {
        vBase_ = vBase;
    }
private:
    std::vector<BaseItem> vBase_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // PROTECTOR_BASE_IFNO_H