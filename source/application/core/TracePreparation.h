#pragma once

#include <functional>
#include <memory>

#include <QVector>
#include <QString>

#include "core/SceneInstanceBuilder.h"

class AirTransmission;
class InstanceNode;
class PhotonsBuffer;
class Random;
class SunAperture;
class SunPosition;
class SunShape;
class TSceneKit;
struct RayTraceExecutorResult;
struct RayTracerHit;

using PreparedTraceHitCallback = std::function<void(const RayTracerHit&)>;
using TracePreparationProgress = std::function<void(const QString&)>;

struct GuiTracePreparationInput
{
    TSceneKit* scene = nullptr;
    InstanceNode* layoutRoot = nullptr;
    InstanceNode* sunInstance = nullptr;
    Random* random = nullptr;
    PhotonsBuffer* photonBuffer = nullptr;
    QVector<InstanceNode*> exportSurfaceList;
    PreparedTraceHitCallback hitCallback;
    std::function<void()> synchronizeScene;
    AirTransmission* tracingAir = nullptr;
    ulong rays = 0;
    int sunWidthDivisions = 0;
    int sunHeightDivisions = 0;
    bool sizeSunFromScene = false;
};

struct HeadlessTracePreparationInput
{
    TSceneKit* scene = nullptr;
    PreparedTraceHitCallback hitCallback;
    TracePreparationProgress progress;
    ulong rays = 0;
    ulong seed = 0;
    int sunWidthDivisions = 200;
    int sunHeightDivisions = 200;
};

class PreparedTraceContext
{
public:
    PreparedTraceContext();
    ~PreparedTraceContext();
    PreparedTraceContext(const PreparedTraceContext&) = delete;
    PreparedTraceContext& operator=(const PreparedTraceContext&) = delete;
    PreparedTraceContext(PreparedTraceContext&&) noexcept;
    PreparedTraceContext& operator=(PreparedTraceContext&&) noexcept;

    ulong rays() const { return m_rays; }
    double sunApertureArea() const;
    double irradiance() const;
    double powerPerRay() const;
    PhotonsBuffer* photonBuffer() const { return m_photonBuffer; }

private:
    TSceneKit* m_scene = nullptr;
    SceneInstanceTree m_ownedInstanceTree;
    std::unique_ptr<InstanceNode> m_ownedSunInstance;
    std::unique_ptr<Random> m_ownedRandom;

    InstanceNode* m_layoutRoot = nullptr;
    InstanceNode* m_sunInstance = nullptr;
    SunPosition* m_sunPosition = nullptr;
    SunAperture* m_sunAperture = nullptr;
    SunShape* m_sunShape = nullptr;
    AirTransmission* m_tracingAir = nullptr;
    Random* m_random = nullptr;
    PhotonsBuffer* m_photonBuffer = nullptr;
    QVector<InstanceNode*> m_exportSurfaceList;
    PreparedTraceHitCallback m_hitCallback;
    ulong m_rays = 0;

    friend class TracePreparation;
    friend class RayTraceExecutor;
};

class TracePreparation
{
public:
    static bool prepareGuiTrace(const GuiTracePreparationInput& input,
                                PreparedTraceContext* context,
                                QString* errorMessage = nullptr);
    static bool prepareHeadlessTrace(const HeadlessTracePreparationInput& input,
                                     PreparedTraceContext* context,
                                     QString* errorMessage = nullptr);
    static void initializeResult(const PreparedTraceContext& context, RayTraceExecutorResult* result);
    static bool finalizeResult(const PreparedTraceContext& context,
                               bool exportFailed,
                               double elapsedSeconds,
                               RayTraceExecutorResult* result,
                               QString* errorMessage = nullptr);
};
