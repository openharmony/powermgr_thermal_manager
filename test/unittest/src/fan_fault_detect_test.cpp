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

#include "fan_fault_detect_test.h"

#include <atomic>
#include <condition_variable>
#include <mutex>

#include "hisysevent.h"
#include "hisysevent_listener.h"
#include "hisysevent_manager.h"
#include "hisysevent_record.h"
#include "thermal_log.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;
using namespace OHOS::PowerMgr;

namespace {
const std::string DOMAIN = "THERMAL";
const std::string EVENT = "FAN_FAULT";
const std::string TAG = "ID";
const std::string FAN = "fan";
const std::string GPU = "gpu";
const std::string SOC = "soc";
const int32_t FAN_SLOW_THERHOLD = 500;
const int32_t FAN_FAST_THERHOLD = 1500;
const int32_t TEMP_HIGH_THERHOLD = 50000;
const int32_t TEMP_LOW_THERHOLD = 30000;
const int32_t FAN_SLOW_SPEED = 400;
const int32_t FAN_FAST_SPEED = 1600;
const int32_t TEMP_HIGH = 60000;
const int32_t TEMP_LOW = 20000;
const int64_t TIME_OUT = 2;
std::mutex g_mutex;
std::condition_variable g_callbackCV;
std::atomic_bool g_eventTriggered = false;

class Watcher : public HiSysEventListener {
public:
    explicit Watcher(std::function<void(std::shared_ptr<HiSysEventRecord>)> func) : func_(func) {}
    virtual ~Watcher() {}

    void OnEvent(std::shared_ptr<HiSysEventRecord> sysEvent) final
    {
        if (sysEvent == nullptr || func_ == nullptr) {
            return;
        }
        func_(sysEvent);
    }

    void OnServiceDied() final {}

private:
    std::function<void(std::shared_ptr<HiSysEventRecord>)> func_;
};
} // namespace

void FanFaultDetectTest::SetUpTestCase() {}

void FanFaultDetectTest::TearDownTestCase() {}

void FanFaultDetectTest::SetUp() {}

void FanFaultDetectTest::TearDown() {}

void FanFaultDetectTest::InitFanFaultInfoMap(const std::shared_ptr<FanFaultDetect>& fanFaultDetect)
{
    FanSensorInfo fanSlowSensorInfo;
    fanSlowSensorInfo.insert(std::make_pair(FAN, FAN_SLOW_THERHOLD));
    fanSlowSensorInfo.insert(std::make_pair(SOC, TEMP_HIGH_THERHOLD));
    fanSlowSensorInfo.insert(std::make_pair(GPU, TEMP_HIGH_THERHOLD));

    FanSensorInfo fanFastSensorInfo;
    fanFastSensorInfo.insert(std::make_pair(FAN, FAN_FAST_THERHOLD));
    fanFastSensorInfo.insert(std::make_pair(SOC, TEMP_LOW_THERHOLD));
    fanFastSensorInfo.insert(std::make_pair(GPU, TEMP_LOW_THERHOLD));

    FanFaultInfoMap fanFaultInfoMap;
    fanFaultInfoMap.insert(std::make_pair(FAN_FAULT_TOO_SLOW, fanSlowSensorInfo));
    fanFaultInfoMap.insert(std::make_pair(FAN_FAULT_TOO_FAST, fanFastSensorInfo));

    fanFaultDetect->SetFaultInfoMap(fanFaultInfoMap);
}

void FanFaultDetectTest::GetFaultId(int64_t& faultId, const FanSensorInfo& report)
{
    std::shared_ptr<FanFaultDetect> fanFaultDetect = std::make_shared<FanFaultDetect>();
    EXPECT_NE(fanFaultDetect, nullptr);
    InitFanFaultInfoMap(fanFaultDetect);

    auto watcher = std::make_shared<Watcher>([&faultId] (std::shared_ptr<HiSysEventRecord> sysEvent) {
        if (sysEvent == nullptr) {
            return;
        }
        sysEvent->GetParamValue(TAG, faultId);
        g_eventTriggered = true;
        g_callbackCV.notify_one();
    });

    OHOS::HiviewDFX::ListenerRule listenerRule(DOMAIN, EVENT, OHOS::HiviewDFX::RuleType::WHOLE_WORD);
    std::vector<OHOS::HiviewDFX::ListenerRule> sysRules;
    sysRules.emplace_back(listenerRule);
    auto ret = OHOS::HiviewDFX::HiSysEventManager::AddListener(watcher, sysRules);
    EXPECT_TRUE(ret == SUCCESS);

    fanFaultDetect->OnFanSensorInfoChanged(report);
    std::unique_lock<std::mutex> lock(g_mutex);
    g_callbackCV.wait_for(lock, std::chrono::seconds(TIME_OUT), [] {
        return g_eventTriggered.load();
    });
    g_eventTriggered = false;

    ret = OHOS::HiviewDFX::HiSysEventManager::RemoveListener(watcher);
    EXPECT_TRUE(ret == SUCCESS);
}

namespace {
#if EVENT_FAN
/**
 * @tc.name: FanFaultDetectTest001
 * @tc.desc: test class FanFaultDetectTest function
 * @tc.type: FUNC
 */
HWTEST_F(FanFaultDetectTest, FanFaultDetectTest001, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "FanFaultDetectTest001 start");
    int64_t faultId = FAN_FAULT_OK;
    FanSensorInfo report;
    report.insert(std::make_pair(FAN, FAN_SLOW_SPEED));
    report.insert(std::make_pair(SOC, TEMP_HIGH));
    report.insert(std::make_pair(GPU, TEMP_HIGH));
    GetFaultId(faultId, report);
    EXPECT_TRUE(faultId == FAN_FAULT_TOO_SLOW);
    THERMAL_HILOGD(LABEL_TEST, "FanFaultDetectTest001 end");
}

/**
 * @tc.name: FanFaultDetectTest002
 * @tc.desc: test class FanFaultDetectTest function
 * @tc.type: FUNC
 */
HWTEST_F(FanFaultDetectTest, FanFaultDetectTest002, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "FanFaultDetectTest002 start");
    int64_t faultId = FAN_FAULT_OK;
    FanSensorInfo report;
    report.insert(std::make_pair(FAN, FAN_SLOW_SPEED));
    report.insert(std::make_pair(SOC, TEMP_HIGH));
    report.insert(std::make_pair(GPU, TEMP_LOW));
    GetFaultId(faultId, report);
    EXPECT_TRUE(faultId == FAN_FAULT_TOO_SLOW);
    THERMAL_HILOGD(LABEL_TEST, "FanFaultDetectTest002 end");
}

/**
 * @tc.name: FanFaultDetectTest003
 * @tc.desc: test class FanFaultDetectTest function
 * @tc.type: FUNC
 */
HWTEST_F(FanFaultDetectTest, FanFaultDetectTest003, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "FanFaultDetectTest003 start");
    int64_t faultId = FAN_FAULT_OK;
    FanSensorInfo report;
    report.insert(std::make_pair(FAN, FAN_SLOW_SPEED));
    report.insert(std::make_pair(SOC, TEMP_LOW));
    report.insert(std::make_pair(GPU, TEMP_HIGH));
    GetFaultId(faultId, report);
    EXPECT_TRUE(faultId == FAN_FAULT_TOO_SLOW);
    THERMAL_HILOGD(LABEL_TEST, "FanFaultDetectTest003 end");
}

/**
 * @tc.name: FanFaultDetectTest004
 * @tc.desc: test class FanFaultDetectTest function
 * @tc.type: FUNC
 */
HWTEST_F(FanFaultDetectTest, FanFaultDetectTest004, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "FanFaultDetectTest004 start");
    int64_t faultId = FAN_FAULT_OK;
    FanSensorInfo report;
    report.insert(std::make_pair(FAN, FAN_FAST_SPEED));
    report.insert(std::make_pair(SOC, TEMP_LOW));
    report.insert(std::make_pair(GPU, TEMP_LOW));
    GetFaultId(faultId, report);
    EXPECT_TRUE(faultId == FAN_FAULT_TOO_FAST);
    THERMAL_HILOGD(LABEL_TEST, "FanFaultDetectTest004 end");
}
#endif

/**
 * @tc.name: FanFaultDetectTest005
 * @tc.desc: test class FanFaultDetectTest function
 * @tc.type: FUNC
 */
HWTEST_F(FanFaultDetectTest, FanFaultDetectTest005, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "FanFaultDetectTest005 start");
    int64_t faultId = FAN_FAULT_OK;
    FanSensorInfo report;
    report.insert(std::make_pair(FAN, FAN_SLOW_SPEED));
    report.insert(std::make_pair(SOC, TEMP_LOW));
    report.insert(std::make_pair(GPU, TEMP_LOW));
    GetFaultId(faultId, report);
    EXPECT_TRUE(faultId == FAN_FAULT_OK);
    THERMAL_HILOGD(LABEL_TEST, "FanFaultDetectTest005 end");
}

/**
 * @tc.name: FanFaultDetectTest006
 * @tc.desc: test class FanFaultDetectTest function
 * @tc.type: FUNC
 */
HWTEST_F(FanFaultDetectTest, FanFaultDetectTest006, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "FanFaultDetectTest006 start");
    int64_t faultId = FAN_FAULT_OK;
    FanSensorInfo report;
    report.insert(std::make_pair(FAN, FAN_FAST_SPEED));
    report.insert(std::make_pair(SOC, TEMP_HIGH));
    report.insert(std::make_pair(GPU, TEMP_LOW));
    GetFaultId(faultId, report);
    EXPECT_TRUE(faultId == FAN_FAULT_OK);
    THERMAL_HILOGD(LABEL_TEST, "FanFaultDetectTest006 end");
}

/**
 * @tc.name: FanFaultDetectTest007
 * @tc.desc: test class FanFaultDetectTest function
 * @tc.type: FUNC
 */
HWTEST_F(FanFaultDetectTest, FanFaultDetectTest007, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "FanFaultDetectTest007 start");
    int64_t faultId = FAN_FAULT_OK;
    FanSensorInfo report;
    report.insert(std::make_pair(FAN, FAN_FAST_SPEED));
    report.insert(std::make_pair(SOC, TEMP_LOW));
    report.insert(std::make_pair(GPU, TEMP_HIGH));
    GetFaultId(faultId, report);
    EXPECT_TRUE(faultId == FAN_FAULT_OK);
    THERMAL_HILOGD(LABEL_TEST, "FanFaultDetectTest007 end");
}

/**
 * @tc.name: FanFaultDetectTest008
 * @tc.desc: test class FanFaultDetectTest function
 * @tc.type: FUNC
 */
HWTEST_F(FanFaultDetectTest, FanFaultDetectTest008, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "FanFaultDetectTest008 start");
    int64_t faultId = FAN_FAULT_OK;
    FanSensorInfo report;
    report.insert(std::make_pair(FAN, FAN_FAST_SPEED));
    report.insert(std::make_pair(SOC, TEMP_HIGH));
    report.insert(std::make_pair(GPU, TEMP_HIGH));
    GetFaultId(faultId, report);
    EXPECT_TRUE(faultId == FAN_FAULT_OK);
    THERMAL_HILOGD(LABEL_TEST, "FanFaultDetectTest008 end");
}
} // namespace
