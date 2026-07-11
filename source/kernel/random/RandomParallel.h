#pragma once

#include "kernel/TonatiuhKernel.h"

#include <QMutex>
#include <QtGlobal>

#include "Random.h"


class TONATIUH_KERNEL RandomParallel: public Random
{

public:
    RandomParallel(Random* rand, QMutex* mutex, ulong size = 100'000);

    void FillArray(std::vector<double>& array);

    quint64 refillCount() const { return m_refillCount; }
    qint64 mutexWaitNanoseconds() const { return m_mutexWaitNanoseconds; }
    qint64 refillNanoseconds() const { return m_refillNanoseconds; }

protected:
    Random* m_rand;
    QMutex* m_mutex;
    bool m_diagnosticsEnabled = false;
    quint64 m_refillCount = 0;
    qint64 m_mutexWaitNanoseconds = 0;
    qint64 m_refillNanoseconds = 0;
};
