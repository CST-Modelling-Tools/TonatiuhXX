#include <gtest/gtest.h>

#include "source/kernel/random/StandardRandom.h"

#include <array>
#include <cstdint>
#include <limits>
#include <unordered_set>
#include <vector>

TEST(StandardRandomTest, MatchesMt1993764ReferenceSequence)
{
    StandardRandom random(5489ULL);
    constexpr std::array<std::uint64_t, 5> expected = {
        14514284786278117030ULL,
        4620546740167642908ULL,
        13109570281517897720ULL,
        17462938647148434322ULL,
        355488278567739596ULL
    };

    for (const std::uint64_t value : expected)
        EXPECT_EQ(random.nextUInt64(), value);
}

TEST(StandardRandomTest, ConvertsHigh53BitsToHalfOpenUnitInterval)
{
    EXPECT_DOUBLE_EQ(StandardRandom::uniformDouble(0), 0.0);
    EXPECT_DOUBLE_EQ(StandardRandom::uniformDouble(0x8000000000000000ULL), 0.5);
    EXPECT_DOUBLE_EQ(StandardRandom::uniformDouble(std::numeric_limits<std::uint64_t>::max()),
                     0x1.fffffffffffffp-1);

    StandardRandom random(1);
    for (int index = 0; index < 10000; ++index) {
        const double value = random.RandomDouble();
        EXPECT_GE(value, 0.0);
        EXPECT_LT(value, 1.0);
    }
}

TEST(StandardRandomTest, DerivesStableChunkSeeds)
{
    EXPECT_EQ(StandardRandom::deriveChunkSeed(0, 0), 0x98BC9B3A9F64DA94ULL);
    EXPECT_EQ(StandardRandom::deriveChunkSeed(0, 1), 0x92E5B929D9A8E421ULL);
    EXPECT_EQ(StandardRandom::deriveChunkSeed(1, 0), 0x1EAEEEE5F7689509ULL);
    EXPECT_EQ(StandardRandom::deriveChunkSeed(123456789, 42), 0x55FB077BF1619575ULL);
}

TEST(StandardRandomTest, ProducesUniqueSeedsForRepresentativeChunkRange)
{
    std::unordered_set<std::uint64_t> seeds;
    for (std::uint64_t chunkIndex = 0; chunkIndex < 4096; ++chunkIndex)
        EXPECT_TRUE(seeds.insert(StandardRandom::deriveChunkSeed(123456789, chunkIndex)).second);
}

TEST(StandardRandomTest, ChunkStreamsAreRepeatableAndSchedulingOrderIndependent)
{
    auto sampleChunk = [](std::uint64_t chunkIndex) {
        StandardRandom random(StandardRandom::deriveChunkSeed(987654321, chunkIndex));
        std::array<std::uint64_t, 4> values{};
        for (std::uint64_t& value : values)
            value = random.nextUInt64();
        return values;
    };

    const std::array<std::uint64_t, 4> forwardOrder = {0, 1, 2, 3};
    const std::array<std::uint64_t, 4> reverseOrder = {3, 2, 1, 0};
    std::array<std::array<std::uint64_t, 4>, 4> forward{};
    std::array<std::array<std::uint64_t, 4>, 4> reverse{};

    for (const std::uint64_t chunkIndex : forwardOrder)
        forward[chunkIndex] = sampleChunk(chunkIndex);
    for (const std::uint64_t chunkIndex : reverseOrder)
        reverse[chunkIndex] = sampleChunk(chunkIndex);

    EXPECT_EQ(forward, reverse);
}

TEST(StandardRandomTest, ChunkStreamsDoNotDependOnWorkerAssignment)
{
    auto sampleForAssignment = [](const std::array<std::uint64_t, 4>& workerChunks) {
        std::array<std::uint64_t, 4> firstValues{};
        for (const std::uint64_t chunkIndex : workerChunks) {
            StandardRandom random(StandardRandom::deriveChunkSeed(987654321, chunkIndex));
            firstValues[chunkIndex] = random.nextUInt64();
        }
        return firstValues;
    };

    EXPECT_EQ(sampleForAssignment({0, 2, 1, 3}), sampleForAssignment({3, 1, 2, 0}));
}

TEST(StandardRandomTest, RepeatedFixedSeedStreamsAgreeInOneProcess)
{
    StandardRandom first(StandardRandom::deriveChunkSeed(123456789, 17));
    StandardRandom second(StandardRandom::deriveChunkSeed(123456789, 17));

    for (int index = 0; index < 32; ++index)
        EXPECT_EQ(first.nextUInt64(), second.nextUInt64());
}
