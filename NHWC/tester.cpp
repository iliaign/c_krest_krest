#include "strategies/Cache.hpp"
#include "strategies/lfu_hands.hpp"

#include <iostream>
#include <string>

int main() {
    int value1 = 0;
    LFU_cache<int, int> cache(3);

    cache.put(1, 100);
    cache.put(2, 200);
    cache.put(3, 300);
    cache.get(1, value1);
    cache.get(2, value1);
    cache.put(4, 400); // This should evict key 3 (least frequently used)
    
    for(int i = 1; i <= 4; ++i) {
        if (cache.get(i, value1)) {
            std::cout << "Key " << i << " is in cache with value: " << value1 << std::endl;
        } else {    
            std::cout << "Key " << i << " is not in cache." << std::endl;
        }

    }
    }