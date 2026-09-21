#include "strategies/Cache.hpp"
#include "strategies/lfu_hands.hpp"
#include "strategies/arc_hands.hpp"
#include "strategies/LIRC.hpp"

#include <iostream>
#include <chrono>


void test_lfu() {
    std::cout << "===== LFU TEST =====" << std::endl;

    int value1 = 0;
    LFU_cache<int, int> cache(3);

    cache.put(1, 100);
    cache.put(2, 200);
    cache.put(3, 300);

    cache.get(1, value1);
    cache.get(2, value1);

    cache.put(4, 400);

    for (int i = 1; i <= 4; ++i) {
        if (cache.get(i, value1)) {
            std::cout << "Key " << i
                      << " is in cache with value: "
                      << value1 << std::endl;
        }
        else {
            std::cout << "Key " << i
                      << " is not in cache." << std::endl;
        }
    }

    std::cout << std::endl;
}


void test_arc() {
    std::cout << "===== ARC TEST =====" << std::endl;

    int value1 = 0;
    ARCcache<int, int> cache(3);

    cache.put(1, 100);
    cache.put(2, 200);
    cache.put(3, 300);

    cache.get(1, value1);
    cache.get(2, value1);

    cache.put(4, 400);

    for (int i = 1; i <= 4; ++i) {
        if (cache.get(i, value1)) {
            std::cout << "Key " << i
                      << " is in cache with value: "
                      << value1 << std::endl;
        }
        else {
            std::cout << "Key " << i
                      << " is not in cache." << std::endl;
        }
    }

    std::cout << std::endl;
}

void test_lirc() {
    std::cout << "===== LIRC TEST =====" << std::endl;

    int value1 = 0;
    LircCache<int, int> cache(3);

    cache.put(1, 100);
    cache.put(2, 200);
    cache.put(3, 300);

    cache.get(1, value1);
    cache.get(2, value1);

    cache.put(4, 400);

    for (int i = 1; i <= 4; ++i) {
        if (cache.get(i, value1)) {
            std::cout << "Key " << i
                      << " is in cache with value: "
                      << value1 << std::endl;
        }
        else {
            std::cout << "Key " << i
                      << " is not in cache." << std::endl;
        }
    }

    std::cout << std::endl;
}


/*
 * Проверяем зависимость времени от capacity.
 *
 * Количество операций фиксировано:
 * 1 000 000
 *
 * Если время примерно одинаковое при разных
 * capacity, это соответствует O(1) на операцию
 * относительно размера кэша.
 */
void benchmark_lfu() {
    std::cout << "===== LFU BENCHMARK =====" << std::endl;

    const int operations = 1000000;
    const int capacities[] = {100, 1000, 10000, 100000};

    for (int capacity : capacities) {
        LFU_cache<int, int> cache(capacity);

        for (int i = 0; i < capacity; ++i) {
            cache.put(i, i);
        }

        int value = 0;

        std::chrono::high_resolution_clock::time_point start =
            std::chrono::high_resolution_clock::now();

        for (int i = 0; i < operations; ++i) {
            int key = i % capacity;

            if (i % 2 == 0) {
                cache.get(key, value);
            }
            else {
                cache.put(key, i);
            }
        }

        std::chrono::high_resolution_clock::time_point finish =
            std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> elapsed = finish - start;

        std::cout << "Capacity: " << capacity
                  << ", operations: " << operations
                  << ", time: " << elapsed.count()
                  << " sec" << std::endl;
    }

    std::cout << std::endl;
}


void benchmark_arc() {
    std::cout << "===== ARC BENCHMARK =====" << std::endl;

    const int operations = 1000000;
    const int capacities[] = {100, 1000, 10000, 100000};

    for (int capacity : capacities) {
        ARCcache<int, int> cache(capacity);

        for (int i = 0; i < capacity; ++i) {
            cache.put(i, i);
        }

        int value = 0;

        std::chrono::high_resolution_clock::time_point start =
            std::chrono::high_resolution_clock::now();

        for (int i = 0; i < operations; ++i) {
            int key = i % capacity;

            if (i % 2 == 0) {
                cache.get(key, value);
            }
            else {
                cache.put(key, i);
            }
        }

        std::chrono::high_resolution_clock::time_point finish =
            std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> elapsed = finish - start;

        std::cout << "Capacity: " << capacity
                  << ", operations: " << operations
                  << ", time: " << elapsed.count()
                  << " sec" << std::endl;
    }

    std::cout << std::endl;
}

void benchmark_lirc() {
    std::cout << "===== LIRC BENCHMARK =====" << std::endl;

    const int operations = 1000000;
    const int capacities[] = {100, 1000, 10000, 100000};

    for (int capacity : capacities) {
        LircCache<int, int> cache(capacity);

        for (int i = 0; i < capacity; ++i) {
            cache.put(i, i);
        }

        int value = 0;

        std::chrono::high_resolution_clock::time_point start =
            std::chrono::high_resolution_clock::now();

        for (int i = 0; i < operations; ++i) {
            int key = i % capacity;

            if (i % 2 == 0) {
                cache.get(key, value);
            }
            else {
                cache.put(key, i);
            }
        }

        std::chrono::high_resolution_clock::time_point finish =
            std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> elapsed = finish - start;

        std::cout << "Capacity: " << capacity
                  << ", operations: " << operations
                  << ", time: " << elapsed.count()
                  << " sec" << std::endl;
    }

    std::cout << std::endl;
}

/*
 * Проверяем зависимость времени от количества операций.
 *
 * Capacity фиксирована = 10000.
 *
 * Если количество операций увеличивается в 10 раз,
 * а время тоже примерно увеличивается в 10 раз,
 * это соответствует линейной зависимости O(N).
 */
void benchmark_operations_lfu() {
    std::cout << "===== LFU OPERATIONS SCALING =====" << std::endl;

    const int capacity = 10000;
    const int operations[] = {
        100000,
        1000000,
        10000000
    };

    for (int count : operations) {
        LFU_cache<int, int> cache(capacity);

        for (int i = 0; i < capacity; ++i) {
            cache.put(i, i);
        }

        int value = 0;

        std::chrono::high_resolution_clock::time_point start =
            std::chrono::high_resolution_clock::now();

        for (int i = 0; i < count; ++i) {
            int key = i % capacity;

            if (i % 2 == 0) {
                cache.get(key, value);
            }
            else {
                cache.put(key, i);
            }
        }

        std::chrono::high_resolution_clock::time_point finish =
            std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> elapsed = finish - start;

        std::cout << "Operations: " << count
                  << ", time: " << elapsed.count()
                  << " sec" << std::endl;
    }

    std::cout << std::endl;
}


void benchmark_operations_arc() {
    std::cout << "===== ARC OPERATIONS SCALING =====" << std::endl;

    const int capacity = 10000;
    const int operations[] = {
        100000,
        1000000,
        10000000
    };

    for (int count : operations) {
        ARCcache<int, int> cache(capacity);

        for (int i = 0; i < capacity; ++i) {
            cache.put(i, i);
        }

        int value = 0;

        std::chrono::high_resolution_clock::time_point start =
            std::chrono::high_resolution_clock::now();

        for (int i = 0; i < count; ++i) {
            int key = i % capacity;

            if (i % 2 == 0) {
                cache.get(key, value);
            }
            else {
                cache.put(key, i);
            }
        }

        std::chrono::high_resolution_clock::time_point finish =
            std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> elapsed = finish - start;

        std::cout << "Operations: " << count
                  << ", time: " << elapsed.count()
                  << " sec" << std::endl;
    }

    std::cout << std::endl;
}

void benchmark_operations_lirc() {
    std::cout << "===== LIRC OPERATIONS SCALING =====" << std::endl;

    const int capacity = 10000;
    const int operations[] = {
        100000,
        1000000,
        10000000
    };

    for (int count : operations) {
        LircCache<int, int> cache(capacity);

        for (int i = 0; i < capacity; ++i) {
            cache.put(i, i);
        }

        int value = 0;

        std::chrono::high_resolution_clock::time_point start =
            std::chrono::high_resolution_clock::now();

        for (int i = 0; i < count; ++i) {
            int key = i % capacity;

            if (i % 2 == 0) {
                cache.get(key, value);
            }
            else {
                cache.put(key, i);
            }
        }

        std::chrono::high_resolution_clock::time_point finish =
            std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> elapsed = finish - start;

        std::cout << "Operations: " << count
                  << ", time: " << elapsed.count()
                  << " sec" << std::endl;
    }

    std::cout << std::endl;
}

int main() {
    // Проверка корректности работы
    test_lfu();
    test_arc();
    test_lirc();

    // Зависимость времени от capacity
    benchmark_lfu();
    benchmark_arc();
    benchmark_lirc();

    // Зависимость времени от количества операций
    benchmark_operations_lfu();
    benchmark_operations_arc();
    benchmark_operations_lirc();

    return 0;
}
