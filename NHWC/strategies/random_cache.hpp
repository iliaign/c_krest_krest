#pragma once

#include "Cache.hpp"

#include <unordered_map>
#include <vector>
#include <cstdlib>

template <typename Key, typename Value>
class RandomCache : public Cache<Key, Value>
{
private:
    std::unordered_map<Key, Value> data;
    std::vector<Key> keys;

public:
    RandomCache(std::size_t capacity)
        : Cache<Key, Value>(capacity)
    {
    }

    void put(const Key& key, const Value& value) override
    {
        if (this->getCapacity() == 0)
            return;

        auto it = data.find(key);

        if (it != data.end())
        {
            it->second = value;
            return;
        }

        if (data.size() >= this->getCapacity())
        {
            std::size_t index = std::rand() % keys.size();

            data.erase(keys[index]);
            keys[index] = key;
        }
        else
        {
            keys.push_back(key);
        }

        data[key] = value;
    }

    bool get(const Key& key, Value& value) override
    {
        auto it = data.find(key);

        if (it == data.end())
            return false;

        value = it->second;
        return true;
    }

    void clear() override
    {
        data.clear();
        keys.clear();
    }
};