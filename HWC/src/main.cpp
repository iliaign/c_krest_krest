#include <gtest/gtest.h>

#include <cstddef>
#include <optional>
#include <random>

import dynamic_set;
import set_implementation;    // class SetImplementation (adjust if named differently)
import vector_implementation; // class VectorImplementation
import avl_implementation;    // class AVLImplementation

// A fixture template: each test below is run once per implementation.
template <typename T>
class DynamicSetTest : public ::testing::Test
{
  protected:
    T set;
};

using Implementations = ::testing::Types<SetImplementation, VectorImplementation, AVLImplementation>;
TYPED_TEST_SUITE(DynamicSetTest, Implementations);

// Insertion, including duplicates.
TYPED_TEST(DynamicSetTest, InsertIgnoresDuplicates)
{
    this->set.insert(5);
    this->set.insert(5);
    this->set.insert(5);
    this->set.insert(3);

    EXPECT_EQ(this->set.count_less(100), 2); // Only 3 and 5 are stored.
    EXPECT_EQ(this->set.kth(1), 3);
    EXPECT_EQ(this->set.kth(2), 5);
    EXPECT_EQ(this->set.kth(3), std::nullopt);
}

TYPED_TEST(DynamicSetTest, CountInRange)
{
    for (int x : {1, 3, 5, 7})
    {
        this->set.insert(x);
    }

    EXPECT_EQ(this->set.count_in_range(1, 5), 2);      // 3, 5 (left bound excluded)
    EXPECT_EQ(this->set.count_in_range(0, 1), 1);      // 1 (right bound included)
    EXPECT_EQ(this->set.count_in_range(-100, 100), 4); // everything
    EXPECT_EQ(this->set.count_in_range(7, 100), 0);    // nothing above 7
    EXPECT_EQ(this->set.count_in_range(5, 5), 0);      // empty range
    EXPECT_EQ(this->set.count_in_range(6, 2), 0);      // l > r
}

TYPED_TEST(DynamicSetTest, Kth)
{
    for (int x : {10, -4, 7})
    {
        this->set.insert(x);
    }

    EXPECT_EQ(this->set.kth(1), -4);
    EXPECT_EQ(this->set.kth(2), 7);
    EXPECT_EQ(this->set.kth(3), 10);

    // Out of bounds.
    EXPECT_EQ(this->set.kth(0), std::nullopt);
    EXPECT_EQ(this->set.kth(-1), std::nullopt);
    EXPECT_EQ(this->set.kth(4), std::nullopt);
}

TYPED_TEST(DynamicSetTest, KthOnEmptySet)
{
    EXPECT_EQ(this->set.kth(1), std::nullopt);
}

TYPED_TEST(DynamicSetTest, CountLess)
{
    for (int x : {1, 3, 5, 7})
    {
        this->set.insert(x);
    }

    EXPECT_EQ(this->set.count_less(1), 0); // strictly less
    EXPECT_EQ(this->set.count_less(2), 1);
    EXPECT_EQ(this->set.count_less(7), 3);
    EXPECT_EQ(this->set.count_less(8), 4);
}

// The example from the assignment:
// k 8, k 2, k -1, m 1, m 2, n 3, q -1 8  ->  -1 2 2 2
TYPED_TEST(DynamicSetTest, AssignmentExample)
{
    this->set.insert(8);
    this->set.insert(2);
    this->set.insert(-1);

    EXPECT_EQ(this->set.kth(1), -1);
    EXPECT_EQ(this->set.kth(2), 2);
    EXPECT_EQ(this->set.count_less(3), 2);
    EXPECT_EQ(this->set.count_in_range(-1, 8), 2);
}

// Extra: AVL (rotations!) must agree with the simple vector implementation
// on random data, including sorted insertion, which triggers many rotations.
TEST(AVLImplementationTest, MatchesVectorOnRandomData)
{
    AVLImplementation avl;
    VectorImplementation reference;

    for (int i = 0; i < 1000; ++i) // Sorted input.
    {
        avl.insert(i);
        reference.insert(i);
    }

    std::mt19937 rng(12345);
    std::uniform_int_distribution<int> dist(-2000, 2000);
    for (int i = 0; i < 2000; ++i)
    {
        const int x = dist(rng);
        avl.insert(x);
        reference.insert(x);
    }

    for (int i = 0; i < 500; ++i)
    {
        const int a = dist(rng);
        const int b = dist(rng);
        EXPECT_EQ(avl.count_less(a), reference.count_less(a));
        EXPECT_EQ(avl.count_in_range(a, b), reference.count_in_range(a, b));
        EXPECT_EQ(avl.kth(i + 1), reference.kth(i + 1));
    }
    EXPECT_EQ(avl.kth(100000), std::nullopt);
}