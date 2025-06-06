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

#ifndef STRING_OPERATION_H
#define STRING_OPERATION_H

#include <iosfwd>
#include <vector>

namespace OHOS {
namespace PowerMgr {
class StringOperation {
public:
    static void SplitString(const std::string& str, std::vector<std::string>& ret, const std::string& sep);
    static void ClearAllSpace(std::string s);
    static bool Compare(const std::string& origin, const std::string& target);
    static bool Find(const std::string& origin, const std::string& target);
    static bool StrToUint(const std::string& str, uint32_t& value);
    static bool StrToDouble(const std::string& str, double& value);
    static bool ParseStrtollResult(const std::string& str, int64_t& result);
    static bool ParseStrtoulResult(const std::string& str, unsigned long& result);
};
} // namespace PowerMgr
} // namespace OHOS
#endif // STRING_OPERATION_H
