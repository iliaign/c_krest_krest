
module;

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <random>

export module tests;

import dynamic_set;
import set_implementation;
import vector_implementation;

export void run_tests(std::size_t operations = 10000) {
    SetImplementation set;
    VectorImplementation vector;

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> value_dist(-10000, 10000);
    std::uniform_int_distribution<int> command_dist(0, 3);

    using Clock = std::chrono::steady_clock;

    std::size_t size = 0;

    auto set_start = Clock::now();
    auto vector_start = Clock::now();

    std::chrono::nanoseconds set_time{0};
    std::chrono::nanoseconds vector_time{0};

    std::cout << "\n=== TESTING ===\n";

    for (std::size_t i = 0; i < operations; ++i) {
        int command = command_dist(rng);
        int x = value_dist(rng);
        int y = value_dist(rng);

        if (command == 0) {
            auto start = Clock::now();
            set.insert(x);
            set_time += Clock::now() - start;

            start = Clock::now();
            vector.insert(x);
            vector_time += Clock::now() - start;

            ++size;
        }
        else if (command == 1) {
            int left = std::min(x, y);
            int right = std::max(x, y);

            auto a = set.count_in_range(left, right);
            auto b = vector.count_in_range(left, right);

            if (a != b) {
                std::cerr << "FAIL: count_in_range, operation "
                          << i << '\n';
                return;
            }
        }
        else if (command == 2) {
            std::uniform_int_distribution<long long> dist(
                -2, static_cast<long long>(size) + 2
            );

            auto index = static_cast<std::ptrdiff_t>(dist(rng));

            auto a = set.kth(index);
            auto b = vector.kth(index);

            if (a != b) {
                std::cerr << "FAIL: kth, operation "
                          << i << '\n';
                return;
            }
        }
        else {
            auto a = set.count_less(x);
            auto b = vector.count_less(x);

            if (a != b) {
                std::cerr << "FAIL: count_less, operation "
                          << i << '\n';
                return;
            }
        }
    }

    std::cout << "All " << operations << " operations passed!\n";

    std::cout << "\nSet time: "
              << set_time.count() << " ns\n";

    std::cout << "Vector time: "
              << vector_time.count() << " ns\n";

    std::cout << "\nALL TESTS PASSED!\n";
}
