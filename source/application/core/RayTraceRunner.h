#pragma once

#include <functional>

#include <QVector>
#include <QString>
#include <qglobal.h>

class InstanceNode;
class PhotonsBuffer;
class TSceneKit;
struct RayTracerHit;

enum class RayTraceOutputMode
{
    NoOutput,
    PhotonBuffer
};

struct RayTraceOptions
{
    ulong rays = 0;
    ulong seed = 0;
    int sunWidthDivisions = 100;
    int sunHeightDivisions = 100;
    int workerCount = 1;
    ulong chunkSize = 10000;
    RayTraceOutputMode outputMode = RayTraceOutputMode::NoOutput;
    PhotonsBuffer* photonBuffer = nullptr;
    QVector<InstanceNode*> exportSurfaceList;
};

struct RayTraceResult
{
    RayTraceOutputMode outputMode = RayTraceOutputMode::NoOutput;
    double elapsedSeconds = 0.;
    double raysPerSecond = 0.;
    double sunApertureArea = 0.;
    double irradiance = 0.;
    double powerPerRay = 0.;
    ulong raysTraced = 0;
    int workerCount = 1;
    ulong chunkSize = 0;
    qulonglong chunkCount = 0;
    bool canceled = false;
    bool exportFailed = false;
};

class RayTraceRunner
{
public:
    using ProgressCallback = std::function<void(const QString&)>;
    using CancellationCallback = std::function<bool()>;
    using HitCallback = std::function<void(const RayTracerHit&)>;
    using WorkerHitCallbackFactory = std::function<HitCallback(int)>;

    // Experimental worker/chunk runner retained for internal comparison and
    // possible future migration work. Release headless tracing uses
    // ParallelRayTraceExecutor so it follows the GUI QtConcurrent/RayTracer
    // execution model instead of this custom std::thread scheduler.
    bool trace(TSceneKit* scene,
               const RayTraceOptions& options,
               RayTraceResult* result,
               QString* errorMessage,
               const ProgressCallback& progress = ProgressCallback(),
               const HitCallback& hitCallback = HitCallback(),
               const WorkerHitCallbackFactory& workerHitCallbackFactory = WorkerHitCallbackFactory(),
               const CancellationCallback& cancellation = CancellationCallback()) const;
};
