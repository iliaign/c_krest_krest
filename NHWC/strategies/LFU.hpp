#pragma once

#include "Cache.hpp"

#include <unordered_map>

template <typename Key, typename Value>
class LFUCache : public Cache<Key, Value> {
private:
    struct Entry {
        Value value;
        size_t frequency;
    };

    std::unordered_map<Key, Entry> data;

public:
    LFUCache(size_t capacity)
        : Cache<Key, Value>(capacity) {}

    void put(const Key& key, const Value& value) override {
        // Ключ уже существует
        auto it = data.find(key);

        if (it != data.end()) {
            it->second.value = value;
            return;
        }

        // Если кэш заполнен
        if (data.size() >= this->getCapacity()) {
            auto leastUsed = data.begin();

            for (auto current = data.begin();
                 current != data.end();
                 ++current) {

                if (current->second.frequency <
                    leastUsed->second.frequency) {
                    leastUsed = current;
                }
            }

            data.erase(leastUsed);
        }

        data[key] = {value, 1};
    }

    bool get(const Key& key, Value& value) override {
        auto it = data.find(key);

        if (it == data.end()) {
            return false;
        }

        value = it->second.value;
        it->second.frequency++;

        return true;
    }

    void clear() override {
        data.clear();
    }
};