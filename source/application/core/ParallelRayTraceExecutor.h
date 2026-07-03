#pragma once

#include <functional>

#include <QVector>
#include <QString>
#include <qglobal.h>

class InstanceNode;
class PhotonsBuffer;
class TSceneKit;
struct RayTracerHit;

struct ParallelRayTraceOptions
{
    ulong rays = 0;
    ulong seed = 0;
    int sunWidthDivisions = 200;
    int sunHeightDivisions = 200;
    ulong photonBufferSize = 1'000'000;
    PhotonsBuffer* photonBuffer = nullptr;
    QVector<InstanceNode*> exportSurfaceList;
    bool recordPhotons = true;
    bool diagnosticOutput = false;
    int requestedWorkerCount = 0;
};

struct ParallelRayTraceResult
{
    double elapsedSeconds = 0.;
    double raysPerSecond = 0.;
    double sunApertureArea = 0.;
    double irradiance = 0.;
    double powerPerRay = 0.;
    ulong raysTraced = 0;
    int workerCount = 1;
    ulong chunkSize = 0;
    qulonglong chunkCount = 0;
    bool exportFailed = false;
};

// Experimental/internal executor retained for source-level comparison with the
// GUI QtConcurrent model. Release headless commands should route through
// RayTraceRunner instead.
class ParallelRayTraceExecutor
{
public:
    using ProgressCallback = std::function<void(const QString&)>;
    using HitCallback = std::function<void(const RayTracerHit&)>;
    using TaskHitCallbackFactory = std::function<HitCallback(int)>;

    static QVector<ulong> guiRaysPerThread(ulong rays);
    static qulonglong taskCountForRays(ulong rays);

    bool trace(TSceneKit* scene,
               const ParallelRayTraceOptions& options,
               ParallelRayTraceResult* result,
               QString* errorMessage,
               const ProgressCallback& progress = ProgressCallback(),
               const HitCallback& hitCallback = HitCallback(),
               const TaskHitCallbackFactory& taskHitCallbackFactory = TaskHitCallbackFactory()) const;
};
