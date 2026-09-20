#pragma once
#include "Cache.hpp"
#include <unordered_map>
#include <list>
#include <optional>

class ARCcache: public Cache<int, int>{

    //4 списка работают по приципу LRU
    //4 очереди
    std::list<int>  T1;
    std::list<int>  T2;
    //госты не хранят значения, только ключ
    //они нужны для баланисровки
    std::list<int>  B1;
    std::list<int>  B2;

    std::size_t p;



    struct NodeData{   
        std::optional<int> data;
    };

    struct Node{   

        Node(char state, std::list<int>::iterator pos): state(state), pos(pos) {}
        //"a" - T1; "b"  T2 
        //"c" B1; "d"  B2;
        char state;
        std::list<int>::iterator pos;
        NodeData obj;

    };

    std::unordered_map<int, Node> cashe_elements;

    //чистим Т1 или Т2
    void replace(){
        std::unordered_map<int, Node>::iterator old_el;
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
        if ( (B1.size()+B2.size()) >= 2*getCapacity()){
            if (B1.size()>B2.size()){
                int key = B1.front();
                B1.pop_front();
                cashe_elements.erase(key);
            }
            else{
                int key = B2.front();
                B2.pop_front();
                cashe_elements.erase(key);
            }
        }
        
    }

    void add_to_t2(std::unordered_map<int, Node>::iterator el, int data){
            T2.push_back(el->first);
            el->second.pos = (--T2.end());
            el->second.obj.data = data;
            el->second.state = 'B';
    }

public:
    ARCcache(): p(0) {}

    bool get(int key, int& value){
        std::unordered_map<int, Node>::iterator el = cashe_elements.find(key);
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


    void put(int key, int data){
        if (getCapacity() == 0){
            return;
        }
        std::unordered_map<int, Node>::iterator el = cashe_elements.find(key);
        //проверяем, есть ли элемент у нас

        
        if (el == cashe_elements.end()){
            //занчит его вообще нигде нет
            //в том числе в гостах!
            if ( (T1.size()+T2.size()) >=getCapacity()){
                //вставка с осчисткой
                replace();
                ch_gost_size();
            }
            //простая вставка и создание элемента
            T1.push_back(key);
            cashe_elements[key] = Node('A', (-- T1.end()) );
            cashe_elements[key].obj.data = data;

    
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
            if (p<getCapacity()){
            ++p;
            }
            //добавляем в т2(если конечно влазим)
            if( (T1.size()+T2.size()) >= getCapacity()){
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
            if( (T1.size()+T2.size()) >= getCapacity() ){
                replace();
                ch_gost_size();
            }
            B2.erase(el->second.pos);
            add_to_t2(el, data);
        }

    }
    void clean();

};