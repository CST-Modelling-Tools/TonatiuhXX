#pragma once

#include "kernel/TonatiuhKernel.h"
#include "kernel/raytracing/TAbstract.h"
#include <qglobal.h>

//!  RandomDeviate is the base class for random generators.
/*!
   A random generator class can be written based on this class.
 */
class TONATIUH_KERNEL RandomDeviate
{
public:
    explicit RandomDeviate(const ulong arraySize = 100'000);
    virtual ~RandomDeviate();

    double RandomDouble();
    virtual void FillArray(double* array, const ulong arraySize) = 0;

    ulong NumbersGenerated() const {return m_total;}
    ulong NumbersProvided() const { return m_total - m_size + m_index;}

    NAME_ICON_FUNCTIONS("X", ":/RandomX.png")

private:
    double* m_numbers;
    const ulong m_size;
    ulong m_index;
    ulong m_total;
};

inline RandomDeviate::RandomDeviate(const ulong arraySize):
    m_size(arraySize), m_index(arraySize), m_total(0)
{
    m_numbers = new double[arraySize];
}

inline RandomDeviate::~RandomDeviate()
{
    if (m_numbers) delete[] m_numbers;
}

inline double RandomDeviate::RandomDouble()
{
    if (m_index >= m_size) {
        FillArray(m_numbers, m_size);
        m_index = 0;
        m_total += m_size;
    }
    return m_numbers[m_index++];
}
