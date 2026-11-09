module;

#include <cstddef>
#include <iterator>
#include <optional>
#include <set>

export module set_implementation;

import dynamic_set;

// Implementation based on std::set.
export class SetImplementation final : public DynamicSet {
public:
    void insert(int x) override {
        values_.insert(x);
    }

    std::ptrdiff_t count_in_range(int l, int r) const override {
        if (l >= r) {
            return 0;
        }
        // Elements in (l, r]: from the first element > l to the first element > r.
        return std::distance(values_.upper_bound(l), values_.upper_bound(r));
    }

    std::optional<int> kth(std::ptrdiff_t i) const override {
        if (i < 1 || i > std::ssize(values_)) {
            return std::nullopt;
        }
// std::set does not support access by index (values_[i]),
// because its elements are stored in a tree, not an array.
// std::next() moves the iterator to the required position,
// and * dereferences the iterator to get the element itself.
        return *std::next(values_.begin(), i - 1);
    }

    std::ptrdiff_t count_less(int x) const override {
        return std::distance(values_.begin(), values_.lower_bound(x));
    }

private:
    std::set<int> values_;
};