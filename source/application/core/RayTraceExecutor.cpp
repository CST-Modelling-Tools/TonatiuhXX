#include "RayTraceExecutor.h"

#include <exception>

#include <QDebug>
#include <QFuture>
#include <QMutex>
#include <QThread>
#include <QThreadPool>
#include <QtConcurrentMap>

#include "core/TracePreparation.h"
#include "kernel/run/InstanceNode.h"
#include "kernel/run/RayTracer.h"

namespace
{
constexpr ulong kMaximumRayWorkItemSize = 100'000;
constexpr int kTargetWorkItemsPerThread = 8;

}

QVector<ulong> RayTraceExecutor::guiRaysPerThread(ulong rays)
{
    QVector<ulong> workItems;
    if (rays == 0)
        return workItems;

    const qulonglong threadCount = static_cast<qulonglong>(qMax(1, QThreadPool::globalInstance()->maxThreadCount()));
    const qulonglong targetWorkItemCount = threadCount * static_cast<qulonglong>(kTargetWorkItemsPerThread);
    const qulonglong rayCount = static_cast<qulonglong>(rays);
    const qulonglong balancedWorkItemSize =
        rayCount / targetWorkItemCount + (rayCount % targetWorkItemCount != 0 ? 1 : 0);
    const ulong workItemSize = static_cast<ulong>(qMax<qulonglong>(
        1,
        qMin<qulonglong>(kMaximumRayWorkItemSize, balancedWorkItemSize)
    ));
    const qulonglong workItemCount =
        rayCount / workItemSize + (rayCount % workItemSize != 0 ? 1 : 0);
    workItems.reserve(static_cast<qsizetype>(workItemCount));

    ulong remaining = rays;
    while (remaining > 0) {
        const ulong currentWorkItemSize = qMin(workItemSize, remaining);
        workItems << currentWorkItemSize;
        remaining -= currentWorkItemSize;
    }

    return workItems;
}

qulonglong RayTraceExecutor::taskCountForRays(ulong rays)
{
    return static_cast<qulonglong>(guiRaysPerThread(rays).size());
}

RayTraceExecution RayTraceExecutor::start(PreparedTraceContext&& context)
{
    RayTraceExecution execution;
    if (!context.m_layoutRoot || !context.m_sunInstance || !context.m_sunAperture
        || !context.m_sunShape || !context.m_random || context.m_rays == 0) {
        execution.errorMessage = "RayTraceExecutor requires a valid prepared trace context.";
        return execution;
    }

    bool expected = false;
    if (!m_active.compare_exchange_strong(expected, true)) {
        execution.errorMessage = "RayTraceExecutor already has an active execution.";
        return execution;
    }

    m_exportFailed.store(false);
    m_diagnostics.reset();
    try {
        m_activeContext = std::make_shared<PreparedTraceContext>(std::move(context));
        execution.preparedContext = m_activeContext;
        PreparedTraceContext& prepared = *m_activeContext;
        m_rayPartitions = guiRaysPerThread(prepared.m_rays);
        if (qEnvironmentVariableIntValue("TONATIUHPP_TRACE_THREAD_DIAGNOSTICS") > 0) {
            m_diagnostics = std::make_shared<RayTraceDiagnostics>();
            m_diagnostics->start();
            int nonzeroPartitions = 0;
            for (ulong partitionRays : std::as_const(m_rayPartitions)) {
                if (partitionRays > 0)
                    ++nonzeroPartitions;
            }
            qInfo().nospace()
                << "RayTraceExecutor diagnostics: partitions=" << m_rayPartitions.size()
                << ", nonzero_partitions=" << nonzeroPartitions
                << ", global_pool_max_threads=" << QThreadPool::globalInstance()->maxThreadCount()
                << ", ideal_threads=" << QThread::idealThreadCount();
        }
        execution.future = QtConcurrent::map(
            m_rayPartitions,
            RayTracer(prepared.m_layoutRoot,
                      prepared.m_sunInstance,
                      prepared.m_sunAperture,
                      prepared.m_sunShape,
                      prepared.m_tracingAir,
                      prepared.m_random,
                      &m_randomMutex,
                      prepared.m_photonBuffer,
                      prepared.m_photonBuffer ? &m_photonBufferMutex : nullptr,
                      prepared.m_exportSurfaceList,
                      &m_exportFailed,
                      prepared.m_hitCallback,
                      m_diagnostics.get())
        );
        if (!execution.future.isValid()) {
            execution.errorMessage = "Could not start ray tracing because QtConcurrent returned an invalid future.";
            m_activeContext.reset();
            m_active.store(false);
            return execution;
        }
        execution.started = true;
        execution.owner = this;
    } catch (const std::exception& error) {
        execution.errorMessage = QString("Could not start ray tracing: %1").arg(error.what());
        m_activeContext.reset();
        m_active.store(false);
    } catch (...) {
        execution.errorMessage = "Could not start ray tracing because an unknown exception was raised.";
        m_activeContext.reset();
        m_active.store(false);
    }
    return execution;
}

bool RayTraceExecutor::waitForFinished(RayTraceExecution* execution, QString* errorMessage) noexcept
{
    if (!execution || !execution->started || execution->owner != this) {
        if (errorMessage)
            *errorMessage = "Ray-trace execution was not started.";
        return false;
    }

    struct ActiveRunReset
    {
        std::atomic_bool* active;
        std::shared_ptr<PreparedTraceContext>* context;
        ~ActiveRunReset()
        {
            context->reset();
            active->store(false);
        }
    } reset{&m_active, &m_activeContext};

    try {
        execution->future.waitForFinished();
        if (m_diagnostics) {
            qInfo().nospace()
                << "RayTraceExecutor aggregate diagnostics: total_rng_refills=" << m_diagnostics->totalRefillCount.load()
                << ", summed_rng_mutex_wait_ms=" << m_diagnostics->summedMutexWaitNanoseconds.load() / 1.e6
                << ", maximum_partition_rng_mutex_wait_ms=" << m_diagnostics->maximumPartitionMutexWaitNanoseconds.load() / 1.e6
                << ", summed_rng_refill_generation_ms=" << m_diagnostics->summedRefillNanoseconds.load() / 1.e6
                << ", maximum_partition_rng_refill_generation_ms=" << m_diagnostics->maximumPartitionRefillNanoseconds.load() / 1.e6
                << ", tracing_wall_ms=" << m_diagnostics->wallNanoseconds() / 1.e6
                << ", distinct_worker_threads=" << m_diagnostics->distinctWorkerThreadCount()
                << ", maximum_active_partitions=" << m_diagnostics->maximumActivePartitions.load();
            m_diagnostics.reset();
        }
        execution->started = false;
        execution->owner = nullptr;
        return true;
    } catch (const std::exception& error) {
        if (errorMessage)
            *errorMessage = QString("Ray tracing worker failed: %1").arg(error.what());
    } catch (...) {
        if (errorMessage)
            *errorMessage = "Ray tracing worker failed with an unknown exception.";
    }
    execution->started = false;
    execution->owner = nullptr;
    return false;
}
