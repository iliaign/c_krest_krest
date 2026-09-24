
#pragma once
#include "Cache.hpp"
#include <unordered_map>
#include <list>
#include <optional>

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
        //"a" - T1; "b"  T2 
        //"c" B1; "d"  B2;
        char state;
        typename std::list<Key>::iterator pos;
        NodeData obj;

    };

    std::unordered_map<Key, Node> cashe_elements;

    //чистим Т1 или Т2
    void replace(){
        typename std::unordered_map<Key, Node>::iterator old_el;
        if (p<T1.size()){
            //T1 больше целевого размера p
            //вытесняем с т1 и отправляем в b1
            //в гостах данных нет, поэтому удаляем их
            old_el = cashe_elements.find(T1.front()); 
            
            //пока не смотрим на вместимость b1, позже сделаю
            //old_el->second.obj.data =NAN;
            
            T1.erase(old_el->second.pos);
            B1.push_back(old_el->first);
            old_el->second.state = 'C';
            old_el->second.pos = -- B1.end(); 
            old_el->second.obj.data.reset();      
        }
        else{
             old_el = cashe_elements.find(T2.front()); 
            
            //пока не смотрим на вместимость b2, позже сделаю
            //old_el->second.obj.data = NAN;
            
            T2.erase(old_el->second.pos);
            B2.push_back(old_el->first);
            old_el->second.state = 'D';
            old_el->second.pos = -- B2.end();
            old_el->second.obj.data.reset();      
        }
    }    

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

    bool get(const Key& key, Value& value){
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

    void put(const Key& key, Value data){
        if (this->getCapacity() == 0){
            return;
        }
        typename std::unordered_map<Key, Node>::iterator el = cashe_elements.find(key);
        //проверяем, есть ли элемент у нас

        if (el == cashe_elements.end()){
            //занчит его вообще нигде нет
            //в том числе в гостах!
            if ( (T1.size()+T2.size()) >=this->getCapacity()){
                //вставка с осчисткой
                replace();
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
            //вставляют элемнет из т2
            //в т2 должны подвигать его вперед
            T2.erase(el->second.pos);
            add_to_t2(el, data);
        }
        else if (el->second.state == 'C'){
            //кэш мис, но! удалили из т1, двигаем п, увеличиваем его
            if (p<this->getCapacity()){
            ++p;
            }
            //добавляем в т2(если конечно влазим)
            if( (T1.size()+T2.size()) >= this->getCapacity()){
                replace();
                ch_gost_size();

            }
            B1.erase(el->second.pos);
            add_to_t2(el, data);
            
        }
        else if (el->second.state == 'D'){
            //кэш мис, но! удалили из т2, двигаем п, уменьшаем его
            if (p>0){
            --p;
            }
            //добавляем в т2(если конечно влазим)
            if( (T1.size()+T2.size()) >= this->getCapacity() ){
                replace();
                ch_gost_size();
            }
            B2.erase(el->second.pos);
            add_to_t2(el, data);
        }
    }
    void clear(){
        T1.clear();
        T2.clear();
        B1.clear();
        B2.clear();
        cashe_elements.clear();
        p = 0;
    };
};