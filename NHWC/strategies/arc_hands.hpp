#pragma once
#include "Cache.hpp"
#include <unordered_map>
#include <list>

class ARCcache: public Cache<int, int>{

    std::list<int>  resent;
    std::list<int>  frequent;
    //госты не хранят значения, только ключ
    //они нужны для баланисровки
    std::list<int>  ghost_resent;
    std::list<int>  ghost_frequent;

    std::size_t p;
    
    struct Node
    {   
        //resent 1; frequent 2; ghost 0
        int list_pos;
        std::list<int>::iterator pos;
        
    };
    
    std::unordered_map<int, Node> data;


public:

    bool get(int key, int value);
    void put(int key);
    void clean();

};