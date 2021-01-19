#include <gtest/gtest.h>
#include <variant>
#include <optional>
#include "constexpr_containers.hpp"

using cexpr::vector;
using cexpr::Box;

constexpr auto test_const(){
    int n = 0;
    vector v{1,2,3,4};
    n = v.size();
    return n;
}

constexpr auto test_cv(){
    vector v{1,2};
    v.push_back(3);
    v.push_back(4);
    return v;
}

constexpr auto test_cv_copy(){
    vector v{1,2};
    vector v2{1,2,3,4,5};
    v = v2;

    return v.capacity();
}

constexpr bool test_box_const(){
    Box b{1};
    static_assert(std::is_same_v<typename decltype(b)::element_type , int>);
    return *b == 1;
}

TEST(test_cexpr , test_vector){
    static_assert(test_const() == 4);
    static_assert(test_cv().size() == 4);
    static_assert(test_cv_copy() == 7);
    static_assert(test_box_const());

    vector<int> v{1,2,3,4};
    EXPECT_EQ(v.size() , 4);
    v.push_back(5);
    v.emplace_back(6);
    for(auto i = 1 ; auto x : v){
        EXPECT_EQ(x , i);
        ++i;
    }

    EXPECT_EQ(v.capacity() , 6);
    auto v2 = v;

    EXPECT_EQ(v2.size() , v.size());
    EXPECT_EQ(v2.capacity() , v.capacity());
    
    for(auto i = 1 ; auto x : v2){
        EXPECT_EQ(x , i);
        ++i;
    }

    v = std::move(v);
    EXPECT_EQ(v.size() , 6);
    EXPECT_EQ(v.capacity() , 6);

    vector<int> v3{1};
    v3.push_back(1);
    EXPECT_EQ(v3.size() , 2);

    vector<int> v4{};

    auto v5 = v4;
    EXPECT_EQ(v5.size() , 0);
    EXPECT_EQ(v4.capacity() , v5.capacity());
    EXPECT_EQ(v4.capacity() , 0);
    EXPECT_EQ(v5.begin() , nullptr);
}

TEST(test_cexpr , test_box){
    Box b{1};   
    EXPECT_TRUE(b);
    EXPECT_EQ(*b , 1);
}