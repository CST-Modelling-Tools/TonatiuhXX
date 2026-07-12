# Tonatiuh++ v0.1.8.26 Release Notes

Tonatiuh++ v0.1.8.26 follows the published v0.1.8.25 release. It consolidates one production tracing architecture, deterministic per-chunk random streams, improved hybrid-CPU scheduling, and benchmark reference v2.

## Highlights

- Uses one `TracePreparation` / `PreparedTraceContext` / `RayTraceExecutor` / `RayTracer` path for GUI tracing, Flux Analysis, headless `trace-scene`, benchmark execution, and scripting.
- Uses long-lived `QRunnable` workers on the global `QThreadPool`, fixed 10,000-ray chunks, atomic dynamic chunk acquisition, and `QPromise` / `QFuture` completion.
- Uses one private deterministic `std::mt19937_64` stream per stable chunk with no shared RNG or RNG mutex.
- Requests high thread quality of service for long-lived tracing workers on Qt 6.9 and newer, correcting observed Windows hybrid-CPU scheduling on the validated host.
- Adds deterministic 500-million-ray benchmark reference v2.

## Headless automation

The existing true-headless commands remain available through `QCoreApplication`:

- `tonatiuhpp --headless --help`
- `tonatiuhpp --headless validate-scene <scene.tnhpp>`
- `tonatiuhpp --headless trace-scene <scene.tnhpp> --rays N --seed S --no-export`
- `tonatiuhpp --headless benchmark <benchmark_config.json>`
- `tonatiuhpp --headless run-script <script.tnhpps>`

The minimal headless script API remains limited to `print`, `tn.writeJson`, `tn.validateScene`, `tn.runBenchmark`, and deterministic no-export `tn.traceScene`.

## Canonical tracing architecture

Obsolete `RayTraceRunner` and `ParallelRayTraceExecutor` backends have been removed. All production entry points prepare the same immutable context and execute through the same scheduler. Scene ownership, preparation, photon handling, cancellation, progress, overlap protection, and exception propagation remain centralized.

## Parallel execution

The executor keeps a small set of long-lived workers rather than scheduling one task per chunk. Workers atomically acquire fixed 10,000-ray chunks until the trace completes. Chunk identity is independent of worker identity and scheduling order.

## Random-number subsystem

- `std::mt19937_64` is the canonical deterministic Monte Carlo engine.
- Every stable chunk owns one private engine for the lifetime of that chunk.
- A fixed-width SplitMix64-style derivation combines the resolved master seed and stable chunk index.
- Engine output is converted explicitly from its high 53 bits to an IEEE-754 binary64 value in `[0, 1)` without `std::uniform_real_distribution`.
- `RandomParallel`, `RandomSTL`, legacy custom RNG plugins, RNG factories, GUI algorithm selection, refill caching, and shared RNG synchronization have been removed.
- Embedded tracing and RNG profiling switches, timers, counters, and aggregate diagnostic logs have been removed; external profilers are required for performance analysis.

This engine is intended for deterministic Monte Carlo ray tracing and is not cryptographically secure.

## Windows hybrid-CPU performance

On Qt 6.9 and newer, each long-lived tracing worker saves its existing `QThread::QualityOfService`, requests `High` while tracing, and restores the previous value on exit. No affinity, priority, CPU-set, worker-count, chunk-size, ray-order, or RNG-identity change is made.

On the externally validated Windows 11 / Intel Core i7-12700K host, this corrected headless execution that had time-sliced 20 runnable workers across four efficiency cores. A 10-million-ray run completed in 15.319 seconds at 652,784 rays/s. A repeated 500-million-ray run completed in 798.608 seconds at 626,089 rays/s with all 20 logical processors in use. These results are host-specific and are not a universal performance guarantee.

## Benchmark reference v2

Reference v2 retains the benchmark-v1 scene, receiver mapping, 100 x 100 grid, formulas, and tolerances while adopting the new deterministic per-chunk streams.

- Rays: 500,000,000
- Master seed: 123456789
- Chunk size: 10,000
- Total power: 42.217052166502086 MW
- Minimum flux: 0.0807424687410667 MW/m2
- Average flux: 2.638565760406375 MW/m2
- Maximum flux: 19.52519778288914 MW/m2
- Binary grid SHA-256: `36c7dc17cee8e7fdef820f08d4482e7c3dcdee3b5eed4748ea3949a6cf4af472`

Two independent 500-million-ray Windows processes produced byte-identical binary grids. The second run passed total-power, maximum-flux, and exact hash comparison with zero metric error. Worker count is runtime metadata, not part of the reference identity.

Reference v1 describes the previous random-stream architecture. Exact historical photon sequences and grid hashes are not expected to match reference v2; this does not by itself indicate a ray-physics change.

## macOS build and release fixes

The existing macOS AGL generated-link sanitizers, app-bundle layout validation, Apple Silicon packaging, and release-workflow hardening remain intact. This release does not redesign those paths.

## Correctness and robustness

- Prepared trace contexts keep borrowed GUI hierarchies and owned headless hierarchies alive through worker completion.
- Executor reuse rejects overlapping runs until the previous future has been joined.
- Benchmark accumulation remains per-thread and merges integer hit counts after tracing.
- Flux Analysis append mode preserves accumulated photon/power data while advancing the deterministic master seed so appended runs use new samples.

## Compatibility and upgrade notes

- RNG algorithm selection and legacy RNG plugins are no longer available.
- Historical exact random sequences, individual ray histories, photon sequences, and flux-grid hashes change under the new stream decomposition.
- Changing the fixed chunk size would change exact streams and results.
- Qt versions older than 6.9 retain their previous scheduling behavior because the quality-of-service API is compile-time guarded.
- The engine stream and uniform conversion are designed to reproduce across conforming standard libraries; final physics results can still differ across platforms because unrelated floating-point operations are not guaranteed bit-identical.

## Validation

- Windows hybrid-CPU utilization was externally validated at 10 million and 500 million rays.
- Benchmark reference v2 was reproduced exactly in two separate Windows processes.
- Focused deterministic tests cover engine output, uniform conversion, seed vectors, representative seed uniqueness, scheduling independence, worker independence, repeatability, and `[0, 1)` endpoints.

## Known limitations

- Linux and macOS scientific/reference-v2 validation remains pending.
- GUI tracing, GUI photon export, cancellation, Flux Analysis, and append behavior require final release-candidate validation.
- True-headless photon-file export is not implemented; headless tracing and `tn.traceScene` remain no-export workflows.
- macOS signing, notarization, Gatekeeper behavior, and updater installation require release validation.
- The v0.1.8.26 Zenodo record has not yet been published; citation metadata retains the current v0.1.8.25 DOI until publication.
