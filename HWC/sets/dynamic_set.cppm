module;

#include <cstddef>
#include <optional>

export module dynamic_set;

// Common interface of a dynamic set of integers.
export class DynamicSet {
public:
    virtual ~DynamicSet() = default;

    // Inserts x (the set does not change if x is already present).
    virtual void insert(int x) = 0;

    // Returns the number of elements in the range (l, r].
    virtual std::ptrdiff_t count_in_range(int l, int r) const = 0;

    // Returns the i-th smallest element (1-based),
    // or std::nullopt if there is no such element.
    virtual std::optional<int> kth(std::ptrdiff_t i) const = 0;

    // Returns the number of elements strictly less than x.
    virtual std::ptrdiff_t count_less(int x) const = 0;
};