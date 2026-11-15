module;

#include <algorithm>
#include <cstddef>
#include <memory>
#include <optional>
#include <utility>

export module avl_implementation;

import dynamic_set;

// Implementation based on an AVL tree.
// Each node stores the size of its subtree, so that all queries
// (count_less, count_in_range, kth) work in O(log n).
export class AVLImplementation final : public DynamicSet
{
  public:
    void insert(int x) override
    {
        insert(root_, x);
    }

    std::ptrdiff_t count_in_range(int l, int r) const override
    {
        if (l >= r)
        {
            return 0;
        }
        // (l, r] = {keys <= r} \ {keys <= l}
        return count_before(r, true) - count_before(l, true);
    }

    std::optional<int> kth(std::ptrdiff_t i) const override
    {
        if (i < 1 || i > node_size(root_))
        {
            return std::nullopt;
        }
        const Node* cur = root_.get();
        while (cur != nullptr)
        {
            const std::ptrdiff_t left_size = node_size(cur->left);
            if (i <= left_size)
            {
                cur = cur->left.get(); // The answer is in the left subtree.
            }
            else if (i == left_size + 1)
            {
                return cur->key; // This node is exactly the i-th one.
            }
            else
            {
                i -= left_size + 1; // Skip the left subtree and this node.
                cur = cur->right.get();
            }
        }
        return std::nullopt; // Unreachable for a consistent tree.
    }

    std::ptrdiff_t count_less(int x) const override
    {
        return count_before(x, false);
    }

  private:
    struct Node
    {
        explicit Node(int k) : key(k) {}

        int key;
        int height = 1;             // Height of the subtree (a leaf has height 1).
        std::ptrdiff_t size = 1;    // Number of keys in the subtree.
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
    };

    using NodePtr = std::unique_ptr<Node>;

    static int node_height(const NodePtr& n)
    {
        return n ? n->height : 0;
    }

    static std::ptrdiff_t node_size(const NodePtr& n)
    {
        return n ? n->size : 0;
    }

    // Recomputes height and size of a node from its children.
    static void update(Node& n)
    {
        n.height = 1 + std::max(node_height(n.left), node_height(n.right));
        n.size = 1 + node_size(n.left) + node_size(n.right);
    }

    // Balance factor: height(left) - height(right).
    static int balance(const Node& n)
    {
        return node_height(n.left) - node_height(n.right);
    }

    //      y            x
    //     / \          / \
    //    x   C  --->  A   y
    //   / \              / \
    //  A   B            B   C
    static void rotate_right(NodePtr& y)
    {
        NodePtr x = std::move(y->left);
        y->left = std::move(x->right);
        update(*y); // y is now below x, so update it first.
        x->right = std::move(y);
        update(*x);
        y = std::move(x);
    }

    static void rotate_left(NodePtr& x)
    {
        NodePtr y = std::move(x->right);
        x->right = std::move(y->left);
        update(*x);
        y->left = std::move(x);
        update(*y);
        x = std::move(y);
    }

    // Updates the node and restores the AVL property if it is broken.
    static void rebalance(NodePtr& n)
    {
        update(*n);
        const int b = balance(*n);
        if (b > 1)
        {
            if (balance(*n->left) < 0) // Left-Right case.
            {
                rotate_left(n->left);
            }
            rotate_right(n);
        }
        else if (b < -1)
        {
            if (balance(*n->right) > 0) // Right-Left case.
            {
                rotate_right(n->right);
            }
            rotate_left(n);
        }
    }

    // Recursive insertion; duplicates are ignored.
    static void insert(NodePtr& node, int x)
    {
        if (!node)
        {
            node = std::make_unique<Node>(x);
            return;
        }
        if (x < node->key)
        {
            insert(node->left, x);
        }
        else if (x > node->key)
        {
            insert(node->right, x);
        }
        else
        {
            return; // Already present: nothing changes, no need to rebalance.
        }
        rebalance(node);
    }

    // Number of keys < x (inclusive == false) or <= x (inclusive == true).
    // A separate flag is used instead of "x + 1" to avoid overflow at INT_MAX.
    std::ptrdiff_t count_before(int x, bool inclusive) const
    {
        std::ptrdiff_t result = 0;
        const Node* cur = root_.get();
        while (cur != nullptr)
        {
            const bool goes_left = inclusive ? (x < cur->key) : (x <= cur->key);
            if (goes_left)
            {
                cur = cur->left.get();
            }
            else
            {
                // The whole left subtree and the node itself are counted.
                result += node_size(cur->left) + 1;
                cur = cur->right.get();
            }
        }
        return result;
    }

    NodePtr root_;
};