#include "strategies/Cache.hpp"
#include "strategies/random_cache.hpp"
#include "strategies/LFU.hpp"
#include "strategies/2Q.hpp"
#include "strategies/LIRC_hands.hpp"

#include <iostream>
#include <vector>
#include <random>
#include <string>
#include <iomanip>


struct Stats {
    std::size_t hits = 0;
    std::size_t misses = 0;

    double hitRate() const {
        const std::size_t total = hits + misses;

        if (total == 0)
            return 0.0;

        return static_cast<double>(hits) / total * 100.0;
    }
};


// ============================================================
// Общий тест
// ============================================================

template <class CacheType>
Stats testCache(CacheType& cache, const std::vector<int>& requests)
{
    Stats stats;
    int value;

    for (int key : requests) {
        if (cache.get(key, value)) {
            ++stats.hits;
        } else {
            ++stats.misses;

            // Значение условно загружаем из внешнего источника
            cache.put(key, key);
        }
    }

    return stats;
}


// ============================================================
// Паттерны запросов
// ============================================================

// Последовательный доступ:
// 0 1 2 3 ... 999
std::vector<int> sequentialPattern(std::size_t count)
{
    std::vector<int> requests;

    for (std::size_t i = 0; i < count; ++i)
        requests.push_back(static_cast<int>(i));

    return requests;
}


// Циклический доступ:
// 0 1 2 3 4 0 1 2 3 4 ...
std::vector<int> cyclicPattern(
    std::size_t count,
    int keyCount)
{
    std::vector<int> requests;

    for (std::size_t i = 0; i < count; ++i)
        requests.push_back(static_cast<int>(i % keyCount));

    return requests;
}


// Горячие и холодные данные.
// 80% запросов идут к 20 горячим ключам.
// 20% — к остальным.
std::vector<int> hotColdPattern(
    std::size_t count,
    std::uint32_t seed)
{
    std::vector<int> requests;

    std::mt19937 generator(seed);

    std::uniform_int_distribution<int> hot(0, 19);
    std::uniform_int_distribution<int> cold(20, 999);
    std::uniform_int_distribution<int> choice(1, 100);

    for (std::size_t i = 0; i < count; ++i) {
        if (choice(generator) <= 80)
            requests.push_back(hot(generator));
        else
            requests.push_back(cold(generator));
    }

    return requests;
}


// Полностью случайный доступ
std::vector<int> randomPattern(
    std::size_t count,
    std::uint32_t seed)
{
    std::vector<int> requests;

    std::mt19937 generator(seed);
    std::uniform_int_distribution<int> distribution(0, 999);

    for (std::size_t i = 0; i < count; ++i)
        requests.push_back(distribution(generator));

    return requests;
}


// ============================================================
// Результат одного паттерна
// ============================================================

struct PatternResult {
    Stats random;
    Stats lfu;
    Stats twoQ;
    Stats lirc;
};


// ============================================================
// Запуск всех кэшей на одном паттерне
// ============================================================

PatternResult runPattern(const std::vector<int>& requests)
{
    constexpr std::size_t capacity = 50;

    RandomCache<int, int> randomCache(capacity);
    LFUCache<int, int> lfuCache(capacity);
    TwoQCache<int, int> twoQCache(capacity);
    LircCache<int, int> lircCache(capacity);

    PatternResult result;

    result.random = testCache(randomCache, requests);
    result.lfu = testCache(lfuCache, requests);
    result.twoQ = testCache(twoQCache, requests);
    result.lirc = testCache(lircCache, requests);

    return result;
}


// ============================================================
// Вывод таблицы
// ============================================================

void printRow(
    const std::string& pattern,
    const PatternResult& result)
{
    std::cout << std::left
              << std::setw(15) << pattern
              << std::setw(15) << result.random.hitRate()
              << std::setw(15) << result.lfu.hitRate()
              << std::setw(15) << result.twoQ.hitRate()
              << std::setw(15) << result.lirc.hitRate()
              << '\n';
}


int main()
{
    constexpr std::size_t requestCount = 10000;

    // Создаём одинаковые последовательности запросов
    // для всех алгоритмов.
    const auto sequential =
        sequentialPattern(requestCount);

    const auto cyclic =
        cyclicPattern(requestCount, 10);

    const auto hotCold =
        hotColdPattern(requestCount, 42);

    const auto random =
        randomPattern(requestCount, 42);


    std::cout << std::fixed << std::setprecision(2);

    std::cout << '\n';
    std::cout << "Cache comparison\n";
    std::cout << "Capacity: 50\n";
    std::cout << "Requests: 10000\n\n";

    std::cout << std::left
              << std::setw(15) << "Pattern"
              << std::setw(15) << "Random"
              << std::setw(15) << "LFU"
              << std::setw(15) << "2Q"
              << std::setw(15) << "LIRC"
              << '\n';

    std::cout << std::string(60, '-') << '\n';


    printRow(
        "Sequential",
        runPattern(sequential)
    );

    printRow(
        "Cyclic",
        runPattern(cyclic)
    );

    printRow(
        "Hot/Cold",
        runPattern(hotCold)
    );

    printRow(
        "Random",
        runPattern(random)
    );

    return 0;
}