#include "RandomMersenneTwister.h"

ulong RandomMersenneTwister::RandomUInt()
{
	return RandomInteger();
}

void RandomMersenneTwister::GenerateNewState()
{
    for (int i = 0; i < (N - M); ++i) m_state[i] = m_state[i + M] ^ Twiddle(m_state[i], m_state[i + 1]);
    for (int i = N - M; i < (N - 1); ++i) m_state[i] = m_state[i + M - N] ^ Twiddle(m_state[i], m_state[i + 1]);
    m_state[N - 1] = m_state[M - 1] ^ Twiddle(m_state[N - 1], m_state[0]);
    m_p = 0; // reset position
}

void RandomMersenneTwister::Seed(ulong seedValue)
{
    m_state[0] = seedValue & 0xFFFFFFFFUL; // for > 32 bit machines
    for (int i = 1; i < N; ++i)
    {
        m_state[i] = 1812433253UL * (m_state[i - 1] ^ (m_state[i - 1] >> 30)) + i;
        m_state[i] &= 0xFFFFFFFFUL; // for > 32 bit machines
    }
    m_p = N; // force GenerateNewState() to be called for next random number
}

void RandomMersenneTwister::Seed(const ulong* seedArray, int arraySize)
{
    Seed(19650218UL);
    int i = 1;
    int j = 0;
    for (int k = (N > arraySize ? N : arraySize); k; --k)
    {
        m_state[i] = (m_state[i] ^ ( (m_state[i - 1] ^ (m_state[i - 1] >> 30) ) * 1664525UL) )
                     + seedArray[j] + j; // non linear
        m_state[i] &= 0xFFFFFFFFUL; // for > 32 bit machines
        ++j; j %= arraySize;
        if ((++i) == N)
        {
            m_state[0] = m_state[N - 1];
            i = 1;
        }
    }
    for (int k = N - 1; k; --k)
    {
        m_state[i] = (m_state[i] ^ ( (m_state[i - 1] ^ (m_state[i - 1] >> 30) ) * 1566083941UL) ) - i;
        m_state[i] &= 0xFFFFFFFFUL; // for > 32 bit machines
        if ( (++i) == N)
        {
            m_state[0] = m_state[N - 1];
            i = 1;
        }
    }
    m_state[0] = 0x80000000UL; // MSB is 1; assuring non-zero initial array
    m_p = N; // force GenerateNewState() to be called for next random number
}
