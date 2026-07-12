#pragma once

#include "kernel/TonatiuhKernel.h"
#include <atomic>
#include <functional>
#include <vector>

#include <QVector>
#include <QMutex>
#include <QPair>
#include <QSet>
#include <QObject>

#include "libraries/math/3D/Transform.h"
#include "libraries/math/3D/vec3d.h"

class InstanceNode;
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
              PhotonsBuffer* photonBuffer,
              QMutex* mutexPhotons,
              QVector<InstanceNode*> exportSuraceList,
              std::atomic_bool* exportFailed = nullptr,
              HitCallback hitCallback = HitCallback());

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
    PhotonsBuffer* m_photonBuffer;
    QMutex* m_mutexPhotonsBuffer;
    std::atomic_bool* m_exportFailed;
    HitCallback m_hitCallback;
    QVector<InstanceNode*> m_exportSurfaceList;

    const std::vector< QPair<int, int> >&  m_sunCells;
};
