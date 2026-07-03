#include "ParallelRayTraceExecutor.h"

#include <atomic>
#include <memory>

#include <QElapsedTimer>
#include <QFuture>
#include <QMutex>
#include <QMutexLocker>
#include <QStringList>
#include <QTextStream>
#include <QThread>
#include <QThreadPool>
#include <QtConcurrentMap>

#include "core/SceneInstanceBuilder.h"
#include "kernel/air/AirTransmission.h"
#include "kernel/air/AirVacuum.h"
#include "kernel/node/TonatiuhFunctions.h"
#include "kernel/photons/PhotonsBuffer.h"
#include "kernel/random/RandomSTL.h"
#include "kernel/run/InstanceNode.h"
#include "kernel/run/RayTracer.h"
#include "kernel/scene/TSceneKit.h"
#include "kernel/sun/SunAperture.h"
#include "kernel/sun/SunKit.h"
#include "kernel/sun/SunPosition.h"
#include "kernel/sun/SunShape.h"
#include "libraries/math/3D/Transform.h"

namespace
{
constexpr int kGuiProgressPartitions = 100;

struct RayTraceTask
{
    int index = 0;
    ulong rays = 0;
};

bool fail(QString* errorMessage, const QString& message)
{
    if (errorMessage)
        *errorMessage = message;
    return false;
}

void reportProgress(const ParallelRayTraceExecutor::ProgressCallback& progress, const QString& message)
{
    if (progress)
        progress(message);
}

QString formatRayProgress(ulong traced, ulong total)
{
    return QString("Traced %1/%2 rays.")
        .arg(QString::number(static_cast<qulonglong>(traced)))
        .arg(QString::number(static_cast<qulonglong>(total)));
}

QVector<RayTraceTask> makeTasks(ulong rays)
{
    const QVector<ulong> partitions = ParallelRayTraceExecutor::guiRaysPerThread(rays);
    QVector<RayTraceTask> tasks;
    tasks.reserve(partitions.size());
    for (int index = 0; index < partitions.size(); ++index)
        tasks.push_back(RayTraceTask{index, partitions[index]});
    return tasks;
}

QString formatFirstRayCounts(const QVector<ulong>& raysPerThread)
{
    QStringList values;
    const int count = qMin(5, raysPerThread.size());
    for (int index = 0; index < count; ++index)
        values << QString::number(static_cast<qulonglong>(raysPerThread[index]));

    if (raysPerThread.size() > count)
        values << "...";

    return values.join(", ");
}

void writeRayLoopDiagnostics(const ParallelRayTraceOptions& options,
                             const QVector<ulong>& raysPerThread,
                             const QVector<RayTraceTask>& tasks,
                             bool photonBufferEnabled,
                             bool hitCallbackEnabled)
{
    if (!options.diagnosticOutput)
        return;

    QTextStream out(stdout);
    out << "ray_loop_diagnostics:" << Qt::endl;
    out << "  rays_per_thread_size: " << raysPerThread.size() << Qt::endl;
    out << "  rays_per_thread_first: [" << formatFirstRayCounts(raysPerThread) << "]" << Qt::endl;
    out << "  task_count: " << tasks.size() << Qt::endl;
    if (options.requestedWorkerCount > 0)
        out << "  requested_worker_count: " << options.requestedWorkerCount << Qt::endl;
    else
        out << "  requested_worker_count: not_set" << Qt::endl;
    out << "  ideal_thread_count: " << QThread::idealThreadCount() << Qt::endl;
    out << "  thread_pool_max_thread_count: " << QThreadPool::globalInstance()->maxThreadCount() << Qt::endl;
    out << "  thread_pool_active_thread_count: " << QThreadPool::globalInstance()->activeThreadCount() << Qt::endl;
    out << "  record_photons: " << (options.recordPhotons ? "true" : "false") << Qt::endl;
    out << "  photon_buffer_enabled: " << (photonBufferEnabled ? "true" : "false") << Qt::endl;
    out << "  hit_callback_enabled: " << (hitCallbackEnabled ? "true" : "false") << Qt::endl;
}

void writeTaskDiagnostic(const ParallelRayTraceOptions& options,
                         QMutex* diagnosticMutex,
                         const QString& event,
                         const RayTraceTask& task,
                         int count,
                         ulong raysCompleted = 0)
{
    if (!options.diagnosticOutput || count > 5)
        return;

    QMutexLocker lock(diagnosticMutex);
    QTextStream out(stdout);
    out << "ray_task_" << event
        << ": count=" << count
        << ", index=" << task.index
        << ", rays=" << static_cast<qulonglong>(task.rays)
        << ", active_threads=" << QThreadPool::globalInstance()->activeThreadCount();
    if (raysCompleted > 0)
        out << ", rays_completed=" << static_cast<qulonglong>(raysCompleted);
    out << Qt::endl;
}
}

QVector<ulong> ParallelRayTraceExecutor::guiRaysPerThread(ulong rays)
{
    QVector<ulong> raysPerThread;
    const ulong raysPerProgressStep = rays / kGuiProgressPartitions;
    for (int progress = 0; progress < kGuiProgressPartitions; ++progress)
        raysPerThread << raysPerProgressStep;

    if (raysPerProgressStep * kGuiProgressPartitions < rays)
        raysPerThread << rays - raysPerProgressStep * kGuiProgressPartitions;

    return raysPerThread;
}

qulonglong ParallelRayTraceExecutor::taskCountForRays(ulong rays)
{
    return static_cast<qulonglong>(guiRaysPerThread(rays).size());
}

bool ParallelRayTraceExecutor::trace(TSceneKit* scene,
                                     const ParallelRayTraceOptions& options,
                                     ParallelRayTraceResult* result,
                                     QString* errorMessage,
                                     const ProgressCallback& progress,
                                     const HitCallback& hitCallback,
                                     const TaskHitCallbackFactory& taskHitCallbackFactory) const
{
    if (result)
        *result = ParallelRayTraceResult();

    if (!scene)
        return fail(errorMessage, "Scene is not loaded.");
    if (options.rays == 0)
        return fail(errorMessage, "Ray count must be greater than zero.");
    if (options.sunWidthDivisions <= 0 || options.sunHeightDivisions <= 0)
        return fail(errorMessage, "Sun grid dimensions must be greater than zero.");

    QElapsedTimer timer;
    timer.start();

    SunKit* sunKit = static_cast<SunKit*>(scene->getPart("world.sun", false));
    if (!sunKit)
        return fail(errorMessage, "Scene has no sun at world.sun.");

    SunShape* sunShape = static_cast<SunShape*>(sunKit->getPart("shape", false));
    if (!sunShape)
        return fail(errorMessage, "Scene sun is missing shape data.");

    SunAperture* sunAperture = static_cast<SunAperture*>(sunKit->getPart("aperture", false));
    if (!sunAperture)
        return fail(errorMessage, "Scene sun is missing aperture data.");

    SunPosition* sunPosition = static_cast<SunPosition*>(sunKit->getPart("position", false));
    if (!sunPosition)
        return fail(errorMessage, "Scene sun is missing position data.");

    reportProgress(progress, "Preparing scene.");
    reportProgress(progress, "Building ray-tracing instance tree.");
    SceneInstanceTree instanceTree = SceneInstanceBuilder::build(scene);
    if (!instanceTree.sceneRoot)
        return fail(errorMessage, "Could not create ray-tracing scene root.");
    InstanceNode* instanceLayout = instanceTree.layoutRoot;
    if (!instanceLayout)
        return fail(errorMessage, "Scene has no layout at group.");

    // GUI parity notes:
    // - MainWindow::Run() calls UpdateLightSize(), which sizes the sun with
    //   SunKit::setBox(TSceneKit*) before the ray loop. That path relies on
    //   Coin bounding-box traversal through the GUI scene graph; in headless
    //   QCoreApplication runs we size from the RT InstanceNode box instead,
    //   matching the geometry that RayTracer will actually traverse.
    // - MainWindow::Run() then updates the InstanceNode tree and calls
    //   SunKit::findTexture().
    // - The ray loop below preserves the GUI's 100-part raysPerThread split,
    //   shared RandomParallel stream, and QtConcurrent::map(RayTracer(...))
    //   execution model without depending on MainWindow, dialogs, or SoQt views.
    reportProgress(progress, "Updating ray-tracing instance tree.");
    instanceLayout->updateTree(Transform::Identity);

    const Box3D& layoutBox = instanceLayout->getBox();
    if (!layoutBox.isValid())
        return fail(errorMessage, "Scene layout has no valid ray-tracing bounds. Check that it contains ray-traceable surfaces with shape and profile data.");

    reportProgress(progress, "Sizing sun aperture.");
    sunKit->setBox(layoutBox);

    InstanceNode instanceSun(sunKit);
    instanceSun.setTransform(tgf::makeTransform(sunKit->m_transform));

    reportProgress(progress, "Finding sun aperture cells.");
    if (!sunKit->findTexture(options.sunWidthDivisions, options.sunHeightDivisions, instanceLayout))
        return fail(errorMessage, "There are no surfaces defined for ray tracing.");

    AirTransmission* air = static_cast<AirTransmission*>(scene->getPart("world.air.transmission", false));
    AirTransmission* tracingAir = nullptr;
    if (air && air->getTypeId() != AirVacuum::getClassTypeId())
        tracingAir = air;

    std::unique_ptr<PhotonsBuffer> localPhotonBuffer;
    PhotonsBuffer* photonBuffer = options.recordPhotons ? options.photonBuffer : nullptr;
    if (options.recordPhotons && !photonBuffer) {
        localPhotonBuffer = std::make_unique<PhotonsBuffer>(options.photonBufferSize, options.photonBufferSize);
        photonBuffer = localPhotonBuffer.get();
    }

    QVector<InstanceNode*> exportSurfaceList = options.exportSurfaceList;
    RandomSTL random(options.seed);
    QMutex mutexRandom;
    QMutex mutexPhotonBuffer;
    QMutex progressMutex;
    QMutex diagnosticMutex;
    std::atomic_bool exportFailed(false);
    std::atomic<ulong> traced(0);
    std::atomic<int> tasksStarted(0);
    std::atomic<int> tasksFinished(0);

    const QVector<ulong> raysPerThread = guiRaysPerThread(options.rays);
    QVector<RayTraceTask> tasks = makeTasks(options.rays);
    if (result) {
        result->sunApertureArea = sunAperture->getArea();
        result->irradiance = sunPosition->irradiance.getValue();
        result->powerPerRay = options.rays > 0 ? result->sunApertureArea * result->irradiance / options.rays : 0.;
        result->workerCount = qMax(1, QThread::idealThreadCount());
        result->chunkSize = raysPerThread.isEmpty() ? 0 : raysPerThread.first();
        result->chunkCount = static_cast<qulonglong>(tasks.size());
    }

    reportProgress(progress, "Starting ray loop.");
    writeRayLoopDiagnostics(
        options,
        raysPerThread,
        tasks,
        photonBuffer != nullptr,
        static_cast<bool>(hitCallback) || static_cast<bool>(taskHitCallbackFactory)
    );
    QFuture<void> future = QtConcurrent::map(tasks, [&](RayTraceTask& task) {
        if (exportFailed.load())
            return;

        const int startedCount = tasksStarted.fetch_add(1) + 1;
        writeTaskDiagnostic(options, &diagnosticMutex, "start", task, startedCount);

        const HitCallback taskHitCallback = taskHitCallbackFactory ? taskHitCallbackFactory(task.index) : hitCallback;
        RayTracer::TraceCallback traceCallback;
        if (options.diagnosticOutput && startedCount <= 5) {
            traceCallback = [&, task, startedCount](const char* event, ulong raysCompleted) {
                writeTaskDiagnostic(
                    options,
                    &diagnosticMutex,
                    QString::fromLatin1(event),
                    task,
                    startedCount,
                    raysCompleted
                );
            };
        }
        RayTracer tracer(
            instanceLayout,
            &instanceSun,
            sunAperture,
            sunShape,
            tracingAir,
            &random,
            &mutexRandom,
            photonBuffer,
            photonBuffer ? &mutexPhotonBuffer : nullptr,
            exportSurfaceList,
            &exportFailed,
            taskHitCallback,
            traceCallback
        );
        writeTaskDiagnostic(options, &diagnosticMutex, "constructed", task, startedCount);
        writeTaskDiagnostic(options, &diagnosticMutex, "operator_call", task, startedCount);
        tracer(task.rays);
        const int finishedCount = tasksFinished.fetch_add(1) + 1;
        writeTaskDiagnostic(options, &diagnosticMutex, "finish", task, finishedCount);
        if (exportFailed.load())
            return;

        const ulong tracedNow = traced.fetch_add(task.rays) + task.rays;
        if (progress && task.rays > 0) {
            QMutexLocker lock(&progressMutex);
            reportProgress(progress, formatRayProgress(qMin(tracedNow, options.rays), options.rays));
        }
    });
    future.waitForFinished();

    const double powerPerRay = result ? result->powerPerRay : (sunAperture->getArea() * sunPosition->irradiance.getValue() / options.rays);
    if (localPhotonBuffer && !localPhotonBuffer->endExport(powerPerRay))
        exportFailed.store(true);

    const ulong raysTraced = traced.load();
    if (!exportFailed.load() && raysTraced != options.rays)
        return fail(errorMessage, "Ray tracing did not complete all requested rays.");

    const double elapsedSeconds = static_cast<double>(timer.elapsed()) / 1000.;
    if (result) {
        result->elapsedSeconds = elapsedSeconds;
        result->raysTraced = raysTraced;
        result->raysPerSecond = elapsedSeconds > 0. ? static_cast<double>(raysTraced) / elapsedSeconds : 0.;
        result->exportFailed = exportFailed.load() || (photonBuffer && photonBuffer->hasExportFailed());
    }

    if (exportFailed.load())
        return fail(errorMessage, "Photon buffer/export failed during ray tracing.");

    return true;
}
