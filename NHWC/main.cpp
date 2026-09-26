#include "strategies/Cache.hpp"
#include "strategies/lfu_hands.hpp"
#include "strategies/2q_hands.hpp"
#include "strategies/arc_hands.hpp"
#include "strategies/LIRC_hands.hpp"
#include <iostream>
#include <vector>
#include <random>
#include <fstream>
struct Stats {
    int hits = 0;
    int misses = 0;

    double hitRate() const {
        return 100.0 * hits / (hits + misses);
    }
};
Stats testCache(
    Cache<int, int>& cache,
    const std::vector<int>& requests)
{
    Stats stats;
    int value;
    for (int key : requests) {
        if (cache.get(key, value)) {
            ++stats.hits;
        }
        else {
            ++stats.misses;
            cache.put(key, key);
        }
    }
    return stats;
}
std::vector<int> cyclicPattern(int count)
{
    std::vector<int> requests;

    for (int i = 0; i < count; ++i) {
        requests.push_back(i % 10);
    }

    return requests;
}

std::vector<int> sequentialPattern(int count)
{
    std::vector<int> requests;
    for (int i = 0; i < count; ++i) {
        requests.push_back(i);
    }
    return requests;
}
std::vector<int> hotPattern(int count)
{
    std::vector<int> requests;
    for (int i = 0; i < count; ++i) {
        if (i % 10 < 7) {
            requests.push_back(i % 5);
        }
        else {
            requests.push_back(100 + i % 50);
        }
    }
    return requests;
}
std::vector<int> randomPattern(int count)
{
    std::vector<int> requests;
    std::mt19937 generator(42);
    std::uniform_int_distribution<int> distribution(0, 99);
    for (int i = 0; i < count; ++i) {
        requests.push_back(distribution(generator));
    }
    return requests;
}
void writeResult(std::ofstream& file, const std::string& pattern, const std::string& cacheName,
    const Stats& stats){

    file << pattern << ","
         << cacheName << ","
         << stats.hits << ","
         << stats.misses << ","
         << stats.hitRate() << "\n";
}
void runTest(const std::string& patternName, const std::vector<int>& requests, int capacity,
    std::ofstream& file){

    LFU_cache<int, int> lfu(capacity);
    TwoQ<int, int> twoQ(capacity);
    ARCcache<int, int> arc(capacity);
    LircCache<int, int> lirc(capacity);
    Stats lfuStats = testCache(lfu, requests);
    Stats twoQStats = testCache(twoQ, requests);
    Stats arcStats = testCache(arc, requests);
    Stats lircStats = testCache(lirc, requests);
    std::cout << "\n" << patternName << "\n";
    std::cout << "LFU:  " << lfuStats.hitRate() << "%\n";
    std::cout << "2Q:   " << twoQStats.hitRate() << "%\n";
    std::cout << "ARC:  " << arcStats.hitRate() << "%\n";
    std::cout << "LIRS: " << lircStats.hitRate() << "%\n";

    writeResult(file, patternName, "LFU", lfuStats);
    writeResult(file, patternName, "2Q", twoQStats);
    writeResult(file, patternName, "ARC", arcStats);
    writeResult(file, patternName, "LIRS", lircStats);
}
int main(){
    const int capacity = 50;
    const int requestCount = 10000;
    std::ofstream file("results.csv");
    file << "pattern,cache,hits,misses,hit_rate\n";
    runTest(
        "Cyclic",
        cyclicPattern(requestCount),
        capacity,
        file
    );
    runTest(
        "Sequential",
        sequentialPattern(requestCount),
        capacity,
        file
    );
    runTest(
        "Hot",
        hotPattern(requestCount),
        capacity,
        file
    );
    runTest(
        "Random",
        randomPattern(requestCount),
        capacity,
        file
    );
    file.close();
    return 0;
}