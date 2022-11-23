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

#include "file_operation.h"

#include <cstdio>
#include <string>
#include <fcntl.h>
#include <mutex>
#include <unistd.h>
#include <sys/stat.h>
#include "bits/fcntl.h"
#include "errors.h"
#include "thermal_log.h"

namespace OHOS {
namespace PowerMgr {
static std::mutex g_mutex;
int32_t FileOperation::CreateNodeDir(std::string dir)
{
    THERMAL_HILOGD(COMP_SVC, "Enter");
    if (access(dir.c_str(), 0) != ERR_OK) {
        int flag = mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH| S_IXOTH);
        if (flag == ERR_OK) {
            THERMAL_HILOGI(COMP_SVC, "Create directory successfully.");
        } else {
            THERMAL_HILOGE(COMP_SVC, "Fail to create directory, flag: %{public}d", flag);
            return flag;
        }
    } else {
        THERMAL_HILOGE(COMP_SVC, "This directory already exists.");
    }
    return ERR_OK;
}

int32_t FileOperation::CreateNodeFile(std::string filePath)
{
    if (access(filePath.c_str(), 0) != 0) {
        int32_t fd = open(filePath.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP| S_IROTH);
        if (fd < ERR_OK) {
            THERMAL_HILOGE(COMP_SVC, "open failed to file.");
            return fd;
        }
        close(fd);
    } else {
        THERMAL_HILOGI(COMP_SVC, "the file already exists.");
    }
    return ERR_OK;
}

int32_t FileOperation::WriteFile(std::string path, std::string buf, size_t size)
{
    FILE *stream = fopen(path.c_str(), "w+");
    if (stream == nullptr) {
        return ERR_INVALID_VALUE;
    }
    size_t ret = fwrite(buf.c_str(), strlen(buf.c_str()), 1, stream);
    if (ret == ERR_OK) {
        THERMAL_HILOGE(COMP_SVC, "ret=%{public}zu", ret);
    }
    int32_t state = fseek(stream, 0, SEEK_SET);
    if (state != ERR_OK) {
        fclose(stream);
        return state;
    }
    state = fclose(stream);
    if (state != ERR_OK) {
        return state;
    }
    return ERR_OK;
}

int32_t FileOperation::ReadFile(const char *path, char *buf, size_t size)
{
    std::lock_guard<std::mutex> lck(g_mutex);
    int32_t ret = -1;

    int32_t fd = open(path, O_RDONLY, S_IRUSR | S_IRGRP | S_IROTH);
    if (fd < ERR_OK) {
        THERMAL_HILOGE(COMP_SVC, "open failed to file.");
        return fd;
    }

    ret = read(fd, buf, size);
    if (ret < ERR_OK) {
        THERMAL_HILOGE(COMP_SVC, "failed to read file.");
        close(fd);
        return ret;
    }

    close(fd);
    buf[size - 1] = '\0';
    return ERR_OK;
}

int32_t FileOperation::ConvertInt(const std::string &value)
{
    return std::stoi(value);
}
} // namespace PowerMgr
} // namespace OHOS