# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Build
mkdir build && cd build
cmake ..
cmake --build .

# Run application
./build/flm_application

# Run tests (requires Google Test)
./build/flm_tests

# Build options
cmake -DBUILD_TESTS=OFF ..      # Disable tests
cmake -DENABLE_COVERAGE=ON ..   # Enable coverage
cmake -DCMAKE_BUILD_TYPE=Debug ..  # Debug build
```

## Architecture

This is an AUTOSAR Classic Platform R23-11 Front Light Management (FLM) ECU implementation with ASIL B functional safety requirements per ISO 26262.

### Layer Structure

1. **Application Layer** (`src/Application/`) - Software Components (SWCs):
   - `SwitchEvent` (ASIL B): CAN light switch signal reception with E2E Profile 01 validation
   - `LightRequest` (ASIL A): ADC ambient light sensor with 4-sample averaging and plausibility checks
   - `FLM` (ASIL B): Main control logic with state machine (INIT→NORMAL→DEGRADED→SAFE_STATE)
   - `Headlight` (ASIL B): DIO output control with feedback current monitoring
   - `SafetyMonitor` (ASIL B): Fault aggregation and FTTI (200ms) monitoring

2. **RTE Layer** (`include/Rte/`) - Runtime Environment interfaces between SWCs

3. **BSW Layer** (`src/BSW/`) - Basic Software:
   - `E2E`: Profile 01 end-to-end protection (CRC-8 SAE-J1850 + 4-bit counter)
   - `WdgM`: Watchdog Manager (alive, deadline, logical supervision)
   - `Dem`: Diagnostic Event Manager
   - `Com`: Communication module
   - `BswM`: BSW Mode Manager

4. **MCAL Layer** (`src/MCAL/`) - Microcontroller Abstraction:
   - `Adc`: ADC driver for ambient light and current sensing
   - `Dio`: Digital I/O for headlight relay control
   - `Can`: CAN driver for BCM communication

### Key Configuration

All timing and threshold parameters are in `config/FLM_Config.h`:
- CAN timeout: 50ms
- E2E timeout: 100ms
- FTTI: 200ms
- Safe state transition: 100ms
- Ambient light thresholds: 800 (ON), 1000 (OFF) with hysteresis

### Safety Mechanisms

- E2E Profile 01 protection on CAN messages
- Watchdog supervision (alive, deadline, logical)
- ADC plausibility checks (rate of change, open/short circuit)
- Headlight feedback current monitoring (20ms fault detection)
- State machine with degraded and safe state handling
