#pragma once

#include "kernel/random/Random.h"

#include <cstdint>
#include <random>

class TONATIUH_KERNEL StandardRandom final: public Random
{
public:
    explicit StandardRandom(std::uint64_t seed);

    // Returns an explicitly mapped IEEE-754 binary64 value in [0, 1).
    double RandomDouble() override;
    std::uint64_t nextUInt64();

    // SplitMix64 finalizer with fixed-width modulo-2^64 arithmetic.
    static std::uint64_t mix64(std::uint64_t value);
    // Domain-separated, bijective-for-fixed-master mapping of stable chunk indexes.
    static std::uint64_t deriveChunkSeed(std::uint64_t masterSeed, std::uint64_t chunkIndex);
    // Maps the high 53 engine bits to [0, 1), independently of STL distributions.
    static double uniformDouble(std::uint64_t engineValue);

private:
    std::mt19937_64 m_engine;
};
