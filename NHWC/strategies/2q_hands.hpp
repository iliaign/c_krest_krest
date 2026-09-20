#pragma once
#include<cstddef>
#include<list>
#include<unordered_map>
#include<algoritm>
#include"Cache.hpp"
template <typename Key, typename Value>
class TwoQ : public Cache<Key, Value>
{
private:
    enum class Queue{
        A1IN,
        AM
    };
    struct Node{
        Value value;
        Queue queue;
        typename std::list<Key>::iterator pos;
    };
    std::unordered_map<Key, Node> data;
    std::list<Key> a1in;
    std::list<Key> a1out;
    std::list<Key> am;
    std::size_t a1inCapacity;
    std::size_t a1outCapacity;
    std::size_t amCapacity;
    void addToA1In(const Key& key, Value value){
        if (a1in.size() >= a1inCapacity){
            moveA1InToA1Out();
        }
        a1in.push_front(key);
        Node node{
            std::move(value),
            Queue::A1IN,
            a1in.begin()
        };
        data.emplace(key, std::move(node));
    }
    void moveA1InToA1Out(){
        if (a1in.empty()){
            return;
        }
        Key oldKey = a1in.back();
        a1in.pop_back();
        data.erase(oldKey);
        a1out.push_front(oldKey);
        if (a1out.size() > a1outCapacity){
            a1out.pop_back();
        }
    }
    void addToAm(const Key& key, Value value){
        if (am.size() >= amCapacity){
            RemOldestFromAm();
        }
        am.push_front(key);
        Node node{
            std::move(value),
            Queue::AM,
            am.begin()
        };
        data.emplace(key, std::move(node));
    }
    void touchAm(typename std::unordered_map<Key, Node>::iterator it){
        Key key = it->first;
        am.erase(it->second.pos);
        am.push_front(key);
        it->second.pos = am.begin();
    }
    void RemOldestFromAm(){
        if (am.empty()){
            return;
        }
        Key oldKey = am.back();
        am.pop_back();
        data.erase(oldKey);
    }
    void RemOldestFromA1Out(){
        if (!a1out.empty()){
            a1out.pop_back();
        }
    }
public:
    explicit TwoQ(std::size_t capacity)
       :Cache<Key, Value>(capacity),
        a1inCapacity((capacity + 3) / 4),
        a1outCapacity((capacity + 3) / 4),
        amCapacity(capacity - a1inCapacity) {}
    void put(const Key& key, Value value)override{
        if (this->getCapacity() == 0){
            return;
        }
        auto it = data.find(key);
        if (it != data.end()){
            it->second.value = std::move(value);
            if (it->second.queue == Queue::AM){
                touchAm(it);
            }
            return;
        }
        auto ghost = std::find(a1out.begin(), a1out.end(), key);
        if (ghost != a1out.end()){
            a1out.erase(ghost);
            addToAm(key, std::move(value));
            return;
        }
        addToA1In(key, std::move(value));
    }
    bool get(const Key& key, Value& value)override{
        auto it = data.find(key);
        if (it == data.end()){
            auto ghost = std::find(a1out.begin(), a1out.end(), key);
            if (ghost != a1out.end()){
                a1out.erase(ghost);
            }
            return false;
        }
        value = it->second.value;
        if (it->second.queue == Queue::A1IN){
            Key oldKey = it->first;
            Value oldValue = it->second.value;
            a1in.erase(it->second.pos);
            data.erase(it);
            addToAm(oldKey, std::move(oldValue));
        }
        else{
            touchAm(it);
        }
        return true;
    }
    void clear()override{
        data.clear();
        a1in.clear();
        a1out.clear();
        am.clear();
    }
};