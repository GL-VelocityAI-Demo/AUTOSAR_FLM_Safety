/**
 * @file main.cpp
 * @brief FLM Application Entry Point and Task Scheduler
 * @details Main entry point implementing simplified RTOS scheduling
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 */

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include <iostream>
#include <chrono>
#include <thread>
#include <cstdint>
#include <csignal>

/* Standard AUTOSAR types */
#include "Std_Types.h"
#include "FLM_Config.h"

/* MCAL */
#include "MCAL/Adc/Adc.h"
#include "MCAL/Dio/Dio.h"
#include "MCAL/Can/Can.h"

/* BSW */
#include "BSW/E2E/E2E_P01.h"
#include "BSW/WdgM/WdgM.h"
#include "BSW/Dem/Dem.h"
#include "BSW/Com/Com.h"
#include "BSW/BswM/BswM.h"

/* Application SWCs */
#include "Application/SwitchEvent/SwitchEvent.h"
#include "Application/LightRequest/LightRequest.h"
#include "Application/FLM/FLM_Application.h"
#include "Application/Headlight/Headlight.h"
#include "Application/SafetyMonitor/SafetyMonitor.h"

/*============================================================================*
 * LOCAL DEFINITIONS
 *============================================================================*/

/** @brief Maximum simulation ticks (0 = infinite) */
#define MAX_SIMULATION_TICKS    1000U

/** @brief Enable real-time simulation */
#define REAL_TIME_SIMULATION    1

/*============================================================================*
 * LOCAL VARIABLES
 *============================================================================*/

/** @brief System running flag */
static volatile boolean System_Running = TRUE;

/** @brief Current tick counter (ms) */
static uint32_t System_TickMs = 0U;

/*============================================================================*
 * LOCAL FUNCTION PROTOTYPES
 *============================================================================*/

static void System_Init(void);
static void System_DeInit(void);
static void System_RunScheduler(void);
static void System_Task_5ms(void);
static void System_Task_10ms(void);
static void System_Task_20ms(void);
static void System_SimulateInputs(void);
static void System_PrintStatus(void);
static void System_SignalHandler(int signal);

/*============================================================================*
 * MAIN FUNCTION
 *============================================================================*/

/**
 * @brief Application entry point
 */
int main(int argc, char* argv[]) {
    STD_UNUSED(argc);
    STD_UNUSED(argv);

    std::cout << "========================================" << std::endl;
    std::cout << "AUTOSAR FLM Safety Use Case" << std::endl;
    std::cout << "Front Light Management System" << std::endl;
    std::cout << "AUTOSAR Classic Platform R23-11" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    /* Register signal handler for graceful shutdown */
    std::signal(SIGINT, System_SignalHandler);

    /* Initialize system */
    System_Init();

    std::cout << "System initialized. Running scheduler..." << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;
    std::cout << std::endl;

    /* Run main scheduler loop */
    System_RunScheduler();

    /* De-initialize system */
    System_DeInit();

    std::cout << std::endl;
    std::cout << "System shutdown complete." << std::endl;

    return 0;
}

/*============================================================================*
 * LOCAL FUNCTION IMPLEMENTATIONS
 *============================================================================*/

/**
 * @brief Initialize all system components
 */
static void System_Init(void) {
    /* ADC configuration (stub) */
    static const Adc_ConfigType adcConfig = {
        .NumGroups = 2,
        .Groups = NULL_PTR,
        .NumChannels = 2,
        .Channels = NULL_PTR
    };

    /* CAN configuration (stub) */
    static const Can_ConfigType canConfig = {
        .NumControllers = 1,
        .Controllers = NULL_PTR
    };

    /* WdgM configuration */
    static const WdgM_ConfigType wdgmConfig = {
        .numSupervisedEntities = WDGM_NUM_SUPERVISED_ENTITIES,
        .supervisionCycleMs = WDGM_SUPERVISION_CYCLE_MS,
        .failedRefCycles = WDGM_FAILED_REFERENCE_CYCLES
    };

    /* BswM configuration (stub) */
    static const BswM_ConfigType bswmConfig = {
        .numModes = 5
    };

    std::cout << "Initializing MCAL..." << std::endl;

    /* Initialize MCAL */
    Adc_Init(&adcConfig);
    Dio_Init();
    Can_Init(&canConfig);

    std::cout << "Initializing BSW..." << std::endl;

    /* Initialize BSW */
    Dem_Init();
    WdgM_Init(&wdgmConfig);
    Com_Init();
    BswM_Init(&bswmConfig);

    /* Start CAN controller */
    Can_SetControllerMode(0, CAN_MODE_START);

    std::cout << "Initializing Application SWCs..." << std::endl;

    /* Initialize Application SWCs */
    SwitchEvent_Init();
    LightRequest_Init();
    FLM_Init();
    Headlight_Init();
    SafetyMonitor_Init();

    /* Set initial simulation values */
    Adc_SimSetValue(FLM_ADC_CHANNEL_AMBIENT, 2000U);  /* Mid-range ambient */
    Adc_SimSetValue(FLM_ADC_CHANNEL_CURRENT, 0U);     /* No current (lights off) */

    std::cout << "Initialization complete." << std::endl;
}

/**
 * @brief De-initialize all system components
 */
static void System_DeInit(void) {
    std::cout << "De-initializing system..." << std::endl;

    /* De-initialize in reverse order */
    BswM_Deinit();
    WdgM_DeInit();
    Dem_Shutdown();
    Com_DeInit();
    Can_DeInit();
    Adc_DeInit();
}

/**
 * @brief Main scheduler loop
 */
static void System_RunScheduler(void) {
    uint32_t tickCount = 0U;

    while (System_Running) {
        /* Simulate inputs (for demonstration) */
        System_SimulateInputs();

        /* 5ms tasks */
        if ((System_TickMs % 5U) == 0U) {
            System_Task_5ms();
        }

        /* 10ms tasks */
        if ((System_TickMs % 10U) == 0U) {
            System_Task_10ms();
        }

        /* 20ms tasks */
        if ((System_TickMs % 20U) == 0U) {
            System_Task_20ms();
        }

        /* Print status every 100ms */
        if ((System_TickMs % 100U) == 0U) {
            System_PrintStatus();
        }

        /* Check for safe state */
        if (SafetyMonitor_IsInSafeState()) {
            std::cout << "*** SAFE STATE ENTERED ***" << std::endl;
            std::cout << "Reason: " << static_cast<int>(SafetyMonitor_GetSafeStateReason()) << std::endl;
            /* Continue running in safe state for demonstration */
        }

        /* Increment tick counter */
        System_TickMs += FLM_SYSTEM_TICK_MS;
        tickCount++;

        /* Check simulation limit */
        if ((MAX_SIMULATION_TICKS > 0U) && (tickCount >= MAX_SIMULATION_TICKS)) {
            std::cout << "Simulation limit reached." << std::endl;
            System_Running = FALSE;
        }

#if REAL_TIME_SIMULATION
        /* Sleep for 1ms to simulate real-time */
        std::this_thread::sleep_for(std::chrono::milliseconds(FLM_SYSTEM_TICK_MS));
#endif
    }
}

/**
 * @brief 5ms task
 */
static void System_Task_5ms(void) {
    /* Safety Monitor - highest priority */
    SafetyMonitor_MainFunction();

    /* Watchdog Manager */
    WdgM_MainFunction();

    /* BSW Mode Manager */
    BswM_MainFunction();
}

/**
 * @brief 10ms task
 */
static void System_Task_10ms(void) {
    /* COM RX processing */
    Com_MainFunctionRx();

    /* CAN RX processing */
    Can_MainFunction_Read();

    /* SwitchEvent - CAN light switch processing */
    SwitchEvent_MainFunction();

    /* FLM Application - main control logic */
    FLM_MainFunction();

    /* Headlight - output control */
    Headlight_MainFunction();

    /* CAN TX processing */
    Can_MainFunction_Write();

    /* COM TX processing */
    Com_MainFunctionTx();

    /* DEM main function */
    Dem_MainFunction();
}

/**
 * @brief 20ms task
 */
static void System_Task_20ms(void) {
    /* LightRequest - ambient light sensor */
    LightRequest_MainFunction();
}

/**
 * @brief Simulate inputs for demonstration
 */
static void System_SimulateInputs(void) {
    static uint32_t simCounter = 0U;
    static uint8_t e2eCounter = 0U;
    E2E_P01ConfigType e2eConfig;
    E2E_P01ProtectStateType e2eProtectState;
    uint8_t canMessage[4] = {0};

    /* Configure E2E for simulation */
    e2eConfig.DataLength = FLM_E2E_LIGHTSWITCH_DATA_LENGTH;
    e2eConfig.DataID = FLM_E2E_LIGHTSWITCH_DATA_ID;
    e2eConfig.CounterOffset = FLM_E2E_COUNTER_OFFSET;
    e2eConfig.CRCOffset = FLM_E2E_CRC_OFFSET;

    /* Simulate CAN message every 20ms */
    if ((simCounter % 20U) == 0U) {
        e2eProtectState.Counter = e2eCounter;

        /* Set light switch command (cycle through modes) */
        uint8_t mode = static_cast<uint8_t>((simCounter / 500U) % 4U);
        canMessage[COM_LIGHTSWITCH_CMD_BYTE] = mode;

        /* Add E2E protection */
        E2E_P01Protect(&e2eConfig, &e2eProtectState, canMessage, 4U);

        /* Send to COM layer */
        PduInfoType pduInfo;
        pduInfo.SduDataPtr = canMessage;
        pduInfo.SduLength = 4U;
        Com_RxIndication(COM_IPDU_LIGHTSWITCH_RX, &pduInfo);

        e2eCounter++;
        if (e2eCounter > 14U) {
            e2eCounter = 0U;
        }
    }

    /* Simulate ambient light changes */
    if ((simCounter % 100U) == 0U) {
        /* Vary ambient light between 500 and 2500 */
        uint16_t ambient = 1500U + static_cast<uint16_t>((simCounter / 100U) % 10U) * 100U;
        if ((simCounter / 1000U) % 2U == 1U) {
            ambient = 500U;  /* Simulate dark condition */
        }
        Adc_SimSetValue(FLM_ADC_CHANNEL_AMBIENT, ambient);
    }

    /* Simulate headlight current feedback */
    HeadlightCommand cmd = Headlight_GetCurrentCommand();
    if (cmd != HEADLIGHT_CMD_OFF) {
        /* Simulate current when lights are on */
        Headlight_SimSetFeedbackCurrent(5000U);  /* 5A */
    } else {
        Headlight_SimSetFeedbackCurrent(0U);
    }

    simCounter++;
}

/**
 * @brief Print system status
 */
static void System_PrintStatus(void) {
    LightSwitchStatus switchStatus = SwitchEvent_GetLightRequest();
    AmbientLightLevel ambientLight = LightRequest_GetAmbientLight();
    FLM_StateType flmState = FLM_GetCurrentState();
    HeadlightCommand headlightCmd = FLM_GetHeadlightCommand();
    SafetyStatusType safetyStatus = SafetyMonitor_GetGlobalStatus();

    const char* stateNames[] = {"INIT", "NORMAL", "DEGRADED", "SAFE"};
    const char* cmdNames[] = {"OFF", "LOW_BEAM", "HIGH_BEAM"};
    const char* switchNames[] = {"OFF", "LOW", "HIGH", "AUTO"};
    const char* safetyNames[] = {"OK", "WARNING", "DEGRADED", "SAFE_STATE"};

    std::cout << "[" << System_TickMs << "ms] ";
    std::cout << "State:" << stateNames[flmState] << " ";
    std::cout << "Switch:" << (switchStatus.isValid ? switchNames[switchStatus.command] : "INVALID") << " ";
    std::cout << "Ambient:" << (ambientLight.isValid ? std::to_string(ambientLight.adcValue) : "INVALID") << " ";
    std::cout << "Headlight:" << cmdNames[headlightCmd] << " ";
    std::cout << "Safety:" << safetyNames[safetyStatus];
    std::cout << std::endl;
}

/**
 * @brief Signal handler for graceful shutdown
 */
static void System_SignalHandler(int signal) {
    STD_UNUSED(signal);
    std::cout << std::endl << "Received shutdown signal..." << std::endl;
    System_Running = FALSE;
}
