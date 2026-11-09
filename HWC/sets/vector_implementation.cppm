module;

#include <algorithm>
#include <cstddef>
#include <optional>
#include <vector>

export module vector_implementation;

import dynamic_set;

// Implementation based on a sorted std::vector.
export class VectorImplementation final : public DynamicSet {
public:
    void insert(int x) override {
        auto it = std::lower_bound(values_.begin(), values_.end(), x);
        if (it == values_.end() || *it != x) {
            values_.insert(it, x);
        }
    }

    std::ptrdiff_t count_in_range(int l, int r) const override {
        if (l >= r) {
            return 0;
        }
        // Elements in (l, r]: from the first element > l to the first element > r.
        auto first = std::upper_bound(values_.begin(), values_.end(), l);
        auto last = std::upper_bound(values_.begin(), values_.end(), r);
        return last - first;
    }

    std::optional<int> kth(std::ptrdiff_t i) const override {
        if (i < 1 || i > std::ssize(values_)) {
            return std::nullopt;
        }
        return *(values_.begin() + (i - 1));
    }

    std::ptrdiff_t count_less(int x) const override {
        return std::lower_bound(values_.begin(), values_.end(), x) - values_.begin();
    }

private:
    std::vector<int> values_;  // Always sorted in ascending order, no duplicates.
};