#pragma once

#include <algorithm>
#include <random>
#include <string>
#include <vector>

using Generator = std::vector<int>(*)(int, int, unsigned);

struct Pattern {
    std::string name;
    std::string category;
    bool deterministic;
    Generator generate;
};

inline std::vector<int> loopFits(int n, int cap, unsigned) {
    std::vector<int> r;
    for (int i = 0; i < n; ++i) r.push_back(i % cap);
    return r;
}

inline std::vector<int> loopPattern(int n, int cap, unsigned) {
    std::vector<int> r;
    const int size = cap + cap / 5 + 1;
    for (int i = 0; i < n; ++i) r.push_back(i % size);
    return r;
}

inline std::vector<int> uniformPattern(int n, int cap, unsigned seed) {
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> dist(0, 2 * cap - 1);
    std::vector<int> r;
    for (int i = 0; i < n; ++i) r.push_back(dist(gen));
    return r;
}

inline std::vector<int> zipfPattern(int n, int cap, unsigned seed) {
    std::mt19937 gen(seed);
    std::vector<double> weights(10 * cap);
    for (int i = 0; i < 10 * cap; ++i) weights[i] = 1.0 / (i + 1);
    std::discrete_distribution<int> dist(weights.begin(), weights.end());
    std::vector<int> r;
    for (int i = 0; i < n; ++i) r.push_back(dist(gen));
    return r;
}

inline std::vector<int> hotScanPattern(int n, int cap, unsigned seed) {
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> hot(0, std::max(1, cap / 2) - 1);
    std::vector<int> r;
    const int hotLength = 500, scanLength = 2 * cap;
    const int period = hotLength + scanLength;
    int uniqueKey = 1000000;
    for (int i = 0; i < n; ++i) {
        if (i % period < hotLength) r.push_back(hot(gen));
        else r.push_back(uniqueKey++);
    }
    return r;
}

inline std::vector<int> workingSetShiftPattern(int n, int cap, unsigned seed) {
    std::mt19937 gen(seed);
    const int size = std::max(1, cap * 8 / 10);
    std::uniform_int_distribution<int> dist(0, size - 1);
    std::vector<int> r;
    for (int i = 0; i < n; ++i)
        r.push_back((i / 2000) * 100000 + dist(gen));
    return r;
}

inline std::vector<int> lruStackPattern(int n, int cap, unsigned seed) {
    std::mt19937 gen(seed);
    const double probability = std::min(0.9, 2.0 / std::max(1, cap));
    std::geometric_distribution<int> depth(probability);
    std::uniform_real_distribution<double> coin(0.0, 1.0);
    std::vector<int> stack, r;
    int nextKey = 0;

    for (int i = 0; i < n; ++i) {
        int key;
        if (stack.empty() || coin(gen) < 0.05) {
            key = nextKey++;
        } else {
            const std::size_t d = std::min<std::size_t>(
                static_cast<std::size_t>(depth(gen)), stack.size() - 1);
            key = stack[d];
            stack.erase(stack.begin() + static_cast<std::ptrdiff_t>(d));
        }
        stack.insert(stack.begin(), key);
        r.push_back(key);
    }
    return r;
}

inline std::vector<int> lfuPollutionPattern(int n, int cap, unsigned seed) {
    std::mt19937 gen(seed);
    const int size = std::max(1, cap * 8 / 10);
    std::uniform_int_distribution<int> dist(0, size - 1);
    std::vector<int> r;
    for (int i = 0; i < n; ++i) {
        const int base = i < n * 4 / 10 ? 0 : 100000;
        r.push_back(base + dist(gen));
    }
    return r;
}

inline std::vector<int> oneHitWondersPattern(int n, int cap, unsigned seed) {
    std::mt19937 gen(seed);
    std::uniform_real_distribution<double> coin(0.0, 1.0);
    std::uniform_int_distribution<int> hot(0, std::max(1, cap / 2) - 1);
    std::vector<int> r;
    int uniqueKey = 1000000;
    for (int i = 0; i < n; ++i) {
        if (coin(gen) < 0.5) r.push_back(hot(gen));
        else r.push_back(uniqueKey++);
    }
    return r;
}

inline std::vector<int> multiLoopPattern(int n, int cap, unsigned) {
    std::vector<int> r;
    const int small = std::max(1, cap / 4), big = 2 * cap;
    int s = 0, b = 0;
    for (int i = 0; i < n; ++i) {
        if (i % 2 == 0) r.push_back(s++ % small);
        else r.push_back(100000 + b++ % big);
    }
    return r;
}

inline std::vector<int> alternatingPhasesPattern(int n, int cap, unsigned seed) {
    std::mt19937 gen(seed);
    std::vector<double> weights(3 * cap);
    for (int i = 0; i < 3 * cap; ++i) weights[i] = 1.0 / (i + 1);
    std::discrete_distribution<int> dist(weights.begin(), weights.end());
    std::vector<int> r;
    const int loopSize = cap + cap / 10 + 1;
    int pos = 0;
    for (int i = 0; i < n; ++i) {
        if ((i / 1000) % 2 == 0) r.push_back(dist(gen));
        else r.push_back(500000 + pos++ % loopSize);
    }
    return r;
}

inline std::vector<Pattern> allPatterns() {
    return {
        {"LoopFits", "basic", true, loopFits},
        {"Loop", "basic", true, loopPattern},
        {"Uniform", "basic", false, uniformPattern},
        {"Zipf", "basic", false, zipfPattern},
        {"HotScan", "advanced", false, hotScanPattern},
        {"WorkingSetShift", "advanced", false, workingSetShiftPattern},
        {"LruStack", "advanced", false, lruStackPattern},
        {"LfuPollution", "advanced", false, lfuPollutionPattern},
        {"OneHitWonders", "advanced", false, oneHitWondersPattern},
        {"MultiLoop", "advanced", true, multiLoopPattern},
        {"AlternatingPhases", "advanced", false, alternatingPhasesPattern}
    };
}
