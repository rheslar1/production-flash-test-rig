# Production Flash and Test Rig Architecture

## Goal

Clear pass/fail evidence for firmware updates, board bring-up, and field acceptance.

## Runtime Shape

1. A release operator provides a firmware manifest and a populated fixture slot.
2. The rig preflights fixture identity, supply voltage, idle current, and manifest compatibility.
3. A programmer adapter erases, writes, and verifies the target board.
4. A boot probe validates firmware version, SoC identity, and boot timing.
5. A soak runner performs update/reboot cycles while tracking failures, board temperature, and run current.
6. A report sink emits release or quarantine evidence for the board record.

## C++17 Design Shape

- `ProductionTestRig` owns orchestration and release/quarantine decisions.
- `IProgrammer`, `IBootProbe`, `ISoakRunner`, and `IReportSink` isolate hardware-specific dependencies.
- `FirmwareImage`, `BoardSlot`, `AcceptanceLimits`, and result structures keep the decision surface explicit.
- `SimulatedProgrammer`, `SimulatedBootProbe`, and `SimulatedSoakRunner` provide deterministic CI behavior.
- `TextReportSink` serializes evidence in a compact format suitable for CI logs and station archives.

## SOLID Notes

- Single Responsibility: orchestration, programming, boot probing, soak execution, and reporting are separated.
- Open/Closed: hardware adapters can be replaced without changing the release decision flow.
- Liskov Substitution: simulators and real station drivers share the same focused interfaces.
- Interface Segregation: each station capability has a small role-specific abstraction.
- Dependency Inversion: the rig depends on interfaces instead of concrete hardware transports.

## Boundaries

- `include/flash_rig/`: public C++ domain types and adapter interfaces.
- `src/`: production rig orchestration, simulator adapters, and CLI demo.
- `docs/`: validation plans, timing notes, hardware captures, and acceptance evidence.
- `tests/`: host-side unit tests for release and quarantine decisions.
- `.github/workflows/`: CI entry point for build and validation evidence.

## Validation Plan

- Build the host station model with CMake.
- Run the executable and confirm one simulated board is released and one incompatible board is quarantined.
- Run CTest to validate pass, preflight, manifest, boot, and soak decisions.
- Add hardware-specific logs after the first real programmer, serial, and power-instrument integration.
- Capture CI, terminal, and hardware evidence for the portfolio detail page.

## Expansion Notes

- Add concrete adapters for SWD/JTAG programmers, UART or USB boot probes, relay-controlled power cycles, and bench supply telemetry.
- Persist report sink output to JSONL, CSV, or a manufacturing execution system endpoint.
- Include signed SWUpdate or Yocto artifact metadata when release images are pulled from a build server.
