#pragma once

#include "CacheTests.hpp"

#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <vector>

namespace cache_csv {

// CSV escaping: quote fields containing commas, quotes, or line breaks,
// and double any quote characters inside quoted fields.
inline std::string escape(const std::string& value) {
    if (value.find_first_of(",\"\r\n") == std::string::npos)
        return value;

    std::string result = "\"";
    for (const char ch : value) {
        if (ch == '"')
            result += "\"\"";
        else
            result += ch;
    }
    result += '"';
    return result;
}

inline void requireOpen(const std::ofstream& file) {
    if (!file.is_open() || !file.good())
        throw std::runtime_error("CSV output stream is not open or writable");
}

inline void writeResultsHeader(std::ofstream& file) {
    requireOpen(file);
    file << "category,pattern,capacity,seed,requests,unique_keys,"
            "cache,hits,misses,hit_rate,belady_ratio\n";
}

inline void writeResult(std::ofstream& file,
                        const std::string& category,
                        const std::string& pattern,
                        int capacity,
                        unsigned seed,
                        std::size_t requestCount,
                        std::size_t uniqueKeys,
                        const std::string& cacheName,
                        const cache_tests::Stats& stats,
                        const cache_tests::Stats& belady) {
    requireOpen(file);
    file << escape(category) << ','
         << escape(pattern) << ','
         << capacity << ',' << seed << ','
         << requestCount << ',' << uniqueKeys << ','
         << escape(cacheName) << ','
         << stats.hits << ',' << stats.misses << ','
         << std::setprecision(10) << stats.hitRate() << ','
         << cache_tests::beladyRatio(stats, belady) << '\n';
}

inline void writeTwoLevelHeader(std::ofstream& file) {
    requireOpen(file);
    file << "category,pattern,total_capacity,seed,requests,unique_keys,"
            "kind,mode,l1_algo,l2_algo,l1_capacity,l2_capacity,"
            "hits,misses,l1_hits,l2_hits,hit_rate,belady_ratio\n";
}

inline void writeTwoLevelResult(std::ofstream& file,
                                const std::string& category,
                                const std::string& pattern,
                                int totalCapacity,
                                unsigned seed,
                                std::size_t requestCount,
                                std::size_t uniqueKeys,
                                const std::string& kind,
                                const std::string& mode,
                                const std::string& l1Algo,
                                const std::string& l2Algo,
                                int l1Capacity,
                                int l2Capacity,
                                const cache_tests::Stats& stats,
                                long long l1Hits,
                                long long l2Hits,
                                const cache_tests::Stats& belady) {
    requireOpen(file);
    file << escape(category) << ','
         << escape(pattern) << ','
         << totalCapacity << ',' << seed << ','
         << requestCount << ',' << uniqueKeys << ','
         << escape(kind) << ',' << escape(mode) << ','
         << escape(l1Algo) << ',' << escape(l2Algo) << ','
         << l1Capacity << ',' << l2Capacity << ','
         << stats.hits << ',' << stats.misses << ','
         << l1Hits << ',' << l2Hits << ','
         << std::setprecision(10) << stats.hitRate() << ','
         << cache_tests::beladyRatio(stats, belady) << '\n';
}

inline void writeTimeHeader(std::ofstream& file,
                            const std::vector<std::string>& cacheNames) {
    requireOpen(file);
    file << "pattern,requests,capacity";
    for (const auto& name : cacheNames)
        file << ',' << escape(name);
    file << '\n';
}

inline void writeTimeResult(std::ofstream& file,
                            const std::string& pattern,
                            std::size_t requestCount,
                            int capacity,
                            const std::vector<long long>& times) {
    requireOpen(file);
    file << escape(pattern) << ',' << requestCount << ',' << capacity;
    for (const long long time : times)
        file << ',' << time;
    file << '\n';
}

} // namespace cache_csv
