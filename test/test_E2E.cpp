/**
 * @file test_E2E.cpp
 * @brief Unit Tests for E2E Profile 01
 * @details Tests E2E protection functionality
 * @version 1.0.0
 * @date 2024
 */

#include <gtest/gtest.h>
#include "BSW/E2E/E2E_P01.h"
#include "FLM_Config.h"
#include <cstring>

/**
 * @brief E2E Test Fixture
 */
class E2ETest : public ::testing::Test {
protected:
    E2E_P01ConfigType config;
    E2E_P01ProtectStateType protectState;
    E2E_P01CheckStateType checkState;

    void SetUp() override {
        /* Configure E2E Profile 01 */
        config.DataLength = 32;  /* 4 bytes */
        config.DataID = 0x0100;
        config.MaxDeltaCounter = 2;
        config.MaxNoNewOrRepeatedData = 5;
        config.SyncCounter = 2;
        config.CounterOffset = 8;
        config.CRCOffset = 0;
        config.DataIDNibbleOffset = 0;
        config.DataIDMode = false;

        /* Initialize states */
        E2E_P01ProtectInit(&protectState);
        E2E_P01CheckInit(&checkState);
    }
};

/**
 * @test Valid message with correct CRC and counter
 */
TEST_F(E2ETest, ValidMessage_CorrectCRCAndCounter) {
    uint8_t data[4] = {0};
    E2E_P01CheckStatusType status;

    /* Protect the data */
    data[2] = 0x01;  /* Light switch command */
    ASSERT_EQ(E2E_P01Protect(&config, &protectState, data, 4), E_OK);

    /* Check the data */
    status = E2E_P01Check(&config, &checkState, data, 4);

    /* First message should be INITIAL */
    EXPECT_EQ(status, E2E_P01STATUS_INITIAL);
}

/**
 * @test Consecutive valid messages
 */
TEST_F(E2ETest, ConsecutiveValidMessages) {
    uint8_t data[4] = {0};
    E2E_P01CheckStatusType status;

    /* Send first message */
    data[2] = 0x01;
    E2E_P01Protect(&config, &protectState, data, 4);
    status = E2E_P01Check(&config, &checkState, data, 4);
    EXPECT_EQ(status, E2E_P01STATUS_INITIAL);

    /* Send second message */
    data[2] = 0x02;
    E2E_P01Protect(&config, &protectState, data, 4);
    status = E2E_P01Check(&config, &checkState, data, 4);
    EXPECT_EQ(status, E2E_P01STATUS_OK);

    /* Send third message */
    data[2] = 0x03;
    E2E_P01Protect(&config, &protectState, data, 4);
    status = E2E_P01Check(&config, &checkState, data, 4);
    EXPECT_EQ(status, E2E_P01STATUS_OK);
}

/**
 * @test Invalid CRC detection
 */
TEST_F(E2ETest, InvalidCRC_Detected) {
    uint8_t data[4] = {0};
    E2E_P01CheckStatusType status;

    /* Protect the data */
    data[2] = 0x01;
    E2E_P01Protect(&config, &protectState, data, 4);

    /* Corrupt the CRC */
    data[0] ^= 0xFF;

    /* Check should fail */
    status = E2E_P01Check(&config, &checkState, data, 4);
    EXPECT_EQ(status, E2E_P01STATUS_WRONGCRC);
}

/**
 * @test Counter jump detection
 */
TEST_F(E2ETest, CounterJump_Detected) {
    uint8_t data[4] = {0};
    E2E_P01CheckStatusType status;

    /* Send first message */
    data[2] = 0x01;
    E2E_P01Protect(&config, &protectState, data, 4);
    E2E_P01Check(&config, &checkState, data, 4);

    /* Skip multiple counters (simulate lost messages) */
    protectState.Counter = 5;  /* Jump counter */
    data[2] = 0x02;
    E2E_P01Protect(&config, &protectState, data, 4);
    status = E2E_P01Check(&config, &checkState, data, 4);

    /* Should detect wrong sequence (jump > MaxDeltaCounter) */
    EXPECT_EQ(status, E2E_P01STATUS_WRONGSEQUENCE);
}

/**
 * @test Repeated data detection
 */
TEST_F(E2ETest, RepeatedData_Detected) {
    uint8_t data[4] = {0};
    E2E_P01CheckStatusType status;

    /* Send first message */
    data[2] = 0x01;
    E2E_P01Protect(&config, &protectState, data, 4);
    E2E_P01Check(&config, &checkState, data, 4);

    /* Send same message again (same counter) - simulate by not incrementing */
    uint8_t savedCounter = protectState.Counter;
    protectState.Counter = savedCounter - 1;  /* Use same counter */
    data[2] = 0x02;
    E2E_P01Protect(&config, &protectState, data, 4);
    status = E2E_P01Check(&config, &checkState, data, 4);

    /* Should detect repeated data */
    EXPECT_EQ(status, E2E_P01STATUS_REPEATED);
}

/**
 * @test Message timeout detection (no new data)
 */
TEST_F(E2ETest, MessageTimeout_NoNewData) {
    E2E_P01CheckStatusType status;

    /* Call check without any data multiple times */
    for (int i = 0; i < 6; i++) {
        status = E2E_P01Check(&config, &checkState, nullptr, 0);
    }

    /* Should report no new data */
    EXPECT_EQ(status, E2E_P01STATUS_NONEWDATA);
}

/**
 * @test Some data lost but within tolerance
 */
TEST_F(E2ETest, SomeDataLost_WithinTolerance) {
    uint8_t data[4] = {0};
    E2E_P01CheckStatusType status;

    /* Send first message */
    data[2] = 0x01;
    E2E_P01Protect(&config, &protectState, data, 4);
    E2E_P01Check(&config, &checkState, data, 4);

    /* Skip one counter (within MaxDeltaCounter of 2) */
    protectState.Counter++;  /* Skip one */
    data[2] = 0x02;
    E2E_P01Protect(&config, &protectState, data, 4);
    status = E2E_P01Check(&config, &checkState, data, 4);

    /* Should be OK with some lost */
    EXPECT_EQ(status, E2E_P01STATUS_OKSOMELOST);
}

/**
 * @test CRC calculation consistency
 */
TEST_F(E2ETest, CRC_Consistency) {
    uint8_t data1[4] = {0x00, 0x00, 0x01, 0x00};
    uint8_t data2[4] = {0x00, 0x00, 0x01, 0x00};

    /* Protect both with same initial state */
    E2E_P01ProtectStateType state1, state2;
    E2E_P01ProtectInit(&state1);
    E2E_P01ProtectInit(&state2);

    E2E_P01Protect(&config, &state1, data1, 4);
    E2E_P01Protect(&config, &state2, data2, 4);

    /* CRCs should be identical */
    EXPECT_EQ(data1[0], data2[0]);
}

/**
 * @test State machine initialization
 */
TEST_F(E2ETest, StateMachine_Initialization) {
    E2E_SMConfigType smConfig;
    E2E_SMCheckStateType smState;

    smConfig.WindowSize = 5;
    smConfig.MinOkStateInit = 2;
    smConfig.MaxErrorStateInit = 2;
    smConfig.MinOkStateValid = 2;
    smConfig.MinOkStateInvalid = 3;
    smConfig.MaxErrorStateValid = 2;
    smConfig.MaxErrorStateInvalid = 3;

    E2E_SMCheckInit(&smState);

    /* Initial state should be DEINIT */
    EXPECT_EQ(smState.SMState, E2E_SM_DEINIT);
}

/**
 * @test State machine transitions
 */
TEST_F(E2ETest, StateMachine_Transitions) {
    E2E_SMConfigType smConfig;
    E2E_SMCheckStateType smState;
    E2E_SMStateType result;

    smConfig.WindowSize = 5;
    smConfig.MinOkStateInit = 2;
    smConfig.MaxErrorStateInit = 2;
    smConfig.MinOkStateValid = 2;
    smConfig.MinOkStateInvalid = 3;
    smConfig.MaxErrorStateValid = 2;
    smConfig.MaxErrorStateInvalid = 3;

    E2E_SMCheckInit(&smState);

    /* First check should transition from DEINIT to NODATA */
    result = E2E_SMCheck(&smConfig, &smState, E2E_P01STATUS_NONEWDATA);
    EXPECT_EQ(result, E2E_SM_NODATA);

    /* OK status should transition to INIT */
    result = E2E_SMCheck(&smConfig, &smState, E2E_P01STATUS_INITIAL);
    EXPECT_EQ(result, E2E_SM_INIT);

    /* Multiple OK should transition to VALID */
    result = E2E_SMCheck(&smConfig, &smState, E2E_P01STATUS_OK);
    result = E2E_SMCheck(&smConfig, &smState, E2E_P01STATUS_OK);
    EXPECT_EQ(result, E2E_SM_VALID);
}
