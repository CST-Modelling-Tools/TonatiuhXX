#pragma once

#include <functional>
#include <atomic>
#include <memory>
#include <utility>

#include <QFuture>
#include <QMutex>
#include <QVector>
#include <QString>
#include <qglobal.h>

class InstanceNode;
class PhotonsBuffer;
class Random;
class AirTransmission;
class SunAperture;
class SunShape;
class TSceneKit;
class RayTraceExecutor;
struct RayTracerHit;
struct RayTraceDiagnostics;

struct RayTraceExecutorOptions
{
    ulong rays = 0;
    ulong seed = 0;
    int sunWidthDivisions = 200;
    int sunHeightDivisions = 200;
    ulong photonBufferSize = 1'000'000;
    PhotonsBuffer* photonBuffer = nullptr;
    QVector<InstanceNode*> exportSurfaceList;
    bool recordPhotons = true;
};

struct RayTraceExecutorResult
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

struct RayTraceExecution
{
    RayTraceExecution() = default;
    RayTraceExecution(const RayTraceExecution&) = delete;
    RayTraceExecution& operator=(const RayTraceExecution&) = delete;
    RayTraceExecution(RayTraceExecution&& other):
        started(other.started),
        future(std::move(other.future)),
        errorMessage(std::move(other.errorMessage)),
        owner(other.owner)
    {
        other.started = false;
        other.owner = nullptr;
    }
    RayTraceExecution& operator=(RayTraceExecution&&) = delete;

    bool started = false;
    QFuture<void> future;
    QString errorMessage;

private:
    const RayTraceExecutor* owner = nullptr;
    friend class RayTraceExecutor;
};

class RayTraceExecutor
{
public:
    using ProgressCallback = std::function<void(const QString&)>;
    using HitCallback = std::function<void(const RayTracerHit&)>;

    static QVector<ulong> guiRaysPerThread(ulong rays);
    static qulonglong taskCountForRays(ulong rays);

    // One executor instance supports one active execution at a time. Every
    // successful launch must be completed through waitForFinished().
    RayTraceExecution start(ulong rays,
                            InstanceNode* instanceLayout,
                            InstanceNode* instanceSun,
                            SunAperture* sunAperture,
                            SunShape* sunShape,
                            AirTransmission* air,
                            Random* random,
                            PhotonsBuffer* photonBuffer,
                            const QVector<InstanceNode*>& exportSurfaceList,
                            const HitCallback& hitCallback = HitCallback());

    bool waitForFinished(RayTraceExecution* execution, QString* errorMessage = nullptr) noexcept;
    bool isRunning() const { return m_active.load(); }

    bool exportFailed() const { return m_exportFailed.load(); }

    bool trace(TSceneKit* scene,
               const RayTraceExecutorOptions& options,
               RayTraceExecutorResult* result,
               QString* errorMessage,
               const ProgressCallback& progress = ProgressCallback(),
               const HitCallback& hitCallback = HitCallback());

private:
    QMutex m_randomMutex;
    QMutex m_photonBufferMutex;
    std::atomic_bool m_active{false};
    std::atomic_bool m_exportFailed{false};
    QVector<ulong> m_rayPartitions;
    std::shared_ptr<RayTraceDiagnostics> m_diagnostics;
};
