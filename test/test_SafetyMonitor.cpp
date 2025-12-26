/**
 * @file test_SafetyMonitor.cpp
 * @brief Unit Tests for SafetyMonitor SWC
 * @details Tests fault aggregation, FTTI, and safe state control
 * @version 1.0.0
 * @date 2024
 */

#include <gtest/gtest.h>
#include "Application/SafetyMonitor/SafetyMonitor.h"
#include "Application/SwitchEvent/SwitchEvent.h"
#include "Application/LightRequest/LightRequest.h"
#include "Application/FLM/FLM_Application.h"
#include "Application/Headlight/Headlight.h"
#include "MCAL/Adc/Adc.h"
#include "MCAL/Dio/Dio.h"
#include "FLM_Config.h"

/* External test helper function declaration */
extern void SafetyMonitor_SimSetWdgmStatus(WdgM_GlobalStatusType status);

/**
 * @brief SafetyMonitor Test Fixture
 */
class SafetyMonitorTest : public ::testing::Test {
protected:
    void SetUp() override {
        /* Initialize MCAL */
        static const Adc_ConfigType adcConfig = {0};
        Adc_Init(&adcConfig);
        Dio_Init();

        /* Initialize all SWCs */
        SwitchEvent_Init();
        LightRequest_Init();
        FLM_Init();
        Headlight_Init();
        SafetyMonitor_Init();

        /* Set valid ambient light */
        LightRequest_SimSetAdcValue(2000);
    }

    void TearDown() override {
        Adc_DeInit();
    }
};

/**
 * @test Initial state is OK
 */
TEST_F(SafetyMonitorTest, InitialState_OK) {
    EXPECT_EQ(SafetyMonitor_GetGlobalStatus(), SAFETY_STATUS_OK);
    EXPECT_FALSE(SafetyMonitor_IsInSafeState());
}

/**
 * @test Manual safe state trigger
 */
TEST_F(SafetyMonitorTest, ManualSafeState_Trigger) {
    SafetyMonitor_TriggerSafeState(SAFE_STATE_REASON_MANUAL);
    SafetyMonitor_MainFunction();

    EXPECT_TRUE(SafetyMonitor_IsInSafeState());
    EXPECT_EQ(SafetyMonitor_GetSafeStateReason(), SAFE_STATE_REASON_MANUAL);
    EXPECT_EQ(SafetyMonitor_GetGlobalStatus(), SAFETY_STATUS_SAFE_STATE);
}

/**
 * @test Safe state headlight command (day)
 */
TEST_F(SafetyMonitorTest, SafeStateCommand_Day) {
    /* Set bright ambient (day) */
    LightRequest_SimSetAdcValue(3000);

    for (int i = 0; i < 10; i++) {
        LightRequest_MainFunction();
        SafetyMonitor_MainFunction();
    }

    SafetyMonitor_TriggerSafeState(SAFE_STATE_REASON_MANUAL);
    SafetyMonitor_MainFunction();

    /* Day - should command OFF */
    EXPECT_EQ(SafetyMonitor_GetSafeStateCommand(), HEADLIGHT_CMD_OFF);
}

/**
 * @test Safe state headlight command (night)
 */
TEST_F(SafetyMonitorTest, SafeStateCommand_Night) {
    /* Set dark ambient (night) */
    LightRequest_SimSetAdcValue(500);

    for (int i = 0; i < 10; i++) {
        LightRequest_MainFunction();
        SafetyMonitor_MainFunction();
    }

    SafetyMonitor_TriggerSafeState(SAFE_STATE_REASON_MANUAL);
    SafetyMonitor_MainFunction();

    /* Night - should command LOW_BEAM */
    EXPECT_EQ(SafetyMonitor_GetSafeStateCommand(), HEADLIGHT_CMD_LOW_BEAM);
}

/**
 * @test WdgM failure triggers safe state
 */
TEST_F(SafetyMonitorTest, WdgMFailure_TriggersSafeState) {
    /* Simulate WdgM failure */
    SafetyMonitor_SimSetWdgmStatus(WDGM_GLOBAL_STATUS_FAILED);

    SafetyMonitor_MainFunction();

    EXPECT_TRUE(SafetyMonitor_IsInSafeState());
    EXPECT_EQ(SafetyMonitor_GetSafeStateReason(), SAFE_STATE_REASON_WDGM_FAILURE);
}

/**
 * @test State access
 */
TEST_F(SafetyMonitorTest, StateAccess) {
    const SafetyMonitor_StateType* state = SafetyMonitor_GetState();

    EXPECT_NE(state, nullptr);
    EXPECT_TRUE(state->isInitialized);
}

/**
 * @test Safe state is persistent
 */
TEST_F(SafetyMonitorTest, SafeState_IsPersistent) {
    SafetyMonitor_TriggerSafeState(SAFE_STATE_REASON_MANUAL);

    for (int i = 0; i < 20; i++) {
        SafetyMonitor_MainFunction();
    }

    /* Safe state should remain */
    EXPECT_TRUE(SafetyMonitor_IsInSafeState());
}
