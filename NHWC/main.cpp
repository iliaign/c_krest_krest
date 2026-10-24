#include "strategies/lfu_hands.hpp"
#include "strategies/2q_hands.hpp"
#include "strategies/arc_hands.hpp"
#include "strategies/LIRC_hands.hpp"
#include "strategies/BeladyCache.hpp"
#include "strategies/TwoLevelCache.hpp"

#include "CacheTests.hpp"
#include "CsvWriter.hpp"
#include "Patterns.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>

using cache_tests::Stats;
using cache_tests::testCache;
using cache_tests::countUnique;

using CacheFactory = std::shared_ptr<Cache<int, int>> (*)(int);

struct Algo {
    std::string name;
    CacheFactory make;
};

std::shared_ptr<Cache<int, int>> makeLFU(int c) {
    return std::make_shared<LFU_cache<int, int>>(c);
}
std::shared_ptr<Cache<int, int>> make2Q(int c) {
    return std::make_shared<TwoQ<int, int>>(c);
}
std::shared_ptr<Cache<int, int>> makeARC(int c) {
    return std::make_shared<ARCcache<int, int>>(c);
}
std::shared_ptr<Cache<int, int>> makeLIRS(int c) {
    return std::make_shared<LircCache<int, int>>(c);
}

std::vector<Algo> allAlgos() {
    return {{"LFU", makeLFU}, {"2Q", make2Q},
            {"ARC", makeARC}, {"LIRS", makeLIRS}};
}

int l1Capacity(int total, double share) {
    return std::max(2, std::min(static_cast<int>(total * share + 0.5),
                                total - 2));
}

template<class C>
long long medianTime(C& cache, const std::vector<int>& requests) {
    return cache_tests::measureMicroseconds(cache, requests);
}

void runSingle(const Pattern& p, const std::vector<int>& requests,
               int cap, unsigned seed, std::ofstream& csv) {
    BeladyCache<int, int> optimal(cap);
    optimal.setRequests(requests);
    const Stats belady = testCache(optimal, requests);
    const auto unique = countUnique(requests);

    for (const auto& algo : allAlgos()) {
        auto cache = algo.make(cap);
        const Stats stats = testCache(*cache, requests);
        cache_csv::writeResult(csv, p.category, p.name, cap, seed,
            requests.size(), unique, algo.name, stats, belady);
    }

    cache_csv::writeResult(csv, p.category, p.name, cap, seed,
        requests.size(), unique, "Belady", belady, belady);
}

void runTwoLevel(const Pattern& p, const std::vector<int>& requests,
                 int total, unsigned seed, std::ofstream& csv) {
    BeladyCache<int, int> optimal(total);
    optimal.setRequests(requests);
    const Stats belady = testCache(optimal, requests);
    const auto unique = countUnique(requests);
    const auto n = requests.size();

    cache_csv::writeTwoLevelResult(csv, p.category, p.name, total, seed,
        n, unique, "belady", "-", "Belady", "-", total, 0,
        belady, belady.hits, 0, belady);

    const double shares[] = {0.2, 0.5};
    const TwoLevelMode modes[] = {
        TwoLevelMode::FillBoth, TwoLevelMode::FillL2Only
    };
    const auto algos = allAlgos();

    for (double share : shares) {
        const int c1 = l1Capacity(total, share);
        const int c2 = total - c1;

        for (auto mode : modes)
            for (const auto& a1 : algos)
                for (const auto& a2 : algos) {
                    TwoLevelCache<int, int> cache(a1.make(c1), a2.make(c2), mode);
                    const Stats stats = testCache(cache, requests);

                    cache_csv::writeTwoLevelResult(csv, p.category, p.name,
                        total, seed, n, unique, "two_level", toString(mode),
                        a1.name, a2.name, c1, c2, stats,
                        static_cast<long long>(cache.l1Hits()),
                        static_cast<long long>(cache.l2Hits()), belady);
                }
    }
}

template<class Factory>
std::vector<long long> measureFive(Factory make, const std::vector<int>& requests) {
    std::vector<long long> times;
    for (int i = 0; i < 5; ++i) {
        auto cache = make();
        times.push_back(medianTime(*cache, requests));
    }
    return {cache_tests::median(times)};
}

void runTiming(const Pattern& p, const std::vector<int>& requests,
               int cap, std::ofstream& csv) {
    std::vector<long long> times;
    for (const auto& algo : allAlgos()) {
        std::vector<long long> runs;
        for (int i = 0; i < 5; ++i) {
            auto cache = algo.make(cap);
            runs.push_back(medianTime(*cache, requests));
        }
        times.push_back(cache_tests::median(runs));
    }
    std::vector<long long> runs;
    for (int i = 0; i < 5; ++i) {
        BeladyCache<int, int> cache(cap);
        cache.setRequests(requests);
        runs.push_back(medianTime(cache, requests));
    }
    times.push_back(cache_tests::median(runs));
    cache_csv::writeTimeResult(csv, p.name, requests.size(), cap, times);
}

int main() {
    constexpr int requestCount = 10000;
    const std::vector<int> capacities = {10, 50, 100, 500};
    const std::vector<unsigned> seeds = {1, 2, 3, 4, 5};
    const std::vector<unsigned> twoLevelSeeds = {1, 2, 3};
    const auto patterns = allPatterns();

    std::ofstream single("results.csv");
    std::ofstream twoLevel("two_level_results.csv");
    std::ofstream timing("time_results.csv");
    if (!single || !twoLevel || !timing) {
        std::cerr << "Cannot open output CSV files\n";
        return 1;
    }

    cache_csv::writeResultsHeader(single);
    cache_csv::writeTwoLevelHeader(twoLevel);
    cache_csv::writeTimeHeader(timing, {"LFU", "2Q", "ARC", "LIRS", "Belady"});

    for (int cap : capacities)
        for (const auto& p : patterns)
            for (unsigned seed : seeds) {
                if (p.deterministic && seed != seeds.front()) continue;
                const auto requests = p.generate(requestCount, cap, seed);
                runSingle(p, requests, cap, seed, single);
            }

    for (int cap : capacities)
        for (const auto& p : patterns)
            for (unsigned seed : twoLevelSeeds) {
                if (p.deterministic && seed != twoLevelSeeds.front()) continue;
                const auto requests = p.generate(requestCount, cap, seed);
                runTwoLevel(p, requests, cap, seed, twoLevel);
            }

    constexpr int timeCapacity = 1000;
    for (const auto& p : patterns) {
        if (p.name != "Zipf" && p.name != "Loop" && p.name != "HotScan") continue;
        for (int n : {1000, 10000, 100000}) {
            const auto requests = p.generate(n, timeCapacity, 42);
            runTiming(p, requests, timeCapacity, timing);
        }
    }

    std::cout << "Saved: results.csv, two_level_results.csv, time_results.csv\n";
}
