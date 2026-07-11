#include "RayTraceExecutor.h"

#include <memory>

#include <QElapsedTimer>
#include <QFuture>
#include <QMutex>
#include <QThread>
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

bool fail(QString* errorMessage, const QString& message)
{
    if (errorMessage)
        *errorMessage = message;
    return false;
}

void reportProgress(const RayTraceExecutor::ProgressCallback& progress, const QString& message)
{
    if (progress)
        progress(message);
}

}

QVector<ulong> RayTraceExecutor::guiRaysPerThread(ulong rays)
{
    QVector<ulong> raysPerThread;
    const ulong raysPerProgressStep = rays / kGuiProgressPartitions;
    for (int progress = 0; progress < kGuiProgressPartitions; ++progress)
        raysPerThread << raysPerProgressStep;

    if (raysPerProgressStep * kGuiProgressPartitions < rays)
        raysPerThread << rays - raysPerProgressStep * kGuiProgressPartitions;

    return raysPerThread;
}

qulonglong RayTraceExecutor::taskCountForRays(ulong rays)
{
    return static_cast<qulonglong>(guiRaysPerThread(rays).size());
}

bool RayTraceExecutor::trace(TSceneKit* scene,
                                     const RayTraceExecutorOptions& options,
                                     RayTraceExecutorResult* result,
                                     QString* errorMessage,
                                     const ProgressCallback& progress,
                                     const HitCallback& hitCallback)
{
    if (result)
        *result = RayTraceExecutorResult();

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
    QVector<ulong> raysPerThread = guiRaysPerThread(options.rays);
    if (result) {
        result->sunApertureArea = sunAperture->getArea();
        result->irradiance = sunPosition->irradiance.getValue();
        result->powerPerRay = options.rays > 0 ? result->sunApertureArea * result->irradiance / options.rays : 0.;
        result->workerCount = qMax(1, QThread::idealThreadCount());
        result->chunkSize = raysPerThread.isEmpty() ? 0 : raysPerThread.first();
        result->chunkCount = static_cast<qulonglong>(raysPerThread.size());
    }

    reportProgress(progress, "Starting ray loop.");
    QFuture<void> future = start(options.rays, instanceLayout, &instanceSun, sunAperture, sunShape,
                                 tracingAir, &random, photonBuffer, exportSurfaceList,
                                 hitCallback);
    future.waitForFinished();

    const double powerPerRay = result ? result->powerPerRay : (sunAperture->getArea() * sunPosition->irradiance.getValue() / options.rays);
    if (localPhotonBuffer && !localPhotonBuffer->endExport(powerPerRay))
        m_exportFailed.store(true);

    const ulong raysTraced = m_exportFailed.load() ? 0 : options.rays;

    const double elapsedSeconds = static_cast<double>(timer.elapsed()) / 1000.;
    if (result) {
        result->elapsedSeconds = elapsedSeconds;
        result->raysTraced = raysTraced;
        result->raysPerSecond = elapsedSeconds > 0. ? static_cast<double>(raysTraced) / elapsedSeconds : 0.;
        result->exportFailed = m_exportFailed.load() || (photonBuffer && photonBuffer->hasExportFailed());
    }

    if (m_exportFailed.load())
        return fail(errorMessage, "Photon buffer/export failed during ray tracing.");

    return true;
}

QFuture<void> RayTraceExecutor::start(ulong rays,
                                      InstanceNode* instanceLayout,
                                      InstanceNode* instanceSun,
                                      SunAperture* sunAperture,
                                      SunShape* sunShape,
                                      AirTransmission* air,
                                      Random* random,
                                      PhotonsBuffer* photonBuffer,
                                      const QVector<InstanceNode*>& exportSurfaceList,
                                      const HitCallback& hitCallback)
{
    m_exportFailed.store(false);
    m_rayPartitions = guiRaysPerThread(rays);
    return QtConcurrent::map(
        m_rayPartitions,
        RayTracer(instanceLayout,
                  instanceSun,
                  sunAperture,
                  sunShape,
                  air,
                  random,
                  &m_randomMutex,
                  photonBuffer,
                  photonBuffer ? &m_photonBufferMutex : nullptr,
                  exportSurfaceList,
                  &m_exportFailed,
                  hitCallback)
    );
}
