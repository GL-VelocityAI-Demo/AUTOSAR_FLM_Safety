/**
 * @file test_LightRequest.cpp
 * @brief Unit Tests for LightRequest SWC
 * @details Tests ADC reading, filtering, and plausibility checks
 * @version 1.0.0
 * @date 2024
 */

#include <gtest/gtest.h>
#include "Application/LightRequest/LightRequest.h"
#include "MCAL/Adc/Adc.h"
#include "FLM_Config.h"

/**
 * @brief LightRequest Test Fixture
 */
class LightRequestTest : public ::testing::Test {
protected:
    void SetUp() override {
        /* Initialize ADC */
        static const Adc_ConfigType adcConfig = {0};
        Adc_Init(&adcConfig);

        /* Initialize LightRequest */
        LightRequest_Init();
    }

    void TearDown() override {
        Adc_DeInit();
    }
};

/**
 * @test Normal ADC reading and filtering
 */
TEST_F(LightRequestTest, NormalReading_Filtering) {
    /* Set simulated ADC value */
    LightRequest_SimSetAdcValue(2000);

    /* Run multiple cycles to fill averaging buffer */
    for (int i = 0; i < FLM_ADC_SAMPLES + 2; i++) {
        LightRequest_MainFunction();
    }

    /* Check filtered value */
    AmbientLightLevel level = LightRequest_GetAmbientLight();
    EXPECT_TRUE(level.isValid);
    EXPECT_NEAR(level.adcValue, 2000, 10);
}

/**
 * @test Open circuit detection
 */
TEST_F(LightRequestTest, OpenCircuit_Detection) {
    /* Set ADC value below open circuit threshold */
    LightRequest_SimSetAdcValue(50);  /* < 100 */

    /* Run multiple cycles */
    for (int i = 0; i < FLM_ADC_SAMPLES + 2; i++) {
        LightRequest_MainFunction();
    }

    /* Check signal status */
    SignalStatus status = LightRequest_GetSignalStatus();
    EXPECT_EQ(status, SIGNAL_STATUS_OPEN_CIRCUIT);

    AmbientLightLevel level = LightRequest_GetAmbientLight();
    EXPECT_FALSE(level.isValid);
}

/**
 * @test Short circuit detection
 */
TEST_F(LightRequestTest, ShortCircuit_Detection) {
    /* Set ADC value above short circuit threshold */
    LightRequest_SimSetAdcValue(4000);  /* > 3995 */

    /* Run multiple cycles */
    for (int i = 0; i < FLM_ADC_SAMPLES + 2; i++) {
        LightRequest_MainFunction();
    }

    /* Check signal status */
    SignalStatus status = LightRequest_GetSignalStatus();
    EXPECT_EQ(status, SIGNAL_STATUS_SHORT_CIRCUIT);

    AmbientLightLevel level = LightRequest_GetAmbientLight();
    EXPECT_FALSE(level.isValid);
}

/**
 * @test Rate of change plausibility check
 */
TEST_F(LightRequestTest, RateOfChange_Plausibility) {
    /* Start with stable value */
    LightRequest_SimSetAdcValue(1500);
    for (int i = 0; i < 10; i++) {
        LightRequest_MainFunction();
    }

    /* Sudden large change (> 500 per 100ms) */
    LightRequest_SimSetAdcValue(3000);  /* 1500 change */

    /* Run enough cycles to trigger rate check (5 cycles * 20ms = 100ms) */
    for (int i = 0; i < 20; i++) {
        LightRequest_MainFunction();
    }

    /* Check for plausibility fault */
    EXPECT_TRUE(LightRequest_IsPlausibilityFault());
}

/**
 * @test Normal rate of change
 */
TEST_F(LightRequestTest, NormalRateOfChange) {
    /* Start with stable value */
    LightRequest_SimSetAdcValue(1500);
    for (int i = 0; i < 10; i++) {
        LightRequest_MainFunction();
    }

    /* Small change (< 500 per 100ms) */
    LightRequest_SimSetAdcValue(1600);  /* 100 change */

    for (int i = 0; i < 10; i++) {
        LightRequest_MainFunction();
    }

    /* Should not trigger plausibility fault */
    EXPECT_FALSE(LightRequest_IsPlausibilityFault());
    EXPECT_EQ(LightRequest_GetSignalStatus(), SIGNAL_STATUS_VALID);
}

/**
 * @test ADC to Lux conversion
 */
TEST_F(LightRequestTest, AdcToLux_Conversion) {
    /* Set mid-range ADC value */
    LightRequest_SimSetAdcValue(800);  /* ~200 lux */

    for (int i = 0; i < FLM_ADC_SAMPLES + 2; i++) {
        LightRequest_MainFunction();
    }

    AmbientLightLevel level = LightRequest_GetAmbientLight();
    /* lux = ADC / 4, so 800 / 4 = 200 */
    EXPECT_EQ(level.luxValue, 200);
}

/**
 * @test State access
 */
TEST_F(LightRequestTest, StateAccess) {
    const LightRequest_StateType* state = LightRequest_GetState();

    EXPECT_NE(state, nullptr);
    EXPECT_TRUE(state->isInitialized);
}
