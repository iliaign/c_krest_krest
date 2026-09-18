#pragma once

#include <cstddef>

template <typename Key, typename Value>
class Cache {
    size_t capacity;

public:
    Cache(size_t capacity) : capacity(capacity) {}

    virtual void put(const Key& key, const Value& value) = 0;
    virtual bool get(const Key& key, Value& value) = 0;
    virtual void clear() = 0;

    virtual ~Cache() = default;

    size_t getCapacity() const {
        return capacity;
    }
};