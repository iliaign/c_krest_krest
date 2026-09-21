#pragma once

#include "Cache.hpp" // Или путь к файлу с базовым классом Cache
#include <unordered_map>
#include <list>
#include <utility>
#include <algorithm>
#include <cstddef>

template <typename Key, typename Value>
class LircCache : public Cache<Key, Value> {
private:
    enum class Status {
        LIR,
        HIR_RESIDENT,
        HIR_NON_RESIDENT
    };

    struct Node {
        Key key;
        Value value;
        Status status;
        
        // Итераторы в списках для O(1) удаления и перемещения
        typename std::list<Key>::iterator stack_iter;
        typename std::list<Key>::iterator queue_iter;
        bool in_stack = false;
        bool in_queue = false;
    };

    std::size_t max_lir_capacity;
    std::size_t max_hir_capacity;

    std::size_t current_lir_count = 0;
    std::size_t current_hir_resident_count = 0;

    std::list<Key> S; // Стек S (LIR + HIR, недавно посещенные)
    std::list<Key> Q; // Очередь Q (Только HIR-резиденты)

    std::unordered_map<Key, Node> table;

    // Вспомогательная операция обрезки стека S (Stack Pruning)
    // Низ (back) стека S должен ВСЕГДА занимать LIR-блок.
    void prune_stack() {
        while (!S.empty()) {
            Key bottom_key = S.back();
            auto it = table.find(bottom_key);
            
            // Если на дне LIR-блок — прекращаем обрезку
            if (it != table.end() && it->second.status == Status::LIR) {
                break;
            }

            // Если на дне HIR-блок, убираем его из S
            if (it != table.end()) {
                it->second.in_stack = false;
            }
            S.pop_back();
        }
    }

    // Удаление HIR элемента из очереди Q
    void remove_from_queue(Node& node) {
        if (node.in_queue) {
            Q.erase(node.queue_iter);
            node.in_queue = false;
        }
    }

    // Удаление из стека S
    void remove_from_stack(Node& node) {
        if (node.in_stack) {
            S.erase(node.stack_iter);
            node.in_stack = false;
        }
    }

    // Перемещение/Добавление на вершину стека S
    void push_stack(Key key, Node& node) {
        remove_from_stack(node);
        S.push_front(key);
        node.stack_iter = S.begin();
        node.in_stack = true;
    }

    // Добавление в конец очереди Q
    void push_queue(Key key, Node& node) {
        remove_from_queue(node);
        Q.push_back(key);
        node.queue_iter = std::prev(Q.end());
        node.in_queue = true;
    }

    // Вытеснение наименее востребованного HIR-элемента из Q
    void evict_hir() {
        if (Q.empty()) return;

        Key evict_key = Q.front();
        Q.pop_front();

        auto it = table.find(evict_key);
        if (it != table.end()) {
            it->second.in_queue = false;
            
            // Если элемент есть в S, он становится HIR_NON_RESIDENT (метаданные)
            if (it->second.in_stack) {
                it->second.status = Status::HIR_NON_RESIDENT;
                it->second.value = Value(); // Освобождаем память от значения
            } else {
                // Если его нет в S, полностью удаляем из хеш-таблицы
                table.erase(it);
            }
            --current_hir_resident_count;
        }
    }

public:
    explicit LircCache(std::size_t capacity, float hir_ratio = 0.1f)
        : Cache<Key, Value>(capacity) {
        if (capacity == 0) capacity = 1;
        
        max_hir_capacity = static_cast<std::size_t>(capacity * hir_ratio);
        if (max_hir_capacity == 0) max_hir_capacity = 1;
        
        max_lir_capacity = capacity - max_hir_capacity;
        if (max_lir_capacity == 0) {
            max_lir_capacity = 1;
            max_hir_capacity = capacity > 1 ? capacity - 1 : 1;
        }
    }

    bool get(const Key& key, Value& value) override {
        auto it = table.find(key);
        if (it == table.end() || it->second.status == Status::HIR_NON_RESIDENT) {
            return false;
        }

        Node& node = it->second;
        value = node.value;

        bool was_in_stack = node.in_stack;
        push_stack(key, node);

        if (node.status == Status::LIR) {
            if (S.back() == key) {
                prune_stack();
            }
        } else if (node.status == Status::HIR_RESIDENT) {
            if (was_in_stack) {
                // Промпливание до LIR (повторный доступ за короткое время)
                node.status = Status::LIR;
                remove_from_queue(node);
                ++current_lir_count;
                --current_hir_resident_count;

                // Демонтаж самого старого LIR-блока до HIR
                Key bottom_lir_key = S.back();
                Node& bottom_node = table[bottom_lir_key];
                
                bottom_node.status = Status::HIR_RESIDENT;
                --current_lir_count;
                ++current_hir_resident_count;
                
                push_queue(bottom_lir_key, bottom_node);
                prune_stack();
            } else {
                // Остается HIR, но перемещается в конец Q
                push_queue(key, node);
            }
        }

        return true;
    }

    void put(const Key& key, Value value) override {
        auto it = table.find(key);

        if (it != table.end()) {
            // Элемент уже существует в структуре данных
            Node& node = it->second;
            node.value = value;

            if (node.status == Status::LIR) {
                bool is_bottom = (S.back() == key);
                push_stack(key, node);
                if (is_bottom) prune_stack();
            } 
            else if (node.status == Status::HIR_RESIDENT) {
                bool was_in_stack = node.in_stack;
                push_stack(key, node);

                if (was_in_stack) {
                    node.status = Status::LIR;
                    remove_from_queue(node);
                    ++current_lir_count;
                    --current_hir_resident_count;

                    // Понижаем нижний LIR
                    Key bottom_lir_key = S.back();
                    Node& bottom_node = table[bottom_lir_key];
                    
                    bottom_node.status = Status::HIR_RESIDENT;
                    --current_lir_count;
                    ++current_hir_resident_count;

                    push_queue(bottom_lir_key, bottom_node);
                    prune_stack();
                } else {
                    push_queue(key, node);
                }
            } 
            else if (node.status == Status::HIR_NON_RESIDENT) {
                // Повторный доступ к объекту, который был вытеснен из кэша, но был в стеке S
                if (current_hir_resident_count >= max_hir_capacity) {
                    evict_hir();
                }

                node.status = Status::LIR;
                push_stack(key, node);
                ++current_lir_count;

                // Понижаем самый старый LIR
                Key bottom_lir_key = S.back();
                Node& bottom_node = table[bottom_lir_key];

                bottom_node.status = Status::HIR_RESIDENT;
                --current_lir_count;
                ++current_hir_resident_count;

                push_queue(bottom_lir_key, bottom_node);
                prune_stack();
            }
            return;
        }

        // Новый ключ (Cache Miss)
        if (current_lir_count < max_lir_capacity && current_hir_resident_count == 0) {
            // Начальное заполнение: сразу делаем LIR
            Node new_node{key, value, Status::LIR};
            table[key] = new_node;
            push_stack(key, table[key]);
            ++current_lir_count;
        } else {
            // Освобождаем память при необходимости
            if (current_hir_resident_count >= max_hir_capacity) {
                evict_hir();
            }

            Node new_node{key, value, Status::HIR_RESIDENT};
            table[key] = new_node;

            push_stack(key, table[key]);
            push_queue(key, table[key]);
            ++current_hir_resident_count;
        }
    }

    void clear() override {
        table.clear();
        S.clear();
        Q.clear();
        current_lir_count = 0;
        current_hir_resident_count = 0;
    }

    ~LircCache() override = default;
};