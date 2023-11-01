/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "thermal_mock_stub_getdescriptor_test.h"

#include "ipc_types.h"
#include "thermal_action_callback_ipc_interface_code.h"
#include "thermal_action_callback_stub.h"
#include "thermal_level_callback_ipc_interface_code.h"
#include "thermal_level_callback_stub.h"
#include "thermal_log.h"
#include "thermal_mgr_errors.h"
#include "thermal_srv_stub.h"
#include "thermal_temp_callback_ipc_interface_code.h"
#include "thermal_temp_callback_stub.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
/**
 * @tc.name: ThermalMockStubGetdescriptorTest001
 * @tc.desc: stub test
 * @tc.type: FUNC
 * @tc.require: issueI5YZQ2
 */
HWTEST_F(ThermalMockStubGetdescriptorTest, ThermalMockStubGetdescriptorTest001, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubGetdescriptorTest001 start.");
    uint32_t code = 0;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    sptr<ThermalActionCallbackStub> actionStub = new ThermalActionCallbackStub();
    ASSERT_FALSE(actionStub == nullptr);
    EXPECT_TRUE(E_GET_THERMAL_SERVICE_FAILED == actionStub->OnRemoteRequest(code, data, reply, option));
    sptr<ThermalLevelCallbackStub> levelStub = new ThermalLevelCallbackStub();
    ASSERT_FALSE(levelStub == nullptr);
    EXPECT_TRUE(E_GET_THERMAL_SERVICE_FAILED == levelStub->OnRemoteRequest(code, data, reply, option));
    sptr<ThermalTempCallbackStub> tempStub = new ThermalTempCallbackStub();
    ASSERT_FALSE(tempStub == nullptr);
    EXPECT_TRUE(E_GET_THERMAL_SERVICE_FAILED == tempStub->OnRemoteRequest(code, data, reply, option));
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubGetdescriptorTest001 end.");
}

/**
 * @tc.name: ThermalMockStubGetdescriptorTest002
 * @tc.desc: stub CallbackStub
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMockStubGetdescriptorTest, ThermalMockStubGetdescriptorTest002, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubGetdescriptorTest002 start.");
    sptr<ThermalActionCallbackStub> actionStub = new ThermalActionCallbackStub();
    IThermalActionCallback::ActionCallbackMap actionCbMap;
    EXPECT_TRUE(actionStub->OnThermalActionChanged(actionCbMap));

    sptr<ThermalLevelCallbackStub> levelStub = new ThermalLevelCallbackStub();
    ThermalLevel level = ThermalLevel::COOL;
    EXPECT_TRUE(levelStub->OnThermalLevelChanged(level));

    sptr<ThermalTempCallbackStub> tempStub = new ThermalTempCallbackStub();
    IThermalTempCallback::TempCallbackMap tempCbMap;
    EXPECT_TRUE(tempStub->OnThermalTempChanged(tempCbMap));

    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubGetdescriptorTest002 end.");
}

/**
 * @tc.name: ThermalMockStubGetdescriptorTest003
 * @tc.desc: stub OnRemoteRequest Invalid code
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMockStubGetdescriptorTest, ThermalMockStubGetdescriptorTest003, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubGetdescriptorTest003 start.");
    uint32_t code = 9999;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    sptr<ThermalActionCallbackStub> actionStub = new ThermalActionCallbackStub();
    data.WriteInterfaceToken(ThermalActionCallbackStub::GetDescriptor());
    int32_t ret = actionStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, IPC_STUB_UNKNOW_TRANS_ERR) << "ret: " << ret << " code: " << code;

    sptr<ThermalLevelCallbackStub> levelStub = new ThermalLevelCallbackStub();
    data.WriteInterfaceToken(ThermalLevelCallbackStub::GetDescriptor());
    ret = levelStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, IPC_STUB_UNKNOW_TRANS_ERR) << "ret: " << ret << " code: " << code;

    sptr<ThermalTempCallbackStub> tempStub = new ThermalTempCallbackStub();
    data.WriteInterfaceToken(ThermalTempCallbackStub::GetDescriptor());
    ret = tempStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, IPC_STUB_UNKNOW_TRANS_ERR) << "ret: " << ret << " code: " << code;

    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubGetdescriptorTest003 end.");
}

/**
 * @tc.name: ThermalMockStubGetdescriptorTest004
 * @tc.desc: stub ThermalTempCallbackStub OnRemoteRequest THERMAL_TEMPERATURE_CHANGED
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMockStubGetdescriptorTest, ThermalMockStubGetdescriptorTest004, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubGetdescriptorTest004 start.");
    sptr<ThermalTempCallbackStub> tempStub = new ThermalTempCallbackStub();
    uint32_t code =
        static_cast<uint32_t>(PowerMgr::ThermalTempCallbackInterfaceCode::THERMAL_TEMPERATURE_CHANGED);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(ThermalTempCallbackStub::GetDescriptor());
    data.WriteUint32(1);
    data.WriteString("ThermalMockStubGetdescriptorTest004");
    data.WriteInt32(1024);
    int32_t ret = tempStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, ERR_OK) << "ret: " << ret << " code: " << code;

    // Read Uint32 parcel error
    data.WriteInterfaceToken(ThermalTempCallbackStub::GetDescriptor());
    ret = tempStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, E_READ_PARCEL_ERROR_THERMAL) << "ret: " << ret << " code: " << code;

    // Read String parcel error
    data.WriteInterfaceToken(ThermalTempCallbackStub::GetDescriptor());
    data.WriteUint32(1);
    ret = tempStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, E_READ_PARCEL_ERROR_THERMAL) << "ret: " << ret << " code: " << code;

    // Read Int32 parcel error
    data.WriteInterfaceToken(ThermalTempCallbackStub::GetDescriptor());
    data.WriteUint32(1);
    data.WriteString("ThermalMockStubGetdescriptorTest004");
    ret = tempStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, E_READ_PARCEL_ERROR_THERMAL) << "ret: " << ret << " code: " << code;

    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubGetdescriptorTest004 end.");
}

/**
 * @tc.name: ThermalMockStubGetdescriptorTest005
 * @tc.desc: stub ThermalActionCallbackStub OnRemoteRequest THERMAL_ACTION_CHANGED
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMockStubGetdescriptorTest, ThermalMockStubGetdescriptorTest005, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubGetdescriptorTest005 start.");
    uint32_t code =
        static_cast<uint32_t>(PowerMgr::ThermalActionCallbackInterfaceCode::THERMAL_ACTION_CHANGED);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    sptr<ThermalActionCallbackStub> actionStub = new ThermalActionCallbackStub();
    data.WriteInterfaceToken(ThermalActionCallbackStub::GetDescriptor());
    data.WriteUint32(1);
    data.WriteString("ThermalMockStubGetdescriptorTest005");
    data.WriteString("3.14");
    int32_t ret = actionStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, ERR_OK) << "ret: " << ret << " code: " << code;

    // Read Uint32 parcel error
    data.WriteInterfaceToken(ThermalActionCallbackStub::GetDescriptor());
    ret = actionStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, E_READ_PARCEL_ERROR_THERMAL) << "ret: " << ret << " code: " << code;

    // Read String parcel error
    data.WriteInterfaceToken(ThermalActionCallbackStub::GetDescriptor());
    data.WriteUint32(1);
    ret = actionStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, E_READ_PARCEL_ERROR_THERMAL) << "ret: " << ret << " code: " << code;

    // Read Float parcel error
    data.WriteInterfaceToken(ThermalActionCallbackStub::GetDescriptor());
    data.WriteUint32(1);
    data.WriteString("ThermalMockStubGetdescriptorTest005");
    ret = actionStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, E_READ_PARCEL_ERROR_THERMAL) << "ret: " << ret << " code: " << code;

    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubGetdescriptorTest005 end.");
}

/**
 * @tc.name: ThermalMockStubGetdescriptorTest006
 * @tc.desc: stub ThermalLevelCallbackStub OnRemoteRequest THERMAL_LEVEL_CHANGED Read Int32 parcel error
 * @tc.type: FUNC
 */
HWTEST_F(ThermalMockStubGetdescriptorTest, ThermalMockStubGetdescriptorTest006, TestSize.Level0)
{
    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubGetdescriptorTest006 start.");
    uint32_t code =
        static_cast<uint32_t>(PowerMgr::ThermalLevelCallbackInterfaceCode::THERMAL_LEVEL_CHANGED);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    sptr<ThermalLevelCallbackStub> actionStub = new ThermalLevelCallbackStub();
    data.WriteInterfaceToken(ThermalLevelCallbackStub::GetDescriptor());
    int32_t ret = actionStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, E_READ_PARCEL_ERROR_THERMAL) << "ret: " << ret << " code: " << code;

    THERMAL_HILOGD(LABEL_TEST, "ThermalMockStubGetdescriptorTest006 end.");
}
} // namespace
