#pragma once

#include "Cache.hpp"

#include <unordered_map>
#include <list>
#include <algorithm>

template <typename Key, typename Value>
class TwoQCache : public Cache<Key, Value> {
private:
    std::unordered_map<Key, Value> data;

    // Недавно добавленные элементы
    std::list<Key> q1;

    // Часто используемые элементы
    std::list<Key> q2;

public:
    TwoQCache(size_t capacity)
        : Cache<Key, Value>(capacity) {}

    void put(const Key& key, const Value& value) override {
        auto it = data.find(key);

        // Ключ уже есть
        if (it != data.end()) {
            it->second = value;
            return;
        }

        // Если кэш заполнен
        if (data.size() >= this->getCapacity()) {
            if (!q1.empty()) {
                // Сначала удаляем из Q1
                Key oldKey = q1.front();
                q1.pop_front();

                data.erase(oldKey);
            }
            else if (!q2.empty()) {
                // Если Q1 пустая — удаляем из Q2
                Key oldKey = q2.front();
                q2.pop_front();

                data.erase(oldKey);
            }
        }

        data[key] = value;

        // Новый элемент попадает в Q1
        q1.push_back(key);
    }

    bool get(const Key& key, Value& value) override {
        auto it = data.find(key);

        if (it == data.end()) {
            return false;
        }

        value = it->second;

        // Проверяем, находится ли элемент в Q1
        auto q1It = std::find(q1.begin(), q1.end(), key);

        if (q1It != q1.end()) {
            // Перемещаем элемент из Q1 в Q2
            q1.erase(q1It);
            q2.push_back(key);
        }

        return true;
    }

    void clear() override {
        data.clear();
        q1.clear();
        q2.clear();
    }
};