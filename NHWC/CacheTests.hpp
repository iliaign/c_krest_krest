#pragma once

#include "strategies/Cache.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

namespace cache_tests {

struct Stats {
    std::size_t hits = 0;
    std::size_t misses = 0;

    std::size_t total() const { return hits + misses; }

    double hitRate() const {
        const auto n = total();
        return n == 0 ? 0.0 : 100.0 * static_cast<double>(hits) /
                                  static_cast<double>(n);
    }
};

inline double beladyRatio(const Stats& tested, const Stats& belady) {
    return belady.hitRate() == 0.0
        ? 1.0
        : tested.hitRate() / belady.hitRate();
}

inline std::size_t countUnique(const std::vector<int>& requests) {
    return std::unordered_set<int>(requests.begin(), requests.end()).size();
}

// Simulates a workload. On a miss, inserts key -> key, matching the original main.
template<class CacheType>
Stats testCache(CacheType& cache, const std::vector<int>& requests) {
    Stats stats;
    int value{};

    for (const int key : requests) {
        if (cache.get(key, value)) {
            ++stats.hits;
        } else {
            ++stats.misses;
            cache.put(key, key);
        }
    }
    return stats;
}

// Runs get/put correctness checks for a fresh cache instance.
// The cache must be empty and have room for at least two entries.
template<class CacheType>
void testBasicPutGet(CacheType& cache) {
    int value = 0;

    if (cache.get(101, value))
        throw std::runtime_error("Empty cache unexpectedly contains key 101");

    cache.put(101, 1001);
    if (!cache.get(101, value) || value != 1001)
        throw std::runtime_error("put/get failed for key 101");

    cache.put(202, 2002);
    if (!cache.get(202, value) || value != 2002)
        throw std::runtime_error("put/get failed for key 202");
}

// Checks that clear() removes entries. Use a fresh cache instance.
template<class CacheType>
void testClear(CacheType& cache) {
    int value = 0;
    cache.put(303, 3003);
    cache.clear();

    if (cache.get(303, value))
        throw std::runtime_error("clear() did not remove key 303");
}

// A small deterministic behavioral test that works with the common Cache interface.
// The capacity should be at least 2. It does not assume a particular eviction policy.
template<class CacheType>
void testWorkloadAccounting(CacheType& cache) {
    const std::vector<int> requests{1, 2, 1, 3, 1, 2};
    const Stats stats = testCache(cache, requests);

    if (stats.total() != requests.size())
        throw std::runtime_error("Hit/miss accounting does not match request count");

    if (stats.hits + stats.misses != 6)
        throw std::runtime_error("Incorrect hit/miss accounting");
}

inline void checkAgainstBelady(const std::string& pattern,
                               const std::string& cacheName,
                               const Stats& tested,
                               const Stats& belady,
                               std::ostream& out = std::cout) {
    // Belady is optimal only when configured with the same request sequence
    // and the same effective capacity as the tested cache.
    if (tested.hitRate() > belady.hitRate() + 1e-9) {
        out << "Warning: " << cacheName << " beats Belady on " << pattern
            << " (" << tested.hitRate() << "% > "
            << belady.hitRate() << "%)\n";
    }
}

template<class CacheType>
long long measureMicroseconds(CacheType& cache,
                              const std::vector<int>& requests) {
    int value{};
    const auto start = std::chrono::steady_clock::now();

    for (const int key : requests) {
        if (!cache.get(key, value))
            cache.put(key, key);
    }

    const auto finish = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(
        finish - start).count();
}

inline long long median(std::vector<long long> values) {
    if (values.empty())
        throw std::invalid_argument("median() requires at least one value");

    std::sort(values.begin(), values.end());
    return values[values.size() / 2];
}

} // namespace cache_tests
