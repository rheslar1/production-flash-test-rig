# Validation Plan

## Current Host Checks

- CMake configure completes.
- C++17 rig executable builds.
- Executable emits evidence for a passing board and a quarantined board.
- CTest verifies manifest validation, hardware revision gating, flash/verify flow, boot version checks, and soak current limits.
- GitHub Actions runs configure, build, executable smoke run, and CTest.

## Hardware Evidence To Add

- Fixture photo with board serial label and slot assignment visible.
- Programmer log showing erase, program, verify, and readback digest.
- Boot probe transcript showing firmware version, SoC identity, and boot timing.
- Power/thermal soak CSV showing update/reboot cycles, failures, max temperature, and max current.
- CI screenshot after the public repository is pushed.

## Project-Specific Evidence Target

Clear pass/fail evidence for firmware updates, board bring-up, and field acceptance.

## First Hardware Bring-Up

1. Replace `SimulatedProgrammer` with a concrete SWD/JTAG or eMMC programmer adapter.
2. Replace `SimulatedBootProbe` with a UART, USB CDC, or network health probe.
3. Replace `SimulatedSoakRunner` with relay-controlled reboot/update cycles and bench supply telemetry.
4. Archive the `TextReportSink` output alongside raw programmer, boot, and power logs.
