# AUTOSAR Front Light Management (FLM) Safety Use Case

An AUTOSAR-compliant implementation of a Front Light Management ECU system based on AUTOSAR Classic Platform R23-11 Safety Use Case. This system controls vehicle headlights based on driver input and ambient light conditions, with functional safety requirements up to ASIL B according to ISO 26262.

## Overview

The FLM system:
- Receives light switch status via CAN bus from the Body Control Module
- Reads ambient light level from an analog sensor (ADC)
- Controls headlight activation via digital output (DIO)
- Implements safety mechanisms including E2E protection, watchdog supervision, and plausibility checks

## Project Structure

```
AUTOSAR_FLM_SafetyUseCase/
├── CMakeLists.txt              # Build configuration
├── README.md                   # This file
├── include/
│   ├── Std_Types.h             # AUTOSAR standard types
│   ├── ComStack_Types.h        # Communication stack types
│   └── Rte/                    # RTE interfaces
│       ├── Rte_Type.h
│       ├── Rte_SwitchEvent.h
│       ├── Rte_LightRequest.h
│       ├── Rte_FLM.h
│       ├── Rte_Headlight.h
│       └── Rte_SafetyMonitor.h
├── src/
│   ├── Application/            # Application SWCs
│   │   ├── SwitchEvent/        # CAN switch signal processing (ASIL B)
│   │   ├── LightRequest/       # Ambient light sensor (ASIL A)
│   │   ├── FLM/                # Main control logic (ASIL B)
│   │   ├── Headlight/          # Output control (ASIL B)
│   │   └── SafetyMonitor/      # Safety aggregation (ASIL B)
│   ├── BSW/                    # Basic Software
│   │   ├── Com/                # Communication module
│   │   ├── E2E/                # E2E Profile 01 library
│   │   ├── WdgM/               # Watchdog Manager
│   │   ├── Dem/                # Diagnostic Event Manager
│   │   └── BswM/               # BSW Mode Manager
│   ├── MCAL/                   # Microcontroller Abstraction Layer
│   │   ├── Adc/                # ADC driver
│   │   ├── Dio/                # DIO driver
│   │   └── Can/                # CAN driver
│   └── main.cpp                # Application entry and scheduler
├── config/                     # Configuration files
│   ├── FLM_Config.h
│   ├── Com_Cfg.h
│   ├── WdgM_Cfg.h
│   └── Dem_Cfg.h
└── test/                       # Unit tests
    ├── test_E2E.cpp
    ├── test_SwitchEvent.cpp
    ├── test_LightRequest.cpp
    ├── test_FLM.cpp
    └── test_SafetyMonitor.cpp
```

## Safety Requirements

| ID | Description | ASIL | Implementation |
|----|-------------|------|----------------|
| FunSafReq01-01 | Light request plausibility check | B | SwitchEvent: E2E + range check |
| FunSafReq01-02 | Ambient light sensor validation | A | LightRequest: Analytical redundancy |
| FunSafReq01-03 | Safe state transition within 100ms | B | SafetyMonitor: State machine |
| SysSafReq01 | CAN timeout detection 50ms | B | SwitchEvent: Message timeout |
| SysSafReq02 | E2E protection | B | E2E Profile 01 |
| SysSafReq03 | Watchdog supervision | B | WdgM: Alive + Deadline + Logical |
| SysSafReq10 | Output stage diagnosis 20ms | B | Headlight: Feedback monitoring |
| ECU17 | FTTI 200ms | B | SafetyMonitor: Fault tolerance |

## Software Components

### SwitchEvent (ASIL B)
- Receives CAN light switch signals
- Performs E2E Profile 01 validation (CRC + counter)
- Detects message timeout (50ms threshold)
- Reports to WdgM for alive supervision

### LightRequest (ASIL A)
- Reads ADC ambient light sensor
- Applies 4-sample averaging filter
- Detects open/short circuit faults
- Performs rate of change plausibility check

### FLM Application (ASIL B)
- Main control logic with state machine
- States: INIT → NORMAL → DEGRADED → SAFE_STATE
- AUTO mode with hysteresis (ON: 800, OFF: 1000)
- Handles degraded modes when inputs invalid

### Headlight (ASIL B)
- Controls DIO outputs for headlight relays
- Monitors feedback current
- Detects open load and short circuit within 20ms

### SafetyMonitor (ASIL B)
- Aggregates fault status from all components
- Implements FTTI monitoring (200ms)
- Determines safe state based on conditions
- Safe state: Day→OFF, Night→LOW_BEAM

## Building

### Prerequisites
- CMake 3.16 or higher
- C++17 compatible compiler (GCC, Clang, or MSVC)
- Google Test (optional, for unit tests)

### Build Commands

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build .

# Run application
./flm_application

# Run tests (if GTest available)
./flm_tests
```

### Build Options

```bash
# Disable tests
cmake -DBUILD_TESTS=OFF ..

# Enable coverage
cmake -DENABLE_COVERAGE=ON ..

# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..
```

## Running the Application

The application runs a simulation loop that:
1. Simulates CAN messages with E2E protection
2. Varies ambient light conditions
3. Cycles through light switch modes
4. Prints status every 100ms

```bash
./flm_application
```

Output example:
```
========================================
AUTOSAR FLM Safety Use Case
Front Light Management System
AUTOSAR Classic Platform R23-11
========================================

System initialized. Running scheduler...
Press Ctrl+C to stop

[100ms] State:INIT Switch:OFF Ambient:2000 Headlight:OFF Safety:OK
[200ms] State:NORMAL Switch:LOW Ambient:2000 Headlight:LOW_BEAM Safety:OK
...
```

## Configuration

Key configuration parameters in `config/FLM_Config.h`:

```cpp
// Timing
#define FLM_CAN_TIMEOUT_MS              50      // CAN message timeout
#define FLM_E2E_TIMEOUT_MS              100     // E2E protection timeout
#define FLM_FTTI_MS                     200     // Fault Tolerant Time Interval
#define FLM_SAFE_STATE_TRANSITION_MS    100     // Safe state transition time

// Ambient light thresholds
#define FLM_AMBIENT_THRESHOLD_ON        800     // Turn on threshold
#define FLM_AMBIENT_THRESHOLD_OFF       1000    // Turn off threshold (hysteresis)
#define FLM_AMBIENT_RATE_LIMIT          500     // Max change per 100ms

// Fault detection
#define FLM_HEADLIGHT_FAULT_DETECT_MS   20      // Output diagnosis time
```

## Testing

Unit tests cover:
- E2E Profile 01 protection and validation
- CAN timeout and E2E failure handling
- ADC filtering and plausibility checks
- State machine transitions
- Safe state behavior

```bash
cd build
./flm_tests
```

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      Application Layer                       │
├─────────────┬─────────────┬──────────┬───────────┬──────────┤
│ SwitchEvent │ LightRequest│   FLM    │ Headlight │ Safety   │
│  (ASIL B)   │  (ASIL A)   │ (ASIL B) │ (ASIL B)  │ Monitor  │
│             │             │          │           │ (ASIL B) │
└─────────────┴─────────────┴──────────┴───────────┴──────────┘
                              │
┌─────────────────────────────────────────────────────────────┐
│                         RTE Layer                            │
└─────────────────────────────────────────────────────────────┘
                              │
┌─────────────────────────────────────────────────────────────┐
│                      BSW Layer                               │
├───────────┬───────────┬───────────┬───────────┬─────────────┤
│    COM    │    E2E    │   WdgM    │    DEM    │    BswM    │
│           │ Profile 01│           │           │             │
└───────────┴───────────┴───────────┴───────────┴─────────────┘
                              │
┌─────────────────────────────────────────────────────────────┐
│                     MCAL Layer                               │
├───────────────────┬───────────────────┬─────────────────────┤
│        ADC        │        DIO        │        CAN          │
└───────────────────┴───────────────────┴─────────────────────┘
```

## State Machine

```
                    ┌──────────────────┐
                    │       INIT       │
                    │   Headlight OFF  │
                    └────────┬─────────┘
                             │ All inputs valid
                             ▼
                    ┌──────────────────┐
        ┌──────────│      NORMAL      │◄─────────┐
        │          │ Normal operation  │          │
        │          └────────┬─────────┘          │
        │                   │ One input invalid   │ All inputs valid
        │                   ▼                     │
        │          ┌──────────────────┐          │
        │          │     DEGRADED     │──────────┘
        │          │  Limited function │
        │          └────────┬─────────┘
        │                   │ Critical fault/FTTI
        │                   ▼
        │          ┌──────────────────┐
        └─────────►│    SAFE_STATE    │
  Critical fault   │ Day: OFF         │
                   │ Night: LOW_BEAM  │
                   └──────────────────┘
```

## License

This implementation is based on the AUTOSAR Classic Platform R23-11 Safety Use Case specification.

## References

- AUTOSAR Classic Platform R23-11
- AUTOSAR_SWS_E2ELibrary
- AUTOSAR_SWS_WatchdogManager
- AUTOSAR_SWS_DiagnosticEventManager
- ISO 26262 Road vehicles - Functional safety
