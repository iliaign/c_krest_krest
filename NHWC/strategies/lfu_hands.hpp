#pragma once
#include "Cache.hpp"
#include <unordered_map>
#include <list>

template <typename K, typename V>
class LFUdata : public Cache<K, V> {
    int minfreq;

    struct Node {
        V value;
        int frequency;
        typename std::list<K>::iterator pos;

        Node(V value, typename std::list<K>::iterator pos)
            : value(value), frequency(1), pos(pos) {}
    };

    // ключ -> данные
    std::unordered_map<K, Node> data;

    // частота -> список ключей
    std::unordered_map<int, std::list<K>> freqList;

    void up_freq(typename std::unordered_map<K, Node>::iterator it ) {
        int old_freq = it->second.frequency;
        freqList[old_freq].erase(it->second.pos);
        int new_freq = ++it->second.frequency;
        freqList[new_freq].push_back(it->first);
        it->second.pos = --freqList[new_freq].end();

        if (old_freq == minfreq &&
            freqList[old_freq].empty()) {
            ++minfreq;
        }
    }

    void del_el() {
        K current_key = freqList[minfreq].front();

        freqList[minfreq].pop_front();
        data.erase(current_key);
    }

public:
    LFUdata(size_t capacity): Cache<K, V>(capacity), minfreq(0) {}


    bool get(const K& key, V& value) override {
        typename std::unordered_map<K, Node>::iterator it = data.find(key);
        if (it == data.end()) { 
            return false;
        }
        up_freq(it);
        value = it->second.value;

        return true;
    }

    void put(const K& key, V value) override {
        if (this->getCapacity() == 0) { 
            return;
        }
        typename std::unordered_map<K, Node>::iterator it = data.find(key);

        if (it == data.end()) {
            if (data.size() >= this->getCapacity()) { 
                del_el();
            }

            freqList[1].push_back(key);
            minfreq = 1;
            data.emplace(key, Node(value, --freqList[1].end())
            );
        }
        else {
            it->second.value = value;
            up_freq(it);
        }
    }

    void clear() override {
        data.clear();
        freqList.clear();
        minfreq = 0;
    }
};