#include <cstddef>
#include <iostream>
#include <memory>
#include <optional>
#include <string>

import dynamic_set;
import set_implementation;
import vector_implementation;
import avl_implementation;

// Creates the structure chosen by name; returns nullptr for unknown names.
static std::unique_ptr<DynamicSet> make_set(const std::string& name)
{
    if (name == "set")
    {
        return std::make_unique<SetImplementation>();
    }
    if (name == "vector")
    {
        return std::make_unique<VectorImplementation>();
    }
    if (name == "avl")
    {
        return std::make_unique<AVLImplementation>();
    }
    return nullptr;
}

int main(int argc, char* argv[])
{
    // Fast input/output: do not synchronize C++ streams with C stdio.
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    const std::string name = (argc > 1) ? argv[1] : "avl";
    std::unique_ptr<DynamicSet> set = make_set(name);
    if (!set)
    {
        std::cerr << "Unknown structure '" << name << "'. Use: set | vector | avl\n";
        return 1;
    }

    bool first = true; // Used to separate outputs with spaces.

    // Prints one result, separated from the previous one by a space.
    auto print = [&first](const auto& value)
    {
        if (!first)
        {
            std::cout << ' ';
        }
        std::cout << value;
        first = false;
    };

    std::string command;
    while (std::cin >> command)
    {
        if (command == "k") // k x: insert x
        {
            int x;
            std::cin >> x;
            if (std::cin)
            {
                set->insert(x);
            }
        }
        else if (command == "q") // q l r: number of keys in (l, r]
        {
            int l, r;
            std::cin >> l >> r;
            if (std::cin)
            {
                print(set->count_in_range(l, r));
            }
        }
        else if (command == "m") // m i: i-th smallest key
        {
            std::ptrdiff_t i;
            std::cin >> i;
            if (std::cin)
            {
                const std::optional<int> result = set->kth(i);
                if (result)
                {
                    print(*result);
                }
                else
                {
                    print("none"); // i is out of bounds
                }
            }
        }
        else if (command == "n") // n x: number of keys strictly less than x
        {
            int x;
            std::cin >> x;
            if (std::cin)
            {
                print(set->count_less(x));
            }
        }
        else
        {
            std::cerr << "Unknown command: " << command << '\n';
            return 1;
        }

        if (!std::cin)
        {
            std::cerr << "Malformed input\n";
            return 1;
        }
    }

    std::cout << '\n';
    return 0;
}