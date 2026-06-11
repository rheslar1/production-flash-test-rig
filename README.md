# Production Flash and Test Rig

Repeatable board flashing, update-cycle validation, and long-run soak checks for deployment confidence.

## Portfolio Purpose

This repository models a production fixture that accepts a firmware release image and a board slot, then emits pass/fail evidence for manufacturing release. The current implementation is host-runnable C++17 with simulated adapters so CI can verify the production workflow before real programmers, serial probes, and power instruments are wired in.

## Stack

- C++17
- C++ Design Patterns
- SOLID
- CMake / CTest
- SWUpdate
- Yocto
- Shell
- QA logs
- Hardware lab

## Quick Start

```bash
cmake -S . -B build
cmake --build build
./build/production_flash_test_rig
ctest --test-dir build --output-on-failure
```

## Implementation Slices

- Firmware manifest validation for version, target hardware revision, digest, and image size.
- Board fixture preflight checks for serial, slot identity, voltage, and idle current.
- Interface boundaries for programmer, boot probe, soak runner, and report sink adapters.
- Simulated production station adapters for deterministic CI and local validation.
- Orchestrated erase, program, verify, boot-health, power/thermal soak, and release decision steps.
- Text evidence reports that can be archived with production records or CI logs.
- CTest coverage for accepted boards, wrong hardware revision, invalid images, boot mismatch, and soak failures.

## Evidence Target

Clear pass/fail evidence for firmware updates, board bring-up, and field acceptance.

Example station output:

```text
serial=PFT-000427 result=PASS reason="accepted"
  [PASS] fixture-preflight: FIXTURE-A/SLOT-01 stable at 5.03 V and 38.5 mA idle
  [PASS] firmware-manifest: version 2026.06.10-production for hardware REV-C digest 3b7e1f0c9d4a
  [PASS] erase-program-verify: erased, programmed 4194304 bytes, verified 3b7e1f0c9d4a
  [PASS] boot-health: firmware 2026.06.10-production responded in 1180 ms
  [PASS] power-thermal-soak: 12 cycles, max 48.5 C, 181.0 mA
  [PASS] release-decision: board released for packaging with complete evidence
```

## Remote

Intended public repository: https://github.com/rheslar1/production-flash-test-rig

<!-- cpp17-solid-implementation:start -->
## C++17, Design Patterns, and SOLID Implementation

This repository includes a host-buildable C++17 implementation, not only documentation. The implementation applies:

- Strategy pattern for validation rules.
- Adapter interfaces for input samples and telemetry/reporting.
- Composite validation for combining safety and readiness checks.
- Facade orchestration through the project runtime class.
- SOLID boundaries between profile data, input acquisition, validation, telemetry encoding, and tests.
<!-- cpp17-solid-implementation:end -->

<!-- deep-architecture-links:start -->
## Deep Architecture and UML

- [Deep architecture](docs/deep-architecture.md)
- [Full UML Draw.io source](docs/diagrams/full-system-uml.drawio)
- [Full UML PNG export](docs/diagrams/full-system-uml.png)
<!-- deep-architecture-links:end -->

<!-- DESIGN_PACKAGE_START -->
## Detailed Design Package

This repository includes a structured design package for **Production Flash and Test Rig**. The package captures the system boundary, runtime flow, hardware/software interfaces, validation strategy, and implementation roadmap.

| Artifact | Link |
| --- | --- |
| Design Index | [docs/design/README.md](docs/design/README.md) |
| System Design | [docs/design/system-design.md](docs/design/system-design.md) |
| Requirements | [docs/design/requirements.md](docs/design/requirements.md) |
| Interface Control | [docs/design/interface-control.md](docs/design/interface-control.md) |
| Runtime Design | [docs/design/runtime-design.md](docs/design/runtime-design.md) |
| Validation Plan | [docs/design/validation-plan.md](docs/design/validation-plan.md) |
| Implementation Roadmap | [docs/design/implementation-roadmap.md](docs/design/implementation-roadmap.md) |
| Draw.io UML | [docs/design/diagrams/system-design.drawio](docs/design/diagrams/system-design.drawio) |
| PNG UML | [docs/design/diagrams/system-design.png](docs/design/diagrams/system-design.png) |
<!-- DESIGN_PACKAGE_END -->
