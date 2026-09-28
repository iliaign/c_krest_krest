#include "strategies/Cache.hpp"
#include "strategies/lfu_hands.hpp"
#include "strategies/2q_hands.hpp"
#include "strategies/arc_hands.hpp"
#include "strategies/LIRC_hands.hpp"
#include "strategies/BeladyCache.hpp"

#include <iostream>
#include <vector>
#include <random>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <string>

// Статистика работы кэша
struct Stats {
    int hits = 0;    // Запрос найден в кэше
    int misses = 0;  // Запрос не найден в кэше

    // Процент попаданий в кэш
    double hitRate() const {
        return 100.0 * hits / (hits + misses);
    }
};

// Отношение hit rate алгоритма к hit rate Белади.
// Если у Белади 0%, то и у остальных 0%, поэтому отношение считаем равным 1.
double ratioToBelady(const Stats& stats, const Stats& belady)
{
    double beladyRate = belady.hitRate();

    if (beladyRate == 0.0) {
        return 1.0;
    }

    return stats.hitRate() / beladyRate;
}

// Проверяет работу кэша на заданных запросах
Stats testCache(
    Cache<int, int>& cache,
    const std::vector<int>& requests)
{
    Stats stats;
    int value;

    for (int key : requests) {

        // Пытаемся найти ключ в кэше
        if (cache.get(key, value)) {
            ++stats.hits;
        }
        else {
            ++stats.misses;

            // Если ключ не найден, добавляем его в кэш
            cache.put(key, key);
        }
    }

    return stats;
}

// ---------------------------------------------------------------
// Паттерны запросов. Все размеры считаются от capacity, чтобы
// тест оставался осмысленным при любом размере кэша.
// ---------------------------------------------------------------

// Цикл чуть больше кэша (capacity * 1.2): 0 1 2 ... L-1 0 1 2 ...
// Классический "убийца" LRU-подобных политик.
std::vector<int> loopPattern(int count, int capacity)
{
    std::vector<int> requests;
    int loopSize = capacity + capacity / 5 + 1;

    for (int i = 0; i < count; ++i) {
        requests.push_back(i % loopSize);
    }

    return requests;
}

// Горячий набор (capacity / 2 ключей) + периодические проходы (scan)
// по уникальным ключам длиной 2 * capacity.
// Проверяет устойчивость алгоритма к вытеснению горячих данных сканом.
std::vector<int> hotScanPattern(int count, int capacity, unsigned seed)
{
    std::vector<int> requests;
    std::mt19937 generator(seed);

    int hotSize = std::max(1, capacity / 2);
    int hotLength = 500;               // запросов к горячему набору
    int scanLength = 2 * capacity;     // уникальных ключей в скане
    int period = hotLength + scanLength;

    std::uniform_int_distribution<int> hot(0, hotSize - 1);
    int scanKey = 1000000;             // ключи скана нигде не повторяются

    for (int i = 0; i < count; ++i) {
        if (i % period < hotLength) {
            requests.push_back(hot(generator));
        }
        else {
            requests.push_back(scanKey++);
        }
    }

    return requests;
}

// Распределение Ципфа (weight ~ 1 / rank) по 10 * capacity ключей:
// реалистичная нагрузка, где несколько ключей очень популярны.
std::vector<int> zipfPattern(int count, int capacity, unsigned seed)
{
    std::vector<int> requests;
    std::mt19937 generator(seed);

    int keys = 10 * capacity;
    std::vector<double> weights(keys);

    for (int k = 0; k < keys; ++k) {
        weights[k] = 1.0 / (k + 1);
    }

    std::discrete_distribution<int> distribution(weights.begin(), weights.end());

    for (int i = 0; i < count; ++i) {
        requests.push_back(distribution(generator));
    }

    return requests;
}

// Смена рабочего набора: каждые 2000 запросов набор из 0.8 * capacity
// ключей заменяется на совершенно новый. Проверяет скорость адаптации.
std::vector<int> workingSetShiftPattern(int count, int capacity, unsigned seed)
{
    std::vector<int> requests;
    std::mt19937 generator(seed);

    int setSize = std::max(1, capacity * 8 / 10);
    int phaseLength = 2000;

    std::uniform_int_distribution<int> distribution(0, setSize - 1);

    for (int i = 0; i < count; ++i) {
        int phase = i / phaseLength;
        requests.push_back(phase * 100000 + distribution(generator));
    }

    return requests;
}

// Равномерный случайный доступ к 2 * capacity ключей.
// Базовая линия: локальности нет, ожидаем ~50% у всех.
std::vector<int> uniformPattern(int count, int capacity, unsigned seed)
{
    std::vector<int> requests;
    std::mt19937 generator(seed);
    std::uniform_int_distribution<int> distribution(0, 2 * capacity - 1);

    for (int i = 0; i < count; ++i) {
        requests.push_back(distribution(generator));
    }

    return requests;
}

// Записывает результаты теста в CSV-файл
void writeResult(
    std::ofstream& file,
    const std::string& pattern,
    int capacity,
    unsigned seed,
    const std::string& cacheName,
    const Stats& stats,
    const Stats& beladyStats)
{
    file << pattern << ","
         << capacity << ","
         << seed << ","
         << cacheName << ","
         << stats.hits << ","
         << stats.misses << ","
         << stats.hitRate() << ","
         << ratioToBelady(stats, beladyStats) << "\n";
}

// Предупреждает, если алгоритм оказался лучше Белади (это невозможно,
// значит, где-то баг в реализации или в тесте)
void checkAgainstBelady(
    const std::string& patternName,
    const std::string& cacheName,
    const Stats& stats,
    const Stats& beladyStats)
{
    if (stats.hitRate() > beladyStats.hitRate() + 1e-9) {
        std::cout << "!!! ОШИБКА: " << cacheName << " лучше Belady на "
                  << patternName << " (" << stats.hitRate() << "% > "
                  << beladyStats.hitRate() << "%)\n";
    }
}

// Запускает один паттерн для всех алгоритмов кэширования
void runTest(
    const std::string& patternName,
    const std::vector<int>& requests,
    int capacity,
    unsigned seed,
    bool printToConsole,
    std::ofstream& file)
{
    // Создаём кэши одинакового размера
    LFU_cache<int, int> lfu(capacity);
    TwoQ<int, int> twoQ(capacity);
    ARCcache<int, int> arc(capacity);
    LircCache<int, int> lirc(capacity);

    BeladyCache<int, int> belady(capacity);
    belady.setRequests(requests); //предрасчет

    // Все алгоритмы получают одинаковые запросы
    Stats lfuStats = testCache(lfu, requests);
    Stats twoQStats = testCache(twoQ, requests);
    Stats arcStats = testCache(arc, requests);
    Stats lircStats = testCache(lirc, requests);

    Stats beladyStats = testCache(belady, requests);

    // Проверка корректности: никто не должен обгонять Белади
    checkAgainstBelady(patternName, "LFU", lfuStats, beladyStats);
    checkAgainstBelady(patternName, "2Q", twoQStats, beladyStats);
    checkAgainstBelady(patternName, "ARC", arcStats, beladyStats);
    checkAgainstBelady(patternName, "LIRS", lircStats, beladyStats);

    // Выводим результаты в консоль (только для первого seed, чтобы не засорять вывод)
    if (printToConsole) {
        std::cout << "\n" << patternName << " (capacity = " << capacity << ")\n";
        std::cout << "LFU:    " << lfuStats.hitRate() << "%  (к Belady: "
                  << ratioToBelady(lfuStats, beladyStats) << ")\n";
        std::cout << "2Q:     " << twoQStats.hitRate() << "%  (к Belady: "
                  << ratioToBelady(twoQStats, beladyStats) << ")\n";
        std::cout << "ARC:    " << arcStats.hitRate() << "%  (к Belady: "
                  << ratioToBelady(arcStats, beladyStats) << ")\n";
        std::cout << "LIRS:   " << lircStats.hitRate() << "%  (к Belady: "
                  << ratioToBelady(lircStats, beladyStats) << ")\n";
        std::cout << "Belady: " << beladyStats.hitRate() << "%  (к Belady: 1)\n";
    }

    // Сохраняем результаты в CSV (все seed)
    writeResult(file, patternName, capacity, seed, "LFU",    lfuStats,    beladyStats);
    writeResult(file, patternName, capacity, seed, "2Q",     twoQStats,   beladyStats);
    writeResult(file, patternName, capacity, seed, "ARC",    arcStats,    beladyStats);
    writeResult(file, patternName, capacity, seed, "LIRS",   lircStats,   beladyStats);
    writeResult(file, patternName, capacity, seed, "Belady", beladyStats, beladyStats);
}

// Измеряет общее время обработки запросов
long long testTime(
    Cache<int, int>& cache,
    const std::vector<int>& requests)
{
    int value;

    // Запоминаем время начала теста
    std::chrono::high_resolution_clock::time_point start =
        std::chrono::high_resolution_clock::now();

    // Выполняем все запросы
    for (int key : requests) {
        if (!cache.get(key, value))
            cache.put(key, key);
    }

    // Запоминаем время окончания теста
    std::chrono::high_resolution_clock::time_point end =
        std::chrono::high_resolution_clock::now();

    // Возвращаем время выполнения в микросекундах
    return std::chrono::duration_cast<
        std::chrono::microseconds>(end - start).count();
}

// Медиана набора измерений
long long median(std::vector<long long> values)
{
    std::sort(values.begin(), values.end());
    return values[values.size() / 2];
}

// Запускает замер времени для всех алгоритмов.
// Каждый замер повторяется несколько раз на свежих кэшах, берётся медиана.
void runTimeTests(
    const std::vector<int>& requests,
    int capacity,
    std::ofstream& file)
{
    const int repeats = 5;

    std::vector<long long> lfuTimes, twoQTimes, arcTimes, lircTimes, beladyTimes;

    for (int r = 0; r < repeats; ++r) {
        // Создаём кэши одинакового размера
        LFU_cache<int, int> lfu(capacity);
        TwoQ<int, int> twoQ(capacity);
        ARCcache<int, int> arc(capacity);
        LircCache<int, int> lirc(capacity);

        BeladyCache<int, int> belady(capacity);
        belady.setRequests(requests); // предрасчёт не входит в замер

        lfuTimes.push_back(testTime(lfu, requests));
        twoQTimes.push_back(testTime(twoQ, requests));
        arcTimes.push_back(testTime(arc, requests));
        lircTimes.push_back(testTime(lirc, requests));
        beladyTimes.push_back(testTime(belady, requests));
    }

    // Записываем медианное время каждого алгоритма в CSV
    file << requests.size() << ","
         << capacity << ","
         << median(lfuTimes) << ","
         << median(twoQTimes) << ","
         << median(arcTimes) << ","
         << median(lircTimes) << ","
         << median(beladyTimes) << "\n";
}


int main()
{
    const int requestCount = 10000;
    const std::vector<int> capacities = {10, 50, 100};
    const std::vector<unsigned> seeds = {1, 2, 3};

    // Открываем файл для записи результатов
    std::ofstream file("results.csv");

    file << "pattern,capacity,seed,cache,hits,misses,hit_rate,belady_ratio\n";

    for (int capacity : capacities) {
        for (unsigned seed : seeds) {
            // Console-вывод только для первого seed
            bool print = (seed == seeds.front());

            // Цикл больше кэша (детерминированный, seed не влияет)
            runTest("Loop", loopPattern(requestCount, capacity),
                    capacity, seed, print, file);

            // Горячий набор + сканирование
            runTest("HotScan", hotScanPattern(requestCount, capacity, seed),
                    capacity, seed, print, file);

            // Распределение Ципфа
            runTest("Zipf", zipfPattern(requestCount, capacity, seed),
                    capacity, seed, print, file);

            // Смена рабочего набора
            runTest("WorkingSetShift", workingSetShiftPattern(requestCount, capacity, seed),
                    capacity, seed, print, file);

            // Равномерный случайный доступ (базовая линия)
            runTest("Uniform", uniformPattern(requestCount, capacity, seed),
                    capacity, seed, print, file);
        }
    }

    // Замер времени: ёмкость побольше, чтобы проявились различия структур данных
    const int timeCapacity = 1000;

    std::ofstream timeFile("time_results.csv");

    timeFile << "requests,capacity,LFU,2Q,ARC,LIRS,Belady\n";

    for (int n : {1000, 10000, 100000}) {
        runTimeTests(
            zipfPattern(n, timeCapacity, 42),
            timeCapacity,
            timeFile
        );
    }

    timeFile.close();
    file.close();

    return 0;
}