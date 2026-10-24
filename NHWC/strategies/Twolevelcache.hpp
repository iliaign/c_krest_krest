#pragma once

#include "Cache.hpp"

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>

enum class TwoLevelMode {

    FillBoth,
    FillL2Only
};

inline const char* toString(TwoLevelMode mode)
{
    return mode == TwoLevelMode::FillBoth ? "fill_both" : "fill_l2_only";
}


template <typename Key, typename Value>
class TwoLevelCache {
public:
    TwoLevelCache(std::shared_ptr<Cache<Key, Value>> l1,
                  std::shared_ptr<Cache<Key, Value>> l2,
                  TwoLevelMode mode = TwoLevelMode::FillBoth)
        : l1_(std::move(l1)), l2_(std::move(l2)), mode_(mode)
    {
        if (!l1_ || !l2_) {
            throw std::invalid_argument("TwoLevelCache: level is null");
        }
    }

    bool get(const Key& key, Value& value)
    {
        if (l1_->get(key, value)) {
            ++l1Hits_;
            return true;
        }

        if (l2_->get(key, value)) {
            ++l2Hits_;
            l1_->put(key, value);  
            return true;
        }

        ++misses_;
        return false;
    }

    void put(const Key& key, const Value& value)
    {
        l2_->put(key, value);

        if (mode_ == TwoLevelMode::FillBoth) {
            l1_->put(key, value);
        }
    }

    std::size_t l1Hits() const { return l1Hits_; }
    std::size_t l2Hits() const { return l2Hits_; }
    std::size_t misses() const { return misses_; }
    std::size_t totalHits() const { return l1Hits_ + l2Hits_; }

    TwoLevelMode mode() const { return mode_; }

    void resetCounters()
    {
        l1Hits_ = l2Hits_ = misses_ = 0;
    }

private:
    std::shared_ptr<Cache<Key, Value>> l1_;
    std::shared_ptr<Cache<Key, Value>> l2_;
    TwoLevelMode mode_;

    std::size_t l1Hits_ = 0;
    std::size_t l2Hits_ = 0;
    std::size_t misses_ = 0;
};