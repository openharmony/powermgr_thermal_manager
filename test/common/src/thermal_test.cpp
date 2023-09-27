/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "thermal_test.h"

#include <cstdio>
#include <cstdlib>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <securec.h>
#include <sys/stat.h>

#include "errors.h"
#include "string_ex.h"
#include "thermal_log.h"

using namespace testing::ext;
using namespace OHOS;
using namespace std;

namespace OHOS {
namespace PowerMgr {
int32_t ThermalTest::WriteFile(std::string path, std::string buf)
{
    FILE *fp = fopen(path.c_str(), "w+");
    if (fp == nullptr) {
        return ERR_INVALID_VALUE;
    }
    size_t num = fwrite(buf.c_str(), sizeof(char), buf.length(), fp);
    if (num != buf.length()) {
        THERMAL_HILOGD(COMP_SVC, "Failed to fwrite %{public}s, num=%{public}zu", path.c_str(), num);
        fclose(fp);
        return ERR_INVALID_OPERATION;
    }
    fclose(fp);
    return ERR_OK;
}

int32_t ThermalTest::ReadFile(std::string path, std::string& buf)
{
    std::ifstream input(path);
    if (!input.is_open()) {
        return ERR_INVALID_VALUE;
    }
    getline(input, buf);
    input.close();
    return ERR_OK;
}

int32_t ThermalTest::ConvertInt(const std::string& value)
{
    int32_t result = -1;
    StrToInt(value, result);
    return result;
}

int32_t ThermalTest::InitNode()
{
    char bufTemp[MAX_PATH] = {0};
    int32_t ret = -1;
    std::map<std::string, int32_t> sensor;
    sensor["battery"] = 0;
    sensor["charger"] = 0;
    sensor["pa"] = 0;
    sensor["ap"] = 0;
    sensor["ambient"] = 0;
    sensor["cpu"] = 0;
    sensor["soc"] = 0;
    sensor["shell"] = 0;
    for (auto iter : sensor) {
        ret = snprintf_s(bufTemp, MAX_PATH, sizeof(bufTemp) - 1, SIMULATION_TEMP_DIR, iter.first.c_str());
        if (ret < EOK) {
            return ret;
        }
        std::string temp = std::to_string(iter.second);
        WriteFile(bufTemp, temp);
    }
    return ERR_OK;
}

bool ThermalTest::IsMock(const std::string& path)
{
    struct stat pathStat;
    int32_t ret = stat(path.c_str(), &pathStat);
    if (ret != 0) {
        return false;
    }
    if (S_ISREG(pathStat.st_mode)) {
        return true;
    }
    if (!S_ISDIR(pathStat.st_mode)) {
        return false;
    }
    DIR* dir = opendir(path.c_str());
    if (dir == nullptr) {
        return false;
    }
    struct dirent* ptr = nullptr;
    while ((ptr = readdir(dir)) != nullptr) {
        if (strcmp(".", ptr->d_name) != 0 && strcmp("..", ptr->d_name) != 0) {
            closedir(dir);
            return true;
        }
    }
    closedir(dir);
    return false;
}

bool ThermalTest::IsVendor()
{
    if (access(VENDOR_CONFIG.c_str(), 0) != 0) {
        return false;
    }
    return true;
}

std::string ThermalTest::GetNodeValue(const std::string& path)
{
    std::string value {};
    int32_t ret = ThermalTest::ReadFile(path, value);
    EXPECT_EQ(true, ret == ERR_OK);
    return value;
}

int32_t ThermalTest::SetNodeValue(int32_t value, const std::string& path)
{
    std::string sValue = to_string(value);
    int32_t ret = ThermalTest::WriteFile(path, sValue);
    EXPECT_EQ(true, ret == ERR_OK);
    return ret;
}

int32_t ThermalTest::SetNodeString(std::string str, const std::string& path)
{
    int32_t ret = ThermalTest::WriteFile(path, str);
    EXPECT_EQ(true, ret == ERR_OK);
    return ret;
}
} // namespace PowerMgr
} // namespace OHOS
