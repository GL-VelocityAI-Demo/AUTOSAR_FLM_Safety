/**
 * @file test_FLM.cpp
 * @brief Unit Tests for FLM Application SWC
 * @details Tests state machine and control logic
 * @version 1.0.0
 * @date 2024
 */

#include <gtest/gtest.h>
#include "Application/FLM/FLM_Application.h"
#include "Application/SwitchEvent/SwitchEvent.h"
#include "Application/LightRequest/LightRequest.h"
#include "BSW/E2E/E2E_P01.h"
#include "MCAL/Adc/Adc.h"
#include "MCAL/Dio/Dio.h"
#include "FLM_Config.h"

/**
 * @brief FLM Application Test Fixture
 */
class FLMTest : public ::testing::Test {
protected:
    E2E_P01ConfigType e2eConfig;
    E2E_P01ProtectStateType e2eProtectState;

    void SetUp() override {
        /* Initialize MCAL */
        static const Adc_ConfigType adcConfig = {0};
        Adc_Init(&adcConfig);
        Dio_Init();

        /* Initialize SWCs */
        SwitchEvent_Init();
        LightRequest_Init();
        FLM_Init();

        /* Configure E2E */
        e2eConfig.DataLength = FLM_E2E_LIGHTSWITCH_DATA_LENGTH;
        e2eConfig.DataID = FLM_E2E_LIGHTSWITCH_DATA_ID;
        e2eConfig.CounterOffset = FLM_E2E_COUNTER_OFFSET;
        e2eConfig.CRCOffset = FLM_E2E_CRC_OFFSET;
        E2E_P01ProtectInit(&e2eProtectState);

        /* Set valid ambient light */
        LightRequest_SimSetAdcValue(2000);
    }

    void TearDown() override {
        Adc_DeInit();
    }

    void SendValidMessage(LightSwitchCmd cmd) {
        uint8_t data[4] = {0};
        data[COM_LIGHTSWITCH_CMD_BYTE] = static_cast<uint8_t>(cmd);
        E2E_P01Protect(&e2eConfig, &e2eProtectState, data, 4);
        SwitchEvent_ProcessCanMessage(data, 4);
    }

    void RunAllTasks() {
        SwitchEvent_MainFunction();
        LightRequest_MainFunction();
        FLM_MainFunction();
    }
};

/**
 * @test State machine starts in INIT
 */
TEST_F(FLMTest, StateMachine_StartsInInit) {
    EXPECT_EQ(FLM_GetCurrentState(), FLM_STATE_INIT);
}

/**
 * @test Transition from INIT to NORMAL
 */
TEST_F(FLMTest, StateMachine_InitToNormal) {
    /* Provide valid inputs */
    for (int i = 0; i < 10; i++) {
        SendValidMessage(LIGHT_SWITCH_LOW_BEAM);
        RunAllTasks();
    }

    /* Should transition to NORMAL */
    EXPECT_EQ(FLM_GetCurrentState(), FLM_STATE_NORMAL);
}

/**
 * @test AUTO mode light activation
 */
TEST_F(FLMTest, AutoMode_LightActivation) {
    /* Transition to NORMAL first */
    for (int i = 0; i < 10; i++) {
        SendValidMessage(LIGHT_SWITCH_AUTO);
        RunAllTasks();
    }

    /* Set dark ambient (< 800) */
    LightRequest_SimSetAdcValue(500);

    for (int i = 0; i < 10; i++) {
        SendValidMessage(LIGHT_SWITCH_AUTO);
        RunAllTasks();
    }

    /* Should activate low beam */
    EXPECT_EQ(FLM_GetHeadlightCommand(), HEADLIGHT_CMD_LOW_BEAM);
}

/**
 * @test AUTO mode light deactivation
 */
TEST_F(FLMTest, AutoMode_LightDeactivation) {
    /* Start with lights on */
    LightRequest_SimSetAdcValue(500);
    for (int i = 0; i < 10; i++) {
        SendValidMessage(LIGHT_SWITCH_AUTO);
        RunAllTasks();
    }

    /* Set bright ambient (> 1000 for hysteresis) */
    LightRequest_SimSetAdcValue(1200);

    for (int i = 0; i < 10; i++) {
        SendValidMessage(LIGHT_SWITCH_AUTO);
        RunAllTasks();
    }

    /* Should deactivate lights */
    EXPECT_EQ(FLM_GetHeadlightCommand(), HEADLIGHT_CMD_OFF);
}

/**
 * @test Hysteresis behavior
 */
TEST_F(FLMTest, Hysteresis_Behavior) {
    /* Start in dark condition */
    LightRequest_SimSetAdcValue(500);
    for (int i = 0; i < 10; i++) {
        SendValidMessage(LIGHT_SWITCH_AUTO);
        RunAllTasks();
    }
    EXPECT_EQ(FLM_GetHeadlightCommand(), HEADLIGHT_CMD_LOW_BEAM);

    /* Set to mid-range (between 800 and 1000) */
    LightRequest_SimSetAdcValue(900);
    for (int i = 0; i < 10; i++) {
        SendValidMessage(LIGHT_SWITCH_AUTO);
        RunAllTasks();
    }

    /* Should stay ON due to hysteresis */
    EXPECT_EQ(FLM_GetHeadlightCommand(), HEADLIGHT_CMD_LOW_BEAM);
}

/**
 * @test Manual light switch commands
 */
TEST_F(FLMTest, ManualCommands) {
    /* Transition to NORMAL */
    for (int i = 0; i < 10; i++) {
        SendValidMessage(LIGHT_SWITCH_OFF);
        RunAllTasks();
    }

    /* Test OFF */
    for (int i = 0; i < 5; i++) {
        SendValidMessage(LIGHT_SWITCH_OFF);
        RunAllTasks();
    }
    EXPECT_EQ(FLM_GetHeadlightCommand(), HEADLIGHT_CMD_OFF);

    /* Test LOW_BEAM */
    for (int i = 0; i < 5; i++) {
        SendValidMessage(LIGHT_SWITCH_LOW_BEAM);
        RunAllTasks();
    }
    EXPECT_EQ(FLM_GetHeadlightCommand(), HEADLIGHT_CMD_LOW_BEAM);

    /* Test HIGH_BEAM */
    for (int i = 0; i < 5; i++) {
        SendValidMessage(LIGHT_SWITCH_HIGH_BEAM);
        RunAllTasks();
    }
    EXPECT_EQ(FLM_GetHeadlightCommand(), HEADLIGHT_CMD_HIGH_BEAM);
}

/**
 * @test Safe state trigger
 */
TEST_F(FLMTest, SafeState_Trigger) {
    /* Transition to NORMAL first */
    for (int i = 0; i < 10; i++) {
        SendValidMessage(LIGHT_SWITCH_LOW_BEAM);
        RunAllTasks();
    }

    /* Trigger safe state */
    FLM_TriggerSafeState(SAFE_STATE_REASON_MANUAL);

    RunAllTasks();

    /* Should be in SAFE state */
    EXPECT_EQ(FLM_GetCurrentState(), FLM_STATE_SAFE);
    EXPECT_TRUE(FLM_IsInSafeState());
}

/**
 * @test State access
 */
TEST_F(FLMTest, StateAccess) {
    const FLM_Application_StateType* state = FLM_GetState();

    EXPECT_NE(state, nullptr);
    EXPECT_TRUE(state->isInitialized);
}
