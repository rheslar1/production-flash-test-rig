# Production Flash and Test Rig

Repeatable board flashing, update-cycle validation, and long-run soak checks for deployment confidence.

## Portfolio Purpose

This repository is an Embedded Systems project scaffold for the Rheslar portfolio. It is designed to become a hardware-backed project with build output, validation logs, and reviewable implementation evidence.

## Stack

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
python -m unittest discover -s tests
```

## Implementation Slices

- Native starter executable that exposes the project identity, stack, and validation target.
- Architecture document with control boundaries, data flow, safety assumptions, and evidence plan.
- Unit smoke test that keeps source, docs, and CI files present as the repo grows.
- GitHub Actions workflow for configure, build, executable smoke run, and repository validation.

## Evidence Target

Clear pass/fail evidence for firmware updates, board bring-up, and field acceptance.

## Remote

Intended public repository: https://github.com/rheslar1/production-flash-test-rig
