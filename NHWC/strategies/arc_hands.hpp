#pragma once
#include "Cache.hpp"
#include <unordered_map>
#include <list>
#include <optional>
#include <iterator>
#include <cstddef>

template <typename Key, typename Value>
class ARCcache: public Cache<Key, Value>{

    //4 списка работают по приципу LRU
    //4 очереди
    std::list<Key>  T1;
    std::list<Key>  T2;
    //госты не хранят значения, только ключ
    //они нужны для баланисровки
    std::list<Key>  B1;
    std::list<Key>  B2;

    std::size_t p;

    struct NodeData{
        std::optional<Value> data;
    };

    struct Node{

        Node(char state, typename std::list<Key>::iterator pos): state(state), pos(pos) {}
        //'A' - T1; 'B' - T2
        //'C' - B1; 'D' - B2;
        char state;
        typename std::list<Key>::iterator pos;
        NodeData obj;

    };

    std::unordered_map<Key, Node> cashe_elements;

    //чистим Т1 или Т2
    //in_b2 = true, если вызвали из-за попадания в B2
    void replace(bool in_b2){
        if (T1.empty() && T2.empty()) return;

        //Вытесняем из T1, если она больше целевого размера p
        //(при равенстве и попадании в B2 - тоже из T1, как в статье).
        //Если T2 пуста, вытеснять можно только из T1.
        bool from_t1 = !T1.empty() &&
            (T1.size() > p || (in_b2 && T1.size() == p) || T2.empty());

        if (from_t1){
            Key key = T1.front();
            T1.pop_front();

            //в гостах данных нет, поэтому значение удаляем
            typename std::unordered_map<Key, Node>::iterator old_el = cashe_elements.find(key);
            B1.push_back(key);
            old_el->second.state = 'C';
            old_el->second.pos = std::prev(B1.end());
            old_el->second.obj.data.reset();
        }
        else{
            Key key = T2.front();
            T2.pop_front();

            typename std::unordered_map<Key, Node>::iterator old_el = cashe_elements.find(key);
            B2.push_back(key);
            old_el->second.state = 'D';
            old_el->second.pos = std::prev(B2.end());
            old_el->second.obj.data.reset();
        }
    }

    //ограничиваем суммарный размер гостов
    void ch_gost_size(){
        if ( (B1.size()+B2.size()) >= 2*this->getCapacity()){
            if (B1.size()>B2.size()){
                Key key = B1.front();
                B1.pop_front();
                cashe_elements.erase(key);
            }
            else{
                Key key = B2.front();
                B2.pop_front();
                cashe_elements.erase(key);
            }
        }
    }

    void add_to_t2(typename std::unordered_map<Key, Node>::iterator el, Value data){
            T2.push_back(el->first);
            el->second.pos = (--T2.end());
            el->second.obj.data = data;
            el->second.state = 'B';
    }

public:
    explicit ARCcache(std::size_t capacity): Cache<Key, Value>(capacity), p(0) {}

    bool get(const Key& key, Value& value) override {
        typename std::unordered_map<Key, Node>::iterator el = cashe_elements.find(key);
        if (el == cashe_elements.end()){
            return false;
        }

        //из т1 в т2
        else if (el->second.state == 'A'){
            value = el->second.obj.data.value();
            T1.erase(el->second.pos);
            add_to_t2(el, el->second.obj.data.value());
        }

        else if(el->second.state == 'B'){
            value = el->second.obj.data.value();
            T2.erase(el->second.pos);
            add_to_t2(el, el->second.obj.data.value());
        }
        else if (el->second.state == 'C'){
            return false;
        }
        else if(el->second.state == 'D'){
            return false;
        }
        return true;
    }

    void put(const Key& key, Value data) override {
        if (this->getCapacity() == 0){
            return;
        }
        typename std::unordered_map<Key, Node>::iterator el = cashe_elements.find(key);
        //проверяем, есть ли элемент у нас

        if (el == cashe_elements.end()){
            //значит его вообще нигде нет
            //в том числе в гостах!
            if ( (T1.size()+T2.size()) >=this->getCapacity()){
                //вставка с очисткой
                replace(false);
                ch_gost_size();
            }
            //простая вставка и создание элемента
            T1.push_back(key);
            cashe_elements.emplace(key, Node('A', (-- T1.end()) ));
            cashe_elements.find(key)->second.obj.data = data;
        }
        else if (el->second.state == 'A'){
            //вставляют элемент с ключем из т1
            //дату перезапишем и перекинем элемент в т2
            T1.erase(el->second.pos);
            add_to_t2(el, data);

        }
        else if (el->second.state == 'B'){
            //вставляют элемент из т2
            //в т2 должны подвигать его вперед
            T2.erase(el->second.pos);
            add_to_t2(el, data);
        }
        else if (el->second.state == 'C'){
            //кэш мис, но! удалили из т1, двигаем п, увеличиваем его
            if (p<this->getCapacity()){
                ++p;
            }
            //Сначала убираем элемент из гост-списка. Иначе ch_gost_size()
            //или replace() могут удалить его же, и el станет невалидным.
            B1.erase(el->second.pos);

            //добавляем в т2(если конечно влазим)
            //размер гостов при этом не меняется (-1 из B1, +1 от replace),
            //поэтому ch_gost_size() здесь не нужен
            if( (T1.size()+T2.size()) >= this->getCapacity()){
                replace(false);
            }
            add_to_t2(el, data);

        }
        else if (el->second.state == 'D'){
            //кэш мис, но! удалили из т2, двигаем п, уменьшаем его
            if (p>0){
                --p;
            }
            B2.erase(el->second.pos);

            //добавляем в т2(если конечно влазим)
            if( (T1.size()+T2.size()) >= this->getCapacity() ){
                replace(true);
            }
            add_to_t2(el, data);
        }
    }

    void clear() override {
        T1.clear();
        T2.clear();
        B1.clear();
        B2.clear();
        cashe_elements.clear();
        p = 0;
    }
};