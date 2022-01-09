/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "thermal_common_file.h"

#include <mutex>
#include <fcntl.h>
#include <unistd.h>

#include "thermal_common.h"

namespace OHOS {
namespace PowerMgr {
using namespace testing::ext;
static std::mutex g_mutex;
int32_t ThermalCommonFile::WriteFile(std::string path, std::string buf, size_t size)
{
    std::lock_guard<std::mutex> lck(g_mutex);
    int32_t fd = open(path.c_str(), O_RDWR);
    if (fd < ERR_OK) {
        GTEST_LOG_(INFO) << "WriteFile: failed to open file";
        return ERR_INVALID_VALUE;
    }
    write(fd, buf.c_str(), size);
    close(fd);
    return ERR_OK;
}

int32_t ThermalCommonFile::ReadFile(const char *path, char *buf, size_t size)
{
    std::lock_guard<std::mutex> lck(g_mutex);
    int32_t ret;

    int32_t fd = open(path, O_RDONLY);
    if (fd < ERR_OK) {
        GTEST_LOG_(INFO) << "WriteFile: failed to open file" << fd;
        return ERR_INVALID_VALUE;
    }

    ret = read(fd, buf, size);
    if (ret < ERR_OK) {
        GTEST_LOG_(INFO) << "WriteFile: failed to read file" << ret;
        close(fd);
        return ERR_INVALID_VALUE;
    }

    close(fd);
    buf[size - 1] = '\0';
    return ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS