#include "StandardRandom.h"

namespace
{
constexpr std::uint64_t kChunkDomain = 0xD1B54A32D192ED03ULL;
constexpr double kInverseTwoTo53 = 1.0 / 9007199254740992.0;
}

StandardRandom::StandardRandom(std::uint64_t seed):
    m_engine(seed)
{
}

double StandardRandom::RandomDouble()
{
    return uniformDouble(nextUInt64());
}

std::uint64_t StandardRandom::nextUInt64()
{
    return m_engine();
}

std::uint64_t StandardRandom::mix64(std::uint64_t value)
{
    value += 0x9E3779B97F4A7C15ULL;
    value = (value ^ (value >> 30)) * 0xBF58476D1CE4E5B9ULL;
    value = (value ^ (value >> 27)) * 0x94D049BB133111EBULL;
    return value ^ (value >> 31);
}

std::uint64_t StandardRandom::deriveChunkSeed(std::uint64_t masterSeed, std::uint64_t chunkIndex)
{
    return mix64(masterSeed ^ mix64(chunkIndex ^ kChunkDomain));
}

double StandardRandom::uniformDouble(std::uint64_t engineValue)
{
    return static_cast<double>(engineValue >> 11) * kInverseTwoTo53;
}
