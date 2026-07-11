#include "RandomParallel.h"

#include <QElapsedTimer>

RandomParallel::RandomParallel(Random* rand, QMutex* mutex, ulong size):
    Random(size),
    m_rand(rand),
    m_mutex(mutex),
    m_diagnosticsEnabled(qEnvironmentVariableIntValue("TONATIUHPP_TRACE_THREAD_DIAGNOSTICS") > 0)
{

}

void RandomParallel::FillArray(std::vector<double>& array)
{
    if (!m_diagnosticsEnabled) {
        m_mutex->lock();
        m_rand->FillArray(array);
        m_mutex->unlock();
        return;
    }

    QElapsedTimer timer;
    timer.start();
    m_mutex->lock();
    m_mutexWaitNanoseconds += timer.nsecsElapsed();
    timer.restart();
    m_rand->FillArray(array);
    m_refillNanoseconds += timer.nsecsElapsed();
    ++m_refillCount;
    m_mutex->unlock();
}
