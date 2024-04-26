/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "thermal_mock_stub_test.h"

#include <vector>

#ifdef THERMAL_GTEST
#define private   public
#define protected public
#endif

#include "thermal_action_callback_stub.h"
#include "thermal_level_callback_stub.h"
#include "thermal_log.h"
#include "thermal_mgr_errors.h"
#include "thermal_service.h"
#include "thermal_srv_ipc_interface_code.h"
#include "thermal_srv_proxy.h"
#include "thermal_srv_stub.h"
#include "thermal_temp_callback_stub.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
MessageParcel g_data;
MessageParcel g_reply;
MessageOption g_option;
sptr<ThermalService> g_service = nullptr;
} // namespace

void ThermalMockStubTest::SetUpTestCase()
{
    g_service = ThermalService::GetInstance();
    g_service->InitSystemTestModules();
    g_service->OnStart();
}

namespace {
/**
 * @tc.name: ThermalMockStubTest001
 * @tc.desc: proxy test
 * @tc.type: FUNC
 * @tc.require: issueI5YZQ2
 */
HWTEST_F(ThermalMockStubTest, ThermalMockStubTest001, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubTest001 start.");
    uint32_t code = 0;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    sptr<ThermalActionCallbackStub> actionStub = new ThermalActionCallbackStub();
    ASSERT_FALSE(actionStub == nullptr);
    EXPECT_EQ(E_GET_THERMAL_SERVICE_FAILED, actionStub->OnRemoteRequest(code, data, reply, option));
    sptr<ThermalLevelCallbackStub> levelStub = new ThermalLevelCallbackStub();
    ASSERT_FALSE(levelStub == nullptr);
    EXPECT_EQ(E_GET_THERMAL_SERVICE_FAILED, levelStub->OnRemoteRequest(code, data, reply, option));
    sptr<ThermalTempCallbackStub> tempStub = new ThermalTempCallbackStub();
    ASSERT_FALSE(tempStub == nullptr);
    EXPECT_EQ(E_GET_THERMAL_SERVICE_FAILED, tempStub->OnRemoteRequest(code, data, reply, option));
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubTest001 end.");
}

/**
 * @tc.name: ThermalMockStubTest002
 * @tc.desc: OnRemoteRequest all
 * @tc.type: FUNC
 * @tc.require: issueI6KRS8
 */
HWTEST_F(ThermalMockStubTest, ThermalMockStubTest002, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubTest002 start.");
    uint32_t begin = static_cast<uint32_t>(PowerMgr::ThermalMgrInterfaceCode::REG_THERMAL_TEMP_CALLBACK);
    uint32_t end = static_cast<uint32_t>(PowerMgr::ThermalMgrInterfaceCode::SHELL_DUMP);
    for (uint32_t code = begin; code <= end; ++code) {
        g_data.WriteInterfaceToken(ThermalSrvProxy::GetDescriptor());
        auto ret = g_service->OnRemoteRequest(code, g_data, g_reply, g_option);
        EXPECT_TRUE(ret == E_READ_PARCEL_ERROR_THERMAL || ret == E_GET_SYSTEM_ABILITY_MANAGER_FAILED_THERMAL ||
            ret == 0);
    }
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubTest002 end.");
}

/**
 * @tc.name: ThermalMockStubTest003
 * @tc.desc: OnRemoteRequest ReadInterfaceToken
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMockStubTest, ThermalMockStubTest003, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubTest003 start.");
    uint32_t code = 0;
    int32_t ret = g_service->OnRemoteRequest(code, g_data, g_reply, g_option);
    EXPECT_EQ(ret, E_GET_THERMAL_SERVICE_FAILED) << "code: " << code << " ret:" << ret;
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubTest003 end.");
}

/**
 * @tc.name: ThermalMockStubTest004
 * @tc.desc: OnRemoteRequest Invalid code
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMockStubTest, ThermalMockStubTest004, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubTest004 start.");
    uint32_t code = 9999;
    g_data.WriteInterfaceToken(ThermalSrvProxy::GetDescriptor());
    int32_t ret = g_service->OnRemoteRequest(code, g_data, g_reply, g_option);
    EXPECT_EQ(ret, IPC_STUB_UNKNOW_TRANS_ERR) << "code: " << code << " ret:" << ret;
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubTest004 end.");
}

/**
 * @tc.name: ThermalMockStubTest005
 * @tc.desc: SubscribeThermalTempCallbackStub
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMockStubTest, ThermalMockStubTest005, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubTest005 start.");

    sptr<ThermalTempCallbackStub> tempStub = new ThermalTempCallbackStub();
    std::vector<std::string> typeList;
    g_data.WriteInterfaceToken(ThermalSrvProxy::GetDescriptor());
    THERMAL_WRITE_PARCEL_NO_RET(g_data, RemoteObject, tempStub->AsObject());
    THERMAL_WRITE_PARCEL_NO_RET(g_data, StringVector, typeList);
    uint32_t code = static_cast<uint32_t>(PowerMgr::ThermalMgrInterfaceCode::REG_THERMAL_TEMP_CALLBACK);
    int32_t ret = g_service->OnRemoteRequest(code, g_data, g_reply, g_option);
    EXPECT_EQ(ret, ERR_OK) << " ret:" << ret;

    g_data.WriteInterfaceToken(ThermalSrvProxy::GetDescriptor());
    THERMAL_WRITE_PARCEL_NO_RET(g_data, RemoteObject, tempStub->AsObject());
    code = static_cast<uint32_t>(PowerMgr::ThermalMgrInterfaceCode::UNREG_THERMAL_TEMP_CALLBACK);
    ret = g_service->OnRemoteRequest(code, g_data, g_reply, g_option);
    EXPECT_EQ(ret, ERR_OK) << " ret:" << ret;
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubTest005 end.");
}

/**
 * @tc.name: ThermalMockStubTest006
 * @tc.desc: SubscribeThermalLevelCallbackStub
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMockStubTest, ThermalMockStubTest006, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubTest006 start.");

    MessageParcel data;
    sptr<ThermalLevelCallbackStub> levelStub = new ThermalLevelCallbackStub();
    g_data.WriteInterfaceToken(ThermalSrvProxy::GetDescriptor());
    THERMAL_WRITE_PARCEL_NO_RET(g_data, RemoteObject, levelStub->AsObject());
    uint32_t code = static_cast<uint32_t>(PowerMgr::ThermalMgrInterfaceCode::REG_THERMAL_LEVEL_CALLBACK);
    int32_t ret = g_service->OnRemoteRequest(code, g_data, g_reply, g_option);
    EXPECT_EQ(ret, ERR_OK) << " ret:" << ret;

    code = static_cast<uint32_t>(PowerMgr::ThermalMgrInterfaceCode::UNREG_THERMAL_LEVEL_CALLBACK);
    g_data.WriteInterfaceToken(ThermalSrvProxy::GetDescriptor());
    THERMAL_WRITE_PARCEL_NO_RET(g_data, RemoteObject, levelStub->AsObject());
    ret = g_service->OnRemoteRequest(code, g_data, g_reply, g_option);
    EXPECT_EQ(ret, ERR_OK) << " ret:" << ret;

    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubTest006 end.");
}

/**
 * @tc.name: ThermalMockStubTest007
 * @tc.desc: SubscribeThermalActionCallbackStub
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMockStubTest, ThermalMockStubTest007, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubTest007 start.");

    sptr<ThermalActionCallbackStub> actionStub = new ThermalActionCallbackStub();
    g_data.WriteInterfaceToken(ThermalSrvProxy::GetDescriptor());
    std::vector<std::string> actionList;
    THERMAL_WRITE_PARCEL_NO_RET(g_data, RemoteObject, actionStub->AsObject());
    THERMAL_WRITE_PARCEL_NO_RET(g_data, StringVector, actionList);
    THERMAL_WRITE_PARCEL_NO_RET(g_data, String, "ThermalMockStubTest007");
    uint32_t code = static_cast<uint32_t>(PowerMgr::ThermalMgrInterfaceCode::REG_THERMAL_ACTION_CALLBACK);
    int32_t ret = g_service->OnRemoteRequest(code, g_data, g_reply, g_option);
    EXPECT_EQ(ret, ERR_OK) << " ret:" << ret;

    code = static_cast<uint32_t>(PowerMgr::ThermalMgrInterfaceCode::UNREG_THERMAL_ACTION_CALLBACK);
    g_data.WriteInterfaceToken(ThermalSrvProxy::GetDescriptor());
    THERMAL_WRITE_PARCEL_NO_RET(g_data, RemoteObject, actionStub->AsObject());
    ret = g_service->OnRemoteRequest(code, g_data, g_reply, g_option);
    EXPECT_EQ(ret, ERR_OK) << " ret:" << ret;

    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubTest007 end.");
}

/**
 * @tc.name: ThermalMockStubTest008
 * @tc.desc: GetThermalSrvSensorInfo
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMockStubTest, ThermalMockStubTest008, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubTest008 start.");

    g_data.WriteInterfaceToken(ThermalSrvProxy::GetDescriptor());
    THERMAL_WRITE_PARCEL_NO_RET(g_data, Uint32, static_cast<uint32_t>(SensorType::BATTERY));
    uint32_t code = static_cast<uint32_t>(PowerMgr::ThermalMgrInterfaceCode::GET_SENSOR_INFO);
    int32_t ret = g_service->OnRemoteRequest(code, g_data, g_reply, g_option);
    EXPECT_EQ(ret, ERR_OK) << " ret:" << ret;

    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubTest008 end.");
}

/**
 * @tc.name: ThermalMockStubTest009
 * @tc.desc: SetSceneStub
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMockStubTest, ThermalMockStubTest009, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubTest009 start.");

    g_data.WriteInterfaceToken(ThermalSrvProxy::GetDescriptor());
    THERMAL_WRITE_PARCEL_NO_RET(g_data, String, "test");
    uint32_t code = static_cast<uint32_t>(PowerMgr::ThermalMgrInterfaceCode::SET_SCENE);
    int32_t ret = g_service->OnRemoteRequest(code, g_data, g_reply, g_option);
    EXPECT_EQ(ret, ERR_OK) << " ret:" << ret;

    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubTest009 end.");
}

/**
 * @tc.name: ThermalMockStubTest010
 * @tc.desc: ShellDumpStub
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMockStubTest, ThermalMockStubTest010, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubTest010 start.");

    g_data.WriteInterfaceToken(ThermalSrvProxy::GetDescriptor());
    const int32_t PARAM_MAX_NUM = 100;
    g_data.WriteUint32(PARAM_MAX_NUM);
    uint32_t code = static_cast<uint32_t>(PowerMgr::ThermalMgrInterfaceCode::SHELL_DUMP);
    int32_t ret = g_service->OnRemoteRequest(code, g_data, g_reply, g_option);
    EXPECT_EQ(ret, E_EXCEED_PARAM_LIMIT) << " ret:" << ret;

    g_data.WriteInterfaceToken(ThermalSrvProxy::GetDescriptor());
    g_data.WriteUint32(1);
    g_data.WriteString("");
    ret = g_service->OnRemoteRequest(code, g_data, g_reply, g_option);
    EXPECT_EQ(ret, E_READ_PARCEL_ERROR_THERMAL) << " ret:" << ret;

    g_data.WriteInterfaceToken(ThermalSrvProxy::GetDescriptor());
    g_data.WriteUint32(1);
    g_data.WriteString("123");
    ret = g_service->OnRemoteRequest(code, g_data, g_reply, g_option);
    EXPECT_EQ(ret, ERR_OK) << " ret:" << ret;

    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubTest010 end.");
}
} // namespace
