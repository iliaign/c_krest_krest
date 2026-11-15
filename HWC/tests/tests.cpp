#include <gtest/gtest.h>

#include <cstddef>
#include <limits>
#include <optional>
#include <random>

import dynamic_set;
import set_implementation;
import vector_implementation;
import avl_implementation;

// Each test below runs once for every implementation listed in Implementations.
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

// Extreme int values: checks that there is no overflow (e.g. x + 1 at INT_MAX).
TYPED_TEST(DynamicSetTest, ExtremeValues)
{
    constexpr int lo = std::numeric_limits<int>::min();
    constexpr int hi = std::numeric_limits<int>::max();
    this->set.insert(hi);
    this->set.insert(lo);
    this->set.insert(0);

    EXPECT_EQ(this->set.kth(1), lo);
    EXPECT_EQ(this->set.kth(2), 0);
    EXPECT_EQ(this->set.kth(3), hi);
    EXPECT_EQ(this->set.count_less(lo), 0);
    EXPECT_EQ(this->set.count_less(hi), 2);
    EXPECT_EQ(this->set.count_in_range(lo, hi), 2); // 0 and hi
    EXPECT_EQ(this->set.count_in_range(lo, 0), 1);  // 0
}

// Every implementation must agree with the simple vector one on random data.
// Sorted insertion triggers many AVL rotations.
TYPED_TEST(DynamicSetTest, MatchesVectorOnRandomData)
{
    VectorImplementation reference;

    for (int i = 0; i < 3000; ++i)
    {
        this->set.insert(i);
        reference.insert(i);
    }

    std::mt19937 rng(12345);
    std::uniform_int_distribution<int> dist(-2000, 5000);
    for (int i = 0; i < 2000; ++i)
    {
        const int x = dist(rng);
        this->set.insert(x);
        reference.insert(x);
    }

    for (int i = 0; i < 500; ++i)
    {
        const int a = dist(rng);
        const int b = dist(rng);
        EXPECT_EQ(this->set.count_less(a), reference.count_less(a));
        EXPECT_EQ(this->set.count_in_range(a, b), reference.count_in_range(a, b));
        EXPECT_EQ(this->set.kth(i + 1), reference.kth(i + 1));
    }
    EXPECT_EQ(this->set.kth(100000), std::nullopt);
}