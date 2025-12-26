/**
 * @file test_SwitchEvent.cpp
 * @brief Unit Tests for SwitchEvent SWC
 * @details Tests E2E validation and timeout handling
 * @version 1.0.0
 * @date 2024
 */

#include <gtest/gtest.h>
#include "Application/SwitchEvent/SwitchEvent.h"
#include "BSW/E2E/E2E_P01.h"
#include "FLM_Config.h"

/**
 * @brief SwitchEvent Test Fixture
 */
class SwitchEventTest : public ::testing::Test {
protected:
    E2E_P01ConfigType e2eConfig;
    E2E_P01ProtectStateType e2eProtectState;

    void SetUp() override {
        /* Initialize SwitchEvent */
        SwitchEvent_Init();

        /* Configure E2E for test message generation */
        e2eConfig.DataLength = FLM_E2E_LIGHTSWITCH_DATA_LENGTH;
        e2eConfig.DataID = FLM_E2E_LIGHTSWITCH_DATA_ID;
        e2eConfig.MaxDeltaCounter = FLM_E2E_MAX_DELTA_COUNTER;
        e2eConfig.MaxNoNewOrRepeatedData = FLM_E2E_MAX_NO_NEW_DATA;
        e2eConfig.CounterOffset = FLM_E2E_COUNTER_OFFSET;
        e2eConfig.CRCOffset = FLM_E2E_CRC_OFFSET;

        E2E_P01ProtectInit(&e2eProtectState);
    }

    void SendValidMessage(LightSwitchCmd cmd) {
        uint8_t data[4] = {0};
        data[COM_LIGHTSWITCH_CMD_BYTE] = static_cast<uint8_t>(cmd);
        E2E_P01Protect(&e2eConfig, &e2eProtectState, data, 4);
        SwitchEvent_ProcessCanMessage(data, 4);
    }
};

/**
 * @test Normal operation with valid E2E
 */
TEST_F(SwitchEventTest, NormalOperation_ValidE2E) {
    /* Send valid message */
    SendValidMessage(LIGHT_SWITCH_LOW_BEAM);
    SwitchEvent_MainFunction();

    /* Check status */
    LightSwitchStatus status = SwitchEvent_GetLightRequest();

    /* First message may be INITIAL, subsequent should be OK */
    /* After a few cycles, should be valid */
    for (int i = 0; i < 5; i++) {
        SendValidMessage(LIGHT_SWITCH_LOW_BEAM);
        SwitchEvent_MainFunction();
    }

    status = SwitchEvent_GetLightRequest();
    EXPECT_TRUE(status.isValid);
    EXPECT_EQ(status.command, LIGHT_SWITCH_LOW_BEAM);
}

/**
 * @test E2E failure handling
 */
TEST_F(SwitchEventTest, E2EFailure_Handling) {
    /* First, establish valid communication */
    for (int i = 0; i < 5; i++) {
        SendValidMessage(LIGHT_SWITCH_LOW_BEAM);
        SwitchEvent_MainFunction();
    }

    /* Send corrupted message */
    uint8_t corruptData[4] = {0xFF, 0xFF, 0x01, 0xFF};
    for (int i = 0; i < 6; i++) {
        SwitchEvent_ProcessCanMessage(corruptData, 4);
        SwitchEvent_MainFunction();
    }

    /* E2E status should indicate failure */
    E2E_P01CheckStatusType e2eStatus = SwitchEvent_GetE2EStatus();
    EXPECT_EQ(e2eStatus, E2E_P01STATUS_WRONGCRC);
}

/**
 * @test Message timeout handling
 */
TEST_F(SwitchEventTest, MessageTimeout_Handling) {
    /* Establish valid communication first */
    for (int i = 0; i < 3; i++) {
        SendValidMessage(LIGHT_SWITCH_AUTO);
        SwitchEvent_MainFunction();
    }

    /* Simulate timeout by not sending messages */
    for (int i = 0; i < 10; i++) {  /* 10 * 10ms = 100ms > 50ms timeout */
        SwitchEvent_MainFunction();
    }

    /* Should detect timeout */
    EXPECT_TRUE(SwitchEvent_IsTimeoutActive());

    LightSwitchStatus status = SwitchEvent_GetLightRequest();
    EXPECT_FALSE(status.isValid);
}

/**
 * @test All light switch commands
 */
TEST_F(SwitchEventTest, AllCommands) {
    LightSwitchCmd commands[] = {
        LIGHT_SWITCH_OFF,
        LIGHT_SWITCH_LOW_BEAM,
        LIGHT_SWITCH_HIGH_BEAM,
        LIGHT_SWITCH_AUTO
    };

    for (auto cmd : commands) {
        SwitchEvent_Init();  /* Reset state */
        E2E_P01ProtectInit(&e2eProtectState);

        for (int i = 0; i < 5; i++) {
            SendValidMessage(cmd);
            SwitchEvent_MainFunction();
        }

        LightSwitchStatus status = SwitchEvent_GetLightRequest();
        EXPECT_EQ(status.command, cmd);
    }
}

/**
 * @test State structure access
 */
TEST_F(SwitchEventTest, StateAccess) {
    const SwitchEvent_StateType* state = SwitchEvent_GetState();

    EXPECT_NE(state, nullptr);
    EXPECT_TRUE(state->isInitialized);
}
