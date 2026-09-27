#pragma once

#include "Cache.hpp"
#include <unordered_map>
#include <queue>
#include <vector>
#include <cstddef>

template <typename Key, typename Value>
class BeladyCache : public Cache<Key, Value> {
private:
    //хранилище данных кэша
    std::unordered_map<Key, Value> store;

    //очереди будущих обращених для каждого ключа
    std::unordered_map<Key, std::queue<std::size_t>> future_uses;

    //текущий шаг выполения в общей последовательности
    std::size_t current_step;

public:
    explicit BeladyCache(std::size_t capacity) : Cache<Key, Value>(capacity), current_step(0) {}

    //предварительная загрузка "будущего"
    void setRequests(const std::vector<Key>& requests) {
        clear();
        for (std::size_t i = 0; i < requests.size(); ++i) {
            future_uses[requests[i]].push(i);
        }
    }

    bool get(const Key& key, Value& value) override {
        //убираем текущий шаг из очереди будущих обращений для этого ключа
        auto future_it = future_uses.find(key);
        if (future_it != future_uses.end() && !future_it->second.empty() && future_it->second.front() == current_step) {
        future_it->second.pop();
        }
        //продвигаем глобальный счетчик шагов
        current_step++;

        auto it = store.find(key);
        if (it != store.end()) {
            value = it->second;
            return true;
        }
        return false;
    }

    void put(const Key& key, Value value) override {
        if (this->getCapacity() == 0) return;

        //если ключ уже в кэше - обновляем его
        if (store.find(key) != store.end()) {
            store[key] = value;
            return;
        }

        //если кэш переполнен - ищем что удалить
        if (store.size() >= this->getCapacity()) {
            Key victim_key{};
            std::size_t furthest_time = 0;

            for (const auto& pair : store) {
                const Key& cached_key = pair.first;

                //если элемент больше никогда не понадобиться - сразу удаляем
                if (future_uses[cached_key].empty()) {
                    victim_key = cached_key;
                    break;
                }

                //ищем элемент, обращение к которому будет позже всего
                std::size_t next_use = future_uses[cached_key].front();
                if (next_use > furthest_time) {
                    furthest_time = next_use;
                    victim_key = cached_key;
                }
            }

            store.erase(victim_key);
        }

        // Добавляем новый элемент
        store[key] = value;
    }

    void clear() override {
        store.clear();
        future_uses.clear();
        current_step = 0;
    }
};