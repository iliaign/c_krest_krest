#pragma once
#include <cstddef>

template <typename Key, typename Value>
class Cache {
    std::size_t capacity;

public:
    explicit Cache(std::size_t capacity)
        : capacity(capacity) {}

    virtual void put(const Key& key, Value value) = 0;
    virtual bool get(const Key& key, Value& value) = 0;
    virtual void clear() = 0;
    virtual ~Cache() = default;

    std::size_t getCapacity() const {
        return capacity;
    }
};