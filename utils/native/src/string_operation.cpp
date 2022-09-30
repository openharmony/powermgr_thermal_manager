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

#include "string_operation.h"

#include <cstdint>
#include <string>

namespace OHOS {
namespace PowerMgr {
namespace {
const uint32_t COMPARE_SUCCESS = 0;
}
void StringOperation::SplitString(const std::string &str, std::vector<std::string> &ret, const std::string &sep)
{
    ClearAllSpace(str);
    if (str.empty()) {
        return;
    }

    std::string temp;
    std::string::size_type begin = str.find_first_not_of(sep);
    while (begin != std::string::npos) {
        std::string::size_type pos = str.find(sep, begin);
        if (pos != std::string::npos) {
            temp = str.substr(begin, pos - begin);
            begin = pos + sep.length();
        } else {
            temp = str.substr(begin);
            begin = pos;
        }

        if (!temp.empty()) {
            ret.push_back(temp);
            temp.clear();
        }
    }
    return;
}

void StringOperation::ClearAllSpace(std::string s)
{
    std::string::size_type index = 0;
    if (!s.empty()) {
        index = s.find(' ', index);
        while (index != std::string::npos) {
            s.erase(index, 1);
        }
    }
}

bool StringOperation::Compare(const std::string &origin, const std::string &target)
{
    if (origin.compare(target) == COMPARE_SUCCESS) {
        return true;
    }
    return false;
}

bool StringOperation::Find(const std::string &origin, const std::string &target)
{
    if (origin.find(target) != std::string::npos) {
        return true;
    }
    return false;
}
} // namespace PowerMgr
} // namespace OHOS