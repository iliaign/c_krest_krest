#include "strategies/Cache.hpp"
#include "strategies/lfu_hands.hpp"
#include "strategies/2q_hands.hpp"
#include "strategies/arc_hands.hpp"
#include "strategies/LIRC_hands.hpp"
#include <iostream>
#include <vector>
#include <random>
struct Stats{
    int hits = 0;
    int misses = 0;
    double hitRate()const{
        return 100.0 * hits/(hits + misses);
    }
};
Stats testCache(
    Cache<int, int>& cache,
    const std::vector<int>& requests)
{
    Stats stats;
    int value;

    for (int key : requests){
        if (cache.get(key, value)){
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
    for (int i = 0; i < count; ++i){
        requests.push_back(i % 10);
    }
    return requests;
}
int main()
{
    const int capacity = 50;
    const int requestCount = 10000;
    std::vector<int> requests =
        cyclicPattern(requestCount);
    LFUCache<int, int> lfu(capacity);
    TwoQCache<int, int> twoQ(capacity);
    ARCcache<int, int> arc(capacity);
    LircCache<int, int> lirc(capacity);

    Stats lfuStats = testCache(lfu, requests);
    Stats twoQStats = testCache(twoQ, requests);
    Stats arcStats = testCache(arc, requests);
    Stats lircStats = testCache(lirc, requests);
    std::cout << "LFU:  " << lfuStats.hitRate() << "%\n";
    std::cout << "2Q:   " << twoQStats.hitRate() << "%\n";
    std::cout << "ARC:  " << arcStats.hitRate() << "%\n";
    std::cout << "LIRS: " << lircStats.hitRate() << "%\n";
    return 0;
}