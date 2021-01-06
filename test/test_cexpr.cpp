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

TEST(test_cexpr , test_types){

    static_assert(test_const() == 4);
    static_assert(test_cv().size() == 4);
    static_assert(test_cv_copy() == 7);
    static_assert(test_box_const());

    vector<int> v{1,2,3,4};
    EXPECT_EQ(v.size() , 4);
    v.push_back(5);
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

    Box b{1};
    EXPECT_TRUE(b);
    EXPECT_EQ(*b , 1);
}

// TEST(test_cexpr , test_var){
// }