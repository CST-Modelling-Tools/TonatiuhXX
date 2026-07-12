# Tonatiuh++ v0.1.8.26 Release Checklist

Status labels: **completed externally**, **pending**, and **not run by Codex**. Codex performed only static inspection and lightweight file validation for this preparation pass.

## Completed externally

- [x] **completed externally** — Windows 11 hybrid-CPU headless benchmark uses all 20 logical processors after the Qt 6.9+ worker QoS correction.
- [x] **completed externally** — 10,000,000-ray benchmark: 15.319 seconds, 652,784 rays/s, 20 workers, 1,000 chunks, 10,000 rays/chunk.
- [x] **completed externally** — 500,000,000-ray benchmark reference v2 repeated in a separate process: 798.608 seconds, 626,089 rays/s, 20 workers, 50,000 chunks.
- [x] **completed externally** — Repeated run passed total power, maximum flux, and exact binary-grid hash comparison with zero metric error.
- [x] **completed externally** — Reference binary SHA-256 is `36c7dc17cee8e7fdef820f08d4482e7c3dcdee3b5eed4748ea3949a6cf4af472`.

## Source and metadata preparation

- [x] Confirm published baseline is `v0.1.8.25`.
- [x] Set active source version to `0.1.8.26`.
- [x] Add v0.1.8.26 release notes and benchmark reference v2 documentation.
- [x] Add benchmark reference v2 JSON, binary grid, CSV grid, and example configuration.
- [x] Update citation version while retaining the current published Zenodo DOI.
- [ ] **pending** — Publish the v0.1.8.26 Zenodo version, then replace the retained v0.1.8.25 DOI in `CITATION.cff` with the new version DOI.

## Build, installation, and tests

- [ ] **not run by Codex** — Clean configure with the release toolchain on Windows, Linux, and macOS.
- [ ] **not run by Codex** — Full release build on all three platforms.
- [ ] **not run by Codex** — Install into a clean prefix and inspect the installed payload.
- [ ] **not run by Codex** — Run the complete CTest/GoogleTest suite.
- [ ] **pending** — Confirm installed examples contain the benchmark v2 JSON, binary, CSV, and configuration without local run outputs or profiler captures.

## GUI validation

- [ ] **pending** — Normal GUI startup and representative scene load.
- [ ] **pending** — GUI ray tracing uses the canonical executor and completes with expected CPU utilization.
- [ ] **pending** — Photon export, retained-photon behavior, cancellation, progress, ray display, and cumulative power.
- [ ] **pending** — Flux Analysis produces expected results.
- [ ] **pending** — Flux Analysis Append preserves accumulated data and uses new deterministic samples on each appended run.

## Headless validation

- [ ] **pending** — `--headless --help`.
- [ ] **pending** — `validate-scene` on plugin-free and representative plugin scenes.
- [ ] **pending** — Deterministic no-export `trace-scene` with an explicit seed.
- [ ] **pending** — Benchmark smoke run with benchmark reference v2.
- [ ] **pending** — `run-script`, including `print`, `tn.writeJson`, `tn.validateScene`, `tn.runBenchmark`, and no-export `tn.traceScene`.
- [ ] **pending** — Confirm `trace-scene` and benchmark create no photon files.
- [ ] **pending** — Repeat fixed-seed runs in separate processes and compare scientific fields/hashes on release binaries.
- [ ] **pending** — Re-run the 500-million-ray v2 comparison and confirm `benchmark_pass: true` and the validated SHA-256.

## Platform and package validation

- [ ] **pending** — Windows installer contents, executable icon, file associations, bundled dependencies, and uninstall.
- [ ] **pending** — Linux launcher, `qt.conf`, bundled Qt libraries/plugins, `ldd` resolution, GUI, and headless commands.
- [ ] **pending** — macOS arm64 app bundle, `Info.plist`, Mach-O dependencies, AGL scrub validation, GUI, and headless commands.
- [ ] **pending** — Windows code signing where configured.
- [ ] **pending** — macOS signing, notarization, and Gatekeeper validation.
- [ ] **pending** — Confirm Linux/macOS benchmark-v2 scientific results and document any floating-point differences without assuming physics failure.

## Updater and publication

- [ ] **pending** — Generate Windows, Linux, and macOS IFW repositories and installers for `0.1.8.26`.
- [ ] **pending** — Validate `Updates.xml`, package id `com.tonatiuhpp.app`, version, release date, and platform repository URLs.
- [ ] **pending** — Update from installed `v0.1.8.25` to `v0.1.8.26` through MaintenanceTool on all platforms.
- [ ] **pending** — Create and push tag `v0.1.8.26` only after validation.
- [ ] **pending** — Publish GitHub release assets, checksums, and notes.
- [ ] **pending** — Publish the new Zenodo version and archive benchmark v2 artifacts durably.
- [ ] **pending** — Update the Zenodo DOI in `CITATION.cff` after publication and commit that metadata change.
