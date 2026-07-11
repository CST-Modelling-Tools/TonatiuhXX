#pragma once

#include "kernel/TonatiuhKernel.h"
#include <atomic>
#include <functional>
#include <vector>

#include <QVector>
#include <QElapsedTimer>
#include <QMap>
#include <QMutex>
#include <QPair>
#include <QSet>
#include <QObject>

#include "libraries/math/3D/Transform.h"
#include "libraries/math/3D/vec3d.h"

class InstanceNode;
class RandomParallel;
struct Photon;
class Random;
struct RayTracerPhoton;
class QMutex;
class QPoint;
class PhotonsBuffer;
class SunAperture;
class SunShape;
class AirTransmission;

struct TONATIUH_KERNEL RayTracerHit
{
    vec3d position;
    InstanceNode* surface = nullptr;
    bool isFront = false;
};

struct TONATIUH_KERNEL RayTraceDiagnostics
{
    void start();
    void partitionStarted();
    void partitionFinished();
    void recordRandomStats(quint64 refillCount, qint64 mutexWaitNanoseconds, qint64 refillNanoseconds);
    int distinctWorkerThreadCount() const;
    qint64 wallNanoseconds() const;

    std::atomic<quint64> totalRefillCount{0};
    std::atomic<qint64> summedMutexWaitNanoseconds{0};
    std::atomic<qint64> maximumPartitionMutexWaitNanoseconds{0};
    std::atomic<qint64> summedRefillNanoseconds{0};
    std::atomic<qint64> maximumPartitionRefillNanoseconds{0};
    std::atomic<int> activePartitions{0};
    std::atomic<int> maximumActivePartitions{0};

private:
    mutable QMutex m_workerThreadsMutex;
    QSet<quintptr> m_workerThreads;
    QElapsedTimer m_wallTimer;
};

class TONATIUH_KERNEL RayTracer
{

public:
    using HitCallback = std::function<void(const RayTracerHit&)>;

    RayTracer(InstanceNode* instanceRoot,
              InstanceNode* instanceSun,
              SunAperture* sunAperture,
              SunShape* sunShape,
              AirTransmission* air,
              Random* rand,
              QMutex* mutexRand,
              PhotonsBuffer* photonBuffer,
              QMutex* mutexPhotons,
              QVector<InstanceNode*> exportSuraceList,
              std::atomic_bool* exportFailed = nullptr,
              HitCallback hitCallback = HitCallback(),
              RayTraceDiagnostics* diagnostics = nullptr);

    typedef void result_type;

    void operator()(ulong nRays);

private:
    bool NewPrimitiveRay(Ray* ray, Random& rand);

    InstanceNode* m_instanceLayout;
    InstanceNode* m_instanceSun;
    SunAperture* m_sunAperture;
    SunShape* m_sunShape;
    Transform m_sunTransform;
    AirTransmission* m_air;
    Random* m_rand;
    QMutex* m_mutexRand;
    PhotonsBuffer* m_photonBuffer;
    QMutex* m_mutexPhotonsBuffer;
    std::atomic_bool* m_exportFailed;
    HitCallback m_hitCallback;
    RayTraceDiagnostics* m_diagnostics;
    QVector<InstanceNode*> m_exportSurfaceList;

    const std::vector< QPair<int, int> >&  m_sunCells;
};
