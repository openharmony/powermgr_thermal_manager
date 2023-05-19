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

#include "string_operation.h"

#include <cmath>
#include <cstdint>
#include <limits.h>
#include <string>

namespace OHOS {
namespace PowerMgr {
namespace {
const uint32_t COMPARE_SUCCESS = 0;
}
void StringOperation::SplitString(const std::string& str, std::vector<std::string>& ret, const std::string& sep)
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

bool StringOperation::Compare(const std::string& origin, const std::string& target)
{
    if (origin.compare(target) == COMPARE_SUCCESS) {
        return true;
    }
    return false;
}

bool StringOperation::Find(const std::string& origin, const std::string& target)
{
    if (origin.find(target) != std::string::npos) {
        return true;
    }
    return false;
}

bool StringOperation::StrToUint(const std::string& str, uint32_t& value)
{
    if (str.empty() || (!isdigit(str.front()) && (str.front() != '-'))) {
        return false;
    }

    char* end = nullptr;
    errno = 0;
    auto addr = str.c_str();
    auto result = strtoul(addr, &end, 10); // 10 means decimal
    if ((end == addr) || (end[0] != '\0') || (errno == ERANGE) || (result >= ULONG_MAX)) {
        return false;
    }
    value = static_cast<uint32_t>(result);
    return true;
}

bool StringOperation::StrToDouble(const std::string& str, double& value)
{
    if (str.empty()) {
        return false;
    }

    char* end = nullptr;
    errno = 0;
    auto addr = str.c_str();
    auto result = strtod(addr, &end);
    if ((end == addr) || (end[0] != '\0') || (errno == ERANGE) || (result == HUGE_VAL)) {
        return false;
    }
    value = result;
    return true;
}
} // namespace PowerMgr
} // namespace OHOS
