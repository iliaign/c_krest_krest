#include "strategies/Cache.hpp"
#include "strategies/lfu_hands.hpp"
#include "strategies/2q_hands.hpp"
#include "strategies/arc_hands.hpp"
#include "strategies/LIRC_hands.hpp"

#include <iostream>
#include <vector>
#include <random>
#include <fstream>
#include <chrono>

// Статистика работы кэша
struct Stats {
    int hits = 0;    // Запрос найден в кэше
    int misses = 0;  // Запрос не найден в кэше

    // Процент попаданий в кэш
    double hitRate() const {
        return 100.0 * hits / (hits + misses);
    }
};

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

// Циклическая последовательность: 0 1 2 ... 9 0 1 2 ...
std::vector<int> cyclicPattern(int count)
{
    std::vector<int> requests;

    for (int i = 0; i < count; ++i) {
        requests.push_back(i % 10);
    }

    return requests;
}

// Последовательная последовательность: 0 1 2 3 4 ...
std::vector<int> sequentialPattern(int count)
{
    std::vector<int> requests;

    for (int i = 0; i < count; ++i) {
        requests.push_back(i);
    }

    return requests;
}

// Hot-паттерн: часто используются ключи 0-4
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

// Случайная последовательность ключей от 0 до 99
std::vector<int> randomPattern(int count)
{
    std::vector<int> requests;

    // Фиксированный seed делает результат воспроизводимым
    std::mt19937 generator(42);
    std::uniform_int_distribution<int> distribution(0, 99);

    for (int i = 0; i < count; ++i) {
        requests.push_back(distribution(generator));
    }

    return requests;
}

// Записывает результаты теста в CSV-файл
void writeResult(
    std::ofstream& file,
    const std::string& pattern,
    const std::string& cacheName,
    const Stats& stats)
{
    file << pattern << ","
         << cacheName << ","
         << stats.hits << ","
         << stats.misses << ","
         << stats.hitRate() << "\n";
}

// Запускает один паттерн для всех алгоритмов кэширования
void runTest(
    const std::string& patternName,
    const std::vector<int>& requests,
    int capacity,
    std::ofstream& file)
{
    // Создаём кэши одинакового размера
    LFU_cache<int, int> lfu(capacity);
    TwoQ<int, int> twoQ(capacity);
    ARCcache<int, int> arc(capacity);
    LircCache<int, int> lirc(capacity);

    // Все алгоритмы получают одинаковые запросы
    Stats lfuStats = testCache(lfu, requests);
    Stats twoQStats = testCache(twoQ, requests);
    Stats arcStats = testCache(arc, requests);
    Stats lircStats = testCache(lirc, requests);

    // Выводим результаты в консоль
    std::cout << "\n" << patternName << "\n";
    std::cout << "LFU:  " << lfuStats.hitRate() << "%\n";
    std::cout << "2Q:   " << twoQStats.hitRate() << "%\n";
    std::cout << "ARC:  " << arcStats.hitRate() << "%\n";
    std::cout << "LIRS: " << lircStats.hitRate() << "%\n";

    // Сохраняем результаты в CSV
    writeResult(file, patternName, "LFU", lfuStats);
    writeResult(file, patternName, "2Q", twoQStats);
    writeResult(file, patternName, "ARC", arcStats);
    writeResult(file, patternName, "LIRS", lircStats);
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


// Запускает замер времени для всех алгоритмов
void runTimeTests(
    const std::vector<int>& requests,
    int capacity,
    std::ofstream& file)
{
    // Создаём кэши одинакового размера
    LFU_cache<int, int> lfu(capacity);
    TwoQ<int, int> twoQ(capacity);
    ARCcache<int, int> arc(capacity);
    LircCache<int, int> lirc(capacity);

    // Записываем время работы каждого алгоритма в CSV
    file << requests.size() << ","
         << testTime(lfu, requests) << ","
         << testTime(twoQ, requests) << ","
         << testTime(arc, requests) << ","
         << testTime(lirc, requests) << "\n";
}


int main()
{
    // Размер кэша и количество запросов
    const int capacity = 50;
    const int requestCount = 10000;

    // Открываем файл для записи результатов
    std::ofstream file("results.csv");

    file << "pattern,cache,hits,misses,hit_rate\n";

    // Тестируем циклические запросы
    runTest(
        "Cyclic",
        cyclicPattern(requestCount),
        capacity,
        file
    );

    // Тестируем последовательные запросы
    runTest(
        "Sequential",
        sequentialPattern(requestCount),
        capacity,
        file
    );

    // Тестируем запросы с часто используемыми элементами
    runTest(
        "Hot",
        hotPattern(requestCount),
        capacity,
        file
    );

    // Тестируем случайные запросы
    runTest(
        "Random",
        randomPattern(requestCount),
        capacity,
        file
    );

    std::ofstream timeFile("time_results.csv");

    timeFile << "requests,LFU,2Q,ARC,LIRS\n";

    for (int n : {10, 100, 1000, 10000, 100000}) {
        runTimeTests(
            randomPattern(n),
            capacity,
            timeFile
        );
    }

    timeFile.close();
    file.close();


    return 0;
}